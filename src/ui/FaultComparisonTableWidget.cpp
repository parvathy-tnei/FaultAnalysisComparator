#include "FaultComparisonTableWidget.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableView>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPainter>
#include <QAbstractItemView>
#include <QPaintEvent>
#include <QScrollBar>
#include <QLabel>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QSignalBlocker>
#include <QMap>
#include <QColor>
#include <QBrush>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QHBoxLayout>
#include <QVector>
#include <QScrollArea>
#include <QSet>
#include <functional>
#include <cmath>

// ============================================================
// Clean Row Drag & Drop Filter (Prevents Cell Item Corruption)
// ============================================================
class TableRowDragFilter : public QObject
{
public:
    TableRowDragFilter(QAbstractItemView *view, std::function<void(int fromRow, int toRow)> onRowMoved, QObject *parent = nullptr)
        : QObject(parent)
        , mView(view)
        , mOnRowMoved(onRowMoved)
        , mDragRow(-1)
        , mDragging(false)
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (!mView)
            return QObject::eventFilter(watched, event);

        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton)
            {
                mDragStartPos = me->position().toPoint();
                QModelIndex idx = mView->indexAt(mDragStartPos);
                mDragRow = idx.isValid() ? idx.row() : -1;
                mDragging = false;
            }
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if ((me->buttons() & Qt::LeftButton) && mDragRow >= 0)
            {
                if (!mDragging && (me->position().toPoint() - mDragStartPos).manhattanLength() >= 8)
                {
                    mDragging = true;
                    if (QWidget *vp = qobject_cast<QWidget *>(watched))
                        vp->setCursor(Qt::ClosedHandCursor);
                }
                if (mDragging)
                {
                    return true;
                }
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (mDragging && me->button() == Qt::LeftButton)
            {
                mDragging = false;
                if (QWidget *vp = qobject_cast<QWidget *>(watched))
                    vp->unsetCursor();

                QPoint releasePos = me->position().toPoint();
                QModelIndex targetIdx = mView->indexAt(releasePos);
                int targetRow = targetIdx.isValid() ? targetIdx.row() : -1;

                if (targetRow < 0)
                {
                    if (releasePos.y() <= 0)
                        targetRow = 0;
                    else if (mView->model())
                        targetRow = mView->model()->rowCount() - 1;
                }

                if (mDragRow >= 0 && targetRow >= 0 && mDragRow != targetRow && mOnRowMoved)
                {
                    mOnRowMoved(mDragRow, targetRow);
                }
                mDragRow = -1;
                return true;
            }
            mDragging = false;
            mDragRow = -1;
            if (QWidget *vp = qobject_cast<QWidget *>(watched))
                vp->unsetCursor();
        }

        return QObject::eventFilter(watched, event);
    }

private:
    QAbstractItemView *mView;
    std::function<void(int fromRow, int toRow)> mOnRowMoved;
    QPoint mDragStartPos;
    int mDragRow;
    bool mDragging;
};
// ============================================================
// Draggable Header Label for Stacked Mode Cards
// ============================================================
class StackedCardHeaderLabel : public QLabel
{
public:
    explicit StackedCardHeaderLabel(const QString &text, const QString &paramName, QWidget *parent = nullptr)
        : QLabel(text, parent)
        , mParamName(paramName)
        , mDragging(false)
        , mDragStartY(-1)
    {
        setProperty("paramName", paramName);
        setCursor(Qt::OpenHandCursor);
    }

    QString paramName() const { return mParamName; }

    std::function<void(const QString &srcParam, int globalY)> onDragDrop;

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            mDragging = true;
            mDragStartY = event->globalPosition().toPoint().y();
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }
        QLabel::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (mDragging && event->button() == Qt::LeftButton)
        {
            mDragging = false;
            setCursor(Qt::OpenHandCursor);
            if (onDragDrop)
                onDragDrop(mParamName, event->globalPosition().toPoint().y());
            event->accept();
            return;
        }
        QLabel::mouseReleaseEvent(event);
    }

private:
    QString mParamName;
    bool mDragging;
    int mDragStartY;
};

// ============================================================
// Helper Event Filter to Keep Stacked Frozen View Pinned
// ============================================================
class StackedFrozenFilter : public QObject
{
public:
    StackedFrozenFilter(QTableWidget *parentTable, QTableView *frozenView, QObject *parent = nullptr)
        : QObject(parent)
        , mTable(parentTable)
        , mFrozen(frozenView)
    {
    }

    void updateGeometry()
    {
        if (!mTable || !mFrozen || mTable->columnCount() <= 2)
        {
            if (mFrozen) mFrozen->hide();
            return;
        }

        int frozenWidth = mTable->columnWidth(0) + mTable->columnWidth(1);
        int contentHeight = mTable->horizontalHeader()->height();
        for (int r = 0; r < mTable->rowCount(); ++r)
        {
            contentHeight += mTable->rowHeight(r);
        }

        const int viewportHeight = mTable->viewport()->height() + mTable->horizontalHeader()->height();
        const int finalHeight = qMin(contentHeight, viewportHeight);
        const int fWidth = mTable->frameWidth();

        mFrozen->setGeometry(fWidth, fWidth, frozenWidth, finalHeight);
        mFrozen->show();
        mFrozen->raise();
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == mTable->viewport() || watched == mTable->horizontalHeader())
        {
            if (event->type() == QEvent::Resize || event->type() == QEvent::Paint)
            {
                updateGeometry();
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QTableWidget *mTable;
    QTableView *mFrozen;
};

// ============================================================
// Two-level custom header with full Group Drag & Drop Support
// ============================================================
class FaultTableHeader : public QHeaderView
{
public:
    explicit FaultTableHeader(
        Qt::Orientation orientation,
        QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
        , mGroupDragging(false)
        , mGroupDragStartX(-1)
    {
        setDefaultAlignment(Qt::AlignCenter);
        setSectionsMovable(true);
        setSectionsClickable(false);
        setHighlightSections(false);
        setMinimumSectionSize(50);
        setDefaultSectionSize(140);
    }

    void setGroupNames(const QStringList &groups)
    {
        mGroupNames = groups;
        viewport()->update();
    }

    QSize sizeHint() const override
    {
        return QSize(QHeaderView::sizeHint().width(), 56);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event && event->button() == Qt::LeftButton && event->position().y() < 26)
        {
            const int visualIndex = visualIndexAt(static_cast<int>(event->position().x()));
            if (visualIndex >= 0)
            {
                const int logicalSection = this->logicalIndex(visualIndex);
                if (logicalSection >= 2 && logicalSection < mGroupNames.size())
                {
                    const QString group = mGroupNames.at(logicalSection);
                    if (!group.isEmpty())
                    {
                        mGroupDragging = true;
                        mDraggedGroup = group;
                        mGroupDragStartX = static_cast<int>(event->position().x());
                        event->accept();
                        return;
                    }
                }
            }
        }
        QHeaderView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (mGroupDragging)
        {
            event->accept();
            viewport()->update();
            return;
        }
        QHeaderView::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (mGroupDragging)
        {
            if (event)
            {
                const int targetVisual = visualIndexAt(static_cast<int>(event->position().x()));
                if (targetVisual >= 0)
                {
                    const int targetLogical = this->logicalIndex(targetVisual);
                    QString targetGroup;
                    if (targetLogical >= 2 && targetLogical < mGroupNames.size())
                    {
                        targetGroup = mGroupNames.at(targetLogical);
                    }

                    if (!targetGroup.isEmpty() && targetGroup != mDraggedGroup)
                    {
                        moveGroup(mDraggedGroup, targetGroup);
                    }
                }
            }

            mGroupDragging = false;
            mDraggedGroup.clear();
            mGroupDragStartX = -1;
            viewport()->update();
            if (event) event->accept();
            return;
        }
        QHeaderView::mouseReleaseEvent(event);
    }

    void moveGroup(const QString &sourceGroup, const QString &targetGroup)
    {
        if (sourceGroup.isEmpty() || targetGroup.isEmpty() || sourceGroup == targetGroup)
            return;

        QList<int> currentOrder;
        for (int v = 0; v < count(); ++v)
            currentOrder.append(logicalIndex(v));

        QList<QString> groupOrder;
        QMap<QString, QList<int>> groupCols;

        for (int l : currentOrder)
        {
            if (l < 2 || l >= mGroupNames.size()) continue;
            QString g = mGroupNames.at(l);
            if (g.isEmpty()) continue;
            if (!groupOrder.contains(g)) groupOrder.append(g);
            groupCols[g].append(l);
        }

        int srcPos = groupOrder.indexOf(sourceGroup);
        int tgtPos = groupOrder.indexOf(targetGroup);
        if (srcPos < 0 || tgtPos < 0) return;

        groupOrder.swapItemsAt(srcPos, tgtPos);

        QList<int> desiredOrder;
        for (int l : currentOrder)
        {
            if (l < 2) desiredOrder.append(l);
        }

        for (const QString &g : groupOrder)
        {
            desiredOrder.append(groupCols.value(g));
        }

        QSignalBlocker blocker(this);
        for (int targetVisual = 0; targetVisual < desiredOrder.size(); ++targetVisual)
        {
            int desiredLogical = desiredOrder.at(targetVisual);
            int currentVisual = visualIndex(desiredLogical);
            if (currentVisual >= 0 && currentVisual != targetVisual)
                moveSection(currentVisual, targetVisual);
        }
        viewport()->update();
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing, false);

        const int sectionCount = count();
        if (sectionCount <= 0)
            return;

        const int topBannerH = 26;
        const int subHeaderH = height() - topBannerH;

        for (int visualIndex = 0; visualIndex < sectionCount; ++visualIndex)
        {
            const int logical = this->logicalIndex(visualIndex);
            if (logical < 0)
                continue;

            const int x = sectionViewportPosition(logical);
            const int sectionWidth = sectionSize(logical);
            if (sectionWidth <= 0)
                continue;

            const bool isPinned = (logical < 2);

            QRect cellRect = isPinned
                                 ? QRect(x, 0, sectionWidth, height())
                                 : QRect(x, topBannerH, sectionWidth, subHeaderH);

            painter.fillRect(cellRect, QColor("#F8FAFC"));

            QString colTitle;
            if (model())
                colTitle = model()->headerData(logical, Qt::Horizontal, Qt::DisplayRole).toString();

            QFont font = painter.font();
            font.setBold(true);
            font.setPointSize(9);
            painter.setFont(font);
            painter.setPen(QColor("#374151"));

            painter.drawText(
                cellRect.adjusted(4, 2, -4, -2),
                Qt::AlignCenter | Qt::TextWordWrap,
                colTitle
                );

            painter.setPen(QColor("#CBD5E1"));
            painter.drawRect(QRect(x, cellRect.top(), sectionWidth - 1, cellRect.height() - 1));

            if (logical == 1 && sectionCount > 2)
            {
                painter.setPen(QPen(QColor("#94A3B8"), 1));
                painter.drawLine(x + sectionWidth - 1, 0, x + sectionWidth - 1, height() - 1);
            }
            else if (logical >= 2 && logical < mGroupNames.size())
            {
                if (visualIndex + 1 < sectionCount)
                {
                    int nextLogical = this->logicalIndex(visualIndex + 1);
                    if (nextLogical < mGroupNames.size() && mGroupNames.at(logical) != mGroupNames.at(nextLogical))
                    {
                        painter.setPen(QPen(QColor("#94A3B8"), 1));
                        painter.drawLine(x + sectionWidth - 1, 0, x + sectionWidth - 1, height() - 1);
                    }
                }
            }
        }

        int startVisualIndex = 0;
        while (startVisualIndex < sectionCount && this->logicalIndex(startVisualIndex) < 2)
        {
            ++startVisualIndex;
        }

        while (startVisualIndex < sectionCount)
        {
            const int firstLogical = this->logicalIndex(startVisualIndex);
            if (firstLogical < 2 || firstLogical >= mGroupNames.size())
            {
                ++startVisualIndex;
                continue;
            }

            const QString groupName = mGroupNames.at(firstLogical);
            if (groupName.isEmpty())
            {
                ++startVisualIndex;
                continue;
            }

            int endVisualIndex = startVisualIndex;
            while (endVisualIndex + 1 < sectionCount)
            {
                const int nextLogical = this->logicalIndex(endVisualIndex + 1);
                if (nextLogical < 0 || nextLogical >= mGroupNames.size())
                    break;
                if (mGroupNames.at(nextLogical) != groupName)
                    break;
                ++endVisualIndex;
            }

            const int firstLogicalCol = this->logicalIndex(startVisualIndex);
            const int lastLogicalCol  = this->logicalIndex(endVisualIndex);

            if (firstLogicalCol >= 0 && lastLogicalCol >= 0)
            {
                const int left = sectionViewportPosition(firstLogicalCol);
                const int right = sectionViewportPosition(lastLogicalCol) + sectionSize(lastLogicalCol);
                const int groupWidth = right - left;

                if (groupWidth > 0)
                {
                    QRect bannerRect(left, 0, groupWidth, topBannerH);

                    painter.fillRect(bannerRect, QColor("#EAF3FF"));

                    QFont gFont = painter.font();
                    gFont.setBold(true);
                    gFont.setPointSize(9.5);
                    painter.setFont(gFont);
                    painter.setPen(QColor("#0F3D64"));

                    painter.drawText(
                        bannerRect.adjusted(8, 1, -8, -1),
                        Qt::AlignCenter | Qt::TextSingleLine,
                        groupName
                        );

                    painter.setPen(QPen(QColor("#CBD5E1"), 1.0));
                    painter.drawRect(QRect(
                        bannerRect.left(),
                        bannerRect.top(),
                        bannerRect.width() - 1,
                        bannerRect.height() - 1
                        ));

                    painter.setPen(QPen(QColor("#94A3B8"), 1.0));
                    painter.drawLine(bannerRect.right(), 0, bannerRect.right(), topBannerH);
                }
            }

            startVisualIndex = endVisualIndex + 1;
        }

        if (sectionCount > 2)
        {
            int startX = sectionViewportPosition(this->logicalIndex(2));
            painter.setPen(QColor("#CBD5E1"));
            painter.drawLine(startX, topBannerH, width(), topBannerH);
        }

        painter.setPen(QColor("#CBD5E1"));
        painter.drawLine(0, height() - 1, width(), height() - 1);
    }

private:
    QStringList mGroupNames;
    bool mGroupDragging;
    QString mDraggedGroup;
    int mGroupDragStartX;
};

// ============================================================
// Numeric Comparison Helpers
// ============================================================
static constexpr double VALUE_TOLERANCE = 1e-6;

static bool valuesAreSame(const QString &value1, const QString &value2)
{
    bool ok1 = false;
    bool ok2 = false;

    const double number1 = value1.trimmed().toDouble(&ok1);
    const double number2 = value2.trimmed().toDouble(&ok2);

    if (ok1 && ok2)
        return std::abs(number1 - number2) < VALUE_TOLERANCE;

    return value1.trimmed() == value2.trimmed();
}

static bool isEmptyValue(const QString &value)
{
    return value.trimmed().isEmpty();
}

struct ComparisonGroupColor
{
    QColor background;
    QColor foreground;
};

static const QVector<ComparisonGroupColor> comparisonPalette = {
    { QColor("#DCFCE7"), QColor("#166534") },
    { QColor("#FEF3C7"), QColor("#92400E") },
    { QColor("#DBEAFE"), QColor("#1E40AF") },
    { QColor("#EDE9FE"), QColor("#6D28D9") },
    { QColor("#FFE4E6"), QColor("#9F1239") },
    { QColor("#CFFAFE"), QColor("#155E75") }
};

static ComparisonGroupColor comparisonColor(int groupIndex)
{
    if (groupIndex < 0 || comparisonPalette.isEmpty())
        return { QColor(Qt::white), QColor("#1F2937") };

    return comparisonPalette.at(groupIndex % comparisonPalette.size());
}

static QVector<int> buildComparisonGroupIndexes(const QStringList &values)
{
    QVector<int> groupIndexes(values.size(), -1);
    QVector<QString> groupRepresentatives;
    QVector<int> groupSizes;

    for (int i = 0; i < values.size(); ++i)
    {
        const QString value = values.at(i);
        if (isEmptyValue(value))
            continue;

        int existingGroup = -1;
        for (int g = 0; g < groupRepresentatives.size(); ++g)
        {
            if (valuesAreSame(value, groupRepresentatives.at(g)))
            {
                existingGroup = g;
                break;
            }
        }

        if (existingGroup == -1)
        {
            existingGroup = groupRepresentatives.size();
            groupRepresentatives.append(value);
            groupSizes.append(0);
        }

        ++groupSizes[existingGroup];
        groupIndexes[i] = existingGroup;
    }

    for (int i = 0; i < groupIndexes.size(); ++i)
    {
        const int groupIndex = groupIndexes.at(i);
        if (groupIndex >= 0 && groupSizes.at(groupIndex) < 2)
            groupIndexes[i] = -1;
    }

    return groupIndexes;
}

// ============================================================
// Item Delegate with Full Grid Lines
// ============================================================
class FaultComparisonItemDelegate : public QStyledItemDelegate
{
public:
    explicit FaultComparisonItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
        , mTotalColumns(0)
    {
    }

    void setGroupNames(const QStringList &groups)
    {
        mGroupNames = groups;
    }

    void setTotalColumns(int cols)
    {
        mTotalColumns = cols;
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        painter->save();

        const QVariant background = index.data(Qt::BackgroundRole);
        if (background.isValid())
        {
            QBrush brush = qvariant_cast<QBrush>(background);
            if (brush.style() == Qt::NoBrush)
                brush = QBrush(Qt::white);
            painter->fillRect(option.rect, brush);
        }
        else
        {
            painter->fillRect(option.rect, Qt::white);
        }

        painter->setPen(QColor("#E2E8F0"));
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        painter->drawLine(option.rect.topRight(), option.rect.bottomRight());

        QColor textColor("#1F2937");
        const QVariant foreground = index.data(Qt::ForegroundRole);
        if (foreground.isValid())
        {
            const QBrush brush = qvariant_cast<QBrush>(foreground);
            if (brush.style() != Qt::NoBrush)
                textColor = brush.color();
        }

        painter->setPen(textColor);
        QRect textRect = option.rect.adjusted(6, 0, -6, 0);
        painter->drawText(textRect, opt.displayAlignment, index.data(Qt::DisplayRole).toString());

        const int col = index.column();
        bool isThickSeparator = false;

        if (col == 1 && mTotalColumns > 2)
        {
            isThickSeparator = true;
        }
        else if (col >= 2 && col < mGroupNames.size())
        {
            if (col + 1 < mGroupNames.size() && mGroupNames.at(col) != mGroupNames.at(col + 1))
            {
                isThickSeparator = true;
            }
        }

        if (isThickSeparator)
        {
            painter->setPen(QPen(QColor("#94A3B8"), 1));
            painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
        }

        painter->restore();
    }

private:
    QStringList mGroupNames;
    int mTotalColumns;
};

// ============================================================
// Constructor
// ============================================================
FaultComparisonTableWidget::FaultComparisonTableWidget(QWidget *parent)
    : QWidget(parent)
    , mDifferenceEnabled(false)
    , mLayoutMode(TableLayoutMode::SideBySide)
    , mComparisonTitle(nullptr)
    , mComparisonSummary(nullptr)
    , mComparisonLegend(nullptr)
    , mEmptyLabel(nullptr)
    , mTable(nullptr)
    , mFrozenView(nullptr)
    , mStackedScrollArea(nullptr)
    , mStackedContainer(nullptr)
    , mStackedLayout(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(150);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    mComparisonTitle = new QLabel(this);
    mComparisonTitle->setText("Comparison Results");
    QFont titleFont = mComparisonTitle->font();
    titleFont.setPointSize(14);
    titleFont.setWeight(QFont::DemiBold);
    mComparisonTitle->setFont(titleFont);
    mComparisonTitle->setStyleSheet("QLabel { color: #111827; background: transparent; padding: 2px 0px 1px 0px; }");
    layout->addWidget(mComparisonTitle);

    mComparisonSummary = new QLabel(this);
    QFont summaryFont = mComparisonSummary->font();
    summaryFont.setPointSize(9);
    mComparisonSummary->setFont(summaryFont);
    mComparisonSummary->setStyleSheet("QLabel { color: #6B7280; background: transparent; padding: 0px 0px 8px 0px; }");
    layout->addWidget(mComparisonSummary);

    mComparisonLegend = new QWidget(this);
    QHBoxLayout *legendLayout = new QHBoxLayout(mComparisonLegend);
    legendLayout->setContentsMargins(0, 0, 0, 6);
    legendLayout->setSpacing(18);

    auto createLegendItem = [](const QString &text, const QString &backgroundColor) -> QWidget*
    {
        QWidget *widget = new QWidget;
        QHBoxLayout *itemLayout = new QHBoxLayout(widget);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(6);

        QLabel *indicator = new QLabel;
        indicator->setFixedSize(12, 12);
        indicator->setStyleSheet(QString("QLabel { background-color: %1; border: 1px solid #D1D5DB; border-radius: 3px; }").arg(backgroundColor));

        QLabel *label = new QLabel(text);
        label->setStyleSheet("QLabel { color: #4B5563; background: transparent; font-size: 10pt; }");

        itemLayout->addWidget(indicator);
        itemLayout->addWidget(label);
        return widget;
    };

    legendLayout->addWidget(createLegendItem("Matching group 1", "#DCFCE7"));
    legendLayout->addWidget(createLegendItem("Matching group 2", "#FEF3C7"));
    legendLayout->addWidget(createLegendItem("Matching group 3", "#DBEAFE"));
    legendLayout->addWidget(createLegendItem("Not Available", "#F3F4F6"));
    legendLayout->addStretch();
    layout->addWidget(mComparisonLegend);
    mComparisonLegend->hide();

    mEmptyLabel = new QLabel(this);
    mEmptyLabel->setText("No data available.");
    mEmptyLabel->setAlignment(Qt::AlignCenter);
    QFont emptyFont = mEmptyLabel->font();
    emptyFont.setPointSize(11);
    mEmptyLabel->setFont(emptyFont);
    mEmptyLabel->setStyleSheet("QLabel { color: #94A3B8; background-color: white; border: 1px dashed #CBD5E1; border-radius: 8px; margin: 8px 0px; line-height: 1.5; }");
    mEmptyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(mEmptyLabel, 1);

    // MODE A: Main Table
    mTable = new QTableWidget(this);
    mTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTable->setMinimumHeight(120);
    mTable->setItemDelegate(new FaultComparisonItemDelegate(mTable));
    mTable->verticalHeader()->setVisible(false);
    mTable->verticalHeader()->setDefaultSectionSize(32);
    mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTable->setSelectionMode(QAbstractItemView::NoSelection);
    mTable->setFocusPolicy(Qt::NoFocus);
    mTable->setShowGrid(false);

    FaultTableHeader *header = new FaultTableHeader(Qt::Horizontal, mTable);
    mTable->setHorizontalHeader(header);

    mTable->setStyleSheet(
        "QTableWidget {"
        "    background-color: white;"
        "    border: 1px solid #CBD5E1;"
        "    border-radius: 6px;"
        "}"
        );

    mFrozenView = new QTableView(mTable);
    mFrozenView->setFocusPolicy(Qt::NoFocus);
    mFrozenView->verticalHeader()->hide();
    mFrozenView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mFrozenView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mFrozenView->setFrameShape(QFrame::NoFrame);
    mFrozenView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mFrozenView->setSelectionMode(QAbstractItemView::NoSelection);
    mFrozenView->setShowGrid(false);
    mFrozenView->setItemDelegate(new FaultComparisonItemDelegate(mFrozenView));

    FaultTableHeader *frozenHeader = new FaultTableHeader(Qt::Horizontal, mFrozenView);
    mFrozenView->setHorizontalHeader(frozenHeader);

    mFrozenView->setStyleSheet(
        "QTableView {"
        "    background-color: white;"
        "    border: none;"
        "}"
        );

    mTable->viewport()->stackUnder(mFrozenView);

    connect(mTable->verticalScrollBar(), &QScrollBar::valueChanged,
            mFrozenView->verticalScrollBar(), &QScrollBar::setValue);
    connect(mFrozenView->verticalScrollBar(), &QScrollBar::valueChanged,
            mTable->verticalScrollBar(), &QScrollBar::setValue);

    auto moveRowOrder = [this](int fromRow, int toRow) {
        if (fromRow < 0 || toRow < 0 || fromRow == toRow || mCustomRowOrder.isEmpty())
            return;
        if (fromRow < mCustomRowOrder.size() && toRow < mCustomRowOrder.size())
        {
            mCustomRowOrder.move(fromRow, toRow);
            if (mLayoutMode == TableLayoutMode::SideBySide)
                RebuildTable();
            else
                buildStackedView();
        }
    };

    // Install clean mouse drag filters for row rearranging (no QTableWidgetItem data corruption)
    TableRowDragFilter *mainDragFilter = new TableRowDragFilter(mTable, moveRowOrder, this);
    mTable->viewport()->installEventFilter(mainDragFilter);

    TableRowDragFilter *frozenDragFilter = new TableRowDragFilter(mFrozenView, moveRowOrder, this);
    mFrozenView->viewport()->installEventFilter(frozenDragFilter);

    mTable->viewport()->installEventFilter(this);
    mTable->horizontalHeader()->installEventFilter(this);

    layout->addWidget(mTable, 1);
    mTable->hide();

    // MODE B: Stacked container
    mStackedScrollArea = new QScrollArea(this);
    mStackedScrollArea->setObjectName("stackedScrollArea");
    mStackedScrollArea->setWidgetResizable(true);
    mStackedScrollArea->setFrameShape(QFrame::NoFrame);
    mStackedScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mStackedScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mStackedScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mStackedScrollArea->setStyleSheet("QScrollArea#stackedScrollArea { background: transparent; border: none; }");

    mStackedContainer = new QWidget();
    mStackedContainer->setStyleSheet("background: transparent;");

    mStackedLayout = new QVBoxLayout(mStackedContainer);
    mStackedLayout->setContentsMargins(0, 4, 8, 12);
    mStackedLayout->setSpacing(14);
    mStackedLayout->setAlignment(Qt::AlignTop);

    mStackedScrollArea->setWidget(mStackedContainer);
    layout->addWidget(mStackedScrollArea, 1);
    mStackedScrollArea->hide();
}

QTableWidget *FaultComparisonTableWidget::tableWidget() const
{
    return mTable;
}

bool FaultComparisonTableWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == mTable->viewport() || watched == mTable->horizontalHeader())
    {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Paint)
        {
            updateFrozenGeometry();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void FaultComparisonTableWidget::updateFrozenGeometry()
{
    if (!mFrozenView || !mTable || mTable->columnCount() <= 2)
    {
        if (mFrozenView)
            mFrozenView->hide();
        return;
    }

    int frozenWidth = mTable->columnWidth(0) + mTable->columnWidth(1);

    int contentHeight = mTable->horizontalHeader()->height();
    for (int r = 0; r < mTable->rowCount(); ++r)
    {
        contentHeight += mTable->rowHeight(r);
    }

    const int viewportHeight = mTable->viewport()->height() + mTable->horizontalHeader()->height();
    const int finalHeight = qMin(contentHeight, viewportHeight);

    const int fWidth = mTable->frameWidth();
    mFrozenView->setGeometry(
        fWidth,
        fWidth,
        frozenWidth,
        finalHeight
        );

    mFrozenView->show();
    mFrozenView->raise();
}

void FaultComparisonTableWidget::setData(
    const QStringList &fileNames,
    const QList<QStringList> &headersPerFile,
    const QList<QList<QStringList>> &rowsPerFile,
    const QStringList &selectedColumns,
    const QList<QMap<QString, QString>> &columnMappingsPerFile)
{
    mFileNames = fileNames;
    mHeadersPerFile = headersPerFile;
    mRowsPerFile = rowsPerFile;
    mSelectedColumns = selectedColumns;
    mColumnMappingsPerFile = columnMappingsPerFile;

    QStringList discoveredNames;
    for (int fileIndex = 0; fileIndex < mFileNames.size(); ++fileIndex)
    {
        if (fileIndex >= mHeadersPerFile.size() || fileIndex >= mRowsPerFile.size())
            continue;

        const QStringList &headers = mHeadersPerFile.at(fileIndex);
        const int nameIndex = headers.indexOf("Name");
        if (nameIndex < 0)
            continue;

        for (const QStringList &row : mRowsPerFile.at(fileIndex))
        {
            if (nameIndex >= row.size())
                continue;

            const QString name = row.at(nameIndex);
            if (!name.isEmpty() && !discoveredNames.contains(name))
                discoveredNames.append(name);
        }
    }

    QStringList updatedRowOrder;
    for (const QString &existingName : mCustomRowOrder)
    {
        if (discoveredNames.contains(existingName))
            updatedRowOrder.append(existingName);
    }
    for (const QString &newName : discoveredNames)
    {
        if (!updatedRowOrder.contains(newName))
            updatedRowOrder.append(newName);
    }
    mCustomRowOrder = updatedRowOrder;

    if (mFileNames.isEmpty() || mSelectedColumns.isEmpty())
    {
        mTable->hide();
        mStackedScrollArea->hide();
        mEmptyLabel->show();
        mComparisonLegend->hide();
        if (mComparisonSummary)
            mComparisonSummary->setText("No comparison active");
        return;
    }

    mEmptyLabel->hide();
    mComparisonLegend->show();

    if (mLayoutMode == TableLayoutMode::SideBySide)
    {
        mStackedScrollArea->hide();
        mTable->show();
        RebuildTable();
    }
    else
    {
        mTable->hide();
        mStackedScrollArea->show();
        buildStackedView();
    }
}

void FaultComparisonTableWidget::setLayoutMode(TableLayoutMode mode)
{
    if (mLayoutMode == mode)
        return;

    mLayoutMode = mode;

    if (mFileNames.isEmpty() || mSelectedColumns.isEmpty())
    {
        mTable->hide();
        mStackedScrollArea->hide();
        mEmptyLabel->show();
        mComparisonLegend->hide();
        return;
    }

    mEmptyLabel->hide();
    mComparisonLegend->show();

    if (mLayoutMode == TableLayoutMode::SideBySide)
    {
        mStackedScrollArea->hide();
        mTable->show();
        RebuildTable();
    }
    else
    {
        mTable->hide();
        mStackedScrollArea->show();
        buildStackedView();
    }
}

void FaultComparisonTableWidget::RebuildTable()
{
    mTable->clear();
    mTable->setRowCount(0);
    mTable->setColumnCount(0);
    mGroupNames.clear();

    QStringList uniqueSelectedColumns;
    for (const QString &column : mSelectedColumns)
    {
        if (!uniqueSelectedColumns.contains(column))
            uniqueSelectedColumns.append(column);
    }
    mSelectedColumns = uniqueSelectedColumns;

    if (mFileNames.isEmpty() || mSelectedColumns.isEmpty() || mHeadersPerFile.isEmpty() || mRowsPerFile.isEmpty())
        return;

    QStringList names = mCustomRowOrder;
    if (names.isEmpty())
    {
        for (int fileIndex = 0; fileIndex < mFileNames.size(); ++fileIndex)
        {
            if (fileIndex >= mHeadersPerFile.size() || fileIndex >= mRowsPerFile.size())
                continue;

            const QStringList &headers = mHeadersPerFile.at(fileIndex);
            const int nameIndex = headers.indexOf("Name");
            if (nameIndex < 0)
                continue;

            for (const QStringList &row : mRowsPerFile.at(fileIndex))
            {
                if (nameIndex >= row.size())
                    continue;

                const QString name = row.at(nameIndex);
                if (!name.isEmpty() && !names.contains(name))
                    names.append(name);
            }
        }
        mCustomRowOrder = names;
    }

    int parameterCount = 0;
    for (const QString &col : mSelectedColumns)
    {
        if (col != "Name")
            ++parameterCount;
    }

    if (mComparisonSummary)
    {
        mComparisonSummary->setText(
            QString("%1 files · %2 parameters · %3 rows")
                .arg(mFileNames.size())
                .arg(parameterCount)
                .arg(names.size())
            );
    }

    if (names.isEmpty())
        return;

    QStringList tableHeaders;
    tableHeaders.append("#");
    mGroupNames.append("");

    tableHeaders.append("Name");
    mGroupNames.append("");

    for (const QString &column : mSelectedColumns)
    {
        if (column == "Name")
            continue;

        for (int fileIndex = 0; fileIndex < mFileNames.size(); ++fileIndex)
        {
            if (!columnAvailableForFile(fileIndex, column))
                continue;

            tableHeaders.append(mFileNames.at(fileIndex));
            mGroupNames.append(column);
        }
    }

    mTable->setColumnCount(tableHeaders.size());
    mTable->setHorizontalHeaderLabels(tableHeaders);
    mTable->setRowCount(names.size());

    mTable->setColumnWidth(0, 42);
    mTable->setColumnWidth(1, 150);

    if (FaultComparisonItemDelegate *del = dynamic_cast<FaultComparisonItemDelegate *>(mTable->itemDelegate()))
    {
        del->setGroupNames(mGroupNames);
        del->setTotalColumns(tableHeaders.size());
    }

    for (int r = 0; r < names.size(); ++r)
    {
        const QString &name = names.at(r);
        int outCol = 0;
        mTable->setRowHeight(r, 32);

        QTableWidgetItem *numItem = new QTableWidgetItem(QString::number(r + 1));
        numItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        numItem->setForeground(QColor("#64748B"));
        mTable->setItem(r, outCol++, numItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        nameItem->setForeground(QColor("#0F172A"));
        mTable->setItem(r, outCol++, nameItem);

        for (const QString &column : mSelectedColumns)
        {
            if (column == "Name")
                continue;

            QStringList values;
            for (int fileIndex = 0; fileIndex < mFileNames.size(); ++fileIndex)
            {
                if (!columnAvailableForFile(fileIndex, column))
                    continue;

                values.append(valueFor(fileIndex, name, column));
            }

            const QVector<int> comparisonGroups = buildComparisonGroupIndexes(values);
            int vIdx = 0;

            for (int fileIndex = 0; fileIndex < mFileNames.size(); ++fileIndex)
            {
                if (!columnAvailableForFile(fileIndex, column))
                    continue;

                if (vIdx >= values.size())
                    break;

                const QString value = values.at(vIdx);
                QTableWidgetItem *item = new QTableWidgetItem(value);
                item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

                const int groupIndex = comparisonGroups.value(vIdx, -1);

                if (isEmptyValue(value))
                {
                    item->setData(Qt::BackgroundRole, QBrush(QColor("#F3F4F6")));
                    item->setData(Qt::ForegroundRole, QBrush(QColor("#6B7280")));
                }
                else if (groupIndex >= 0)
                {
                    const ComparisonGroupColor color = comparisonColor(groupIndex);
                    item->setData(Qt::BackgroundRole, QBrush(color.background));
                    item->setData(Qt::ForegroundRole, QBrush(color.foreground));
                }
                else
                {
                    item->setData(Qt::BackgroundRole, QBrush(QColor("#FFFFFF")));
                    item->setData(Qt::ForegroundRole, QBrush(QColor("#1F2937")));
                }

                mTable->setItem(r, outCol++, item);
                ++vIdx;
            }
        }
    }

    QFontMetrics headerMetrics(mTable->horizontalHeader()->font());
    QFontMetrics cellMetrics(mTable->font());

    for (int c = 2; c < tableHeaders.size(); ++c)
    {
        const QString &title = tableHeaders.at(c);
        int textW = headerMetrics.horizontalAdvance(title) + 36;

        for (int r = 0; r < names.size(); ++r)
        {
            if (QTableWidgetItem *item = mTable->item(r, c))
            {
                textW = qMax(textW, cellMetrics.horizontalAdvance(item->text()) + 24);
            }
        }

        const int colW = qBound(110, textW, 350);
        mTable->setColumnWidth(c, colW);
    }

    FaultTableHeader *header = static_cast<FaultTableHeader *>(mTable->horizontalHeader());
    header->setGroupNames(mGroupNames);

    if (tableHeaders.size() > 2)
    {
        mFrozenView->setModel(mTable->model());
        for (int col = 0; col < mTable->columnCount(); ++col)
        {
            if (col < 2)
            {
                mFrozenView->setColumnHidden(col, false);
                mFrozenView->setColumnWidth(col, mTable->columnWidth(col));
            }
            else
            {
                mFrozenView->setColumnHidden(col, true);
            }
        }
        for (int r = 0; r < names.size(); ++r)
        {
            mFrozenView->setRowHeight(r, 32);
        }

        FaultTableHeader *frozenHeader = static_cast<FaultTableHeader *>(mFrozenView->horizontalHeader());
        frozenHeader->setGroupNames(mGroupNames);

        if (FaultComparisonItemDelegate *del = dynamic_cast<FaultComparisonItemDelegate *>(mFrozenView->itemDelegate()))
        {
            del->setGroupNames(mGroupNames);
            del->setTotalColumns(tableHeaders.size());
        }

        updateFrozenGeometry();
    }
    else
    {
        mFrozenView->hide();
    }

    mTable->viewport()->update();
    mTable->horizontalScrollBar()->setValue(0);
}

// ============================================================
// MODE B: STACKED VIEW (WITH PINNED COLUMNS & DRAG REORDERING)
// ============================================================
void FaultComparisonTableWidget::buildStackedView()
{
    while (QLayoutItem *item = mStackedLayout->takeAt(0))
    {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    if (mFileNames.isEmpty() || mSelectedColumns.isEmpty())
        return;

    QStringList uniqueColumns;
    for (const QString &col : mSelectedColumns)
    {
        if (!uniqueColumns.contains(col))
            uniqueColumns.append(col);
    }
    mSelectedColumns = uniqueColumns;

    QStringList names = mCustomRowOrder;
    if (names.isEmpty())
    {
        for (int fileIndex = 0; fileIndex < mFileNames.size(); ++fileIndex)
        {
            if (fileIndex >= mHeadersPerFile.size() || fileIndex >= mRowsPerFile.size())
                continue;

            const QStringList &headers = mHeadersPerFile.at(fileIndex);
            const int nameIndex = headers.indexOf("Name");
            if (nameIndex < 0)
                continue;

            for (const QStringList &row : mRowsPerFile.at(fileIndex))
            {
                if (nameIndex >= row.size())
                    continue;

                const QString name = row.at(nameIndex);
                if (!name.isEmpty() && !names.contains(name))
                    names.append(name);
            }
        }
        mCustomRowOrder = names;
    }

    int parameterCount = 0;
    for (const QString &col : mSelectedColumns)
    {
        if (col != "Name")
            ++parameterCount;
    }

    if (mComparisonSummary)
    {
        mComparisonSummary->setText(
            QString("%1 files · %2 parameters · %3 rows")
                .arg(mFileNames.size())
                .arg(parameterCount)
                .arg(names.size())
            );
    }

    if (names.isEmpty())
        return;

    QSet<QString> processedParams;

    auto moveRowOrder = [this](int fromRow, int toRow) {
        if (fromRow < 0 || toRow < 0 || fromRow == toRow || mCustomRowOrder.isEmpty())
            return;
        if (fromRow < mCustomRowOrder.size() && toRow < mCustomRowOrder.size())
        {
            mCustomRowOrder.move(fromRow, toRow);
            if (mLayoutMode == TableLayoutMode::SideBySide)
                RebuildTable();
            else
                buildStackedView();
        }
    };

    for (const QString &column : mSelectedColumns)
    {
        if (column == "Name")
            continue;

        if (processedParams.contains(column))
            continue;
        processedParams.insert(column);

        QList<int> validFileIndices;
        for (int fileIndex = 0; fileIndex < mFileNames.size(); ++fileIndex)
        {
            if (columnAvailableForFile(fileIndex, column))
                validFileIndices.append(fileIndex);
        }

        if (validFileIndices.isEmpty())
            continue;

        QWidget *blockCard = new QWidget(mStackedContainer);
        blockCard->setObjectName("stackedBlockCard");
        blockCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QVBoxLayout *blockLayout = new QVBoxLayout(blockCard);
        blockLayout->setContentsMargins(0, 0, 0, 0);
        blockLayout->setSpacing(0);
        blockLayout->setAlignment(Qt::AlignTop);

        StackedCardHeaderLabel *paramHeader = new StackedCardHeaderLabel(QString(":::  %1").arg(column), column, blockCard);
        paramHeader->setFixedHeight(32);
        paramHeader->setStyleSheet(
            "QLabel {"
            "    background-color: #EAF3FF;"
            "    border: 1px solid #CBD5E1;"
            "    border-top-left-radius: 6px;"
            "    border-top-right-radius: 6px;"
            "    border-bottom: none;"
            "    font-weight: bold;"
            "    font-size: 12px;"
            "    color: #0F3D64;"
            "    padding-left: 12px;"
            "}"
            "QLabel:hover {"
            "    background-color: #DBEAFE;"
            "}"
            );

        paramHeader->onDragDrop = [this](const QString &srcParam, int globalY) {
            QString targetParam;

            for (int i = 0; i < mStackedLayout->count(); ++i)
            {
                QLayoutItem *it = mStackedLayout->itemAt(i);
                if (!it) continue;
                QWidget *w = it->widget();
                if (!w || w->objectName() != "stackedBlockCard") continue;

                QRect gRect = QRect(w->mapToGlobal(QPoint(0, 0)), w->size());
                if (globalY >= gRect.top() && globalY <= gRect.bottom())
                {
                    QLabel *lbl = w->findChild<QLabel*>();
                    if (lbl)
                    {
                        targetParam = lbl->property("paramName").toString();
                    }
                    break;
                }
            }

            if (!targetParam.isEmpty() && targetParam != srcParam)
            {
                int srcIdx = mSelectedColumns.indexOf(srcParam);
                int tgtIdx = mSelectedColumns.indexOf(targetParam);
                if (srcIdx >= 0 && tgtIdx >= 0)
                {
                    mSelectedColumns.swapItemsAt(srcIdx, tgtIdx);
                    buildStackedView();
                }
            }
        };

        blockLayout->addWidget(paramHeader);

        // Primary Mini Table
        QTableWidget *blockTable = new QTableWidget(names.size(), 2 + validFileIndices.size(), blockCard);
        blockTable->setObjectName("stackedMiniTable");
        blockTable->verticalHeader()->setVisible(false);
        blockTable->verticalHeader()->setDefaultSectionSize(32);
        blockTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        blockTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        blockTable->setAlternatingRowColors(false);
        blockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        blockTable->setSelectionMode(QAbstractItemView::NoSelection);
        blockTable->setFocusPolicy(Qt::NoFocus);
        blockTable->setShowGrid(false);

        blockTable->horizontalHeader()->setSectionsMovable(true);
        blockTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

        FaultComparisonItemDelegate *stackedDelegate = new FaultComparisonItemDelegate(blockTable);
        stackedDelegate->setTotalColumns(2 + validFileIndices.size());
        blockTable->setItemDelegate(stackedDelegate);

        blockTable->setStyleSheet(
            "QTableWidget#stackedMiniTable {"
            "    background-color: white;"
            "    border: 1px solid #CBD5E1;"
            "    border-bottom-left-radius: 6px;"
            "    border-bottom-right-radius: 6px;"
            "}"
            "QHeaderView::section {"
            "    background-color: #F8FAFC;"
            "    border: 1px solid #CBD5E1;"
            "    font-weight: bold;"
            "    font-size: 11px;"
            "    color: #374151;"
            "    padding: 4px;"
            "}"
            );

        QStringList headerLabels;
        headerLabels.append("#");
        headerLabels.append("Name");
        for (int fileIdx : validFileIndices)
            headerLabels.append(mFileNames.at(fileIdx));

        blockTable->setHorizontalHeaderLabels(headerLabels);
        blockTable->setColumnWidth(0, 42);
        blockTable->setColumnWidth(1, 150);

        for (int r = 0; r < names.size(); ++r)
        {
            blockTable->setRowHeight(r, 32);
            const QString &name = names.at(r);

            QTableWidgetItem *numItem = new QTableWidgetItem(QString::number(r + 1));
            numItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            numItem->setForeground(QColor("#64748B"));
            blockTable->setItem(r, 0, numItem);

            QTableWidgetItem *nameItem = new QTableWidgetItem(name);
            nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            nameItem->setForeground(QColor("#0F172A"));
            blockTable->setItem(r, 1, nameItem);

            QStringList values;
            for (int fileIdx : validFileIndices)
                values.append(valueFor(fileIdx, name, column));

            const QVector<int> comparisonGroups = buildComparisonGroupIndexes(values);

            for (int vIdx = 0; vIdx < values.size(); ++vIdx)
            {
                const QString val = values.at(vIdx);
                QTableWidgetItem *cell = new QTableWidgetItem(val);
                cell->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

                const int groupIndex = comparisonGroups.value(vIdx, -1);

                if (isEmptyValue(val))
                {
                    cell->setData(Qt::BackgroundRole, QBrush(QColor("#F3F4F6")));
                    cell->setData(Qt::ForegroundRole, QBrush(QColor("#6B7280")));
                }
                else if (groupIndex >= 0)
                {
                    const ComparisonGroupColor color = comparisonColor(groupIndex);
                    cell->setData(Qt::BackgroundRole, QBrush(color.background));
                    cell->setData(Qt::ForegroundRole, QBrush(color.foreground));
                }
                else
                {
                    cell->setData(Qt::BackgroundRole, QBrush(QColor("#FFFFFF")));
                    cell->setData(Qt::ForegroundRole, QBrush(QColor("#1F2937")));
                }

                blockTable->setItem(r, 2 + vIdx, cell);
            }
        }

        QFontMetrics stackedHeaderMetrics(blockTable->horizontalHeader()->font());
        QFontMetrics stackedCellMetrics(blockTable->font());

        for (int c = 2; c < headerLabels.size(); ++c)
        {
            const QString &title = headerLabels.at(c);
            int textW = stackedHeaderMetrics.horizontalAdvance(title) + 36;

            for (int r = 0; r < names.size(); ++r)
            {
                if (QTableWidgetItem *item = blockTable->item(r, c))
                {
                    textW = qMax(textW, stackedCellMetrics.horizontalAdvance(item->text()) + 24);
                }
            }

            const int colW = qBound(110, textW, 350);
            blockTable->setColumnWidth(c, colW);
        }

        const int rowHeight = 32;
        const int headerHeight = 32;
        const int scrollbarHeight = 16;
        const int totalTableHeight = headerHeight + (names.size() * rowHeight) + scrollbarHeight + 6;
        blockTable->setFixedHeight(totalTableHeight);

        // Pinned Overlay for Stacked Mini-Table (# & Busbar Name)
        QTableView *stackedFrozen = new QTableView(blockTable);
        stackedFrozen->setFocusPolicy(Qt::NoFocus);
        stackedFrozen->verticalHeader()->hide();
        stackedFrozen->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        stackedFrozen->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        stackedFrozen->setFrameShape(QFrame::NoFrame);
        stackedFrozen->setEditTriggers(QAbstractItemView::NoEditTriggers);
        stackedFrozen->setSelectionMode(QAbstractItemView::NoSelection);
        stackedFrozen->setShowGrid(false);

        FaultComparisonItemDelegate *frozenDelegate = new FaultComparisonItemDelegate(stackedFrozen);
        frozenDelegate->setTotalColumns(2 + validFileIndices.size());
        stackedFrozen->setItemDelegate(frozenDelegate);

        stackedFrozen->setStyleSheet(
            "QTableView {"
            "    background-color: white;"
            "    border: none;"
            "}"
            "QHeaderView::section {"
            "    background-color: #F8FAFC;"
            "    border: 1px solid #CBD5E1;"
            "    font-weight: bold;"
            "    font-size: 11px;"
            "    color: #374151;"
            "    padding: 4px;"
            "}"
            );

        stackedFrozen->setModel(blockTable->model());
        for (int col = 0; col < blockTable->columnCount(); ++col)
        {
            if (col < 2)
            {
                stackedFrozen->setColumnHidden(col, false);
                stackedFrozen->setColumnWidth(col, blockTable->columnWidth(col));
            }
            else
            {
                stackedFrozen->setColumnHidden(col, true);
            }
        }
        for (int r = 0; r < names.size(); ++r)
        {
            stackedFrozen->setRowHeight(r, 32);
        }

        blockTable->viewport()->stackUnder(stackedFrozen);

        connect(blockTable->verticalScrollBar(), &QScrollBar::valueChanged,
                stackedFrozen->verticalScrollBar(), &QScrollBar::setValue);
        connect(stackedFrozen->verticalScrollBar(), &QScrollBar::valueChanged,
                blockTable->verticalScrollBar(), &QScrollBar::setValue);

        // Clean row dragging for stacked cards
        TableRowDragFilter *blockDragFilter = new TableRowDragFilter(blockTable, moveRowOrder, blockCard);
        blockTable->viewport()->installEventFilter(blockDragFilter);

        TableRowDragFilter *stackedFrozenDragFilter = new TableRowDragFilter(stackedFrozen, moveRowOrder, blockCard);
        stackedFrozen->viewport()->installEventFilter(stackedFrozenDragFilter);

        StackedFrozenFilter *frozenFilter = new StackedFrozenFilter(blockTable, stackedFrozen, blockTable);
        blockTable->viewport()->installEventFilter(frozenFilter);
        blockTable->horizontalHeader()->installEventFilter(frozenFilter);
        frozenFilter->updateGeometry();

        blockCard->setFixedHeight(32 + totalTableHeight);
        blockLayout->addWidget(blockTable);
        mStackedLayout->addWidget(blockCard);
    }

    mStackedLayout->addStretch(1);
}

QStringList FaultComparisonTableWidget::groupNames() const
{
    return mGroupNames;
}

void FaultComparisonTableWidget::setSelectedColumns(const QStringList &columns)
{
    mSelectedColumns = columns;
    if (mLayoutMode == TableLayoutMode::SideBySide)
        RebuildTable();
    else
        buildStackedView();
}

bool FaultComparisonTableWidget::columnAvailableForFile(int fileIndex, const QString &column) const
{
    if (fileIndex < 0 || fileIndex >= mHeadersPerFile.size())
        return false;

    QString rawColumn = column;

    if (fileIndex < mColumnMappingsPerFile.size())
    {
        const QMap<QString, QString> &mapping = mColumnMappingsPerFile.at(fileIndex);
        if (mapping.contains(column))
            rawColumn = mapping.value(column);
    }

    return mHeadersPerFile.at(fileIndex).contains(rawColumn);
}

QString FaultComparisonTableWidget::valueFor(int fileIndex, const QString &name, const QString &column) const
{
    if (fileIndex < 0 || fileIndex >= mHeadersPerFile.size())
        return "";

    QString rawColumn = column;

    if (fileIndex < mColumnMappingsPerFile.size())
    {
        const QMap<QString, QString> &mapping = mColumnMappingsPerFile.at(fileIndex);
        if (mapping.contains(column))
            rawColumn = mapping.value(column);
    }

    const QStringList &headers = mHeadersPerFile.at(fileIndex);
    const int columnIndex = headers.indexOf(rawColumn);

    if (columnIndex < 0)
        return "";

    const QList<QStringList> &rows = mRowsPerFile.at(fileIndex);

    for (const QStringList &row : rows)
    {
        if (row.isEmpty())
            continue;

        const int nameIndex = headers.indexOf("Name");
        if (nameIndex < 0 || nameIndex >= row.size())
            continue;

        if (row.at(nameIndex) == name)
        {
            if (columnIndex < row.size())
                return row.at(columnIndex);
            return "";
        }
    }

    return "";
}

double FaultComparisonTableWidget::numericValueFor(int fileIndex, const QString &name, const QString &column) const
{
    const QString val = valueFor(fileIndex, name, column);
    bool ok = false;
    const double d = val.trimmed().toDouble(&ok);
    return ok ? d : 0.0;
}

void FaultComparisonTableWidget::saveColumnOrder() {}
void FaultComparisonTableWidget::saveFileOrder() {}
void FaultComparisonTableWidget::restoreFileOrder() {}
void FaultComparisonTableWidget::restoreColumnOrder() {}
void FaultComparisonTableWidget::removeFileFromSavedOrder(const QString &fileName) { Q_UNUSED(fileName); }
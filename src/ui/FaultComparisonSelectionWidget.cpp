#include "FaultComparisonSelectionWidget.h"

#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QIcon>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QStyle>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QDir>
#include <QButtonGroup>
#include <QRadioButton>
#include <QFrame>
#include <QMouseEvent>

static QString getCheckboxIconPath(const QColor &bgColor, const QColor &checkColor, const QString &filename)
{
    QString filePath = QDir::temp().filePath(filename);

    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);

    {
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing, true);

        p.setBrush(bgColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(0, 0, 16, 16), 3.5, 3.5);

        QPainterPath path;
        path.moveTo(3.5, 8.0);
        path.lineTo(6.5, 11.2);
        path.lineTo(12.5, 4.5);

        QPen pen(checkColor, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.strokePath(path, pen);
    }

    pix.save(filePath, "PNG");
    return QDir::fromNativeSeparators(filePath);
}

static QString getRadioIconPath(bool checked, const QColor &ringColor, const QColor &dotColor, const QString &filename)
{
    QString filePath = QDir::temp().filePath(filename);

    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);

    {
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing, true);

        p.setPen(QPen(ringColor, 1.6));
        p.setBrush(Qt::white);
        p.drawEllipse(QRectF(1.0, 1.0, 13.8, 13.8));

        if (checked)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(dotColor);
            p.drawEllipse(QRectF(4.5, 4.5, 6.8, 6.8));
        }
    }

    pix.save(filePath, "PNG");
    return QDir::fromNativeSeparators(filePath);
}

FaultComparisonSelectionWidget::FaultComparisonSelectionWidget(QWidget *parent)
    : QWidget(parent)
    , mCurrentMode(TableLayoutMode::Stacked) // Set Stacked as default
    , mMainLayout(nullptr)
    , mHeaderWidget(nullptr)
    , mHeaderLayout(nullptr)
    , mCollapseButton(nullptr)
    , mTitleLabel(nullptr)
    , mSubtitleLabel(nullptr)
    , mContentWidget(nullptr)
    , mContentLayout(nullptr)
    , mSearchEdit(nullptr)
    , mColumnScrollArea(nullptr)
    , mColumnContainer(nullptr)
    , mColumnLayout(nullptr)
    , mActionWidget(nullptr)
    , mActionLayout(nullptr)
    , mSelectAllButton(nullptr)
    , mClearAllButton(nullptr)
    , mLayoutGroup(nullptr)
    , mSideBySideCard(nullptr)
    , mStackedCard(nullptr)
    , mSideBySideRadio(nullptr)
    , mStackedRadio(nullptr)
{
    setObjectName("paramsCard");

    setMinimumWidth(230);
    setMaximumWidth(280);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    mMainLayout = new QVBoxLayout(this);
    mMainLayout->setContentsMargins(10, 10, 10, 10);
    mMainLayout->setSpacing(0);

    // 1. Header
    mHeaderWidget = new QWidget(this);
    mHeaderWidget->setObjectName("paramsHeader");

    mHeaderLayout = new QHBoxLayout(mHeaderWidget);
    mHeaderLayout->setContentsMargins(0, 0, 0, 6);
    mHeaderLayout->setSpacing(6);

    mCollapseButton = new QToolButton(mHeaderWidget);
    mCollapseButton->setObjectName("paramsCollapseButton");
    mCollapseButton->setCheckable(true);
    mCollapseButton->setChecked(false);
    mCollapseButton->setArrowType(Qt::LeftArrow);
    mCollapseButton->setAutoRaise(true);
    mCollapseButton->setFixedSize(24, 24);
    mCollapseButton->setToolTip("Collapse panel");

    QWidget *headerTextWidget = new QWidget(mHeaderWidget);
    headerTextWidget->setObjectName("paramsHeaderText");

    QVBoxLayout *headerTextLayout = new QVBoxLayout(headerTextWidget);
    headerTextLayout->setContentsMargins(0, 0, 0, 0);
    headerTextLayout->setSpacing(1);

    mTitleLabel = new QLabel("Parameters", headerTextWidget);
    mTitleLabel->setObjectName("paramsTitle");

    mSubtitleLabel = new QLabel("Select parameters to compare", headerTextWidget);
    mSubtitleLabel->setObjectName("paramsSubtitle");

    headerTextLayout->addWidget(mTitleLabel);
    headerTextLayout->addWidget(mSubtitleLabel);

    mHeaderLayout->addWidget(mCollapseButton);
    mHeaderLayout->addWidget(headerTextWidget, 1);

    mMainLayout->addWidget(mHeaderWidget);

    // 2. Content Container
    mContentWidget = new QWidget(this);
    mContentWidget->setObjectName("paramsContent");

    mContentLayout = new QVBoxLayout(mContentWidget);
    mContentLayout->setContentsMargins(0, 0, 0, 0);
    mContentLayout->setSpacing(6);

    // Search input
    mSearchEdit = new QLineEdit(mContentWidget);
    mSearchEdit->setObjectName("paramsSearch");
    mSearchEdit->setPlaceholderText("Search parameters...");
    mSearchEdit->setFixedHeight(28);

    QIcon searchIcon = style()->standardIcon(QStyle::SP_FileDialogContentsView);
    mSearchEdit->addAction(searchIcon, QLineEdit::LeadingPosition);
    mContentLayout->addWidget(mSearchEdit, 0);

    // Parameters Checkbox Scroll Area
    mColumnScrollArea = new QScrollArea(mContentWidget);
    mColumnScrollArea->setObjectName("paramsScrollArea");
    mColumnScrollArea->setWidgetResizable(true);
    mColumnScrollArea->setFrameShape(QFrame::NoFrame);
    mColumnScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mColumnScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mColumnScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mColumnScrollArea->setMinimumHeight(60);

    mColumnContainer = new QWidget();
    mColumnContainer->setObjectName("paramsListContainer");
    mColumnContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    mColumnLayout = new QVBoxLayout(mColumnContainer);
    mColumnLayout->setContentsMargins(2, 2, 2, 2);
    mColumnLayout->setSpacing(2);
    mColumnLayout->setAlignment(Qt::AlignTop);

    mColumnScrollArea->setWidget(mColumnContainer);
    mContentLayout->addWidget(mColumnScrollArea, 1);

    // Action buttons
    mActionWidget = new QWidget(mContentWidget);
    mActionWidget->setObjectName("paramsActionBar");
    mActionWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mActionLayout = new QHBoxLayout(mActionWidget);
    mActionLayout->setContentsMargins(0, 4, 0, 4);
    mActionLayout->setSpacing(6);

    mSelectAllButton = new QPushButton("Select All", mActionWidget);
    mSelectAllButton->setObjectName("paramsListButton");
    mSelectAllButton->setFixedHeight(26);

    mClearAllButton = new QPushButton("Clear All", mActionWidget);
    mClearAllButton->setObjectName("paramsListButton");
    mClearAllButton->setFixedHeight(26);

    mActionLayout->addWidget(mSelectAllButton);
    mActionLayout->addWidget(mClearAllButton);
    mContentLayout->addWidget(mActionWidget, 0);

    // Table Layout Selector
    createLayoutSection();

    mMainLayout->addWidget(mContentWidget, 1);

    connect(mCollapseButton, &QToolButton::toggled, this, &FaultComparisonSelectionWidget::toggleCollapsed);
    connect(mSearchEdit, &QLineEdit::textChanged, this, &FaultComparisonSelectionWidget::filterColumns);
    connect(mSelectAllButton, &QPushButton::clicked, this, &FaultComparisonSelectionWidget::selectAllColumns);
    connect(mClearAllButton, &QPushButton::clicked, this, &FaultComparisonSelectionWidget::clearAllColumns);

    const QString checkedPath = getCheckboxIconPath(QColor("#1769AA"), Qt::white, "param_check_on.png");
    const QString checkedHoverPath = getCheckboxIconPath(QColor("#14588E"), Qt::white, "param_check_hover.png");
    const QString disabledPath = getCheckboxIconPath(QColor("#94A3B8"), Qt::white, "param_check_disabled.png");

    const QString radioOffPath   = getRadioIconPath(false, QColor("#94A3B8"), Qt::transparent, "param_radio_off.png");
    const QString radioHoverPath = getRadioIconPath(false, QColor("#1769AA"), Qt::transparent, "param_radio_hover.png");
    const QString radioOnPath    = getRadioIconPath(true,  QColor("#1769AA"), QColor("#1769AA"), "param_radio_on.png");

    setStyleSheet(QString(
                      "QWidget#paramsCard {"
                      "    background: #FFFFFF;"
                      "    border: 1px solid #E5E7EB;"
                      "    border-radius: 8px;"
                      "}"
                      "QWidget#paramsHeader { background: #FFFFFF; border: none; }"
                      "QWidget#paramsHeaderText { background: transparent; border: none; }"
                      "QToolButton#paramsCollapseButton {"
                      "    background: transparent; border: none; color: #111827; padding: 0px;"
                      "}"
                      "QToolButton#paramsCollapseButton:hover { background: #F3F4F6; border-radius: 4px; }"
                      "QLabel#paramsTitle {"
                      "    background: transparent; border: none; color: #111827; font-size: 13px; font-weight: 600;"
                      "}"
                      "QLabel#paramsSubtitle { background: transparent; border: none; color: #6B7280; font-size: 10.5px; }"
                      "QLineEdit#paramsSearch {"
                      "    background: #F8FAFC; color: #1F2937; border: 1px solid #E5E7EB; border-radius: 5px; padding: 3px 6px; font-size: 11.5px;"
                      "}"
                      "QLineEdit#paramsSearch:focus { background: #FFFFFF; border: 1px solid #2563EB; }"
                      "QScrollArea#paramsScrollArea { background: #FFFFFF; border: none; }"
                      "QScrollArea#paramsScrollArea > QWidget > QWidget { background: #FFFFFF; }"
                      "QWidget#paramsListContainer { background: #FFFFFF; border: none; }"
                      "QCheckBox {"
                      "    min-height: 24px; max-height: 24px; padding: 0px 4px; spacing: 8px; color: #1F2937; background: transparent; font-size: 11.5px;"
                      "}"
                      "QCheckBox:hover { color: #1769AA; }"
                      "QCheckBox::indicator { width: 15px; height: 15px; border: 1px solid #CBD5E1; border-radius: 4px; background-color: #FFFFFF; }"
                      "QCheckBox::indicator:hover { border: 1px solid #1769AA; }"
                      "QCheckBox::indicator:checked { border: none; background: transparent; image: url(%1); }"
                      "QCheckBox::indicator:checked:hover { image: url(%2); }"
                      "QCheckBox::indicator:disabled { border: none; background: transparent; image: url(%3); }"
                      "QPushButton#paramsListButton {"
                      "    background: #FFFFFF; color: #475569; border: 1px solid #E5E7EB; border-radius: 5px; padding: 2px 8px; font-size: 11px; font-weight: 600;"
                      "}"
                      "QPushButton#paramsListButton:hover:!disabled { background: #F8FAFC; color: #1769AA; border: 1px solid #CBD5E1; }"
                      "QFrame#layoutCard {"
                      "    background-color: #FFFFFF; border: 1px solid #D2D6DC; border-radius: 6px;"
                      "}"
                      "QFrame#layoutCard[selected='true'] {"
                      "    background-color: #F0F7FF; border: 1.5px solid #1769AA;"
                      "}"
                      "QLabel#layoutCardTitle { font-size: 11.5px; font-weight: bold; color: #1F2933; }"
                      "QLabel#layoutCardDesc { font-size: 9.5px; color: #6B7280; }"
                      "QRadioButton#layoutRadio { background: transparent; border: none; spacing: 0px; }"
                      "QRadioButton#layoutRadio::indicator { width: 16px; height: 16px; border: none; background: transparent; }"
                      "QRadioButton#layoutRadio::indicator:unchecked { image: url(%4); }"
                      "QRadioButton#layoutRadio::indicator:unchecked:hover { image: url(%5); }"
                      "QRadioButton#layoutRadio::indicator:checked { image: url(%6); }"
                      ).arg(checkedPath, checkedHoverPath, disabledPath,
                           radioOffPath, radioHoverPath, radioOnPath));

    setEmptyState(true);
}

void FaultComparisonSelectionWidget::setTopSectionCollapsed(bool collapsed)
{
    Q_UNUSED(collapsed);
}

void FaultComparisonSelectionWidget::createLayoutSection()
{
    QFrame *separator = new QFrame(mContentWidget);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #E2E8F0; margin-top: 4px; margin-bottom: 2px;");
    mContentLayout->addWidget(separator);

    QLabel *layoutTitle = new QLabel("Table Layout", mContentWidget);
    layoutTitle->setStyleSheet("font-size: 12px; font-weight: 600; color: #1F2937; padding-left: 1px; margin-bottom: 2px;");
    mContentLayout->addWidget(layoutTitle);

    mLayoutGroup = new QButtonGroup(this);
    mLayoutGroup->setExclusive(true);

    // Option 1: Side by side
    mSideBySideCard = new QFrame(mContentWidget);
    mSideBySideCard->setObjectName("layoutCard");
    mSideBySideCard->setFixedHeight(40);
    mSideBySideCard->setCursor(Qt::PointingHandCursor);

    mSideBySideRadio = new QRadioButton(mSideBySideCard);
    mSideBySideRadio->setObjectName("layoutRadio");
    mSideBySideRadio->setFixedSize(16, 16);
    mSideBySideRadio->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QHBoxLayout *sideCardLayout = new QHBoxLayout(mSideBySideCard);
    sideCardLayout->setContentsMargins(10, 2, 8, 2);
    sideCardLayout->setSpacing(8);

    QLabel *sideTitle = new QLabel("Side by side", mSideBySideCard);
    sideTitle->setObjectName("layoutCardTitle");

    sideCardLayout->addWidget(mSideBySideRadio, 0, Qt::AlignVCenter);
    sideCardLayout->addWidget(sideTitle, 1, Qt::AlignVCenter);

    // Option 2: Stacked
    mStackedCard = new QFrame(mContentWidget);
    mStackedCard->setObjectName("layoutCard");
    mStackedCard->setFixedHeight(40);
    mStackedCard->setCursor(Qt::PointingHandCursor);

    mStackedRadio = new QRadioButton(mStackedCard);
    mStackedRadio->setObjectName("layoutRadio");
    mStackedRadio->setFixedSize(16, 16);
    mStackedRadio->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QHBoxLayout *stackedCardLayout = new QHBoxLayout(mStackedCard);
    stackedCardLayout->setContentsMargins(10, 2, 8, 2);
    stackedCardLayout->setSpacing(8);

    QLabel *stackedTitle = new QLabel("Stacked", mStackedCard);
    stackedTitle->setObjectName("layoutCardTitle");

    stackedCardLayout->addWidget(mStackedRadio, 0, Qt::AlignVCenter);
    stackedCardLayout->addWidget(stackedTitle, 1, Qt::AlignVCenter);

    mLayoutGroup->addButton(mSideBySideRadio, static_cast<int>(TableLayoutMode::SideBySide));
    mLayoutGroup->addButton(mStackedRadio, static_cast<int>(TableLayoutMode::Stacked));

    mContentLayout->addWidget(mSideBySideCard, 0);
    mContentLayout->addWidget(mStackedCard, 0);

    // Stacked enabled and checked by default
    mStackedRadio->setChecked(true);
    mStackedCard->setProperty("selected", true);
    mSideBySideRadio->setChecked(false);
    mSideBySideCard->setProperty("selected", false);

    mSideBySideCard->installEventFilter(this);
    mStackedCard->installEventFilter(this);

    connect(mLayoutGroup, &QButtonGroup::idClicked,
            this, &FaultComparisonSelectionWidget::onLayoutCardClicked);
}

bool FaultComparisonSelectionWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            if (watched == mSideBySideCard && mSideBySideRadio)
            {
                mSideBySideRadio->setChecked(true);
                onLayoutCardClicked(static_cast<int>(TableLayoutMode::SideBySide));
                return true;
            }
            else if (watched == mStackedCard && mStackedRadio)
            {
                mStackedRadio->setChecked(true);
                onLayoutCardClicked(static_cast<int>(TableLayoutMode::Stacked));
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void FaultComparisonSelectionWidget::onLayoutCardClicked(int id)
{
    const TableLayoutMode newMode = static_cast<TableLayoutMode>(id);

    mSideBySideCard->setProperty("selected", newMode == TableLayoutMode::SideBySide);
    mStackedCard->setProperty("selected", newMode == TableLayoutMode::Stacked);

    mSideBySideCard->style()->unpolish(mSideBySideCard);
    mSideBySideCard->style()->polish(mSideBySideCard);
    mStackedCard->style()->unpolish(mStackedCard);
    mStackedCard->style()->polish(mStackedCard);

    if (mCurrentMode != newMode)
    {
        mCurrentMode = newMode;
        emit layoutModeChanged(mCurrentMode);
    }
}

TableLayoutMode FaultComparisonSelectionWidget::currentLayoutMode() const
{
    return mCurrentMode;
}

void FaultComparisonSelectionWidget::setAvailableColumns(const QStringList &columns)
{
    createColumnCheckboxes(columns);
    setEmptyState(columns.isEmpty());
}

void FaultComparisonSelectionWidget::createColumnCheckboxes(const QStringList &columns)
{
    while (mColumnLayout->count() > 0)
    {
        QLayoutItem *item = mColumnLayout->takeAt(0);
        if (!item)
            continue;
        if (QWidget *widget = item->widget())
            widget->deleteLater();
        delete item;
    }

    for (const QString &column : columns)
    {
        if (column.trimmed().isEmpty())
            continue;

        QCheckBox *checkBox = new QCheckBox(column, mColumnContainer);
        checkBox->setObjectName("parameterCheckBox");
        checkBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        const bool isName = column.compare("Name", Qt::CaseInsensitive) == 0;

        if (isName)
        {
            checkBox->setChecked(true);
            checkBox->setEnabled(false);
        }
        else
        {
            checkBox->setChecked(false);
            connect(checkBox, &QCheckBox::toggled, this, &FaultComparisonSelectionWidget::selectionChanged);
        }

        mColumnLayout->addWidget(checkBox);
    }
    mColumnContainer->adjustSize();
    emit selectionChanged();
}

bool FaultComparisonSelectionWidget::isNameColumn(QCheckBox *checkBox) const
{
    if (!checkBox)
        return false;
    return checkBox->text().compare("Name", Qt::CaseInsensitive) == 0;
}

QStringList FaultComparisonSelectionWidget::selectedColumns() const
{
    QStringList selected;
    const QList<QCheckBox *> checkBoxes = mColumnContainer->findChildren<QCheckBox *>();

    for (QCheckBox *checkBox : checkBoxes)
    {
        if (!checkBox)
            continue;

        if (isNameColumn(checkBox) || checkBox->isChecked())
        {
            selected.append(checkBox->text());
        }
    }
    return selected;
}

void FaultComparisonSelectionWidget::setSelectedColumns(const QStringList &columns)
{
    const QList<QCheckBox *> checkBoxes = mColumnContainer->findChildren<QCheckBox *>();

    for (QCheckBox *checkBox : checkBoxes)
    {
        if (!checkBox)
            continue;

        QSignalBlocker blocker(checkBox);

        if (isNameColumn(checkBox))
        {
            checkBox->setChecked(true);
            continue;
        }

        checkBox->setChecked(columns.contains(checkBox->text()));
    }

    emit selectionChanged();
}

void FaultComparisonSelectionWidget::filterColumns(const QString &text)
{
    const QString searchText = text.trimmed();
    const QList<QCheckBox *> checkBoxes = mColumnContainer->findChildren<QCheckBox *>();

    for (QCheckBox *checkBox : checkBoxes)
    {
        if (!checkBox)
            continue;

        const bool matches = searchText.isEmpty() ||
                             checkBox->text().contains(searchText, Qt::CaseInsensitive);
        checkBox->setVisible(matches);
    }

    mColumnLayout->invalidate();
    mColumnContainer->adjustSize();
}

void FaultComparisonSelectionWidget::selectAllColumns()
{
    const QList<QCheckBox *> checkBoxes = mColumnContainer->findChildren<QCheckBox *>();
    bool changed = false;

    for (QCheckBox *checkBox : checkBoxes)
    {
        if (!checkBox || isNameColumn(checkBox))
            continue;

        if (checkBox->isVisible() && checkBox->isEnabled() && !checkBox->isChecked())
        {
            checkBox->setChecked(true);
            changed = true;
        }
    }

    if (changed)
        emit selectionChanged();
}

void FaultComparisonSelectionWidget::clearAllColumns()
{
    const QList<QCheckBox *> checkBoxes = mColumnContainer->findChildren<QCheckBox *>();
    bool changed = false;

    for (QCheckBox *checkBox : checkBoxes)
    {
        if (!checkBox || isNameColumn(checkBox))
            continue;

        if (checkBox->isVisible() && checkBox->isEnabled() && checkBox->isChecked())
        {
            checkBox->setChecked(false);
            changed = true;
        }
    }

    if (changed)
        emit selectionChanged();
}

void FaultComparisonSelectionWidget::toggleCollapsed(bool collapsed)
{
    mContentWidget->setVisible(!collapsed);

    if (collapsed)
    {
        mTitleLabel->setVisible(false);
        mSubtitleLabel->setVisible(false);
        mCollapseButton->setArrowType(Qt::RightArrow);
        mCollapseButton->setToolTip("Expand Parameters Panel");

        setMinimumWidth(40);
        setMaximumWidth(40);
    }
    else
    {
        mTitleLabel->setVisible(true);
        mSubtitleLabel->setVisible(true);
        mCollapseButton->setArrowType(Qt::LeftArrow);
        mCollapseButton->setToolTip("Collapse Parameters Panel");

        setMinimumWidth(230);
        setMaximumWidth(280);
    }

    updateGeometry();
    if (parentWidget())
        parentWidget()->updateGeometry();
}

void FaultComparisonSelectionWidget::setEmptyState(bool empty)
{
    if (mSearchEdit)
    {
        mSearchEdit->setEnabled(!empty);
        if (empty)
            mSearchEdit->clear();
    }

    if (mSelectAllButton)
        mSelectAllButton->setEnabled(!empty);

    if (mClearAllButton)
        mClearAllButton->setEnabled(!empty);

    if (mSideBySideCard)
        mSideBySideCard->setEnabled(!empty);

    if (mStackedCard)
        mStackedCard->setEnabled(!empty);

    if (empty)
    {
        while (mColumnLayout->count() > 0)
        {
            QLayoutItem *item = mColumnLayout->takeAt(0);
            if (item && item->widget())
                item->widget()->deleteLater();
            delete item;
        }

        QLabel *emptyLabel = new QLabel("Load CSV files to choose\nparameters to compare", mColumnContainer);
        emptyLabel->setObjectName("paramsEmptyLabel");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet(
            "QLabel#paramsEmptyLabel {"
            "    color: #94A3B8; font-size: 11px; padding: 35px 10px; line-height: 1.4;"
            "}"
            );

        mColumnLayout->addWidget(emptyLabel);
    }
}
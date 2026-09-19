#include "FaultComparisonTableWidget.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPainter>
#include <QAbstractItemView>
#include <QPaintEvent>
#include <QScrollBar>
#include <QLabel>
#include <QFont>
#include <QMouseEvent>
#include <QSignalBlocker>

// ============================================================
// Custom two-level table header
// ============================================================

class FaultTableHeader : public QHeaderView
{
public:

    explicit FaultTableHeader(
        Qt::Orientation orientation,
        QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        setDefaultAlignment(Qt::AlignCenter);

        // Allow columns to be moved
        setSectionsMovable(true);

        setSectionsClickable(false);

        setHighlightSections(false);

        setMinimumSectionSize(90);

        setDefaultSectionSize(110);
    }


    // --------------------------------------------------------
    // Set group name for every logical column
    // --------------------------------------------------------

    void setGroupNames(const QStringList &groups)
    {
        mGroupNames = groups;

        viewport()->update();
    }


    // --------------------------------------------------------
    // Header height
    // --------------------------------------------------------

    QSize sizeHint() const override
    {
        QSize size = QHeaderView::sizeHint();

        size.setHeight(55);

        return size;
    }


protected:

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    // ========================================================
    // PAINT HEADER
    // ========================================================

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(viewport());

        painter.setRenderHint(
            QPainter::Antialiasing,
            false
            );


        const int sectionCount = count();

        if (sectionCount <= 0)
            return;


        const int halfHeight = height() / 2;


        // ====================================================
        // Draw individual column headers
        // ====================================================

        for (int visualIndex = 0;
             visualIndex < sectionCount;
             ++visualIndex)
        {
            // IMPORTANT:
            // logicalIndex() takes a VISUAL SECTION INDEX.
            //
            // Do not use logicalIndexAt().
            //
            const int logical =
                this->logicalIndex(visualIndex);


            if (logical < 0)
                continue;


            const int x =
                sectionViewportPosition(logical);


            const int sectionWidth =
                sectionSize(logical);


            if (sectionWidth <= 0)
                continue;


            QRect fullRect(
                x,
                0,
                sectionWidth,
                height()
                );


            QRect bottomRect(
                x,
                halfHeight,
                sectionWidth,
                height() - halfHeight
                );


            // ------------------------------------------------
            // Header background
            // ------------------------------------------------

            painter.fillRect(
                fullRect,
                palette().window()
                );


            // ------------------------------------------------
            // Get column name
            // ------------------------------------------------

            QString columnName;


            if (model())
            {
                columnName =
                    model()->headerData(
                               logical,
                               Qt::Horizontal,
                               Qt::DisplayRole
                               ).toString();
            }


            // ------------------------------------------------
            // Draw column name
            // ------------------------------------------------

            painter.setPen(
                palette().text().color()
                );


            painter.drawText(
                bottomRect.adjusted(
                    4,
                    2,
                    -4,
                    -2
                    ),
                Qt::AlignCenter |
                    Qt::TextWordWrap,
                columnName
                );


            // ------------------------------------------------
            // Column border
            // ------------------------------------------------

            painter.setPen(
                palette().mid().color()
                );


            painter.drawRect(
                QRect(
                    x,
                    0,
                    sectionWidth - 1,
                    height() - 1
                    )
                );
        }


        // ====================================================
        // Draw file-name groups
        // ====================================================

        int startVisualIndex = 0;


        while (startVisualIndex < sectionCount)
        {
            const int firstLogical =
                this->logicalIndex(startVisualIndex);


            if (firstLogical < 0)
            {
                ++startVisualIndex;
                continue;
            }


            QString groupName;


            if (firstLogical < mGroupNames.size())
            {
                groupName =
                    mGroupNames.at(firstLogical);
            }


            // ------------------------------------------------
            // Find the last VISUAL column in this group
            // ------------------------------------------------

            int endVisualIndex =
                startVisualIndex;


            while (endVisualIndex + 1 < sectionCount)
            {
                const int nextLogical =
                    this->logicalIndex(
                        endVisualIndex + 1
                        );


                if (nextLogical < 0)
                    break;


                QString nextGroup;


                if (nextLogical < mGroupNames.size())
                {
                    nextGroup =
                        mGroupNames.at(nextLogical);
                }


                if (nextGroup != groupName)
                    break;


                ++endVisualIndex;
            }


            // ------------------------------------------------
            // Get actual screen positions
            // ------------------------------------------------

            const int firstGroupLogical =
                this->logicalIndex(startVisualIndex);


            const int lastGroupLogical =
                this->logicalIndex(endVisualIndex);


            if (firstGroupLogical >= 0 &&
                lastGroupLogical >= 0)
            {
                const int left =
                    sectionViewportPosition(
                        firstGroupLogical
                        );


                const int right =
                    sectionViewportPosition(
                        lastGroupLogical
                        )
                    +
                    sectionSize(
                        lastGroupLogical
                        );


                const int groupWidth =
                    right - left;


                if (groupWidth > 0)
                {
                    QRect groupRect(
                        left,
                        0,
                        groupWidth,
                        halfHeight
                        );


                    // ----------------------------------------
                    // Group background
                    // ----------------------------------------

                    painter.fillRect(
                        groupRect,
                        palette().window()
                        );


                    // ----------------------------------------
                    // File name
                    // ----------------------------------------

                    if (!groupName.isEmpty())
                    {
                        painter.setPen(
                            palette().text().color()
                            );


                        painter.drawText(
                            groupRect.adjusted(
                                4,
                                1,
                                -4,
                                -1
                                ),
                            Qt::AlignCenter |
                                Qt::TextSingleLine,
                            groupName
                            );
                    }


                    // ----------------------------------------
                    // Group border
                    // ----------------------------------------

                    painter.setPen(
                        palette().mid().color()
                        );


                    painter.drawRect(
                        QRect(
                            groupRect.left(),
                            groupRect.top(),
                            groupRect.width() - 1,
                            groupRect.height() - 1
                            )
                        );
                }
            }


            startVisualIndex =
                endVisualIndex + 1;
        }


        // ====================================================
        // Horizontal line between header levels
        // ====================================================

        painter.setPen(
            palette().mid().color()
            );


        painter.drawLine(
            0,
            halfHeight,
            width(),
            halfHeight
            );
    }

private:

    // One group name for each LOGICAL column.
    //
    // Example:
    //
    // Column 0 = Name
    // Column 1 = AC Mag -> File 1
    // Column 2 = DC Mag -> File 1
    // Column 3 = AC Mag -> File 2
    // Column 4 = DC Mag -> File 2
    //
    QStringList mGroupNames;

    bool mGroupDragging = false;

    QString mDraggedGroup;

    int mGroupDragStartX = -1;

    void moveGroup(
        const QString &sourceGroup,
        const QString &targetGroup
        );

};

// ============================================================
// Constructor
// ============================================================

FaultComparisonTableWidget::FaultComparisonTableWidget(
    QWidget *parent)
    : QWidget(parent)
{

    mTable = new QTableWidget(this);
    QVBoxLayout *layout =
        new QVBoxLayout(this);


    layout->setContentsMargins(
        0,
        0,
        0,
        0
        );


    layout->setSpacing(0);

    // ========================================================
    // Empty state
    // ========================================================

    mEmptyLabel =
        new QLabel(this);

    mEmptyLabel->setText(
        "No files loaded"
        );

    mEmptyLabel->setAlignment(
        Qt::AlignCenter
        );

    QFont emptyFont =
        mEmptyLabel->font();

    emptyFont.setPointSize(11);

    mEmptyLabel->setFont(
        emptyFont
        );

    mEmptyLabel->setStyleSheet(
        "QLabel {"
        "    color: #666666;"
        "    background-color: white;"
        "}"
        );

    layout->addWidget(
        mEmptyLabel
        );





    // ========================================================
    // Create custom horizontal header
    // ========================================================

    FaultTableHeader *header =
        new FaultTableHeader(
            Qt::Horizontal,
            mTable
            );


    mTable->setHorizontalHeader(
        header
        );


    // ========================================================
    // Table settings
    // ========================================================

    mTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );


    mTable->setSelectionBehavior(
        QAbstractItemView::SelectItems
        );


    mTable->setAlternatingRowColors(
        true
        );


    mTable->verticalHeader()
        ->setVisible(true);


    // ========================================================
    // Column movement
    // ========================================================

    header->setSectionsMovable(true);


    header->setSectionResizeMode(
        QHeaderView::Interactive
        );


    header->setMinimumSectionSize(90);


    header->setDefaultSectionSize(110);


    connect(
        header,
        &QHeaderView::sectionMoved,
        this,
        [this](int, int, int)
        {
            saveFileOrder();
            saveColumnOrder();
        }
        );

    // ========================================================
    // Scrolling
    // ========================================================

    mTable->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAsNeeded
        );


    mTable->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded
        );


    mTable->setWordWrap(true);


    // ========================================================
    // Add table
    // ========================================================

    layout->addWidget(
        mTable
        );


    // ========================================================
    // Initial state
    // ========================================================

    mEmptyLabel->show();

    mTable->hide();

}

QTableWidget *FaultComparisonTableWidget::tableWidget() const
{
    return mTable;
}

// ============================================================
// SET DATA
// ============================================================

void FaultComparisonTableWidget::setData(
    const QStringList &fileNames,
    const QList<QStringList> &headersPerFile,
    const QList<QList<QStringList>> &rowsPerFile,
    const QStringList &selectedColumns,
    const QList<QMap<QString, QString>> &columnMappingsPerFile,
    bool differenceEnabled
    )
{
    // ========================================================
    // SAVE CURRENT USER COLUMN ORDER
    // ========================================================
    //
    // This is the important part.
    //
    // Comparison Selection changes call setData().
    // Therefore we must save the user's current arrangement
    // BEFORE replacing the table data.
    //
    // Do not save when the table is empty because that would
    // save the initial/default order.
    // ========================================================

    if (mTable &&
        mTable->columnCount() > 0)
    {
        saveColumnOrder();
    }


    // ========================================================
    // UPDATE DATA
    // ========================================================

    mFileNames =
        fileNames;

    mHeadersPerFile =
        headersPerFile;

    mRowsPerFile =
        rowsPerFile;

    mSelectedColumns =
        selectedColumns;

    mColumnMappingsPerFile =
        columnMappingsPerFile;

    mDifferenceEnabled =
        differenceEnabled;


    // ========================================================
    // EMPTY STATE
    // ========================================================

    if (mFileNames.isEmpty())
    {
        mTable->hide();
        mEmptyLabel->show();

        return;
    }


    // ========================================================
    // SHOW TABLE
    // ========================================================

    mEmptyLabel->hide();
    mTable->show();


    // ========================================================
    // REBUILD TABLE
    // ========================================================

    RebuildTable();

    if (!mSavedFileOrder.isEmpty())
    {
        restoreFileOrder();
    }
    // ========================================================
    // RESTORE USER'S PREVIOUS ARRANGEMENT
    // ========================================================


}
QStringList FaultComparisonTableWidget::groupNames() const
{
    return mGroupNames;
}

// ============================================================
// GROUP DRAG - MOUSE PRESS
// ============================================================

void FaultTableHeader::mousePressEvent(
    QMouseEvent *event)
{
    if (!event)
        return;

    // --------------------------------------------------------
    // Top half of header = file-group dragging
    // --------------------------------------------------------

    const int halfHeight =
        height() / 2;

    if (event->button() == Qt::LeftButton &&
        event->position().y() < halfHeight)
    {
        const int visualIndex =
            visualIndexAt(
                static_cast<int>(
                    event->position().x()
                    )
                );

        if (visualIndex >= 0)
        {
            const int logicalSection =
                this->logicalIndex(visualIndex);

            if (logicalSection >= 0 &&
                logicalSection < mGroupNames.size())
            {
                const QString group =
                    mGroupNames.at(logicalSection);

                if (!group.isEmpty())
                {
                    mGroupDragging = true;

                    mDraggedGroup =
                        group;

                    mGroupDragStartX =
                        static_cast<int>(
                            event->position().x()
                            );

                    event->accept();

                    return;
                }
            }
        }
    }

    // --------------------------------------------------------
    // Bottom half = normal column dragging
    // --------------------------------------------------------

    QHeaderView::mousePressEvent(event);

}
// ============================================================
// GROUP DRAG - MOUSE MOVE
// ============================================================

void FaultTableHeader::mouseMoveEvent(
    QMouseEvent *event)
{
    if (!event)
        return;

    if (mGroupDragging)
    {
        event->accept();

        viewport()->update();

        return;
    }

    // --------------------------------------------------------
    // Normal column dragging
    // --------------------------------------------------------

    QHeaderView::mouseMoveEvent(event);

}

// ============================================================
// GROUP DRAG - MOUSE RELEASE
// ============================================================

void FaultTableHeader::mouseReleaseEvent(
    QMouseEvent *event)
{
    if (!event)
        return;

    if (!mGroupDragging)
    {
        QHeaderView::mouseReleaseEvent(event);

        return;
    }

    // --------------------------------------------------------
    // Find the group underneath the mouse
    // --------------------------------------------------------

    const int targetVisual =
        visualIndexAt(
            static_cast<int>(
                event->position().x()
                )
            );

    if (targetVisual >= 0)
    {
        const int targetLogical =
            logicalIndex(targetVisual);

        QString targetGroup;

        if (targetLogical >= 0 &&
            targetLogical < mGroupNames.size())
        {
            targetGroup =
                mGroupNames.at(targetLogical);
        }

        if (!targetGroup.isEmpty() &&
            targetGroup != mDraggedGroup)
        {
            moveGroup(
                mDraggedGroup,
                targetGroup
                );
        }
    }

    // --------------------------------------------------------
    // Reset drag state
    // --------------------------------------------------------

    mGroupDragging = false;

    mDraggedGroup.clear();

    mGroupDragStartX = -1;

    viewport()->update();

    event->accept();

}

// ============================================================
// MOVE COMPLETE FILE GROUP
// ============================================================

// ============================================================
// SWAP / REORDER COMPLETE FILE GROUPS
// ============================================================

void FaultTableHeader::moveGroup(
    const QString &sourceGroup,
    const QString &targetGroup)
{
    if (sourceGroup.isEmpty() ||
        targetGroup.isEmpty() ||
        sourceGroup == targetGroup)
    {
        return;
    }

    // --------------------------------------------------------
    // Build the current VISUAL order of logical sections.
    //
    // Example:
    //
    // Name
    // File1 col1
    // File1 col2
    // File1 col3
    // File2 col1
    // File2 col2
    // File2 col3
    // File3 col1
    // File3 col2
    //
    // currentOrder contains the logical indexes in that
    // visual order.
    // --------------------------------------------------------

    QList<int> currentOrder;

    for (int visualIndex = 0;
         visualIndex < count();
         ++visualIndex)
    {
        const int logicalSection =
            this->logicalIndex(visualIndex);

        if (logicalSection >= 0)
        {
            currentOrder.append(
                logicalSection
                );
        }
    }

    // --------------------------------------------------------
    // Build groups from the current visual order.
    //
    // This is important because individual columns may already
    // have been moved by the user.
    // --------------------------------------------------------

    QList<QString> groupOrder;
    QMap<QString, QList<int>> groupColumns;

    for (int logicalSection : currentOrder)
    {
        if (logicalSection < 0 ||
            logicalSection >= mGroupNames.size())
        {
            continue;
        }

        const QString group =
            mGroupNames.at(logicalSection);

        // Name column has an empty group.
        if (group.isEmpty())
        {
            continue;
        }

        if (!groupOrder.contains(group))
        {
            groupOrder.append(group);
        }

        groupColumns[group].append(
            logicalSection
            );
    }

    // --------------------------------------------------------
    // Make sure both groups exist.
    // --------------------------------------------------------

    if (!groupOrder.contains(sourceGroup) ||
        !groupOrder.contains(targetGroup))
    {
        return;
    }

    // --------------------------------------------------------
    // Swap the two complete groups.
    //
    // Example:
    //
    // File1
    // File2
    // File3
    //
    // Drag File3 onto File1
    //
    // becomes:
    //
    // File3
    // File2
    // File1
    // --------------------------------------------------------

    const int sourcePosition =
        groupOrder.indexOf(sourceGroup);

    const int targetPosition =
        groupOrder.indexOf(targetGroup);

    if (sourcePosition < 0 ||
        targetPosition < 0)
    {
        return;
    }

    groupOrder.swapItemsAt(
        sourcePosition,
        targetPosition
        );

    // --------------------------------------------------------
    // Build the desired logical section order.
    //
    // Name remains at the beginning.
    // --------------------------------------------------------

    QList<int> desiredOrder;

    // First add non-file columns such as Name.
    for (int logicalSection : currentOrder)
    {
        if (logicalSection < 0 ||
            logicalSection >= mGroupNames.size())
        {
            continue;
        }

        if (mGroupNames.at(logicalSection).isEmpty())
        {
            desiredOrder.append(
                logicalSection
                );
        }
    }

    // Then add the complete file groups in their new order.
    for (const QString &group : groupOrder)
    {
        const QList<int> columns =
            groupColumns.value(group);

        for (int logicalSection : columns)
        {
            desiredOrder.append(
                logicalSection
                );
        }
    }

    // --------------------------------------------------------
    // Apply the new visual order.
    //
    // IMPORTANT:
    // logical indexes do not change when moveSection() is used.
    // Therefore we can safely move the desired logical section
    // into each visual position.
    // --------------------------------------------------------

    for (int visualIndex = 0;
         visualIndex < desiredOrder.size();
         ++visualIndex)
    {
        const int desiredLogical =
            desiredOrder.at(visualIndex);

        const int currentLogical =
            this->logicalIndex(visualIndex);

        if (currentLogical == desiredLogical)
        {
            continue;
        }

        this->moveSection(
            desiredLogical,
            visualIndex
            );
    }

    viewport()->update();

}

// ============================================================
// SET SELECTED COLUMNS
// ============================================================

void FaultComparisonTableWidget::setSelectedColumns(
    const QStringList &columns)
{
    mSelectedColumns =
        columns;

    RebuildTable();
}

// ============================================================
// CHECK WHETHER A LOGICAL COLUMN BELONGS TO A FILE
// ============================================================

bool FaultComparisonTableWidget::columnAvailableForFile(
    int fileIndex,
    const QString &column
    ) const
{
    if (fileIndex < 0 ||
        fileIndex >= mHeadersPerFile.size())
    {
        return false;
    }

    // --------------------------------------------------------
    // Convert display column name to the original CSV column
    // --------------------------------------------------------

    QString rawColumn = column;

    // --------------------------------------------------------
    // Check file-specific logical mappings
    // --------------------------------------------------------

    if (fileIndex < mColumnMappingsPerFile.size())
    {
        const QMap<QString, QString> &mapping =
            mColumnMappingsPerFile.at(fileIndex);

        if (mapping.contains(column))
        {
            rawColumn =
                mapping.value(column);
        }
    }

    // --------------------------------------------------------
    // Check whether the original CSV contains the column
    // --------------------------------------------------------

    return mHeadersPerFile.at(fileIndex)
        .contains(rawColumn);
}

// ============================================================
// FIND VALUE BY FILE + NAME + COLUMN
// ============================================================
QString FaultComparisonTableWidget::valueFor(
    int fileIndex,
    const QString &name,
    const QString &column
    ) const
{
    if (fileIndex < 0 ||
        fileIndex >= mHeadersPerFile.size())
    {
        return "";
    }

    /*
 * The column received here is the logical/display
 * column selected by the user.
 *
 * Example:
 *
 * "Asymmetric RMS Current"
 *
 * must be converted to the correct RAW CSV
 * column for this particular file.
 */
    QString rawColumn = column;

    if (fileIndex < mColumnMappingsPerFile.size())
    {
        const QMap<QString, QString> &mapping =
            mColumnMappingsPerFile.at(fileIndex);

        if (mapping.contains(column))
        {
            rawColumn =
                mapping.value(column);
        }
    }

    const QStringList &headers =
        mHeadersPerFile.at(fileIndex);

    int columnIndex =
        headers.indexOf(rawColumn);

    if (columnIndex < 0)
    {
        /*
     * This logical parameter is not available
     * for this particular file.
     */
        return "";
    }

    const QList<QStringList> &rows =
        mRowsPerFile.at(fileIndex);

    for (const QStringList &row : rows)
    {
        if (row.isEmpty())
            continue;

        /*
     * Name is the first column in the CSV.
     */
        if (row.at(0) == name)
        {
            if (columnIndex < row.size())
            {
                return row.at(columnIndex);
            }

            return "";
        }
    }

    return "";

}

// ============================================================
// REBUILD TABLE
// ============================================================

void FaultComparisonTableWidget::RebuildTable()
{


    // Clear old table
    mTable->clear();

    mTable->setRowCount(0);

    mTable->setColumnCount(0);

    mGroupNames.clear();


    // ========================================================
    // Validate data
    // ========================================================

    if (mFileNames.isEmpty()){
        mTable->hide();
        mEmptyLabel->show();
        return;}

    mEmptyLabel->hide();
    mTable->show();


    if (mHeadersPerFile.isEmpty())
        return;


    if (mRowsPerFile.isEmpty())
        return;


    // ========================================================
    // Build unique Name list
    // ========================================================

    QStringList names;


    for (int fileIndex = 0;
         fileIndex < mFileNames.size();
         ++fileIndex)
    {
        if (fileIndex >= mHeadersPerFile.size())
            continue;


        if (fileIndex >= mRowsPerFile.size())
            continue;


        const int nameIndex =
            mHeadersPerFile.at(fileIndex)
                .indexOf("Name");


        if (nameIndex < 0)
            continue;


        for (const QStringList &row :
             mRowsPerFile.at(fileIndex))
        {
            if (nameIndex >= row.size())
                continue;


            const QString name =
                row.at(nameIndex).trimmed();


            if (name.isEmpty())
                continue;


            if (!names.contains(name))
            {
                names.append(name);
            }
        }
    }


    if (names.isEmpty())
        return;


    // ========================================================
    // Build table headers
    // ========================================================

    QStringList tableHeaders;


    // ========================================================
    // NAME COLUMN
    // ========================================================

    if (mSelectedColumns.contains("Name"))
    {
        tableHeaders.append("Name");

        // Empty group for Name
        mGroupNames.append(QString());
    }


    // ========================================================
    // DATA COLUMNS FOR EACH FILE
    //
    // File 1
    //   AC Mag.
    //   DC Mag.
    //   DC %
    //
    // File 2
    //   AC Mag.
    //   DC Mag.
    //   DC %
    //
    // File 3
    //   ...
    // ========================================================

    for (int fileIndex = 0;
         fileIndex < mFileNames.size();
         ++fileIndex)
    {
        const QString fileName =
            mFileNames.at(fileIndex);

        for (const QString &column :
             mSelectedColumns)
        {
            if (column == "Name")
                continue;

            // ----------------------------------------------------
            // IMPORTANT:
            // Only create the column if this parameter actually
            // belongs to this file.
            // ----------------------------------------------------

            if (!columnAvailableForFile(
                    fileIndex,
                    column))
            {
                continue;
            }

            tableHeaders.append(column);

            mGroupNames.append(fileName);
        }
    }


    // ========================================================
    // DIFFERENCE COLUMNS
    // ========================================================

    // ========================================================
    // DIFFERENCE COLUMNS
    // ========================================================

    if (mDifferenceEnabled && mFileNames.size() >= 2)
    {
        for (int fileIndex = 1;
             fileIndex < mFileNames.size();
             ++fileIndex)
        {
            QString differenceName;

            if (mFileNames.size() == 2)
            {
                differenceName =
                    "Difference";
            }
            else
            {
                differenceName =
                    QString(
                        "Difference (%1 - %2)"
                        )
                        .arg(
                            mFileNames.at(0)
                            )
                        .arg(
                            mFileNames.at(fileIndex)
                            );
            }

            for (const QString &column :
                 mSelectedColumns)
            {
                if (column == "Name")
                    continue;

                // ------------------------------------------------
                // A difference only makes sense when this
                // parameter exists in BOTH files.
                // ------------------------------------------------

                if (!columnAvailableForFile(
                        0,
                        column))
                {
                    continue;
                }

                if (!columnAvailableForFile(
                        fileIndex,
                        column))
                {
                    continue;
                }

                tableHeaders.append(column);

                mGroupNames.append(
                    differenceName
                    );
            }
        }
    }

    // ========================================================
    // Create table columns
    // ========================================================

    mTable->setColumnCount(
        tableHeaders.size()
        );


    mTable->setHorizontalHeaderLabels(
        tableHeaders
        );


    // ========================================================
    // Create rows
    // ========================================================

    mTable->setRowCount(
        names.size()
        );


    // ========================================================
    // Fill table
    // ========================================================

    for (int rowIndex = 0;
         rowIndex < names.size();
         ++rowIndex)
    {
        const QString name =
            names.at(rowIndex);


        int outputColumn = 0;


        // ====================================================
        // Name
        // ====================================================

        if (mSelectedColumns.contains("Name"))
        {
            QTableWidgetItem *item =
                new QTableWidgetItem(name);


            item->setTextAlignment(
                Qt::AlignLeft |
                Qt::AlignVCenter
                );


            mTable->setItem(
                rowIndex,
                outputColumn,
                item
                );


            ++outputColumn;
        }


        // ====================================================
        // Values from each CSV
        // ====================================================

        for (int fileIndex = 0;
             fileIndex < mFileNames.size();
             ++fileIndex)
        {
            for (const QString &column :
                 mSelectedColumns)
            {
                if (column == "Name")
                    continue;

                // ----------------------------------------------------
                // Do not create a cell for a parameter that does not
                // belong to this file.
                // ----------------------------------------------------

                if (!columnAvailableForFile(
                        fileIndex,
                        column))
                {
                    continue;
                }

                const QString value =
                    valueFor(
                        fileIndex,
                        name,
                        column
                        );

                QTableWidgetItem *item =
                    new QTableWidgetItem(value);

                item->setTextAlignment(
                    Qt::AlignCenter
                    );

                mTable->setItem(
                    rowIndex,
                    outputColumn,
                    item
                    );

                ++outputColumn;
            }
        }


        // ====================================================
        // Difference
        // ====================================================

        if (mDifferenceEnabled &&
            mFileNames.size() >= 2)
        {
            for (int fileIndex = 1;
                 fileIndex < mFileNames.size();
                 ++fileIndex)
            {
                for (const QString &column :
                     mSelectedColumns)
                {
                    if (column == "Name")
                        continue;


                    // ------------------------------------------------
                    // Difference only exists when the parameter is
                    // available in BOTH files.
                    // ------------------------------------------------

                    if (!columnAvailableForFile(
                            0,
                            column))
                    {
                        continue;
                    }


                    if (!columnAvailableForFile(
                            fileIndex,
                            column))
                    {
                        continue;
                    }


                    const QString value1 =
                        valueFor(
                            0,
                            name,
                            column
                            );


                    const QString value2 =
                        valueFor(
                            fileIndex,
                            name,
                            column
                            );


                    bool ok1 = false;
                    bool ok2 = false;


                    const double number1 =
                        value1.toDouble(&ok1);


                    const double number2 =
                        value2.toDouble(&ok2);


                    QString difference;


                    if (ok1 && ok2)
                    {
                        difference =
                            QString::number(
                                number1 - number2,
                                'g',
                                8
                                );
                    }


                    QTableWidgetItem *item =
                        new QTableWidgetItem(
                            difference
                            );


                    item->setTextAlignment(
                        Qt::AlignCenter
                        );


                    mTable->setItem(
                        rowIndex,
                        outputColumn,
                        item
                        );


                    ++outputColumn;
                }
            }
        }
    }


    // ========================================================
    // Column sizing
    // ========================================================

    mTable->resizeColumnsToContents();


    for (int column = 0;
         column < mTable->columnCount();
         ++column)
    {
        int columnWidth =
            mTable->columnWidth(column);


        if (columnWidth < 90)
            columnWidth = 90;


        if (columnWidth > 180)
            columnWidth = 180;


        mTable->setColumnWidth(
            column,
            columnWidth
            );
    }


    // ========================================================
    // Name column width
    // ========================================================

    if (mSelectedColumns.contains("Name"))
    {
        mTable->setColumnWidth(
            0,
            110
            );
    }


    // ========================================================
    // Give group names to custom header
    // ========================================================

    FaultTableHeader *header =
        static_cast<FaultTableHeader *>(
            mTable->horizontalHeader()
            );


    header->setGroupNames(
        mGroupNames
        );

    header->viewport()->update();

}

void FaultComparisonTableWidget::saveColumnOrder()
{
    if (!mTable)
        return;

    QHeaderView *header =
        mTable->horizontalHeader();

    if (!header)
        return;

    QStringList currentOrder;

    // Read the CURRENT visual order.
    for (int visualIndex = 0;
         visualIndex < mTable->columnCount();
         ++visualIndex)
    {
        const int logicalIndex =
            header->logicalIndex(visualIndex);

        if (logicalIndex < 0 ||
            logicalIndex >= mGroupNames.size())
        {
            continue;
        }

        QTableWidgetItem *item =
            mTable->horizontalHeaderItem(
                logicalIndex);

        if (!item)
            continue;

        const QString columnName =
            item->text();

        if (columnName.isEmpty())
            continue;

        // File name + column name together form
        // the unique identity of this table column.
        const QString groupName =
            mGroupNames.at(logicalIndex);

        QString key;

        if (groupName.isEmpty())
        {
            // Name column
            key = QString("GLOBAL|||%1")
                      .arg(columnName);
        }
        else
        {
            key = QString("%1|||%2")
            .arg(groupName)
                .arg(columnName);
        }

        if (!currentOrder.contains(key))
        {
            currentOrder.append(key);
        }
    }

    mSavedColumnOrder =
        currentOrder;
}
void FaultComparisonTableWidget::saveFileOrder()
{
    if (!mTable)
        return;

    QHeaderView *header =
        mTable->horizontalHeader();

    if (!header)
        return;

    QStringList currentOrder;

    // Read the current VISUAL order of groups.
    for (int visualIndex = 0;
         visualIndex < mTable->columnCount();
         ++visualIndex)
    {
        const int logicalIndex =
            header->logicalIndex(visualIndex);

        if (logicalIndex < 0 ||
            logicalIndex >= mGroupNames.size())
        {
            continue;
        }

        const QString group =
            mGroupNames.at(logicalIndex);

        // Ignore Name column.
        if (group.isEmpty())
            continue;

        // Only save actual file groups.
        if (!mFileNames.contains(group))
            continue;

        if (!currentOrder.contains(group))
        {
            currentOrder.append(group);
        }
    }

    if (!currentOrder.isEmpty())
    {
        mSavedFileOrder =
            currentOrder;
    }
}
void FaultComparisonTableWidget::restoreFileOrder()
{
    if (!mTable)
        return;

    QHeaderView *header =
        mTable->horizontalHeader();

    if (!header)
        return;

    if (mSavedFileOrder.isEmpty())
        return;

    // ========================================================
    // Get current file order after RebuildTable()
    // ========================================================

    QStringList currentFileOrder;

    for (int visualIndex = 0;
         visualIndex < mTable->columnCount();
         ++visualIndex)
    {
        const int logicalIndex =
            header->logicalIndex(visualIndex);

        if (logicalIndex < 0 ||
            logicalIndex >= mGroupNames.size())
        {
            continue;
        }

        const QString group =
            mGroupNames.at(logicalIndex);

        if (group.isEmpty())
            continue;

        if (!mFileNames.contains(group))
            continue;

        if (!currentFileOrder.contains(group))
        {
            currentFileOrder.append(group);
        }
    }

    if (currentFileOrder.isEmpty())
        return;


    // ========================================================
    // Build desired file order
    // ========================================================

    QStringList desiredFileOrder;

    // Previously existing files first
    for (const QString &fileName :
         mSavedFileOrder)
    {
        if (currentFileOrder.contains(fileName) &&
            !desiredFileOrder.contains(fileName))
        {
            desiredFileOrder.append(fileName);
        }
    }

    // Newly added files go to the end
    for (const QString &fileName :
         currentFileOrder)
    {
        if (!desiredFileOrder.contains(fileName))
        {
            desiredFileOrder.append(fileName);
        }
    }


    // ========================================================
    // Build desired logical column order
    //
    // IMPORTANT:
    // We take columns in their CURRENT visual order.
    //
    // Therefore this function changes only FILE order.
    // It does NOT reset column order.
    // ========================================================

    QList<int> desiredLogicalOrder;

    // Keep Name at the beginning
    for (int visualIndex = 0;
         visualIndex < mTable->columnCount();
         ++visualIndex)
    {
        const int logicalIndex =
            header->logicalIndex(visualIndex);

        if (logicalIndex < 0 ||
            logicalIndex >= mGroupNames.size())
        {
            continue;
        }

        if (mGroupNames.at(logicalIndex).isEmpty())
        {
            desiredLogicalOrder.append(
                logicalIndex
                );
        }
    }


    // Add files in saved order
    for (const QString &fileName :
         desiredFileOrder)
    {
        for (int visualIndex = 0;
             visualIndex < mTable->columnCount();
             ++visualIndex)
        {
            const int logicalIndex =
                header->logicalIndex(visualIndex);

            if (logicalIndex < 0 ||
                logicalIndex >= mGroupNames.size())
            {
                continue;
            }

            if (mGroupNames.at(logicalIndex) ==
                fileName)
            {
                desiredLogicalOrder.append(
                    logicalIndex
                    );
            }
        }
    }


    // ========================================================
    // Difference groups remain after actual files
    // ========================================================

    for (int visualIndex = 0;
         visualIndex < mTable->columnCount();
         ++visualIndex)
    {
        const int logicalIndex =
            header->logicalIndex(visualIndex);

        if (logicalIndex < 0 ||
            logicalIndex >= mGroupNames.size())
        {
            continue;
        }

        const QString group =
            mGroupNames.at(logicalIndex);

        if (group.isEmpty())
            continue;

        if (mFileNames.contains(group))
            continue;

        if (!desiredLogicalOrder.contains(
                logicalIndex))
        {
            desiredLogicalOrder.append(
                logicalIndex
                );
        }
    }


    // ========================================================
    // Apply desired visual order
    // ========================================================
    QSignalBlocker blocker(header);
    for (int targetVisualIndex = 0;
         targetVisualIndex < desiredLogicalOrder.size();
         ++targetVisualIndex)
    {
        const int desiredLogicalIndex =
            desiredLogicalOrder.at(
                targetVisualIndex
                );

        const int currentVisualIndex =
            header->visualIndex(
                desiredLogicalIndex
                );

        if (currentVisualIndex < 0)
            continue;

        if (currentVisualIndex !=
            targetVisualIndex)
        {
            header->moveSection(
                currentVisualIndex,
                targetVisualIndex
                );
        }
    }

    header->viewport()->update();
}
void FaultComparisonTableWidget::removeFileFromSavedOrder(
    const QString &fileName)
{
    if (fileName.isEmpty())
        return;

    // Remove the file from saved file order
    mSavedFileOrder.removeAll(fileName);


    // Remove all saved columns belonging
    // to this file.
    const QString prefix =
        fileName + "|||";

    QStringList remainingColumns;

    for (const QString &key :
         mSavedColumnOrder)
    {
        if (!key.startsWith(prefix))
        {
            remainingColumns.append(key);
        }
    }

    mSavedColumnOrder =
        remainingColumns;
}
void FaultComparisonTableWidget::restoreColumnOrder()
{
    if (!mTable)
        return;

    QHeaderView *header =
        mTable->horizontalHeader();

    if (!header)
        return;

    if (mSavedColumnOrder.isEmpty())
        return;

    /*
     * The table is organised as:
     *
     * File 1 -> columns
     * File 2 -> columns
     * File 3 -> columns
     *
     * Restore the columns INSIDE each file group.
     *
     * We do not move columns between file groups.
     */

    QStringList groups;

    // Find the current file-group order.
    for (int visualIndex = 0;
         visualIndex < mTable->columnCount();
         ++visualIndex)
    {
        const int logicalIndex =
            header->logicalIndex(visualIndex);

        if (logicalIndex < 0 ||
            logicalIndex >= mGroupNames.size())
        {
            continue;
        }

        const QString group =
            mGroupNames.at(logicalIndex);

        if (group.isEmpty())
            continue;

        if (!groups.contains(group))
        {
            groups.append(group);
        }
    }

    QSignalBlocker blocker(header);

    /*
     * Restore each file group independently.
     */
    for (const QString &group : groups)
    {
        QList<int> groupLogicalIndexes;

        // Find all columns belonging to this file.
        for (int logicalIndex = 0;
             logicalIndex < mGroupNames.size();
             ++logicalIndex)
        {
            if (mGroupNames.at(logicalIndex) ==
                group)
            {
                groupLogicalIndexes.append(
                    logicalIndex
                    );
            }
        }

        if (groupLogicalIndexes.isEmpty())
            continue;

        /*
         * Get the saved order for this particular file.
         */
        QStringList savedKeysForGroup;

        const QString prefix =
            group + "|||";

        for (const QString &key :
             mSavedColumnOrder)
        {
            if (key.startsWith(prefix))
            {
                savedKeysForGroup.append(key);
            }
        }

        if (savedKeysForGroup.isEmpty())
            continue;

        /*
         * Move each saved column to the next position
         * within the same file group.
         */
        int targetVisualIndex =
            header->visualIndex(
                groupLogicalIndexes.first()
                );

        if (targetVisualIndex < 0)
            continue;

        for (const QString &savedKey :
             savedKeysForGroup)
        {
            const QString columnName =
                savedKey.mid(prefix.length());

            int logicalIndex = -1;

            for (int candidate :
                 groupLogicalIndexes)
            {
                QTableWidgetItem *item =
                    mTable->horizontalHeaderItem(
                        candidate
                        );

                if (!item)
                    continue;

                if (item->text() ==
                    columnName)
                {
                    logicalIndex = candidate;
                    break;
                }
            }

            if (logicalIndex < 0)
                continue;

            const int currentVisualIndex =
                header->visualIndex(
                    logicalIndex
                    );

            if (currentVisualIndex < 0)
                continue;

            if (currentVisualIndex !=
                targetVisualIndex)
            {
                header->moveSection(
                    currentVisualIndex,
                    targetVisualIndex
                    );
            }

            ++targetVisualIndex;
        }
    }

    header->viewport()->update();
}


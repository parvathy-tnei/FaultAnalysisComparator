#include "FaultComparisonSelectionWidget.h"

#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QFont>

FaultComparisonSelectionWidget::FaultComparisonSelectionWidget(
    QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(190);

    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);

    mainLayout->setContentsMargins(
        8,
        8,
        8,
        8
        );

    mainLayout->setSpacing(6);


    // ========================================================
    // Title
    // ========================================================

    QLabel *titleLabel =
        new QLabel(
            "Comparison Selection",
            this
            );

    QFont titleFont =
        titleLabel->font();

    titleFont.setBold(true);

    titleLabel->setFont(
        titleFont
        );

    mainLayout->addWidget(
        titleLabel
        );


    // ========================================================
    // COLUMNS LABEL
    // ========================================================

    mColumnsLabel =
        new QLabel(
            "Columns",
            this
            );

    QFont columnsFont =
        mColumnsLabel->font();

    columnsFont.setBold(true);

    mColumnsLabel->setFont(
        columnsFont
        );

    mainLayout->addWidget(
        mColumnsLabel
        );


    // ========================================================
    // Scroll area
    // ========================================================

    mColumnScrollArea =
        new QScrollArea(this);

    mColumnScrollArea->setWidgetResizable(
        true
        );

    mColumnScrollArea->setFrameShape(
        QFrame::NoFrame
        );

    mColumnScrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
        );

    mColumnScrollArea->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded
        );


    // ========================================================
    // Container for column checkboxes
    // ========================================================

    mColumnContainer =
        new QWidget();


    mColumnLayout =
        new QVBoxLayout(
            mColumnContainer
            );

    mColumnLayout->setContentsMargins(
        4,
        4,
        4,
        4
        );

    mColumnLayout->setSpacing(4);


    mColumnScrollArea->setWidget(
        mColumnContainer
        );


    mainLayout->addWidget(
        mColumnScrollArea,
        1
        );

    // ========================================================
    // Difference checkbox
    // ========================================================

    mDifferenceCheckBox =
        new QCheckBox("Difference", this);

    mDifferenceCheckBox->setChecked(false);

    connect(
        mDifferenceCheckBox,
        &QCheckBox::toggled,
        this,
        &FaultComparisonSelectionWidget::selectionChanged
        );

    mainLayout->addWidget(
        mDifferenceCheckBox
        );


    // ========================================================
    // Buttons
    // ========================================================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();

    buttonLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    buttonLayout->setSpacing(6);


    mSelectAllButton =
        new QPushButton(
            "Select All",
            this
            );


    mClearAllButton =
        new QPushButton(
            "Clear All",
            this
            );


    buttonLayout->addWidget(
        mSelectAllButton
        );

    buttonLayout->addWidget(
        mClearAllButton
        );


    mainLayout->addLayout(
        buttonLayout
        );


    // ========================================================
    // Connections
    // ========================================================

    connect(
        mSelectAllButton,
        &QPushButton::clicked,
        this,
        &FaultComparisonSelectionWidget::selectAllColumns
        );


    connect(
        mClearAllButton,
        &QPushButton::clicked,
        this,
        &FaultComparisonSelectionWidget::clearAllColumns
        );
}


// ============================================================
// SET AVAILABLE COLUMNS
// ============================================================

void FaultComparisonSelectionWidget::setAvailableColumns(
    const QStringList &columns)
{
    createColumnCheckboxes(
        columns
        );
    // If there are no columns, put the widget
    // completely into the empty state.
    setEmptyState(columns.isEmpty());
}


// ============================================================
// CREATE COLUMN CHECKBOXES
// ============================================================

void FaultComparisonSelectionWidget::createColumnCheckboxes(
    const QStringList &columns)
{
    // Remove old checkboxes
    while (mColumnLayout->count() > 0)
    {
        QLayoutItem *item =
            mColumnLayout->takeAt(0);

        if (item->widget())
        {
            item->widget()->deleteLater();
        }

        delete item;
    }


    // Create checkbox for every CSV column
    for (const QString &column : columns)
    {
        if (column.trimmed().isEmpty())
            continue;


        QCheckBox *checkBox =
            new QCheckBox(
                column,
                mColumnContainer
                );


        // Name selected by default
        if (column == "Name")
        {
            checkBox->setChecked(true);
        }


        mColumnLayout->addWidget(
            checkBox
            );


        connect(
            checkBox,
            &QCheckBox::toggled,
            this,
            &FaultComparisonSelectionWidget::selectionChanged
            );
    }


    // Keep empty space below checkboxes
    mColumnLayout->addStretch();


    emit selectionChanged();
}


// ============================================================
// GET SELECTED COLUMNS
// ============================================================

QStringList FaultComparisonSelectionWidget::selectedColumns() const
{
    QStringList selected;


    for (int i = 0;
         i < mColumnLayout->count();
         ++i)
    {
        QLayoutItem *item =
            mColumnLayout->itemAt(i);


        QWidget *widget =
            item->widget();


        QCheckBox *checkBox =
            qobject_cast<QCheckBox *>(widget);


        if (!checkBox)
            continue;


        if (checkBox->isChecked())
        {
            selected.append(
                checkBox->text()
                );
        }
    }


    return selected;
}

// ============================================================
// GET DIFFERENCE STATE
// ============================================================

bool FaultComparisonSelectionWidget::differenceEnabled() const
{
    if (!mDifferenceCheckBox)
        return false;


    return mDifferenceCheckBox->isChecked();
}


// ============================================================
// SELECT ALL
// ============================================================

void FaultComparisonSelectionWidget::selectAllColumns()
{
    for (int i = 0;
         i < mColumnLayout->count();
         ++i)
    {
        QLayoutItem *item =
            mColumnLayout->itemAt(i);


        QWidget *widget =
            item->widget();


        QCheckBox *checkBox =
            qobject_cast<QCheckBox *>(widget);


        if (!checkBox)
            continue;


        checkBox->setChecked(true);
    }


    emit selectionChanged();
}


// ============================================================
// CLEAR ALL
// ============================================================

void FaultComparisonSelectionWidget::clearAllColumns()
{
    for (int i = 0;
         i < mColumnLayout->count();
         ++i)
    {
        QLayoutItem *item =
            mColumnLayout->itemAt(i);


        QWidget *widget =
            item->widget();


        QCheckBox *checkBox =
            qobject_cast<QCheckBox *>(widget);


        if (!checkBox)
            continue;


        checkBox->setChecked(false);
    }


    emit selectionChanged();
}

// ============================================================
// RESTORE SELECTED COLUMNS
// ============================================================

void FaultComparisonSelectionWidget::setSelectedColumns(
    const QStringList &columns)
{
    const QList<QCheckBox *> checkBoxes =
        mColumnContainer->findChildren<QCheckBox *>();

    for (QCheckBox *checkBox : checkBoxes)
    {
        if (!checkBox)
            continue;

        checkBox->setChecked(
            columns.contains(checkBox->text())
            );
    }
}

void FaultComparisonSelectionWidget::setEmptyState(
    bool empty)
{
    // ========================================================
    // EMPTY STATE
    // ========================================================

    if (empty)
    {
        // ----------------------------------------------------
        // Clear selected columns
        // ----------------------------------------------------

        // Uncheck all column checkboxes.
        const QList<QCheckBox *> checkBoxes =
            mColumnContainer->findChildren<QCheckBox *>();

        for (QCheckBox *checkBox :
             checkBoxes)
        {
            if (!checkBox)
                continue;

            checkBox->blockSignals(true);
            checkBox->setChecked(false);
            checkBox->setEnabled(false);
            checkBox->hide();
            checkBox->blockSignals(false);
        }


        // ----------------------------------------------------
        // Hide Difference
        // ----------------------------------------------------

        if (mDifferenceCheckBox)
        {
            mDifferenceCheckBox->blockSignals(true);

            mDifferenceCheckBox->setChecked(false);
            mDifferenceCheckBox->setEnabled(false);
            mDifferenceCheckBox->hide();

            mDifferenceCheckBox->blockSignals(false);
        }


        // ----------------------------------------------------
        // Hide Select All
        // ----------------------------------------------------

        if (mSelectAllButton)
        {
            mSelectAllButton->setEnabled(false);
            mSelectAllButton->hide();
        }


        // ----------------------------------------------------
        // Hide Clear All
        // ----------------------------------------------------

        if (mClearAllButton)
        {
            mClearAllButton->setEnabled(false);
            mClearAllButton->hide();
        }

        return;
    }


    // ========================================================
    // NORMAL STATE
    // ========================================================

    // --------------------------------------------------------
    // Show Difference
    // --------------------------------------------------------

    if (mDifferenceCheckBox)
    {
        mDifferenceCheckBox->show();
        mDifferenceCheckBox->setEnabled(true);
    }


    // --------------------------------------------------------
    // Show Select All
    // --------------------------------------------------------

    if (mSelectAllButton)
    {
        mSelectAllButton->show();

        mSelectAllButton->setEnabled(
            !mColumnContainer
                 ->findChildren<QCheckBox *>()
                 .isEmpty()
            );
    }


    // --------------------------------------------------------
    // Show Clear All
    // --------------------------------------------------------

    if (mClearAllButton)
    {
        mClearAllButton->show();

        mClearAllButton->setEnabled(
            !mColumnContainer
                 ->findChildren<QCheckBox *>()
                 .isEmpty()
            );
    }


    // --------------------------------------------------------
    // Show column checkboxes
    // --------------------------------------------------------

    const QList<QCheckBox *> checkBoxes =
        mColumnContainer->findChildren<QCheckBox *>();

    for (QCheckBox *checkBox :
         checkBoxes)
    {
        if (!checkBox)
            continue;

        checkBox->show();
        checkBox->setEnabled(true);
    }
}

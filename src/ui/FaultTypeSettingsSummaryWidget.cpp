#include "FaultTypeSettingsSummaryWidget.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QFileInfo>
#include <QHeaderView>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QLineEdit>

FaultTypeSettingsSummaryWidget::FaultTypeSettingsSummaryWidget(
    QWidget *parent)
    : QWidget(parent),
    mTable(new QTableWidget(this))
{
    QVBoxLayout *layout =
        new QVBoxLayout(this);

    layout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    layout->setSpacing(0);


    // -------------------------------------------------
    // Start with an empty table.
    // Property rows will be created only
    // when files are loaded.
    // -------------------------------------------------

    mTable->setRowCount(0);
    mTable->setColumnCount(0);


    mTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    mTable->setSelectionMode(
        QAbstractItemView::NoSelection
        );


    // -------------------------------------------------
    // Horizontal scrolling
    // -------------------------------------------------

    mTable->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAsNeeded
        );

    mTable->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded
        );


    mTable->horizontalHeader()->setMinimumSectionSize(
        180
        );


    layout->addWidget(
        mTable
        );

    setLayout(layout);
}

FaultTypeSettings
FaultTypeSettingsSummaryWidget::getSettingsForFile(
    const QString &filePath) const
{
    Q_UNUSED(filePath);

    FaultTypeSettings settings;

    settings.calculateType = "None";
    settings.faultType = "None";
    settings.resultType = "None";

    settings.faultResistance = "-";
    settings.faultReactance = "-";
    settings.faultTime = "-";

    settings.configured = false;

    return settings;
}


void FaultTypeSettingsSummaryWidget::setFiles(
    const QStringList &filePaths,
    const QMap<QString, FaultTypeSettings> &settings)
{
    mTable->clear();

    QStringList properties =
        {
            "Calculate Type",
            "Fault Type",
            "Result Type",
            "Rf (pu)",
            "Xf (pu)",
            "Fault Time (s)"
        };

    mTable->setRowCount(properties.size());
    mTable->setColumnCount(filePaths.size() + 1);

    // First column
    mTable->setHorizontalHeaderItem(
        0,
        new QTableWidgetItem("Property")
        );

    // Property names
    for (int row = 0; row < properties.size(); ++row)
    {
        mTable->setItem(
            row,
            0,
            new QTableWidgetItem(properties[row])
            );
    }


    for (int column = 0;
         column < filePaths.size();
         ++column)
    {
        const QString &filePath = filePaths[column];

        QFileInfo fileInfo(filePath);

        QString fileName = fileInfo.fileName();

        // -------------------------------------------------
        // File name header
        // -------------------------------------------------

        mTable->setHorizontalHeaderItem(
            column + 1,
            new QTableWidgetItem(fileName)
            );


        // -------------------------------------------------
        // Get existing settings
        // -------------------------------------------------

        FaultTypeSettings fileSettings;

        if (settings.contains(filePath))
        {
            fileSettings = settings.value(filePath);
        }

        QString calculateType =
            fileSettings.calculateType.isEmpty()
                ? "None"
                : fileSettings.calculateType;

        QString faultType =
            fileSettings.faultType.isEmpty()
                ? "None"
                : fileSettings.faultType;

        QString resultType =
            fileSettings.resultType.isEmpty()
                ? "None"
                : fileSettings.resultType;


        // -------------------------------------------------
        // Calculate Type dropdown
        // -------------------------------------------------

        QComboBox *calculateCombo = new QComboBox();

        calculateCombo->addItem("None");

        calculateCombo->addItem(
            "Fault levels on all busbars"
            );

        calculateCombo->addItem(
            "Fault levels on selected busbars"
            );

        calculateCombo->addItem(
            "Fault on one bus with flows"
            );

        calculateCombo->addItem(
            "Fault along a line"
            );

        int calculateIndex =
            calculateCombo->findText(calculateType);

        if (calculateIndex >= 0)
        {
            calculateCombo->setCurrentIndex(calculateIndex);
        }
        else
        {
            calculateCombo->setCurrentIndex(0);
        }

        mTable->setCellWidget(
            0,
            column + 1,
            calculateCombo
            );


        // -------------------------------------------------
        // Fault Type dropdown
        // -------------------------------------------------

        QComboBox *faultCombo = new QComboBox();

        faultCombo->addItem("None");

        faultCombo->addItem(
            "Line-line-line (three phase)"
            );

        faultCombo->addItem(
            "Line-ground (single phase)"
            );

        faultCombo->addItem(
            "Line-line"
            );

        faultCombo->addItem(
            "Line-line-ground"
            );

        int faultIndex =
            faultCombo->findText(faultType);

        if (faultIndex >= 0)
        {
            faultCombo->setCurrentIndex(faultIndex);
        }
        else
        {
            faultCombo->setCurrentIndex(0);
        }

        mTable->setCellWidget(
            1,
            column + 1,
            faultCombo
            );


        // -------------------------------------------------
        // Result Type dropdown
        // -------------------------------------------------

        QComboBox *resultCombo = new QComboBox();

        resultCombo->addItem("None");

        resultCombo->addItem("Peak");

        resultCombo->addItem(
            "Asymmetric RMS"
            );

        resultCombo->addItem(
            "Symmetric RMS"
            );

        resultCombo->addItem(
            "Plot of waveform"
            );

        int resultIndex =
            resultCombo->findText(resultType);

        if (resultIndex >= 0)
        {
            resultCombo->setCurrentIndex(resultIndex);
        }
        else
        {
            resultCombo->setCurrentIndex(0);
        }

        mTable->setCellWidget(
            2,
            column + 1,
            resultCombo
            );


        // -------------------------------------------------
        // Rf
        // -------------------------------------------------

        QLineEdit *resistanceEdit =
            new QLineEdit();

        resistanceEdit->setText(
            fileSettings.faultResistance
            );

        resistanceEdit->setPlaceholderText(
            "e.g. 0.00"
            );

        mTable->setCellWidget(
            3,
            column + 1,
            resistanceEdit
            );

        // -------------------------------------------------
        // Xf
        // -------------------------------------------------

        QLineEdit *reactanceEdit =
            new QLineEdit();

        reactanceEdit->setText(
            fileSettings.faultReactance
            );

        reactanceEdit->setPlaceholderText(
            "e.g. 0.00"
            );

        mTable->setCellWidget(
            4,
            column + 1,
            reactanceEdit
            );
        // -------------------------------------------------
        // Fault Time
        // -------------------------------------------------

        QLineEdit *faultTimeEdit =
            new QLineEdit();

        faultTimeEdit->setText(
            fileSettings.faultTime
            );

        faultTimeEdit->setPlaceholderText(
            "e.g. 0.06"
            );

        mTable->setCellWidget(
            5,
            column + 1,
            faultTimeEdit
            );


        // -------------------------------------------------
        // Connect all settings
        // -------------------------------------------------

        auto emitUpdatedSettings =
            [this,
             filePath,
             calculateCombo,
             faultCombo,
             resultCombo,
             resistanceEdit,
             reactanceEdit,
             faultTimeEdit]()
        {
            FaultTypeSettings updated;

            updated.calculateType =
                calculateCombo->currentText();

            updated.faultType =
                faultCombo->currentText();

            updated.resultType =
                resultCombo->currentText();

            updated.faultResistance =
                resistanceEdit->text();

            updated.faultReactance =
                reactanceEdit->text();

            updated.faultTime =
                faultTimeEdit->text();

            updated.configured =
                updated.calculateType != "None" ||
                updated.faultType != "None" ||
                updated.resultType != "None";

            emit settingsChanged(
                filePath,
                updated
                );
        };


        // Dropdown changes
        connect(
            calculateCombo,
            &QComboBox::currentTextChanged,
            this,
            emitUpdatedSettings
            );

        connect(
            faultCombo,
            &QComboBox::currentTextChanged,
            this,
            emitUpdatedSettings
            );

        connect(
            resultCombo,
            &QComboBox::currentTextChanged,
            this,
            emitUpdatedSettings
            );


        // Text field changes
        connect(
            resistanceEdit,
            &QLineEdit::editingFinished,
            this,
            emitUpdatedSettings
            );

        connect(
            reactanceEdit,
            &QLineEdit::editingFinished,
            this,
            emitUpdatedSettings
            );

        connect(
            faultTimeEdit,
            &QLineEdit::editingFinished,
            this,
            emitUpdatedSettings
            );
    }


    // -------------------------------------------------
    // Resize columns
    // -------------------------------------------------

    mTable->horizontalHeader()->setSectionResizeMode(
        0,
        QHeaderView::ResizeToContents
        );

    const int fileColumnCount =
        mTable->columnCount() - 1;

    const int maxVisibleFiles = 4;

    if (fileColumnCount <= maxVisibleFiles)
    {
        // Few files:
        // fill the available window width.
        for (int column = 1;
             column < mTable->columnCount();
             ++column)
        {
            mTable->horizontalHeader()->setSectionResizeMode(
                column,
                QHeaderView::Stretch
                );
        }
    }
    else
    {
        // Many files:
        // keep a useful minimum width and allow
        // horizontal scrolling.
        for (int column = 1;
             column < mTable->columnCount();
             ++column)
        {
            mTable->horizontalHeader()->setSectionResizeMode(
                column,
                QHeaderView::Fixed
                );

            mTable->setColumnWidth(
                column,
                220
                );
        }
    }

    mTable->resizeRowsToContents();
}


void FaultTypeSettingsSummaryWidget::clear()
{
    mTable->clear();

    mTable->setRowCount(0);
    mTable->setColumnCount(0);
}
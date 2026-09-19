#include "FaultTypeSettingsWidget.h"

#include <QComboBox>
#include <QFileInfo>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


FaultTypeSettingsWidget::FaultTypeSettingsWidget(
    const QString &filePath,
    QWidget *parent
    )
    : QWidget(parent),
    mFilePath(filePath)
{
    // =========================================================
    // MAIN LAYOUT
    // =========================================================

    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);

    mainLayout->setContentsMargins(
        8,
        6,
        8,
        6
        );

    mainLayout->setSpacing(4);


    // =========================================================
    // FILE NAME
    // =========================================================

    QFileInfo fileInfo(mFilePath);

    mFileLabel =
        new QLabel(
            QString("File: %1")
                .arg(fileInfo.fileName()),
            this
            );

    QFont fileFont =
        mFileLabel->font();

    fileFont.setBold(true);

    mFileLabel->setFont(
        fileFont
        );

    mainLayout->addWidget(
        mFileLabel
        );


    // =========================================================
    // GRID LAYOUT
    //
    // Column layout:
    //
    // 0 = label
    // 1 = control
    // 2 = label
    // 3 = control
    // 4 = label
    // 5 = control
    //
    // =========================================================

    QGridLayout *gridLayout =
        new QGridLayout();

    gridLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    gridLayout->setHorizontalSpacing(
        6
        );

    gridLayout->setVerticalSpacing(
        4
        );


    // =========================================================
    // CALCULATE TYPE
    // =========================================================

    QLabel *calculateLabel =
        new QLabel(
            "Calculate:",
            this
            );

    mCalculateTypeCombo =
        new QComboBox(this);

    mCalculateTypeCombo->addItem(
        "Fault levels on all busbars"
        );

    mCalculateTypeCombo->addItem(
        "Fault levels on selected busbars"
        );

    mCalculateTypeCombo->addItem(
        "Fault on one bus with flows"
        );

    mCalculateTypeCombo->addItem(
        "Fault along a line"
        );


    // =========================================================
    // FAULT TYPE
    // =========================================================

    QLabel *faultTypeLabel =
        new QLabel(
            "Fault type:",
            this
            );

    mFaultTypeCombo =
        new QComboBox(this);

    mFaultTypeCombo->addItem(
        "Line-line-line (three phase)"
        );

    mFaultTypeCombo->addItem(
        "Line-ground (single phase)"
        );

    mFaultTypeCombo->addItem(
        "Line-line"
        );

    mFaultTypeCombo->addItem(
        "Line-line-ground"
        );


    // =========================================================
    // RESULT TYPE
    // =========================================================

    QLabel *resultTypeLabel =
        new QLabel(
            "Result type:",
            this
            );

    mResultTypeCombo =
        new QComboBox(this);

    mResultTypeCombo->addItem(
        "Peak"
        );

    mResultTypeCombo->addItem(
        "Asymmetric RMS"
        );

    mResultTypeCombo->addItem(
        "Symmetric RMS"
        );

    mResultTypeCombo->addItem(
        "Plot of waveform"
        );


    // =========================================================
    // ROW 0
    //
    // Calculate | Fault type | Result type
    //
    // =========================================================

    gridLayout->addWidget(
        calculateLabel,
        0,
        0
        );

    gridLayout->addWidget(
        mCalculateTypeCombo,
        0,
        1
        );

    gridLayout->addWidget(
        faultTypeLabel,
        0,
        2
        );

    gridLayout->addWidget(
        mFaultTypeCombo,
        0,
        3
        );

    gridLayout->addWidget(
        resultTypeLabel,
        0,
        4
        );

    gridLayout->addWidget(
        mResultTypeCombo,
        0,
        5
        );


    // =========================================================
    // FAULT TIME
    // =========================================================

    QLabel *faultTimeLabel =
        new QLabel(
            "Fault time (s):",
            this
            );

    mFaultTimeEdit =
        new QLineEdit(this);

    mFaultTimeEdit->setPlaceholderText(
        "e.g. 0.06"
        );


    // =========================================================
    // RESISTANCE
    // =========================================================

    QLabel *resistanceLabel =
        new QLabel(
            "Resistance (pu):",
            this
            );

    mFaultResistanceEdit =
        new QLineEdit(this);

    mFaultResistanceEdit->setPlaceholderText(
        "e.g. 0.00"
        );


    // =========================================================
    // REACTANCE
    // =========================================================

    QLabel *reactanceLabel =
        new QLabel(
            "Reactance (pu):",
            this
            );

    mFaultReactanceEdit =
        new QLineEdit(this);

    mFaultReactanceEdit->setPlaceholderText(
        "e.g. 0.00"
        );


    // =========================================================
    // ROW 1
    //
    // Fault time | Resistance | Reactance
    //
    // =========================================================

    gridLayout->addWidget(
        faultTimeLabel,
        1,
        0
        );

    gridLayout->addWidget(
        mFaultTimeEdit,
        1,
        1
        );

    gridLayout->addWidget(
        resistanceLabel,
        1,
        2
        );

    gridLayout->addWidget(
        mFaultResistanceEdit,
        1,
        3
        );

    gridLayout->addWidget(
        reactanceLabel,
        1,
        4
        );

    gridLayout->addWidget(
        mFaultReactanceEdit,
        1,
        5
        );


    // =========================================================
    // COLUMN STRETCH
    // =========================================================

    gridLayout->setColumnStretch(
        1,
        1
        );

    gridLayout->setColumnStretch(
        3,
        1
        );

    gridLayout->setColumnStretch(
        5,
        1
        );


    // =========================================================
    // ADD GRID
    // =========================================================

    mainLayout->addLayout(
        gridLayout
        );


    // =========================================================
    // BUTTONS
    // =========================================================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();

    buttonLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    buttonLayout->setSpacing(
        5
        );

    buttonLayout->addStretch();


    mClearButton =
        new QPushButton(
            "Clear",
            this
            );

    mApplyButton =
        new QPushButton(
            "Apply",
            this
            );


    mClearButton->setFixedWidth(
        70
        );

    mApplyButton->setFixedWidth(
        70
        );


    buttonLayout->addWidget(
        mClearButton
        );

    buttonLayout->addWidget(
        mApplyButton
        );


    mainLayout->addLayout(
        buttonLayout
        );


    // =========================================================
    // CONNECTIONS
    // =========================================================

    connect(
        mApplyButton,
        &QPushButton::clicked,
        this,
        &FaultTypeSettingsWidget::applyConfiguration
        );


    connect(
        mClearButton,
        &QPushButton::clicked,
        this,
        &FaultTypeSettingsWidget::clearConfiguration
        );
}


// =============================================================
// SET CONFIGURATION
// =============================================================

void FaultTypeSettingsWidget::setConfiguration(
    const FaultTypeSettings &settings
    )
{
    // ---------------------------------------------------------
    // Calculate type
    // ---------------------------------------------------------

    int calculateIndex =
        mCalculateTypeCombo->findText(
            settings.calculateType
            );

    if (calculateIndex >= 0)
    {
        mCalculateTypeCombo->setCurrentIndex(
            calculateIndex
            );
    }


    // ---------------------------------------------------------
    // Fault type
    // ---------------------------------------------------------

    int faultTypeIndex =
        mFaultTypeCombo->findText(
            settings.faultType
            );

    if (faultTypeIndex >= 0)
    {
        mFaultTypeCombo->setCurrentIndex(
            faultTypeIndex
            );
    }


    // ---------------------------------------------------------
    // Result type
    // ---------------------------------------------------------

    int resultTypeIndex =
        mResultTypeCombo->findText(
            settings.resultType
            );

    if (resultTypeIndex >= 0)
    {
        mResultTypeCombo->setCurrentIndex(
            resultTypeIndex
            );
    }


    // ---------------------------------------------------------
    // Parameters
    // ---------------------------------------------------------

    mFaultTimeEdit->setText(
        settings.faultTime
        );

    mFaultResistanceEdit->setText(
        settings.faultResistance
        );

    mFaultReactanceEdit->setText(
        settings.faultReactance
        );
}


// =============================================================
// GET CONFIGURATION
// =============================================================

FaultTypeSettings
FaultTypeSettingsWidget::configuration() const
{
    FaultTypeSettings settings;


    settings.calculateType =
        mCalculateTypeCombo->currentText();


    settings.faultType =
        mFaultTypeCombo->currentText();


    settings.resultType =
        mResultTypeCombo->currentText();


    settings.faultTime =
        mFaultTimeEdit->text();


    settings.faultResistance =
        mFaultResistanceEdit->text();


    settings.faultReactance =
        mFaultReactanceEdit->text();


    settings.configured =
        true;


    return settings;
}


// =============================================================
// APPLY CONFIGURATION
// =============================================================

void FaultTypeSettingsWidget::applyConfiguration()
{
    FaultTypeSettings settings =
        configuration();


    emit configurationApplied(
        mFilePath,
        settings
        );
}


// =============================================================
// CLEAR CONFIGURATION
// =============================================================

void FaultTypeSettingsWidget::clearConfiguration()
{
    FaultTypeSettings settings;

    settings.configured =
        false;


    // Reset dropdowns

    mCalculateTypeCombo->setCurrentIndex(
        0
        );

    mFaultTypeCombo->setCurrentIndex(
        0
        );

    mResultTypeCombo->setCurrentIndex(
        0
        );


    // Clear parameters

    mFaultTimeEdit->clear();

    mFaultResistanceEdit->clear();

    mFaultReactanceEdit->clear();


    emit configurationCleared(
        mFilePath
        );
}
#include "FaultFileCard.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStyle>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QFontMetrics>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>

namespace
{
QIcon createPenIcon(const QColor &color, int size = 16)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Precise vector pen/pencil icon
    QPainterPath body;
    body.moveTo(11.8, 3.2);
    body.lineTo(12.8, 4.2);
    body.lineTo(6.2, 10.8);
    body.lineTo(3.8, 12.2);
    body.lineTo(5.2, 9.8);
    body.closeSubpath();

    p.setPen(QPen(color, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawPath(body);

    // Pen tip nib separator line
    p.drawLine(QPointF(5.2, 9.8), QPointF(6.2, 10.8));
    p.end();

    return QIcon(pix);
}

void configureComboPopup(QComboBox *combo)
{
    if (!combo)
        return;

    combo->view()->setTextElideMode(Qt::ElideNone);
    combo->view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QFontMetrics metrics(combo->font());
    int maxTextWidth = 0;

    for (int i = 0; i < combo->count(); ++i)
    {
        maxTextWidth = qMax(
            maxTextWidth,
            metrics.horizontalAdvance(combo->itemText(i))
            );
    }

    const int popupWidth = maxTextWidth + 40;
    combo->view()->setMinimumWidth(qMax(popupWidth, combo->width()));
    combo->setMaxVisibleItems(12);
}
}

FaultFileCard::FaultFileCard(
    const QString &filePath,
    const FaultTypeSettings &settings,
    int rowCount,
    int columnCount,
    qint64 fileSizeBytes,
    QWidget *parent
    )
    : QWidget(parent)
    , mFilePath(filePath)
    , mOriginalFileName(QFileInfo(filePath).fileName())
    , mAlias(QString())
    , mShortId(QString())
    , mRowCount(rowCount)
    , mColumnCount(columnCount)
    , mFileSizeBytes(fileSizeBytes)
    , mUpdating(false)
    , mCardFrame(nullptr)
    , mSelectionCheckBox(nullptr)
    , mFileIconLabel(nullptr)
    , mTitleLabel(nullptr)
    , mAliasBadgeLabel(nullptr)
    , mOriginalFileNameLabel(nullptr)
    , mFileInfoLabel(nullptr)
    , mRenameButton(nullptr)
    , mRemoveButton(nullptr)
    , mToggleSettingsButton(nullptr)
    , mSettingsScrollArea(nullptr)
    , mCalculateTypeCombo(nullptr)
    , mFaultTypeCombo(nullptr)
    , mResultTypeCombo(nullptr)
    , mRfSpinBox(nullptr)
    , mXfSpinBox(nullptr)
    , mFaultTimeSpinBox(nullptr)
{
    createUi();

    mSelectionCheckBox->setChecked(true);

    if (!settings.calculateType.isEmpty())
    {
        int index = mCalculateTypeCombo->findText(settings.calculateType);
        if (index >= 0)
            mCalculateTypeCombo->setCurrentIndex(index);
    }

    if (!settings.faultType.isEmpty())
    {
        int index = mFaultTypeCombo->findText(settings.faultType);
        if (index >= 0)
            mFaultTypeCombo->setCurrentIndex(index);
    }

    if (!settings.resultType.isEmpty())
    {
        int index = mResultTypeCombo->findText(settings.resultType);
        if (index >= 0)
            mResultTypeCombo->setCurrentIndex(index);
    }

    if (!settings.faultResistance.isEmpty())
    {
        mRfSpinBox->setValue(settings.faultResistance.toDouble());
    }

    if (!settings.faultReactance.isEmpty())
    {
        mXfSpinBox->setValue(settings.faultReactance.toDouble());
    }

    if (!settings.faultTime.isEmpty())
    {
        mFaultTimeSpinBox->setValue(settings.faultTime.toDouble());
    }

    // Default to collapsed unless the user explicitly clicks the Settings button
    mToggleSettingsButton->setChecked(false);
}


void FaultFileCard::createUi()
{
    setMinimumWidth(0);
    setMaximumWidth(QWIDGETSIZE_MAX);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    setStyleSheet(
        "QFrame#faultFileCard {"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "}"
        "QLabel#cardTitleLabel {"
        "    font-weight: 700;"
        "    font-size: 13px;"
        "    color: #0F172A;"
        "}"
        "QLabel#aliasBadge {"
        "    background-color: #EFF6FF;"
        "    color: #2563EB;"
        "    font-size: 9.5px;"
        "    font-weight: 600;"
        "    border: 1px solid #BFDBFE;"
        "    border-radius: 4px;"
        "    padding: 1px 4px;"
        "}"
        "QLabel#originalFileLabel {"
        "    font-style: normal;"
        "    color: #64748B;"
        "    font-size: 11px;"
        "}"
        "QLabel#fileInfoLabel {"
        "    color: #94A3B8;"
        "    font-size: 10.5px;"
        "}"
        "QPushButton#renameButton {"
        "    border: none;"
        "    background: transparent;"
        "    padding: 2px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton#renameButton:hover {"
        "    background: #EAF4FC;"
        "}"
        "QPushButton#removeButton {"
        "    border: none;"
        "    background: transparent;"
        "    color: #777777;"
        "    font-size: 16px;"
        "    padding: 0px;"
        "}"
        "QPushButton#removeButton:hover {"
        "    color: #C62828;"
        "}"
        "QToolButton#toggleSettingsButton {"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 4px;"
        "    background: #F8FAFC;"
        "    color: #475569;"
        "    font-size: 11px;"
        "    font-weight: 600;"
        "    padding: 2px 8px 4px 8px;"
        "    min-height: 18px;"
        "}"
        "QToolButton#toggleSettingsButton:hover {"
        "    background: #EDF2F7;"
        "    color: #1769AA;"
        "    border: 1px solid #CBD5E1;"
        "}"
        "QComboBox {"
        "    min-height: 24px;"
        "    border: 1px solid #C9D2DB;"
        "    border-radius: 4px;"
        "    padding: 2px 6px;"
        "    background: white;"
        "    color: #1F2933;"
        "}"
        "QDoubleSpinBox {"
        "    min-height: 24px;"
        "    border: 1px solid #C9D2DB;"
        "    border-radius: 4px;"
        "    padding: 2px 8px;"
        "    background: white;"
        "    color: #1F2933;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        "    width: 0px;"
        "    border: none;"
        "}"
        "QComboBox:hover, QDoubleSpinBox:hover {"
        "    border: 1px solid #AAB7C4;"
        "}"
        "QComboBox:focus, QDoubleSpinBox:focus {"
        "    border: 1px solid #1769AA;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background: white;"
        "    color: #1F2933;"
        "    border: 1px solid #C9D2DB;"
        "    padding: 4px;"
        "    selection-background-color: #EAF4FC;"
        "    selection-color: #1769AA;"
        "}"
        );

    setObjectName("faultFileCardRoot");

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    mCardFrame = new QFrame(this);
    mCardFrame->setObjectName("faultFileCard");
    outerLayout->addWidget(mCardFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout(mCardFrame);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(3);

    // 1. TOP HEADER (Checkbox + Icon + Title + Badge + Pen Button + Remove Button)
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(6);
    headerLayout->setAlignment(Qt::AlignVCenter);

    mSelectionCheckBox = new QCheckBox(this);
    mSelectionCheckBox->setToolTip("Include this file in the comparison");

    mFileIconLabel = new QLabel(this);
    mFileIconLabel->setFixedSize(16, 16);
    mFileIconLabel->setAlignment(Qt::AlignCenter);
    mFileIconLabel->setScaledContents(true);
    mFileIconLabel->setPixmap(
        style()->standardIcon(QStyle::SP_FileIcon).pixmap(16, 16)
        );

    mTitleLabel = new QLabel(this);
    mTitleLabel->setObjectName("cardTitleLabel");
    mTitleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    mAliasBadgeLabel = new QLabel("alias", this);
    mAliasBadgeLabel->setObjectName("aliasBadge");
    mAliasBadgeLabel->setVisible(false);

    // Pen Icon Button
    mRenameButton = new QPushButton(this);
    mRenameButton->setObjectName("renameButton");
    mRenameButton->setIcon(createPenIcon(QColor("#1769AA"), 16));
    mRenameButton->setIconSize(QSize(16, 16));
    mRenameButton->setFixedSize(22, 22);
    mRenameButton->setCursor(Qt::PointingHandCursor);
    mRenameButton->setToolTip("Edit label / alias");

    mRemoveButton = new QPushButton("×", this);
    mRemoveButton->setObjectName("removeButton");
    mRemoveButton->setFixedSize(22, 22);
    mRemoveButton->setToolTip("Remove this file");

    headerLayout->addWidget(mSelectionCheckBox);
    headerLayout->addWidget(mFileIconLabel);
    headerLayout->addWidget(mTitleLabel);
    headerLayout->addWidget(mAliasBadgeLabel);
    headerLayout->addStretch(1);
    headerLayout->addWidget(mRenameButton);
    headerLayout->addWidget(mRemoveButton);

    mainLayout->addLayout(headerLayout);

    // 2. ORIGINAL FILE NAME
    QHBoxLayout *origNameLayout = new QHBoxLayout();
    origNameLayout->setContentsMargins(28, 0, 0, 0);
    origNameLayout->setSpacing(0);

    mOriginalFileNameLabel = new QLabel(this);
    mOriginalFileNameLabel->setObjectName("originalFileLabel");
    origNameLayout->addWidget(mOriginalFileNameLabel);
    origNameLayout->addStretch(1);

    mainLayout->addLayout(origNameLayout);

    // 3. META INFO & SETTINGS TOGGLE BUTTON
    QHBoxLayout *metaLayout = new QHBoxLayout();
    metaLayout->setContentsMargins(28, 0, 0, 0);
    metaLayout->setSpacing(8);

    mFileInfoLabel = new QLabel(
        QString("%1 rows · %2 parameters · %3")
            .arg(mRowCount)
            .arg(mColumnCount)
            .arg(formatFileSize(mFileSizeBytes)),
        this
        );
    mFileInfoLabel->setObjectName("fileInfoLabel");

    mToggleSettingsButton = new QToolButton(this);
    mToggleSettingsButton->setObjectName("toggleSettingsButton");
    mToggleSettingsButton->setCheckable(true);
    mToggleSettingsButton->setChecked(false);
    mToggleSettingsButton->setText("Settings ▾");
    mToggleSettingsButton->setCursor(Qt::PointingHandCursor);

    metaLayout->addWidget(mFileInfoLabel);
    metaLayout->addStretch(1);
    metaLayout->addWidget(mToggleSettingsButton);

    mainLayout->addLayout(metaLayout);

    // 4. COLLAPSIBLE SETTINGS SECTION
    mSettingsScrollArea = new QScrollArea(this);
    mSettingsScrollArea->setWidgetResizable(true);
    mSettingsScrollArea->setFrameShape(QFrame::NoFrame);
    mSettingsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mSettingsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mSettingsScrollArea->setFixedHeight(106);

    QWidget *settingsWidget = new QWidget();
    settingsWidget->setMinimumWidth(320);

    QGridLayout *settingsLayout = new QGridLayout(settingsWidget);
    settingsLayout->setContentsMargins(0, 4, 0, 2);
    settingsLayout->setHorizontalSpacing(8);
    settingsLayout->setVerticalSpacing(5);

    // Calculate Type
    QLabel *calculateLabel = new QLabel("Calculate Type", settingsWidget);
    mCalculateTypeCombo = new QComboBox(settingsWidget);
    mCalculateTypeCombo->addItems({
        "None",
        "Fault Levels on all busbars",
        "Fault levels on selected busbars",
        "Fault on one bus with flows",
        "Fault along a line"
    });

    // Rf (pu)
    QLabel *rfLabel = new QLabel("Rf (pu)", settingsWidget);
    mRfSpinBox = new QDoubleSpinBox(settingsWidget);
    mRfSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    mRfSpinBox->setRange(-999999.0, 999999.0);
    mRfSpinBox->setDecimals(4);
    mRfSpinBox->setSingleStep(0.01);
    mRfSpinBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mRfSpinBox->setMinimumWidth(75);

    // Fault Type
    QLabel *faultLabel = new QLabel("Fault Type", settingsWidget);
    mFaultTypeCombo = new QComboBox(settingsWidget);
    mFaultTypeCombo->addItems({
        "None",
        "Line-ground (single phase)",
        "Line-line",
        "Line-line-ground",
        "Line-line-line (three phase)"
    });

    // Xf (pu)
    QLabel *xfLabel = new QLabel("Xf (pu)", settingsWidget);
    mXfSpinBox = new QDoubleSpinBox(settingsWidget);
    mXfSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    mXfSpinBox->setRange(-999999.0, 999999.0);
    mXfSpinBox->setDecimals(4);
    mXfSpinBox->setSingleStep(0.01);
    mXfSpinBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mXfSpinBox->setMinimumWidth(75);

    // Result Type
    QLabel *resultLabel = new QLabel("Result Type", settingsWidget);
    mResultTypeCombo = new QComboBox(settingsWidget);
    mResultTypeCombo->addItems({
        "None",
        "Symmetric RMS",
        "Asymmetric RMS",
        "Peak",
        "Plot of waveform"
    });

    configureComboPopup(mCalculateTypeCombo);
    configureComboPopup(mFaultTypeCombo);
    configureComboPopup(mResultTypeCombo);

    // Fault Time (s)
    QLabel *timeLabel = new QLabel("Fault Time (s)", settingsWidget);
    mFaultTimeSpinBox = new QDoubleSpinBox(settingsWidget);
    mFaultTimeSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    mFaultTimeSpinBox->setRange(-999999.0, 999999.0);
    mFaultTimeSpinBox->setDecimals(4);
    mFaultTimeSpinBox->setSingleStep(0.01);
    mFaultTimeSpinBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mFaultTimeSpinBox->setMinimumWidth(75);

    settingsLayout->addWidget(calculateLabel, 0, 0);
    settingsLayout->addWidget(mCalculateTypeCombo, 0, 1);
    settingsLayout->addWidget(rfLabel, 0, 2);
    settingsLayout->addWidget(mRfSpinBox, 0, 3);

    settingsLayout->addWidget(faultLabel, 1, 0);
    settingsLayout->addWidget(mFaultTypeCombo, 1, 1);
    settingsLayout->addWidget(xfLabel, 1, 2);
    settingsLayout->addWidget(mXfSpinBox, 1, 3);

    settingsLayout->addWidget(resultLabel, 2, 0);
    settingsLayout->addWidget(mResultTypeCombo, 2, 1);
    settingsLayout->addWidget(timeLabel, 2, 2);
    settingsLayout->addWidget(mFaultTimeSpinBox, 2, 3);

    settingsLayout->setColumnStretch(1, 1);
    settingsLayout->setColumnStretch(3, 1);

    mSettingsScrollArea->setWidget(settingsWidget);
    mainLayout->addWidget(mSettingsScrollArea);

    mSettingsScrollArea->setVisible(false);

    updateNameLabels();

    // SIGNALS
    connect(mToggleSettingsButton, &QToolButton::toggled, this, &FaultFileCard::toggleSettingsSection);
    connect(mSelectionCheckBox, &QCheckBox::toggled, this, &FaultFileCard::emitSelectionChanged);
    connect(mCalculateTypeCombo, &QComboBox::currentTextChanged, this, &FaultFileCard::emitSettingsChanged);
    connect(mFaultTypeCombo, &QComboBox::currentTextChanged, this, &FaultFileCard::emitSettingsChanged);
    connect(mResultTypeCombo, &QComboBox::currentTextChanged, this, &FaultFileCard::emitSettingsChanged);
    connect(mRfSpinBox, &QDoubleSpinBox::valueChanged, this, &FaultFileCard::emitSettingsChanged);
    connect(mXfSpinBox, &QDoubleSpinBox::valueChanged, this, &FaultFileCard::emitSettingsChanged);
    connect(mFaultTimeSpinBox, &QDoubleSpinBox::valueChanged, this, &FaultFileCard::emitSettingsChanged);

    connect(mRemoveButton, &QPushButton::clicked, this, [this]() {
        emit removeRequested(mFilePath);
    });

    connect(mRenameButton, &QPushButton::clicked, this, &FaultFileCard::renameFile);
}

QString FaultFileCard::shortId() const
{
    return mShortId;
}

void FaultFileCard::setShortId(const QString &id)
{
    mShortId = id.trimmed();
    updateNameLabels();
}

QString FaultFileCard::effectiveDisplayName() const
{
    if (!mAlias.trimmed().isEmpty())
        return mAlias.trimmed();

    if (!mShortId.trimmed().isEmpty())
        return mShortId.trimmed();

    return mOriginalFileName;
}

void FaultFileCard::updateNameLabels()
{
    const bool hasAlias = !mAlias.trimmed().isEmpty() && (mAlias != mOriginalFileName);
    QString displayName = effectiveDisplayName();

    mTitleLabel->setText(displayName);
    mTitleLabel->setToolTip(QString("Display: %1\nFile: %2").arg(displayName, mFilePath));

    if (mAliasBadgeLabel)
        mAliasBadgeLabel->setVisible(hasAlias);

    mOriginalFileNameLabel->setText(mOriginalFileName);
    mOriginalFileNameLabel->setToolTip(mFilePath);
    mOriginalFileNameLabel->setVisible(true);

    const bool isExpanded = mToggleSettingsButton && mToggleSettingsButton->isChecked();
    if (isExpanded)
    {
        setFixedHeight(204);
    }
    else
    {
        setFixedHeight(84);
    }
}

void FaultFileCard::toggleSettingsSection(bool checked)
{
    mSettingsScrollArea->setVisible(checked);
    mToggleSettingsButton->setText(checked ? "Settings ▾" : "Settings ▾");

    if (checked)
    {
        setFixedHeight(204);
    }
    else
    {
        setFixedHeight(84);
    }

    updateGeometry();
    if (parentWidget())
        parentWidget()->updateGeometry();
}

QString FaultFileCard::filePath() const
{
    return mFilePath;
}

QString FaultFileCard::originalFileName() const
{
    return mOriginalFileName;
}

QString FaultFileCard::alias() const
{
    return mAlias;
}

void FaultFileCard::setAlias(const QString &alias)
{
    mAlias = alias.trimmed();
    updateNameLabels();
}

bool FaultFileCard::isSelected() const
{
    return mSelectionCheckBox && mSelectionCheckBox->isChecked();
}

void FaultFileCard::setSelected(bool selected)
{
    if (!mSelectionCheckBox)
        return;

    QSignalBlocker blocker(mSelectionCheckBox);
    mSelectionCheckBox->setChecked(selected);
}

FaultTypeSettings FaultFileCard::settings() const
{
    FaultTypeSettings result;

    result.calculateType = mCalculateTypeCombo->currentText();
    result.faultType = mFaultTypeCombo->currentText();
    result.resultType = mResultTypeCombo->currentText();

    result.faultResistance = QString::number(mRfSpinBox->value(), 'f', 4);
    result.faultReactance = QString::number(mXfSpinBox->value(), 'f', 4);
    result.faultTime = QString::number(mFaultTimeSpinBox->value(), 'f', 4);

    result.configured = result.faultType != "None" && result.resultType != "None";

    return result;
}

void FaultFileCard::updateSettings()
{
    if (mUpdating)
        return;

    emit settingsChanged(mFilePath, settings());
}

void FaultFileCard::emitSettingsChanged()
{
    updateSettings();
}

void FaultFileCard::emitSelectionChanged()
{
    emit selectionChanged();
}

void FaultFileCard::renameFile()
{
    QString currentVal = !mAlias.isEmpty() ? mAlias : (!mShortId.isEmpty() ? mShortId : mOriginalFileName);

    QString defaultPrompt = QString("Set label for '%1':\n(Leave empty to reset back to %2)")
                                .arg(mOriginalFileName, !mShortId.isEmpty() ? mShortId : "default ID");

    while (true)
    {
        bool ok = false;
        QString newAlias = QInputDialog::getText(
            this,
            "Edit Label",
            defaultPrompt,
            QLineEdit::Normal,
            currentVal,
            &ok
            );

        if (!ok)
            return; // User canceled

        newAlias = newAlias.trimmed();

        // If user clears the input or inputs default identifier, reset back to default
        if (newAlias.isEmpty() || newAlias == mOriginalFileName || newAlias == mShortId)
        {
            setAlias(QString());
            emit renameRequested(mFilePath, effectiveDisplayName());
            return;
        }

        // =========================================================
        // UNIQUE NAME VALIDATION ACROSS ALL LOADED SIBLING CARDS
        // =========================================================
        bool isDuplicate = false;
        QWidget *topWindow = window();
        if (topWindow)
        {
            const QList<FaultFileCard *> allCards = topWindow->findChildren<FaultFileCard *>();
            for (FaultFileCard *otherCard : allCards)
            {
                if (!otherCard || otherCard == this)
                    continue;

                // Compare against the other card's active display name
                if (otherCard->effectiveDisplayName().compare(newAlias, Qt::CaseInsensitive) == 0 ||
                    otherCard->alias().compare(newAlias, Qt::CaseInsensitive) == 0 ||
                    otherCard->shortId().compare(newAlias, Qt::CaseInsensitive) == 0)
                {
                    isDuplicate = true;
                    break;
                }
            }
        }

        if (isDuplicate)
        {
            QMessageBox::warning(
                this,
                "Duplicate Name",
                QString("The name '%1' is already in use by another file.\nPlease choose a unique name.")
                    .arg(newAlias)
                );
            currentVal = newAlias;
            continue; // Loop back and prompt again
        }

        setAlias(newAlias);
        emit renameRequested(mFilePath, effectiveDisplayName());
        break;
    }
}

QString FaultFileCard::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);

    if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));

    return QString("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
}

void FaultFileCard::setDisplayName(const QString &name)
{
    QString cleanName = name.trimmed();
    if (cleanName.isEmpty() || cleanName == mOriginalFileName || cleanName == mShortId)
    {
        setAlias(QString());
    }
    else
    {
        setAlias(cleanName);
    }
}
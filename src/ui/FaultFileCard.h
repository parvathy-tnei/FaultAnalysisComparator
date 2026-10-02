#pragma once

#include <QWidget>
#include <QString>

// Forward declaration of FaultTypeSettings struct if defined in FaultAnalysisWindow
#include "FaultAnalysisWindow.h"

class QCheckBox;
class QLabel;
class QPushButton;
class QToolButton;
class QComboBox;
class QDoubleSpinBox;
class QFrame;
class QScrollArea;

class FaultFileCard : public QWidget
{
    Q_OBJECT

public:
    explicit FaultFileCard(
        const QString &filePath,
        const FaultTypeSettings &settings,
        int rowCount,
        int columnCount,
        qint64 fileSizeBytes,
        QWidget *parent = nullptr
        );

    QString filePath() const;
    QString originalFileName() const;
    QString alias() const;
    QString effectiveDisplayName() const;
    // Backward-compatibility accessors
    QString displayName() const { return effectiveDisplayName(); }
    void setDisplayName(const QString &name);

    void setAlias(const QString &alias);

    QString shortId() const;
    void setShortId(const QString &id);
    bool isSelected() const;
    void setSelected(bool selected);

    FaultTypeSettings settings() const;
    void updateSettings();

signals:
    void selectionChanged();
    void renameRequested(const QString &filePath, const QString &newAlias);
    void removeRequested(const QString &filePath);
    void settingsChanged(const QString &filePath, const FaultTypeSettings &settings);

private slots:
    void emitSettingsChanged();
    void emitSelectionChanged();
    void renameFile();
    void toggleSettingsSection(bool checked);

private:
    void createUi();
    void updateNameLabels();
    QString formatFileSize(qint64 bytes) const;
    QString mShortId;

    QString mFilePath;
    QString mOriginalFileName;
    QString mAlias;
    int mRowCount;
    int mColumnCount;
    qint64 mFileSizeBytes;
    bool mUpdating;

    // Header & Meta Labels
    QFrame *mCardFrame = nullptr;
    QCheckBox *mSelectionCheckBox = nullptr;
    QLabel *mFileIconLabel = nullptr;
    QLabel *mTitleLabel = nullptr;
    QLabel *mAliasBadgeLabel = nullptr;
    QLabel *mOriginalFileNameLabel = nullptr;
    QLabel *mFileInfoLabel = nullptr;
    QPushButton *mRenameButton = nullptr;
    QPushButton *mRemoveButton = nullptr;

    // Collapsible Controls
    QToolButton *mToggleSettingsButton = nullptr;
    QScrollArea *mSettingsScrollArea = nullptr;

    // Settings Controls
    QComboBox *mCalculateTypeCombo = nullptr;
    QComboBox *mFaultTypeCombo = nullptr;
    QComboBox *mResultTypeCombo = nullptr;
    QDoubleSpinBox *mRfSpinBox = nullptr;
    QDoubleSpinBox *mXfSpinBox = nullptr;
    QDoubleSpinBox *mFaultTimeSpinBox = nullptr;

};
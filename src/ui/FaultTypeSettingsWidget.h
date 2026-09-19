#pragma once

#include "FaultTypeSettings.h"
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;

class FaultTypeSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FaultTypeSettingsWidget(const QString &filePath, QWidget *parent = nullptr);

    void setConfiguration(const FaultTypeSettings &settings);

    FaultTypeSettings configuration() const;

signals:

    void configurationApplied(const QString &filePath, const FaultTypeSettings &settings);
    void configurationCleared(const QString &filePath);


private slots:

    void applyConfiguration();
    void clearConfiguration();

private:

    QString mFilePath;

    QLabel *mFileLabel;

    QComboBox *mCalculateTypeCombo;
    QComboBox *mFaultTypeCombo;
    QComboBox *mResultTypeCombo;

    QLineEdit *mFaultTimeEdit;
    QLineEdit *mFaultResistanceEdit;
    QLineEdit *mFaultReactanceEdit;

    QPushButton *mApplyButton;
    QPushButton *mClearButton;
};

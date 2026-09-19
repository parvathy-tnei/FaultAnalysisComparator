#pragma once

#include <QWidget>
#include <QStringList>
#include <QMap>

#include "FaultTypeSettings.h"

class QTableWidget;

class FaultTypeSettingsSummaryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FaultTypeSettingsSummaryWidget(QWidget *parent = nullptr);

    void setFiles(
        const QStringList &filePaths,
        const QMap<QString, FaultTypeSettings> &settings
        );

    void clear();

signals:
    void settingsChanged(
        const QString &filePath,
        const FaultTypeSettings &settings
        );

private:
    QTableWidget *mTable;

    FaultTypeSettings getSettingsForFile(
        const QString &filePath
        ) const;
};
#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QList>
#include <QMap>

#include "FaultTypeSettings.h"

class QHBoxLayout; // chnaged from tab to box
class QVBoxLayout;
class QScrollArea;
class QWidget;
class QAction;
class QPushButton;
class QToolButton;

class FaultComparisonSelectionWidget;
class FaultComparisonTableWidget;
class FaultTypeSettingsWidget;
class FaultTypeSettingsSummaryWidget;

class FaultAnalysisWindow : public QMainWindow  // FaultAnalysiswindow is now a qt widget

{
    Q_OBJECT

public:
    explicit FaultAnalysisWindow(QWidget *parent = nullptr);

private slots:
    void addCsvFiles();
    void closeFile(int index);
    void clearLoadedFiles();
    void updateWindowTitle();
    void updateComparisonTable();
    void exportPdf();
    void showAbout();

    // Fault Type Settings
    void applyFaultTypeConfiguration( const QString &filePath,const FaultTypeSettings &settings);

    void clearFaultTypeConfiguration(const QString &filePath);

    // Collapse / expand
    void toggleFaultTypeSettings();

private:

    void refreshFileTabs();
    // Fault Type Settings UI
    void showFaultTypeSettings();
    void updateFaultTypeSettings();
    void rebuildComparisonSelection();

    /*
     * Returns the display name for a CSV column.
     *
     * At the moment this keeps the original CSV
     * column name. The fault-specific mapping can
     * be added here once the exact IPSA mapping
     * is defined.
     */

    QString displayColumnName(
        const QString &filePath,
        const QString &columnName
        ) const;

    QList<QMap<QString, QString>> buildColumnMappings() const;

private:
    QWidget *mFileContainer;
    QHBoxLayout *mFileLayout;
    QScrollArea *mFileScrollArea;

    QWidget *mContentWidget;
    QVBoxLayout *mContentLayout;

    FaultComparisonSelectionWidget *mComparisonSelection;
    FaultComparisonTableWidget *mComparisonTable;
    FaultTypeSettingsSummaryWidget *mFaultSettingsSummary;

    //QWidget *mFaultSettingsWidget;
    //QScrollArea *mFaultSettingsScrollArea;
    //QWidget *mFaultSettingsContainer;
    //QVBoxLayout *mFaultSettingsLayout;
    QToolButton *mFaultSettingsToggleButton;
    //QPushButton *mSkipSettingsButton;

    QAction *mAddCsvAction;
    QAction *mClearFilesAction;
    QAction *mExportPdfAction;
    QAction *mExitAction;

    QStringList mLoadedFiles;
    QList<QStringList> mCsvHeaders;
    QList<QList<QStringList>> mCsvRows;

    QStringList mDefaultColumns;
    QMap<QString, FaultTypeSettings> mFaultSettings;


};






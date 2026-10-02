#pragma once

#include <QMainWindow>
#include <QStringList>
#include <QList>
#include <QMap>

#include "FaultTypeSettings.h"

class FaultFileCard;
class QGridLayout;
class QLabel;
class QVBoxLayout;
class QScrollArea;
class QWidget;
class QAction;
class QPushButton;
class QToolButton;
class QPropertyAnimation;
class QFrame;

class FaultComparisonSelectionWidget;
class FaultComparisonTableWidget;
class FaultTypeSettingsWidget;
class FaultTypeSettingsSummaryWidget;

class FaultAnalysisWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FaultAnalysisWindow(QWidget *parent = nullptr);

private slots:

    // ---------------------------------------------------------
    // File handling
    // ---------------------------------------------------------
    void addCsvFiles();
    void closeFile(int index);
    void closeFileByPath(const QString &filePath);
    void clearLoadedFiles();

    // ---------------------------------------------------------
    // General window / comparison
    // ---------------------------------------------------------
    void updateWindowTitle();
    void updateComparisonTable();
    void exportPdf();
    void exportWord();
    void showAbout();

    // ---------------------------------------------------------
    // New File Card UI
    // ---------------------------------------------------------
    void updateSelectedFileCount();
    void handleFileSettingsChanged(
        const QString &filePath,
        const FaultTypeSettings &settings);

    void handleFileRename(
        const QString &filePath,
        const QString &newName);


private:

    void updateFileScrollAreaHeight();
    // ---------------------------------------------------------
    // File card UI
    // ---------------------------------------------------------
    void refreshFileCards();

    // ---------------------------------------------------------
    // Comparison selection
    // ---------------------------------------------------------
    void rebuildComparisonSelection();

    // ---------------------------------------------------------
    // Column mapping
    // ---------------------------------------------------------
    QString displayColumnName(
        const QString &filePath,
        const QString &columnName
        ) const;

    QList<QMap<QString, QString>> buildColumnMappings() const;

private:

    // Files to compare panel
    // =========================================================

    QWidget *mFileContainer = nullptr;

    // Changed from QHBoxLayout to QGridLayout
    QGridLayout *mFileLayout = nullptr;

    QScrollArea *mFileScrollArea = nullptr;

    QLabel *mFilesHeadingLabel = nullptr;
    QFrame *mFilesPanel = nullptr;
    QWidget *mFileCardsSection = nullptr;
    QToolButton *mFilesCollapseButton = nullptr;
    bool mFilesSectionCollapsed = false;
    // ===========================================
    // =========================================================
    // Main comparison content
    // =========================================================

    QWidget *mContentWidget = nullptr;
    QVBoxLayout *mContentLayout = nullptr;

    FaultComparisonSelectionWidget *mComparisonSelection = nullptr;
    FaultComparisonTableWidget *mComparisonTable = nullptr;


    // =========================================================
    // File cards
    // =========================================================

    QList<FaultFileCard *> mFileCards;


    // =========================================================
    // File information
    // =========================================================

    QStringList mLoadedFiles;

    QList<QStringList> mCsvHeaders;

    QList<QList<QStringList>> mCsvRows;


    // =========================================================
    // Comparison columns
    // =========================================================

    QStringList mDefaultColumns;


    // =========================================================
    // Fault Type Settings
    //
    // One settings object per loaded CSV file.
    // =========================================================

    QMap<QString, FaultTypeSettings> mFaultSettings;


    // =========================================================
    // Display names
    //
    // Key   = actual CSV file path
    // Value = name displayed in the UI/table/PDF
    //
    // Renaming therefore does NOT modify the real file.
    // =========================================================

    QMap<QString, QString> mDisplayNames;


    // =========================================================
    // File selection state
    //
    // Key   = actual CSV file path
    // Value = checked/unchecked
    //
    // This allows loaded files to remain in the application
    // while only checked files are included in comparison.
    // =========================================================

    QMap<QString, bool> mFileSelectionState;


    // =========================================================
    // Menu actions
    // =========================================================

    QAction *mAddCsvAction = nullptr;
    QAction *mClearFilesAction = nullptr;
    QAction *mExportPdfAction = nullptr;
    QAction *mExportWordAction = nullptr;
    QAction *mExitAction = nullptr;
};
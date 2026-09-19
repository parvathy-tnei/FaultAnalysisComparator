#include "FaultAnalysisWindow.h"

#include "FaultComparisonSelectionWidget.h"
#include "FaultComparisonTableWidget.h"
#include "FaultTypeSettingsSummaryWidget.h"
#include "FaultColumnMapping.h"

#include "../parser/CsvReader.h"

#include <QStatusBar>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
#include <QDateTime>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDir>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QWidget>
#include <QToolButton>
#include <QToolTip>


FaultAnalysisWindow::FaultAnalysisWindow(
    QWidget *parent
    )
    : QMainWindow(parent)
{
    setWindowTitle(
        "Fault Analysis Comparator"
        );

    resize(
        1000,
        700
        );


    // =========================================================
    // FILE MENU
    // =========================================================

    QMenu *fileMenu =
        menuBar()->addMenu("File");

    mAddCsvAction =
        fileMenu->addAction(
            "Add CSV File(s)"
            );

    mClearFilesAction =
        fileMenu->addAction(
            "Clear Loaded Files"
            );

    fileMenu->addSeparator();

    mExitAction =
        fileMenu->addAction(
            "Exit"
            );

    // =========================================================
    // EXPORT MENU
    // =========================================================

    QMenu *exportMenu =
        menuBar()->addMenu("Export");


    mExportPdfAction =
        exportMenu->addAction(
        "Export Fault Analysis as PDF...");

    connect(
        mExportPdfAction,
        &QAction::triggered,
        this,
        &FaultAnalysisWindow::exportPdf
        );

    // =========================================================
    // HELP MENU
    // =========================================================

    QMenu *helpMenu =
        menuBar()->addMenu("Help");

    QAction *aboutAction =
        helpMenu->addAction(
            "About"
            );

    connect(
        aboutAction,
        &QAction::triggered,
        this,
        &FaultAnalysisWindow::showAbout
        );


    // =========================================================
    // ACTION CONNECTIONS
    // =========================================================

    connect(
        mAddCsvAction,
        &QAction::triggered,
        this,
        &FaultAnalysisWindow::addCsvFiles
        );

    connect(
        mClearFilesAction,
        &QAction::triggered,
        this,
        &FaultAnalysisWindow::clearLoadedFiles
        );

    connect(
        mExitAction,
        &QAction::triggered,
        this,
        &QMainWindow::close
        );



    // =========================================================
    // CENTRAL WIDGET
    // =========================================================

    QWidget *centralWidget =
        new QWidget(this);

    setCentralWidget(
        centralWidget
        );


    // =========================================================
    // MAIN LAYOUT
    // =========================================================

    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            centralWidget
            );

    mainLayout->setContentsMargins(
        20,
        10,
        20,
        10
        );

    mainLayout->setSpacing(8);


    // =========================================================
    // FILES TO COMPARE ROW
    // =========================================================

    QHBoxLayout *filesRow =
        new QHBoxLayout();

    filesRow->setContentsMargins(
        0,
        0,
        0,
        0
        );

    filesRow->setSpacing(4);


    QLabel *filesLabel =
        new QLabel(
            "Files to compare",
            centralWidget
            );

    QFont labelFont =
        filesLabel->font();

    labelFont.setBold(true);

    filesLabel->setFont(
        labelFont
        );

    filesLabel->setSizePolicy(
        QSizePolicy::Fixed,
        QSizePolicy::Preferred
        );

    filesRow->addWidget(
        filesLabel
        );


    // =========================================================
    // FILE SCROLL AREA
    // =========================================================

    mFileScrollArea =
        new QScrollArea(
            centralWidget
            );

    mFileScrollArea->setWidgetResizable(
        true
        );

    mFileScrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
        );

    mFileScrollArea->setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
        );

    mFileScrollArea->setFrameShape(
        QFrame::NoFrame
        );

    mFileScrollArea->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
        );

    mFileScrollArea->setMinimumHeight(
        0
        );

    mFileScrollArea->setMaximumHeight(
        0
        );


    // =========================================================
    // FILE TAB CONTAINER
    // =========================================================

    mFileContainer =
        new QWidget();

    mFileLayout =
        new QHBoxLayout(
            mFileContainer
            );

    mFileLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    mFileLayout->setSpacing(
        6
        );

    mFileLayout->setAlignment(
        Qt::AlignVCenter
        );

    mFileLayout->addStretch();


    mFileScrollArea->setWidget(
        mFileContainer
        );


    filesRow->addWidget(
        mFileScrollArea,
        1
        );

    mainLayout->addLayout(
        filesRow
        );


    // =========================================================
    // SEPARATOR
    // =========================================================

    QFrame *separator =
        new QFrame(
            centralWidget
            );

    separator->setFrameShape(
        QFrame::HLine
        );

    separator->setFrameShadow(
        QFrame::Sunken
        );

    mainLayout->addWidget(
        separator
        );


    // =========================================================
    // MAIN CONTENT WIDGET
    // =========================================================

    mContentWidget =
        new QWidget(
            centralWidget
            );

    mContentLayout =
        new QVBoxLayout(
            mContentWidget
            );

    mContentLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    mContentLayout->setSpacing(
        8
        );

    // =========================================================
    // FAULT TYPE SETTINGS TOGGLE
    // =========================================================

    mFaultSettingsToggleButton =
        new QToolButton(
            mContentWidget
            );

    mFaultSettingsToggleButton->setText(
        "▼  Fault Type Settings"
        );

    mFaultSettingsToggleButton->setCheckable(
        true
        );

    mFaultSettingsToggleButton->setChecked(
        true
        );

    mFaultSettingsToggleButton->setToolButtonStyle(
        Qt::ToolButtonTextOnly
        );

    mFaultSettingsToggleButton->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
        );

    mFaultSettingsToggleButton->setStyleSheet(
        "QToolButton {"
        "    text-align: left;"
        "    font-weight: bold;"
        "    padding: 4px;"
        "    border: none;"
        "}"
        ""
        "QToolButton:hover {"
        "    background-color: #eeeeee;"
        "}"
        );

    mContentLayout->addWidget(
        mFaultSettingsToggleButton
        );

    connect(
        mFaultSettingsToggleButton,
        &QToolButton::clicked,
        this,
        &FaultAnalysisWindow::toggleFaultTypeSettings
        );


    // =========================================================
    // =========================================================
    // FAULT TYPE SETTINGS SUMMARY
    // =========================================================

    mFaultSettingsSummary =
        new FaultTypeSettingsSummaryWidget(
            mContentWidget
            );

    mContentLayout->addWidget(
        mFaultSettingsSummary
        );
    mFaultSettingsSummary->hide();
    connect(
        mFaultSettingsSummary,
        &FaultTypeSettingsSummaryWidget::settingsChanged,
        this,
        &FaultAnalysisWindow::applyFaultTypeConfiguration
        );


    // =========================================================
    // COMPARISON AREA
    // =========================================================

    QHBoxLayout *comparisonLayout =
        new QHBoxLayout();

    comparisonLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    comparisonLayout->setSpacing(
        8
        );


    // ---------------------------------------------------------
    // LEFT - COMPARISON SELECTION
    // ---------------------------------------------------------

    mComparisonSelection =
        new FaultComparisonSelectionWidget(
            mContentWidget
            );
    mComparisonSelection->setEmptyState(true);

    comparisonLayout->addWidget(
        mComparisonSelection
        );


    // ---------------------------------------------------------
    // RIGHT - COMPARISON TABLE
    // ---------------------------------------------------------

    mComparisonTable =
        new FaultComparisonTableWidget(
            mContentWidget
            );

    comparisonLayout->addWidget(
        mComparisonTable,
        1
        );


    mContentLayout->addLayout(
        comparisonLayout,
        1
        );


    // =========================================================
    // ADD CONTENT TO MAIN WINDOW
    // =========================================================

    mainLayout->addWidget(
        mContentWidget,
        1
        );


    // =========================================================
    // CONNECTIONS
    // =========================================================

    connect(
        mComparisonSelection,
        &FaultComparisonSelectionWidget::selectionChanged,
        this,
        &FaultAnalysisWindow::updateComparisonTable
        );


    // =========================================================
    // INITIAL STATE
    // =========================================================

    mClearFilesAction->setEnabled(
        false
        );
    mExportPdfAction->setEnabled(
        false
        );

    updateWindowTitle();
}


// =============================================================
// ADD CSV FILES
// =============================================================

void FaultAnalysisWindow::addCsvFiles()
{
    QString initialDirectory =
        QStandardPaths::writableLocation(
            QStandardPaths::DocumentsLocation
            );


    QStringList files =
        QFileDialog::getOpenFileNames(
            this,
            "Select Fault Analysis CSV File(s)",
            initialDirectory,
            "CSV Files (*.csv);;All Files (*)"
            );


    if (files.isEmpty())
        return;


    QStringList failedFiles;


    // =========================================================
    // PROCESS FILES
    // =========================================================

    for (const QString &filePath : files)
    {
        QFileInfo fileInfo(
            filePath
            );


        if (!fileInfo.exists() ||
            !fileInfo.isFile())
        {
            failedFiles.append(
                filePath
                );

            continue;
        }


        QFile file(
            filePath
            );


        if (!file.open(
                QIODevice::ReadOnly |
                QIODevice::Text))
        {
            failedFiles.append(
                filePath
                );

            continue;
        }


        file.close();


        QString absolutePath =
            fileInfo.absoluteFilePath();


        // -----------------------------------------------------
        // PREVENT DUPLICATES
        // -----------------------------------------------------

        bool alreadyLoaded = false;


        for (const QString &loadedFile :
             mLoadedFiles)
        {
            QFileInfo loadedInfo(
                loadedFile
                );


            if (loadedInfo.absoluteFilePath()
                == absolutePath)
            {
                alreadyLoaded = true;
                break;
            }
        }


        if (alreadyLoaded)
            continue;


        // -----------------------------------------------------
        // STORE FILE
        // -----------------------------------------------------

        mLoadedFiles.append(
            absolutePath
            );


        // -----------------------------------------------------
        // CREATE EMPTY SETTINGS
        // -----------------------------------------------------

        FaultTypeSettings settings;

        settings.configured = false;

        mFaultSettings.insert(
            absolutePath,
            settings
            );
    }


    // =========================================================
    // REFRESH FILE DISPLAY
    // =========================================================

    refreshFileTabs();


    // =========================================================
    // READ CSV DATA
    // =========================================================

    mCsvHeaders.clear();

    mCsvRows.clear();

    QStringList allColumns;


    for (const QString &filePath :
         mLoadedFiles)
    {
        QStringList headers;

        QList<QStringList> rows;

        QString errorMessage;


        if (!CsvReader::readData(
                filePath,
                headers,
                rows,
                errorMessage))
        {
            continue;
        }


        mCsvHeaders.append(
            headers
            );

        mCsvRows.append(
            rows
            );


        // -----------------------------------------------------
        // COLLECT ORIGINAL CSV COLUMNS
        // -----------------------------------------------------

        for (const QString &header :
             headers)
        {
            if (!allColumns.contains(
                    header))
            {
                allColumns.append(
                    header
                    );
            }
        }
    }


    // =========================================================
    // STORE ORIGINAL COLUMNS
    // =========================================================

    mDefaultColumns =
        allColumns;


    // =========================================================
    // UPDATE FAULT TYPE SETTINGS
    // =========================================================
    mFaultSettingsSummary->setFiles(
        mLoadedFiles,
        mFaultSettings
        );

    showFaultTypeSettings();


    // =========================================================
    // BUILD COMPARISON SELECTION
    // =========================================================

    rebuildComparisonSelection();


    // =========================================================
    // UPDATE UI
    // =========================================================

    mClearFilesAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );

    mExportPdfAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );

    updateWindowTitle();


    // =========================================================
    // REPORT FAILED FILES
    // =========================================================

    if (!failedFiles.isEmpty())
    {
        QString message =
            "The following files could not be opened:\n\n";


        for (const QString &file :
             failedFiles)
        {
            message += file;
            message += "\n";
        }


        QMessageBox::warning(
            this,
            "Unable to Open File(s)",
            message
            );
    }
}



// =============================================================
// REFRESH FILE TABS
// =============================================================

void FaultAnalysisWindow::refreshFileTabs()
{
    // =========================================================
    // REMOVE OLD FILE TABS
    // =========================================================

    while (mFileLayout->count() > 0)
    {
        QLayoutItem *item =
            mFileLayout->takeAt(0);


        QWidget *widget =
            item->widget();


        if (widget)
            widget->deleteLater();


        delete item;
    }


    // =========================================================
    // NO FILES
    // =========================================================

    if (mLoadedFiles.isEmpty())
    {
        mFileScrollArea->setMinimumHeight(
            0
            );

        mFileScrollArea->setMaximumHeight(
            0
            );

        return;
    }


    // =========================================================
    // FILES EXIST
    // =========================================================

    mFileScrollArea->setMinimumHeight(
        28
        );

    mFileScrollArea->setMaximumHeight(
        34
        );


    // =========================================================
    // CREATE FILE TABS
    // =========================================================

    for (int i = 0;
         i < mLoadedFiles.size();
         ++i)
    {
        QString filePath =
            mLoadedFiles.at(i);


        QFileInfo fileInfo(
            filePath
            );


        QWidget *fileTab =
            new QWidget(
                mFileContainer
                );


        fileTab->setFixedSize(
            180,
            26
            );


        fileTab->setObjectName(
            "fileTab"
            );


        QHBoxLayout *tabLayout =
            new QHBoxLayout(
                fileTab
                );


        tabLayout->setContentsMargins(
            5,
            0,
            3,
            0
            );

        tabLayout->setSpacing(
            3
            );


        tabLayout->setAlignment(
            Qt::AlignVCenter
            );


        QLabel *fileNameLabel =
            new QLabel(
                fileInfo.fileName(),
                fileTab
                );


        fileNameLabel->setToolTip(
            filePath
            );


        QPushButton *closeButton =
            new QPushButton(
                "×",
                fileTab
                );


        closeButton->setFixedSize(
            14,
            14
            );


        closeButton->setToolTip(
            "Remove this file"
            );


        tabLayout->addWidget(
            fileNameLabel
            );


        tabLayout->addWidget(
            closeButton
            );


        fileTab->setStyleSheet(
            "QWidget#fileTab {"
            "    border: 1px solid #c8c8c8;"
            "    border-radius: 4px;"
            "    background: #f5f5f5;"
            "}"
            "QWidget#fileTab:hover {"
            "    background: #e9e9e9;"
            "}"
            "QPushButton {"
            "    border: none;"
            "    background: transparent;"
            "    padding: 0px;"
            "    font-size: 14px;"
            "}"
            "QPushButton:hover {"
            "    background: #dcdcdc;"
            "    border-radius: 3px;"
            "}"
            );


        connect(
            closeButton,
            &QPushButton::clicked,
            this,
            [this, i]()
            {
                closeFile(i);
            }
            );


        mFileLayout->insertWidget(
            i,
            fileTab
            );
    }
}


// =============================================================
// CLOSE ONE FILE
// =============================================================

void FaultAnalysisWindow::closeFile(
    int index
    )
{


    if (index < 0 ||
        index >= mLoadedFiles.size())
    {
        return;
    }

    // ========================================================
    // Preserve the remaining files/columns after this file
    // is removed.
    // ========================================================

    if (mComparisonTable)
    {
        mComparisonTable->removeFileFromSavedOrder(
            mLoadedFiles.at(index)
            );
    }

    QString filePath =
        mLoadedFiles.at(index);


    mLoadedFiles.removeAt(
        index
        );


    if (index < mCsvHeaders.size())
    {
        mCsvHeaders.removeAt(
            index
            );
    }


    if (index < mCsvRows.size())
    {
        mCsvRows.removeAt(
            index
            );
    }


    // Remove settings belonging to
    // this file.

    mFaultSettings.remove(
        filePath
        );


    refreshFileTabs();


    mClearFilesAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );

    mExportPdfAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );

    updateWindowTitle();


    // Rebuild settings panel

    updateFaultTypeSettings();

    if (mLoadedFiles.isEmpty())
    {
        mFaultSettingsSummary->hide();
    }
    else
    {
        mFaultSettingsSummary->setFiles(
            mLoadedFiles,
            mFaultSettings
            );

        mFaultSettingsSummary->show();
    }


    // Rebuild comparison

    rebuildComparisonSelection();
}


// =============================================================
// CLEAR ALL FILES
// =============================================================

void FaultAnalysisWindow::clearLoadedFiles()
{
    if (mLoadedFiles.isEmpty())
        return;


    QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            "Clear Loaded Files",
            "Are you sure you want to clear all loaded CSV files?",
            QMessageBox::Yes |
                QMessageBox::No,
            QMessageBox::No
            );


    if (result != QMessageBox::Yes)
        return;


    mLoadedFiles.clear();

    mCsvHeaders.clear();

    mCsvRows.clear();

    mFaultSettings.clear();

    mDefaultColumns.clear();


    refreshFileTabs();


    updateFaultTypeSettings();
    mFaultSettingsSummary->clear();
    mFaultSettingsSummary->hide();


    mComparisonSelection->setAvailableColumns(
            QStringList()
            );


    mClearFilesAction->setEnabled(
        false
        );
    mExportPdfAction->setEnabled(
        false
        );

    updateWindowTitle();


    updateComparisonTable();
}


// =============================================================
// UPDATE WINDOW TITLE
// =============================================================

void FaultAnalysisWindow::updateWindowTitle()
{
    int fileCount =
        mLoadedFiles.size();


    if (fileCount == 0)
    {
        setWindowTitle(
            "Fault Analysis Comparator"
            );
    }
    else
    {
        setWindowTitle(
            QString(
                "Fault Analysis Comparator - %1 file(s)"
                )
                .arg(fileCount)
            );
    }
}


// =============================================================
// UPDATE COMPARISON TABLE
// =============================================================

void FaultAnalysisWindow::updateComparisonTable()
{
    if (!mComparisonTable ||
        !mComparisonSelection)
        return;

    QStringList selectedColumns =
        mComparisonSelection->selectedColumns();


    bool differenceEnabled =
        mComparisonSelection->differenceEnabled();

    QStringList fileNames;

    for (const QString &filePath :
         mLoadedFiles)
    {
        QFileInfo fileInfo(filePath);

        fileNames.append(
            fileInfo.fileName()
            );
    }

    QList<QMap<QString, QString>>
        columnMappingsPerFile =
        buildColumnMappings();

    mComparisonTable->setData(
        fileNames,
        mCsvHeaders,
        mCsvRows,
        selectedColumns,
        columnMappingsPerFile,
        differenceEnabled
        );
}

QList<QMap<QString, QString>>
FaultAnalysisWindow::buildColumnMappings() const
{
    QList<QMap<QString, QString>> mappings;

    for (int fileIndex = 0;
         fileIndex < mLoadedFiles.size();
         ++fileIndex)
    {
        const QString &filePath =
            mLoadedFiles.at(fileIndex);

        QMap<QString, QString> mapping;

        /*
         * STEP 1:
         * Always keep all original CSV columns available.
         */
        if (fileIndex < mCsvHeaders.size())
        {
            const QStringList &rawHeaders =
                mCsvHeaders.at(fileIndex);

            for (const QString &rawColumn :
                 rawHeaders)
            {

                // Do not expose the original AC Mag. name
                // as a separate logical/display column.
                if (rawColumn == "AC Mag. (kA)")
                {
                    mapping.insert(
                        "AC Mag-Steady State (kA)",
                        "AC Mag. (kA)"
                        );

                    continue;
                }
                mapping.insert(
                    rawColumn,
                    rawColumn
                    );
            }
        }

        /*
         * STEP 2:
         * Add configuration-based logical columns.
         */
        FaultTypeSettings settings =
            mFaultSettings.value(filePath);

        if (settings.configured)
        {
            if (settings.faultType ==
                "Line-line-line (three phase)")
            {
                if (settings.resultType ==
                    "Symmetric RMS")
                {
                    mapping.insert(
                        "Symmetric RMS (kA)",
                        "AC Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Asymmetric RMS")
                {
                    mapping.insert(
                        "Asymmetric RMS (kA)",
                        "Red Phase Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Peak")
                {
                    mapping.insert(
                        "Peak (kA)",
                        "Red Phase Mag. (kA)"
                        );
                }
            }
            else if (settings.faultType ==
                     "Line-ground (single phase)")
            {
                if (settings.resultType ==
                    "Symmetric RMS")
                {
                    mapping.insert(
                        "Symmetric RMS (kA)",
                        "Red Phase Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Asymmetric RMS")
                {
                    mapping.insert(
                        "Asymmetric RMS (kA)",
                        "Red Phase Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Peak")
                {
                    mapping.insert(
                        "Peak (kA)",
                        "Red Phase Mag. (kA)"
                        );
                }
            }
            else if (settings.faultType ==
                     "Line-line")
            {
                if (settings.resultType ==
                    "Symmetric RMS")
                {
                    mapping.insert(
                        "Symmetric RMS (kA)",
                        "Yellow Phase Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Asymmetric RMS")
                {
                    mapping.insert(
                        "Asymmetric RMS (kA)",
                        "Yellow Phase Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Peak")
                {
                    mapping.insert(
                        "Peak (kA)",
                        "Yellow Phase Mag. (kA)"
                        );
                }
            }
            else if (settings.faultType ==
                     "Line-line-ground")
            {
                if (settings.resultType ==
                    "Symmetric RMS")
                {
                    mapping.insert(
                        "Symmetric RMS (kA)",
                        "Yellow Phase Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Asymmetric RMS")
                {
                    mapping.insert(
                        "Asymmetric RMS (kA)",
                        "Yellow Phase Mag. (kA)"
                        );
                }
                else if (settings.resultType ==
                         "Peak")
                {
                    mapping.insert(
                        "Peak (kA)",
                        "Yellow Phase Mag. (kA)"
                        );
                }
            }
        }

        mappings.append(mapping);
    }

    return mappings;
}
// =============================================================
// UPDATE FAULT TYPE SETTINGS
// =============================================================

void FaultAnalysisWindow::updateFaultTypeSettings()
{
    if (!mFaultSettingsSummary)
        return;

    mFaultSettingsSummary->setFiles(
        mLoadedFiles,
        mFaultSettings
        );
}

// =============================================================
// APPLY FAULT TYPE CONFIGURATION
// =============================================================

void FaultAnalysisWindow::applyFaultTypeConfiguration(
    const QString &filePath,
    const FaultTypeSettings &settings
    )
{
    // Store settings for this specific file
    mFaultSettings[filePath] =
        settings;

    // Refresh the settings summary
    mFaultSettingsSummary->setFiles(
        mLoadedFiles,
        mFaultSettings
        );

    // Update comparison logic
    rebuildComparisonSelection();
}

// =============================================================
// CLEAR FAULT TYPE CONFIGURATION
// =============================================================

void FaultAnalysisWindow::clearFaultTypeConfiguration(
    const QString &filePath
    )
{
    FaultTypeSettings settings;

    settings.configured = false;

    mFaultSettings[filePath] =
        settings;

    mFaultSettingsSummary->setFiles(
        mLoadedFiles,
        mFaultSettings
        );

    rebuildComparisonSelection();
}

// =============================================================
// TOGGLE FAULT TYPE SETTINGS
// =============================================================

// =============================================================
// TOGGLE FAULT TYPE SETTINGS
// =============================================================

void FaultAnalysisWindow::toggleFaultTypeSettings()
{
    if (!mFaultSettingsToggleButton ||
        !mFaultSettingsSummary)
    {
        return;
    }

    bool expanded =
        mFaultSettingsToggleButton->isChecked();

    mFaultSettingsSummary->setVisible(
        expanded
        );

    if (expanded)
    {
        mFaultSettingsToggleButton->setText(
            "▼  Fault Type Settings"
            );
    }
    else
    {
        mFaultSettingsToggleButton->setText(
            "▶  Fault Type Settings"
            );
    }
}
// =============================================================
// SHOW FAULT TYPE SETTINGS
// =============================================================

// =============================================================
// SHOW FAULT TYPE SETTINGS
// =============================================================

void FaultAnalysisWindow::showFaultTypeSettings()
{
    updateFaultTypeSettings();

    mFaultSettingsToggleButton->setChecked(
        true
        );

    mFaultSettingsToggleButton->setText(
        "▼  Fault Type Settings"
        );

    mFaultSettingsSummary->show();
}
// =============================================================
// REBUILD COMPARISON SELECTION
// =============================================================

void FaultAnalysisWindow::rebuildComparisonSelection()
{
    if (mComparisonSelection)
    {
        mComparisonSelection->setEmptyState(
            mLoadedFiles.isEmpty()
            );
    }

    QStringList previousSelection =
        mComparisonSelection->selectedColumns();

    QStringList displayColumns;

    // ========================================================
    // STEP 0: NAME FIRST
    // ========================================================

    if (!mCsvHeaders.isEmpty())
    {
        bool hasName = false;

        for (const QStringList &headers : mCsvHeaders)
        {
            if (headers.contains("Name"))
            {
                hasName = true;
                break;
            }
        }

        if (hasName)
        {
            displayColumns.append("Name");
        }
    }

    // ========================================================
    // STEP 1: CONFIGURED LOGICAL COLUMNS
    // ========================================================

    QList<QMap<QString, QString>> mappings =
        buildColumnMappings();

    for (const QMap<QString, QString> &mapping :
         mappings)
    {
        for (auto it = mapping.constBegin();
             it != mapping.constEnd();
             ++it)
        {
            const QString logicalColumn =
                it.key();

            const QString rawColumn =
                it.value();

            if (logicalColumn != rawColumn &&
                !logicalColumn.trimmed().isEmpty())
            {
                if (!displayColumns.contains(logicalColumn))
                {
                    displayColumns.append(logicalColumn);
                }
            }
        }
    }

    // ========================================================
    // STEP 2: ORIGINAL CSV COLUMNS
    // ========================================================

    for (const QStringList &headers :
         mCsvHeaders)
    {
        for (const QString &column :
             headers)
        {
            if (column.trimmed().isEmpty())
                continue;

            // Do not show Include CDPs
            if (column == "Include CDPs")
                continue;

            // Name was already added at the beginning
            if (column == "Name")
                continue;

            // AC Mag. has a display alias
            // and was already added through the mapping.
            if (column == "AC Mag. (kA)")
                continue;

            if (!displayColumns.contains(column))
            {
                displayColumns.append(column);
            }
        }
    }

    // ========================================================
    // SET AVAILABLE COLUMNS
    // ========================================================

    mComparisonSelection->setAvailableColumns(
        displayColumns
        );

    // ========================================================
    // RESTORE PREVIOUS SELECTION
    // ========================================================

    QStringList restoredSelection;

    for (const QString &column :
         previousSelection)
    {
        if (displayColumns.contains(column))
        {
            restoredSelection.append(column);
        }
    }

    // Always keep Name selected
    if (displayColumns.contains("Name") &&
        !restoredSelection.contains("Name"))
    {
        restoredSelection.prepend("Name");
    }

    mComparisonSelection->setSelectedColumns(
        restoredSelection
        );

    updateComparisonTable();
}


// =============================================================
// DISPLAY COLUMN NAME
// =============================================================

QString FaultAnalysisWindow::displayColumnName(
    const QString &filePath,
    const QString &columnName
    ) const
{
    /*
     * Retrieve the configuration for
     * this particular file.
     */

    FaultTypeSettings settings =
        mFaultSettings.value(
            filePath
            );


    /*
     * If the user has not configured this
     * file, keep the original CSV name.
     */

    if (!settings.configured)
    {
        return columnName;
    }


    /*
     * ---------------------------------------------------------
     * IMPORTANT
     * ---------------------------------------------------------
     *
     * Do NOT invent IPSA column mappings here.
     *
     * Once the exact mapping is defined, this
     * function is where we will translate:
     *
     *     CSV column
     *          ↓
     *     Fault type
     *          ↓
     *     Result type
     *          ↓
     *     Display column
     *
     * The original CSV header remains untouched.
     *
     * For now we return the original name.
     */

    Q_UNUSED(settings);

    return columnName;
}

// =============================================================
// =============================================================
// EXPORT FAULT ANALYSIS AS PDF
// =============================================================
void FaultAnalysisWindow::exportPdf()
{
    // =========================================================
    // VALIDATION
    // =========================================================

    if (!mComparisonTable)
    {
        QMessageBox::warning(
            this,
            "Export PDF",
            "Comparison table is not available.");

        return;
    }

    QTableWidget *table =
        mComparisonTable->tableWidget();

    if (!table ||
        table->rowCount() == 0 ||
        table->columnCount() == 0)
    {
        QMessageBox::information(
            this,
            "Export PDF",
            "There is no comparison data to export.");

        return;
    }

    // =========================================================
    // FILE NAME
    // =========================================================

    QString fileName =
        QFileDialog::getSaveFileName(
            this,
            "Export Fault Analysis as PDF",
            QDir::home().filePath(
                "Fault_Analysis_Comparison.pdf"),
            "PDF Files (*.pdf)");

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(
            ".pdf",
            Qt::CaseInsensitive))
    {
        fileName += ".pdf";
    }

    // =========================================================
    // PDF
    // =========================================================

    QPdfWriter pdfWriter(fileName);

    pdfWriter.setResolution(300);

    pdfWriter.setPageSize(
        QPageSize(QPageSize::A4));

    pdfWriter.setPageOrientation(
        QPageLayout::Landscape);

    pdfWriter.setPageMargins(
        QMarginsF(12, 12, 12, 12),
        QPageLayout::Millimeter);

    QPainter painter(&pdfWriter);

    if (!painter.isActive())
    {
        QMessageBox::critical(
            this,
            "Export Failed",
            "Unable to create the PDF file.");

        return;
    }

    // =========================================================
    // PAGE
    // =========================================================

    const QRect pageRect =
        pdfWriter.pageLayout()
            .paintRectPixels(
                pdfWriter.resolution());

    const int left =
        pageRect.left();

    const int top =
        pageRect.top();

    const int pageWidth =
        pageRect.width();

    const int bottom =
        pageRect.bottom();

    // =========================================================
    // FONTS
    // =========================================================

    QFont titleFont(
        "Arial",
        22,
        QFont::Bold);

    QFont sectionFont(
        "Arial",
        13,
        QFont::Bold);

    QFont normalFont(
        "Arial",
        10);

    QFont groupFont(
        "Arial",
        11,
        QFont::Bold);

    QFont headerFont(
        "Arial",
        10,
        QFont::Bold);

    QFont valueFont(
        "Arial",
        10);

    QFont settingsFileFont(
        "Arial",
        9,
        QFont::Bold);

    // =========================================================
    // COLORS
    // =========================================================

    const QColor primaryBlue(
        "#1769AA");

    const QColor lightBlue(
        "#EAF4FC");

    const QColor headerBlue(
        "#D8EBF8");

    const QColor borderBlue(
        "#8DB9D8");

    const QColor bodyText(
        "#1F2933");

    const QColor alternateRow(
        "#F5F9FC");

    const QColor white(
        Qt::white);

    // =========================================================
    // TEXT HELPER
    // =========================================================

    auto drawWrapped =
        [&](const QString &text,
            const QRect &rect,
            const QFont &font,
            Qt::Alignment alignment)
    {
        painter.setFont(font);
        painter.setPen(bodyText);

        painter.drawText(
            rect,
            alignment | Qt::TextWordWrap,
            text);
    };

    // =========================================================
    // PAGE NUMBER
    // =========================================================

    int pageNumber = 1;

    auto drawFooter =
        [&]()
    {
        painter.setFont(
            QFont("Arial", 8));

        painter.setPen(
            QColor(90, 90, 90));

        painter.drawText(
            QRect(
                left,
                bottom - 22,
                pageWidth,
                18),
            Qt::AlignRight |
                Qt::AlignVCenter,
            QString("Page %1")
                .arg(pageNumber));

        painter.setPen(
            Qt::black);
    };

    // =========================================================
    // NEW PAGE
    // =========================================================

    auto newPage =
        [&]()
    {
        drawFooter();

        ++pageNumber;

        pdfWriter.newPage();

        painter.setPen(
            Qt::black);
    };

    // =========================================================
    // START
    // =========================================================

    int y = top;

    // =========================================================
    // TITLE
    // =========================================================

    painter.setFont(titleFont);
    painter.setPen(bodyText);

    painter.drawText(
        QRect(
            left,
            y,
            pageWidth,
            70),
        Qt::AlignCenter |
            Qt::AlignVCenter,
        "Fault Analysis Comparison");

    y += 82;

    // =========================================================
    // GENERATED DATE
    // =========================================================

    painter.setFont(normalFont);
    painter.setPen(bodyText);

    painter.drawText(
        QRect(
            left,
            y,
            pageWidth,
            26),
        Qt::AlignLeft |
            Qt::AlignVCenter,
        QString("Generated: %1")
            .arg(
                QDateTime::currentDateTime()
                    .toString(
                        "dd MMM yyyy  HH:mm")));

    // IMPORTANT:
    // More gap after generated date
    y += 50;

    // =========================================================
    // FILES COMPARED HEADER
    // =========================================================

    const int sectionHeaderHeight = 34;

    painter.fillRect(
        QRect(
            left,
            y,
            pageWidth,
            sectionHeaderHeight),
        primaryBlue);

    painter.setPen(primaryBlue);

    painter.drawRect(
        QRect(
            left,
            y,
            pageWidth,
            sectionHeaderHeight));

    painter.setFont(sectionFont);
    painter.setPen(Qt::white);

    painter.drawText(
        QRect(
            left + 12,
            y,
            pageWidth - 24,
            sectionHeaderHeight),
        Qt::AlignLeft |
            Qt::AlignVCenter,
        "Files Compared");

    y += sectionHeaderHeight;

    // =========================================================
    // FILE LIST
    // =========================================================

    const int fileRowHeight = 28;

    const int fileColumns = 3;

    const int fileColumnWidth =
        pageWidth / fileColumns;

    const int fileCount =
        mLoadedFiles.size();

    const int fileRows =
        fileCount == 0
            ? 1
            : (fileCount +
               fileColumns - 1) /
                  fileColumns;

    const int fileListHeight =
        qMax(
            50,
            fileRows *
                    fileRowHeight +
                18);

    const int fileListTop = y;

    painter.fillRect(
        QRect(
            left,
            fileListTop,
            pageWidth,
            fileListHeight),
        lightBlue);

    painter.setPen(borderBlue);

    painter.drawRect(
        QRect(
            left,
            fileListTop,
            pageWidth,
            fileListHeight));

    for (int i = 0;
         i < fileCount;
         ++i)
    {
        const int column =
            i / fileRows;

        const int row =
            i % fileRows;

        const int x =
            left +
            column *
                fileColumnWidth;

        const int fileY =
            fileListTop +
            8 +
            row *
                fileRowHeight;

        QString fileText =
            QString("• %1")
                .arg(
                    QFileInfo(
                        mLoadedFiles.at(i))
                        .fileName());

        drawWrapped(
            fileText,
            QRect(
                x + 10,
                fileY,
                fileColumnWidth - 20,
                fileRowHeight),
            normalFont,
            Qt::AlignLeft |
                Qt::AlignVCenter);
    }

    // MORE SPACE AFTER FILE LIST
    y += fileListHeight + 24;

    // =========================================================
    // FAULT TYPE SETTINGS
    // =========================================================

    bool hasSettings = false;

    for (const QString &file :
         mLoadedFiles)
    {
        if (mFaultSettings.contains(file) &&
            mFaultSettings.value(file).configured)
        {
            hasSettings = true;
            break;
        }
    }

    if (hasSettings)
    {
        // -----------------------------------------------------
        // SETTINGS SECTION HEADER
        // -----------------------------------------------------

        painter.fillRect(
            QRect(
                left,
                y,
                pageWidth,
                sectionHeaderHeight),
            primaryBlue);

        painter.setPen(primaryBlue);

        painter.drawRect(
            QRect(
                left,
                y,
                pageWidth,
                sectionHeaderHeight));

        painter.setFont(sectionFont);
        painter.setPen(Qt::white);

        painter.drawText(
            QRect(
                left + 12,
                y,
                pageWidth - 24,
                sectionHeaderHeight),
            Qt::AlignLeft |
                Qt::AlignVCenter,
            "Fault Type Settings");

        y += sectionHeaderHeight;

        // -----------------------------------------------------
        // SETTINGS HEADERS
        // -----------------------------------------------------

        const QStringList settingsHeaders =
            {
                "File",
                "Calculate Type",
                "Fault Type",
                "Result Type",
                "Fault Time (s)",
                "Rf (pu)",
                "Xf (pu)"
            };

        // IMPORTANT:
        // Make File column much wider.
        QList<int> settingsWidths =
            {
                850,   // File
                300,   // Calculate
                300,   // Fault
                300,   // Result
                170,   // Time
                120,   // Rf
                120    // Xf
            };

        int settingsTotalWidth = 0;

        for (int width :
             settingsWidths)
        {
            settingsTotalWidth += width;
        }

        if (settingsTotalWidth >
            pageWidth)
        {
            const double scale =
                static_cast<double>(
                    pageWidth) /
                static_cast<double>(
                    settingsTotalWidth);

            for (int &width :
                 settingsWidths)
            {
                width =
                    qMax(
                        90,
                        static_cast<int>(
                            width * scale));
            }
        }

        // Larger header
        const int settingsHeaderHeight =
            52;

        // Larger row so filename can wrap
        const int settingsRowHeight =
            86;

        auto drawSettingsHeader =
            [&]()
        {
            int x = left;

            for (int i = 0;
                 i < settingsHeaders.size();
                 ++i)
            {
                painter.fillRect(
                    QRect(
                        x,
                        y,
                        settingsWidths[i],
                        settingsHeaderHeight),
                    headerBlue);

                painter.setPen(
                    borderBlue);

                painter.drawRect(
                    QRect(
                        x,
                        y,
                        settingsWidths[i],
                        settingsHeaderHeight));

                drawWrapped(
                    settingsHeaders.at(i),
                    QRect(
                        x + 8,
                        y + 4,
                        settingsWidths[i] - 16,
                        settingsHeaderHeight - 8),
                    headerFont,
                    Qt::AlignCenter |
                        Qt::AlignVCenter);

                x +=
                    settingsWidths[i];
            }

            y +=
                settingsHeaderHeight;
        };

        drawSettingsHeader();

        // -----------------------------------------------------
        // SETTINGS ROWS
        // -----------------------------------------------------

        for (const QString &file :
             mLoadedFiles)
        {
            if (!mFaultSettings.contains(file))
                continue;

            const FaultTypeSettings settings =
                mFaultSettings.value(file);

            if (!settings.configured)
                continue;

            if (y +
                    settingsRowHeight >
                bottom - 45)
            {
                newPage();

                y = top;

                // Repeat section header
                painter.fillRect(
                    QRect(
                        left,
                        y,
                        pageWidth,
                        sectionHeaderHeight),
                    primaryBlue);

                painter.setPen(primaryBlue);

                painter.drawRect(
                    QRect(
                        left,
                        y,
                        pageWidth,
                        sectionHeaderHeight));

                painter.setFont(sectionFont);
                painter.setPen(Qt::white);

                painter.drawText(
                    QRect(
                        left + 12,
                        y,
                        pageWidth - 24,
                        sectionHeaderHeight),
                    Qt::AlignLeft |
                        Qt::AlignVCenter,
                    "Fault Type Settings");

                y += sectionHeaderHeight;

                drawSettingsHeader();
            }

            // -------------------------------------------------
            // SETTINGS VALUES
            // -------------------------------------------------

            const QStringList values =
            {
                QFileInfo(file)
            .fileName(),

                settings.calculateType,

                settings.faultType,

                settings.resultType,

                settings.faultTime,

                settings.faultResistance,

                settings.faultReactance
        };

        const int fileIndex =
            mLoadedFiles.indexOf(file);

        const QColor rowColor =
            (fileIndex % 2 == 1)
                ? alternateRow
                : white;

        int x = left;

        for (int i = 0;
             i < values.size();
             ++i)
        {
            painter.fillRect(
                QRect(
                    x,
                    y,
                    settingsWidths[i],
                    settingsRowHeight),
                rowColor);

            painter.setPen(
                borderBlue);

            painter.drawRect(
                QRect(
                    x,
                    y,
                    settingsWidths[i],
                    settingsRowHeight));

            QFont cellFont =
                normalFont;

            if (i == 0)
            {
                cellFont =
                    settingsFileFont;
            }

            drawWrapped(
                values.at(i),
                QRect(
                    x + 10,
                    y + 7,
                    settingsWidths[i] - 20,
                    settingsRowHeight - 14),
                cellFont,
                i == 0
                    ? Qt::AlignLeft |
                          Qt::AlignVCenter
                    : Qt::AlignCenter |
                          Qt::AlignVCenter);

            x +=
                settingsWidths[i];
        }

        y +=
            settingsRowHeight;

        // EXTRA GAP AFTER EACH SETTINGS ROW
        y += 8;
    }

    // EXTRA GAP BEFORE COMPARISON TABLE
    y += 22;
}

// =========================================================
// CURRENT VISUAL TABLE ORDER
// =========================================================

QHeaderView *header =
    table->horizontalHeader();

if (!header)
{
    painter.end();

    QMessageBox::critical(
        this,
        "Export Failed",
        "Unable to read the comparison table header.");

    return;
}

QList<int> visualColumns;

for (int visual = 0;
     visual < table->columnCount();
     ++visual)
{
    const int logical =
        header->logicalIndex(
            visual);

    if (logical >= 0 &&
        logical < table->columnCount())
    {
        visualColumns.append(
            logical);
    }
}

// =========================================================
// NAME COLUMN
// =========================================================

int nameColumn = -1;

for (int column = 0;
     column < table->columnCount();
     ++column)
{
    QTableWidgetItem *item =
        table->horizontalHeaderItem(
            column);

    if (item &&
        item->text().compare(
            "Name",
            Qt::CaseInsensitive) == 0)
    {
        nameColumn = column;
        break;
    }
}

// =========================================================
// COLUMN WIDTH
// =========================================================

auto columnWidth =
    [&](int column) -> int
{
    if (column == nameColumn)
        return 185;

    int width = 135;

    QTableWidgetItem *item =
        table->horizontalHeaderItem(
            column);

    if (item)
    {
        const int length =
            item->text().length();

        if (length > 32)
        {
            width = 175;
        }
        else if (length > 24)
        {
            width = 160;
        }
        else if (length > 16)
        {
            width = 145;
        }
    }

    return width;
};

// =========================================================
// BUILD GROUPS
// =========================================================

const QStringList groupsFromTable =
    mComparisonTable->groupNames();

struct PdfGroup
{
    QString name;
    QList<int> columns;
};

QList<PdfGroup> groups;

PdfGroup currentGroup;

for (int logical :
     visualColumns)
{
    if (logical == nameColumn)
        continue;

    QString groupName;

    if (logical >= 0 &&
        logical < groupsFromTable.size())
    {
        groupName =
            groupsFromTable.at(logical);
    }

    if (groupName.isEmpty())
    {
        groupName =
            "Comparison";
    }

    if (currentGroup.columns.isEmpty())
    {
        currentGroup.name =
            groupName;

        currentGroup.columns.append(
            logical);
    }
    else if (currentGroup.name ==
             groupName)
    {
        currentGroup.columns.append(
            logical);
    }
    else
    {
        groups.append(
            currentGroup);

        currentGroup =
            PdfGroup();

        currentGroup.name =
            groupName;

        currentGroup.columns.append(
            logical);
    }
}

if (!currentGroup.columns.isEmpty())
{
    groups.append(
        currentGroup);
}

// =========================================================
// CLEAN FILE NAME
// =========================================================

auto cleanGroupName =
    [&](const QString &groupName)
    -> QString
{
    for (const QString &file :
         mLoadedFiles)
    {
        const QString fileName =
            QFileInfo(file)
                .fileName();

        if (groupName == file ||
            groupName == fileName)
        {
            return fileName;
        }
    }

    return groupName;
};

// =========================================================
// HORIZONTAL CHUNKS
//
// 4 DATA COLUMNS PER BLOCK
// =========================================================

const int nameWidth =
    nameColumn >= 0
        ? columnWidth(nameColumn)
        : 0;

const int dataWidth =
    pageWidth -
    nameWidth;

const int maxColumnsPerChunk =
    4;

QList<QList<int>> chunks;

QStringList chunkNames;

for (const PdfGroup &group :
     groups)
{
    QList<int> chunk;

    int width = 0;

    for (int column :
         group.columns)
    {
        const int columnW =
            columnWidth(column);

        if (!chunk.isEmpty() &&
            (width + columnW >
                 dataWidth ||
             chunk.size() >=
                 maxColumnsPerChunk))
        {
            chunks.append(
                chunk);

            chunkNames.append(
                group.name);

            chunk.clear();

            width = 0;
        }

        chunk.append(
            column);

        width +=
            columnW;
    }

    if (!chunk.isEmpty())
    {
        chunks.append(
            chunk);

        chunkNames.append(
            group.name);
    }
}

// =========================================================
// TABLE DIMENSIONS
// =========================================================

const int groupHeaderHeight =
    42;

const int tableHeaderHeight =
    62;

const int rowHeight =
    34;

const int tableGap =
    28;

const int bottomMargin =
    38;

// =========================================================
// DRAW GROUP
// =========================================================

auto drawGroup =
    [&](const QList<int> &dataColumns,
        const QString &groupName)
{
    QList<int> columns;

    if (nameColumn >= 0)
    {
        columns.append(
            nameColumn);
    }

    for (int column :
         dataColumns)
    {
        columns.append(
            column);
    }

    // -----------------------------------------------------
    // WIDTHS
    // -----------------------------------------------------

    QList<int> widths;

    int totalWidth = 0;

    for (int column :
         columns)
    {
        const int width =
            columnWidth(column);

        widths.append(
            width);

        totalWidth +=
            width;
    }

    if (totalWidth <
            pageWidth &&
        totalWidth > 0)
    {
        const int extra =
            pageWidth -
            totalWidth;

        for (int i = 0;
             i < widths.size();
             ++i)
        {
            widths[i] +=
                (extra *
                 widths[i]) /
                totalWidth;
        }
    }

    // -----------------------------------------------------
    // CHECK PAGE SPACE
    // -----------------------------------------------------

    const int fullGroupHeight =
        groupHeaderHeight +
        tableHeaderHeight +
        table->rowCount() *
            rowHeight +
        tableGap;

    if (y +
            fullGroupHeight >
        bottom -
            bottomMargin)
    {
        newPage();

        y = top;
    }

    // -----------------------------------------------------
    // GROUP HEADER
    // -----------------------------------------------------

    painter.fillRect(
        QRect(
            left,
            y,
            pageWidth,
            groupHeaderHeight),
        primaryBlue);

    painter.setPen(
        primaryBlue);

    painter.drawRect(
        QRect(
            left,
            y,
            pageWidth,
            groupHeaderHeight));

    painter.setPen(
        Qt::white);

    drawWrapped(
        groupName,
        QRect(
            left + 12,
            y + 2,
            pageWidth - 24,
            groupHeaderHeight - 4),
        groupFont,
        Qt::AlignLeft |
            Qt::AlignVCenter);

    y +=
        groupHeaderHeight;

    // -----------------------------------------------------
    // HEADER
    // -----------------------------------------------------

    int x = left;

    for (int i = 0;
         i < columns.size();
         ++i)
    {
        const int column =
            columns.at(i);

        painter.fillRect(
            QRect(
                x,
                y,
                widths.at(i),
                tableHeaderHeight),
            headerBlue);

        painter.setPen(
            borderBlue);

        painter.drawRect(
            QRect(
                x,
                y,
                widths.at(i),
                tableHeaderHeight));

        QString headerText;

        QTableWidgetItem *headerItem =
            table->horizontalHeaderItem(
                column);

        if (headerItem)
        {
            headerText =
                headerItem->text();
        }

        drawWrapped(
            headerText,
            QRect(
                x + 7,
                y + 5,
                widths.at(i) - 14,
                tableHeaderHeight - 10),
            headerFont,
            column == nameColumn
                ? Qt::AlignLeft |
                      Qt::AlignVCenter
                : Qt::AlignCenter |
                      Qt::AlignVCenter);

        x +=
            widths.at(i);
    }

    y +=
        tableHeaderHeight;

    // -----------------------------------------------------
    // DATA
    // -----------------------------------------------------

    for (int row = 0;
         row < table->rowCount();
         ++row)
    {
        if (y +
                rowHeight >
            bottom -
                bottomMargin)
        {
            newPage();

            y = top;

            // Continued group header
            painter.fillRect(
                QRect(
                    left,
                    y,
                    pageWidth,
                    groupHeaderHeight),
                primaryBlue);

            painter.setPen(
                primaryBlue);

            painter.drawRect(
                QRect(
                    left,
                    y,
                    pageWidth,
                    groupHeaderHeight));

            painter.setPen(
                Qt::white);

            drawWrapped(
                groupName +
                    " — continued",
                QRect(
                    left + 12,
                    y + 2,
                    pageWidth - 24,
                    groupHeaderHeight - 4),
                groupFont,
                Qt::AlignLeft |
                    Qt::AlignVCenter);

            y +=
                groupHeaderHeight;

            // Repeat header
            x = left;

            for (int i = 0;
                 i < columns.size();
                 ++i)
            {
                const int column =
                    columns.at(i);

                painter.fillRect(
                    QRect(
                        x,
                        y,
                        widths.at(i),
                        tableHeaderHeight),
                    headerBlue);

                painter.setPen(
                    borderBlue);

                painter.drawRect(
                    QRect(
                        x,
                        y,
                        widths.at(i),
                        tableHeaderHeight));

                QString headerText;

                QTableWidgetItem *headerItem =
                    table->horizontalHeaderItem(
                        column);

                if (headerItem)
                {
                    headerText =
                        headerItem->text();
                }

                drawWrapped(
                    headerText,
                    QRect(
                        x + 7,
                        y + 5,
                        widths.at(i) - 14,
                        tableHeaderHeight - 10),
                    headerFont,
                    column == nameColumn
                        ? Qt::AlignLeft |
                              Qt::AlignVCenter
                        : Qt::AlignCenter |
                              Qt::AlignVCenter);

                x +=
                    widths.at(i);
            }

            y +=
                tableHeaderHeight;
        }

        // -------------------------------------------------
        // ROW
        // -------------------------------------------------

        x = left;

        for (int i = 0;
             i < columns.size();
             ++i)
        {
            const int column =
                columns.at(i);

            painter.fillRect(
                QRect(
                    x,
                    y,
                    widths.at(i),
                    rowHeight),
                row % 2 == 1
                    ? alternateRow
                    : white);

            painter.setPen(
                borderBlue);

            painter.drawRect(
                QRect(
                    x,
                    y,
                    widths.at(i),
                    rowHeight));

            QString value;

            QTableWidgetItem *item =
                table->item(
                    row,
                    column);

            if (item)
            {
                value =
                    item->text();
            }

            drawWrapped(
                value,
                QRect(
                    x + 6,
                    y + 2,
                    widths.at(i) - 12,
                    rowHeight - 4),
                valueFont,
                column == nameColumn
                    ? Qt::AlignLeft |
                          Qt::AlignVCenter
                    : Qt::AlignCenter |
                          Qt::AlignVCenter);

            x +=
                widths.at(i);
        }

        y +=
            rowHeight;
    }

    // =====================================================
    // GAP AFTER EVERY TABLE
    // =====================================================

    y += tableGap;
};

// =========================================================
// DRAW COMPARISON TABLES
// =========================================================

QString previousGroupName;

for (int i = 0;
     i < chunks.size();
     ++i)
{
    const QString originalGroupName =
        chunkNames.at(i);

    QString displayGroupName =
        cleanGroupName(
            originalGroupName);

    if (originalGroupName ==
        previousGroupName)
    {
        displayGroupName +=
            " — continued";
    }

    drawGroup(
        chunks.at(i),
        displayGroupName);

    previousGroupName =
        originalGroupName;
}

// =========================================================
// FOOTER
// =========================================================

drawFooter();

painter.end();

// =========================================================
// STATUS
// =========================================================

statusBar()->showMessage(
    QString(
        "Exported PDF: %1")
        .arg(fileName),
    5000);

QMessageBox::information(
    this,
    "Export Complete",
    QString(
        "Fault Analysis comparison was exported successfully.\n\n%1")
        .arg(fileName));
}

// =============================================================
// SHOW ABOUT
// =============================================================

void FaultAnalysisWindow::showAbout()
{
    QMessageBox::about(
        this,
        "About Comparator Tool",
        "<h3>Comparator Tool</h3>"
        "<p><b>Fault Analysis Comparison</b></p>"

        "<p>"
        "A tool for comparing fault analysis results "
        "generated from CSV files."
        "</p>"

        "<p><b>Features:</b></p>"
        "<ul>"
        "<li>Load and compare multiple fault analysis CSV files</li>"
        "<li>Select parameters for comparison</li>"
        "<li>Configure fault type and result type for each file</li>"
        "<li>View results side by side</li>"
        "<li>Rearrange comparison columns</li>"
        "<li>Export comparison results as a report</li>"
        "</ul>"
        );
}
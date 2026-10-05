#include "FaultAnalysisWindow.h"
#include "FaultComparisonSelectionWidget.h"
#include "FaultFileCard.h"
#include "FaultComparisonTableWidget.h"
#include "FaultTypeSettingsSummaryWidget.h"
#include "FaultColumnMapping.h"
#include "../parser/CsvReader.h"
#include <QCoreApplication>
#include <QApplication>
#include <QBuffer>
#include <QAction>
#include <QStandardPaths>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QSplitter>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QPainter>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDir>
#include <QStatusBar>
#include <QTimer>

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
            "Export to PDF");

    connect(
        mExportPdfAction,
        &QAction::triggered,
        this,
        &FaultAnalysisWindow::exportPdf
        );
    mExportWordAction =
        exportMenu->addAction(
            "Export to Word");

    connect(
        mExportWordAction,
        &QAction::triggered,
        this,
        &FaultAnalysisWindow::exportWord
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
    // FILES TO COMPARE & FAULT TYPE SETTINGS
    // =========================================================

    mFilesPanel =
        new QFrame(centralWidget);

    QFrame *filesPanel =
        mFilesPanel;

    filesPanel->setObjectName("filesPanel");

    filesPanel->setStyleSheet(
        "QFrame#filesPanel {"
        "    background: #F8FAFC;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 8px;"
        "}"
        );

    QVBoxLayout *filesPanelLayout =
        new QVBoxLayout(filesPanel);

    filesPanelLayout->setContentsMargins(
        14,
        12,
        14,
        12
        );

    filesPanelLayout->setSpacing(8);

    // =========================================================
    // HEADER
    // =========================================================

    QHBoxLayout *filesHeaderLayout =
        new QHBoxLayout();

    filesHeaderLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    filesHeaderLayout->setSpacing(8);

    // =========================================================
    // COLLAPSE / EXPAND BUTTON
    // =========================================================

    mFilesCollapseButton =
        new QToolButton(mFilesPanel);

    mFilesCollapseButton->setCheckable(
        true
        );

    mFilesCollapseButton->setChecked(
        mFilesSectionCollapsed
        );

    mFilesCollapseButton->setArrowType(
        mFilesSectionCollapsed
            ? Qt::RightArrow
            : Qt::DownArrow
        );

    mFilesCollapseButton->setFixedSize(
        24,
        24
        );

    mFilesCollapseButton->setCursor(
        Qt::PointingHandCursor
        );

    mFilesCollapseButton->setAutoRaise(
        true
        );

    mFilesCollapseButton->setToolTip(
        mFilesSectionCollapsed
            ? "Expand file settings"
            : "Collapse file settings"
        );

    mFilesCollapseButton->setStyleSheet(
        "QToolButton {"
        "    border: none;"
        "    background: transparent;"
        "    padding: 0px;"
        "    color: #475569;"
        "}"
        ""
        "QToolButton:hover {"
        "    background: #E2E8F0;"
        "    border-radius: 4px;"
        "}"
        );

    filesHeaderLayout->addWidget(
        mFilesCollapseButton
        );

    // ---------------------------------------------------------
    // ICON
    // ---------------------------------------------------------

    QLabel *filesIcon =
        new QLabel(filesPanel);

    filesIcon->setPixmap(
        style()->standardIcon(
                   QStyle::SP_FileIcon
                   ).pixmap(18, 18)
        );

    filesHeaderLayout->addWidget(
        filesIcon
        );

    // ---------------------------------------------------------
    // HEADING + SUBTITLE
    // ---------------------------------------------------------

    QVBoxLayout *filesTitleLayout =
        new QVBoxLayout();

    filesTitleLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    filesTitleLayout->setSpacing(1);

    mFilesHeadingLabel =
        new QLabel(
            "Files to Compare & Fault Type Settings (0 selected)",
            filesPanel
            );

    QFont headingFont =
        mFilesHeadingLabel->font();

    headingFont.setBold(true);
    headingFont.setPointSize(
        headingFont.pointSize() + 1
        );

    mFilesHeadingLabel->setFont(
        headingFont
        );

    QLabel *filesSubtitle =
        new QLabel(
            "Add CSV files and configure fault type settings for each file",
            filesPanel
            );

    filesSubtitle->setStyleSheet(
        "color: #6B7280;"
        );

    filesTitleLayout->addWidget(
        mFilesHeadingLabel
        );

    filesTitleLayout->addWidget(
        filesSubtitle
        );

    filesHeaderLayout->addLayout(
        filesTitleLayout,
        1
        );

    // ---------------------------------------------------------
    // ADD CSV BUTTON
    // ---------------------------------------------------------

    QPushButton *addCsvButton =
        new QPushButton(
            "+ Add CSV files",
            filesPanel
            );

    addCsvButton->setCursor(
        Qt::PointingHandCursor
        );

    addCsvButton->setStyleSheet(
        "QPushButton {"
        "    background: white;"
        "    color: #1769AA;"
        "    border: 1px solid #1769AA;"
        "    border-radius: 5px;"
        "    padding: 6px 12px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #F1F7FC;"
        "}"
        );

    connect(
        addCsvButton,
        &QPushButton::clicked,
        this,
        &FaultAnalysisWindow::addCsvFiles
        );

    filesHeaderLayout->addWidget(
        addCsvButton
        );

    filesPanelLayout->addLayout(
        filesHeaderLayout
        );

    // =========================================================
    // FILE CARDS SECTION
    // =========================================================

    mFileCardsSection =
        new QWidget(filesPanel);

    mFileCardsSection->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
        );

    QVBoxLayout *fileCardsSectionLayout =
        new QVBoxLayout(
            mFileCardsSection
            );

    fileCardsSectionLayout->setContentsMargins(
        0,
        0,
        0,
        0
        );

    fileCardsSectionLayout->setSpacing(0);

    // =========================================================
    // FILE CARDS SCROLL AREA
    // =========================================================

    mFileScrollArea =
        new QScrollArea(mFileCardsSection);

    mFileScrollArea->setObjectName(
        "fileCardsScrollArea"
        );

    mFileScrollArea->setWidgetResizable(
        true
        );

    mFileScrollArea->setFrameShape(
        QFrame::NoFrame
        );

    mFileScrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
        );

    mFileScrollArea->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded
        );

    mFileScrollArea->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed
        );

    mFileScrollArea->setStyleSheet(
        "QScrollArea#fileCardsScrollArea {"
        "    border: none;"
        "    background: transparent;"
        "}"
        ""
        "QScrollArea#fileCardsScrollArea > QWidget > QWidget {"
        "    background: transparent;"
        "}"
        );

    // =========================================================
    // FILE CARD CONTAINER
    // =========================================================

    mFileContainer =
        new QWidget();
    mFileContainer->setObjectName(
        "fileCardContainer"
        );

    mFileContainer->setStyleSheet(
        "QWidget#fileCardContainer {"
        "    background: transparent;"
        "}"
        );

    mFileContainer->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Preferred
        );

    mFileLayout =
        new QGridLayout(
            mFileContainer
            );

    mFileLayout->setContentsMargins(
        6,
        6,
        6,
        6
        );

    mFileLayout->setHorizontalSpacing(
        14
        );

    mFileLayout->setVerticalSpacing(
        14
        );

    for (int column = 0;
         column < 3;
         ++column)
    {
        mFileLayout->setColumnStretch(
            column,
            1
            );
    }

    mFileScrollArea->setWidget(
        mFileContainer
        );

    fileCardsSectionLayout->addWidget(
        mFileScrollArea
        );

    filesPanelLayout->addWidget(
        mFileCardsSection,
        1
        );

    // =========================================================
    // COLLAPSE / EXPAND CONNECTION
    // =========================================================

    connect(
        mFilesCollapseButton,
        &QToolButton::toggled,
        this,
        [this](bool collapsed)
        {
            mFilesSectionCollapsed = collapsed;

            if (mFilesCollapseButton)
            {
                mFilesCollapseButton->setArrowType(
                    collapsed
                        ? Qt::RightArrow
                        : Qt::DownArrow
                    );

                mFilesCollapseButton->setToolTip(
                    collapsed
                        ? "Expand file settings"
                        : "Collapse file settings"
                    );
            }

            if (mFileCardsSection)
            {
                mFileCardsSection->setVisible(
                    !collapsed
                    );
            }

            if (mComparisonSelection)
            {
                mComparisonSelection->setTopSectionCollapsed(collapsed);
            }

            if (!collapsed)
            {
                QTimer::singleShot(
                    0,
                    this,
                    [this]()
                    {
                        updateFileScrollAreaHeight();
                    }
                    );
            }

            if (mFilesPanel)
            {
                mFilesPanel->updateGeometry();
            }

            if (this->centralWidget())
                this->centralWidget()->updateGeometry();
        }
        );

    mainLayout->addWidget(
        filesPanel, 0
        );

    // =========================================================
    // MAIN CONTENT WIDGET (Comparison Dashboard)
    // =========================================================

    mContentWidget =
        new QWidget(
            centralWidget
            );

    mContentLayout =
        new QVBoxLayout(
            mContentWidget
            );

    mContentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mContentWidget->setMinimumHeight(150);
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

    mComparisonSelection =
        new FaultComparisonSelectionWidget(
            mContentWidget
            );
    mComparisonSelection->setEmptyState(true);

    comparisonLayout->addWidget(
        mComparisonSelection
        );

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

    mainLayout->addWidget(
        mContentWidget,
        1
        );

    // =========================================================
    // LAUNCH EMPTY STATE PLACEHOLDER (Centered In Empty Window)
    // =========================================================
    QWidget *welcomeWidget = new QWidget(centralWidget);
    welcomeWidget->setObjectName("welcomeWidget");
    welcomeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeWidget);
    welcomeLayout->setAlignment(Qt::AlignCenter);
    welcomeLayout->setSpacing(10);

    QLabel *welcomeIcon = new QLabel(welcomeWidget);
    welcomeIcon->setPixmap(style()->standardIcon(QStyle::SP_FileDialogContentsView).pixmap(54, 54));
    welcomeIcon->setAlignment(Qt::AlignCenter);

    QLabel *welcomeTitle = new QLabel("No files loaded", welcomeWidget);
    welcomeTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: #475569;");
    welcomeTitle->setAlignment(Qt::AlignCenter);

    QLabel *welcomeSubtitle = new QLabel("Add CSV files to begin fault analysis comparison.", welcomeWidget);
    welcomeSubtitle->setStyleSheet("font-size: 12px; color: #94A3B8;");
    welcomeSubtitle->setAlignment(Qt::AlignCenter);

    welcomeLayout->addWidget(welcomeIcon);
    welcomeLayout->addWidget(welcomeTitle);
    welcomeLayout->addWidget(welcomeSubtitle);

    mainLayout->addWidget(welcomeWidget, 1);

    // =========================================================
    // CONNECTIONS
    // =========================================================

    connect(
        mComparisonSelection,
        &FaultComparisonSelectionWidget::selectionChanged,
        this,
        &FaultAnalysisWindow::updateComparisonTable
        );

    connect(
        mComparisonSelection,
        &FaultComparisonSelectionWidget::layoutModeChanged,
        mComparisonTable,
        &FaultComparisonTableWidget::setLayoutMode
        );

    mClearFilesAction->setEnabled(
        false
        );
    mExportPdfAction->setEnabled(
        false
        );
    mExportWordAction->setEnabled(false);

    updateWindowTitle();

    mContentWidget->hide();

    mComparisonTable->setData(
        QStringList(),
        QList<QStringList>(),
        QList<QList<QStringList>>(),
        QStringList(),
        QList<QMap<QString, QString>>()
        );

    refreshFileCards();
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

        mLoadedFiles.append(
            absolutePath
            );

        // -----------------------------------------------------
        // DEFAULT DISPLAY NAME: Use compact ID (F1, F2, F3...)
        // -----------------------------------------------------
        QString defaultShortId = QString("F%1").arg(mLoadedFiles.size());
        mDisplayNames.insert(
            absolutePath,
            defaultShortId
            );

        mFileSelectionState.insert(
            absolutePath,
            true
            );

        FaultTypeSettings settings;
        settings.configured = false;
        settings.calculateType = QStringLiteral("None");
        settings.faultType = QStringLiteral("None");
        settings.resultType = QStringLiteral("None");
        settings.faultTime = QStringLiteral("0");
        settings.faultResistance = QStringLiteral("0");
        settings.faultReactance = QStringLiteral("0");

        mFaultSettings.insert(
            absolutePath,
            settings
            );
    }

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

    mDefaultColumns =
        allColumns;

    refreshFileCards();
    rebuildComparisonSelection();

    mClearFilesAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );

    mExportPdfAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );
    mExportWordAction->setEnabled(!mLoadedFiles.isEmpty());

    updateWindowTitle();

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

    mFaultSettings.remove(
        filePath
        );

    mDisplayNames.remove(
        filePath
        );

    mFileSelectionState.remove(
        filePath
        );

    // Re-index remaining unaliased files so IDs remain sequential: F1, F2, F3...
    for (int i = 0; i < mLoadedFiles.size(); ++i)
    {
        const QString &path = mLoadedFiles.at(i);
        QString currentName = mDisplayNames.value(path, "");
        if (currentName.startsWith("F") && currentName.mid(1).toInt() > 0)
        {
            mDisplayNames[path] = QString("F%1").arg(i + 1);
        }
    }

    refreshFileCards();

    mClearFilesAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );

    mExportPdfAction->setEnabled(
        !mLoadedFiles.isEmpty()
        );
    mExportWordAction->setEnabled(!mLoadedFiles.isEmpty());

    updateWindowTitle();
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
    mDisplayNames.clear();
    mFileSelectionState.clear();

    refreshFileCards();

    if (mComparisonSelection)
    {
        mComparisonSelection->setAvailableColumns(
            QStringList()
            );
    }

    if (mContentWidget)
    {
        mContentWidget->hide();
    }

    QWidget *welcomeWidget = centralWidget()->findChild<QWidget *>("welcomeWidget");
    if (welcomeWidget)
    {
        welcomeWidget->show();
    }

    mClearFilesAction->setEnabled(
        false
        );

    mExportPdfAction->setEnabled(
        false
        );
    mExportWordAction->setEnabled(!mLoadedFiles.isEmpty());

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
    {
        return;
    }

    const QStringList selectedColumns =
        mComparisonSelection->selectedColumns();

    QStringList fileNames;
    QList<QStringList> selectedHeaders;
    QList<QList<QStringList>> selectedRows;

    const QList<QMap<QString, QString>>
        allMappings =
        buildColumnMappings();

    QList<QMap<QString, QString>>
        selectedMappings;

    for (int i = 0;
         i < mLoadedFiles.size();
         ++i)
    {
        const QString filePath =
            mLoadedFiles.at(i);

        if (!mFileSelectionState.value(
                filePath,
                true))
        {
            continue;
        }

        // Passes short name F1, F2 or user alias directly to the table headers
        fileNames.append(
            mDisplayNames.value(
                filePath,
                QString("F%1").arg(i + 1)
                )
            );

        if (i < mCsvHeaders.size())
        {
            selectedHeaders.append(
                mCsvHeaders.at(i)
                );
        }
        else
        {
            selectedHeaders.append(
                QStringList()
                );
        }

        if (i < mCsvRows.size())
        {
            selectedRows.append(
                mCsvRows.at(i)
                );
        }
        else
        {
            selectedRows.append(
                QList<QStringList>()
                );
        }

        if (i < allMappings.size())
        {
            selectedMappings.append(
                allMappings.at(i)
                );
        }
        else
        {
            selectedMappings.append(
                QMap<QString, QString>()
                );
        }
    }

    mComparisonTable->setData(
        fileNames,
        selectedHeaders,
        selectedRows,
        selectedColumns,
        selectedMappings
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

        if (fileIndex < mCsvHeaders.size())
        {
            const QStringList &rawHeaders =
                mCsvHeaders.at(fileIndex);

            for (const QString &rawColumn :
                 rawHeaders)
            {
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
// REBUILD COMPARISON SELECTION
// =============================================================

void FaultAnalysisWindow::rebuildComparisonSelection()
{
    const bool hasFiles = !mLoadedFiles.isEmpty();

    if (mContentWidget)
    {
        mContentWidget->setVisible(hasFiles);
    }

    QWidget *welcomeWidget = centralWidget()->findChild<QWidget *>("welcomeWidget");
    if (welcomeWidget)
    {
        welcomeWidget->setVisible(!hasFiles);
    }

    if (mComparisonSelection)
    {
        mComparisonSelection->setEmptyState(!hasFiles);
    }

    if (!hasFiles)
    {
        updateComparisonTable();
        return;
    }

    QStringList previousSelection =
        mComparisonSelection->selectedColumns();

    QStringList displayColumns;

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

    for (const QStringList &headers :
         mCsvHeaders)
    {
        for (const QString &column :
             headers)
        {
            if (column.trimmed().isEmpty())
                continue;

            if (column == "Include CDPs" ||
                column == "Name" ||
                column == "AC Mag. (kA)")
                continue;

            if (!displayColumns.contains(column))
            {
                displayColumns.append(column);
            }
        }
    }

    mComparisonSelection->setAvailableColumns(
        displayColumns
        );

    QStringList restoredSelection;

    for (const QString &column :
         previousSelection)
    {
        if (displayColumns.contains(column))
        {
            restoredSelection.append(column);
        }
    }

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

QString FaultAnalysisWindow::displayColumnName(
    const QString &filePath,
    const QString &columnName
    ) const
{
    Q_UNUSED(filePath);
    return columnName;
}

// =============================================================
// EXPORT FAULT ANALYSIS AS PDF (LARGE IPSA ICON, PADDED & VISIBLE)
// =============================================================
void FaultAnalysisWindow::exportPdf()
{
    if (!mComparisonTable || !mComparisonSelection)
    {
        QMessageBox::warning(this, "Export PDF", "Comparison data is not available.");
        return;
    }

    QTableWidget *table = mComparisonTable->tableWidget();
    if (!table)
    {
        QMessageBox::information(this, "Export PDF", "Comparison data is not available.");
        return;
    }

    // 1. Gather active files, live aliases, and LIVE card settings
    const QList<FaultFileCard*> activeCards = findChildren<FaultFileCard*>();
    QSet<QString> disabledPaths;
    QMap<QString, FaultTypeSettings> liveCardSettings;

    for (FaultFileCard *c : activeCards)
    {
        if (!c) continue;

        if (!c->alias().isEmpty())
        {
            mDisplayNames[c->filePath()] = c->alias();
        }

        liveCardSettings[c->filePath()] = c->settings();

        QCheckBox *cb = c->findChild<QCheckBox*>();
        if (cb && !cb->isChecked())
        {
            disabledPaths.insert(c->filePath());
        }
    }

    QStringList activeFiles;
    for (const QString &fPath : mLoadedFiles)
    {
        if (!disabledPaths.contains(fPath))
        {
            activeFiles.append(fPath);
        }
    }
    if (activeFiles.isEmpty())
    {
        activeFiles = mLoadedFiles;
    }

    TableLayoutMode activeMode = mComparisonSelection->currentLayoutMode();
    const bool isStacked = (activeMode == TableLayoutMode::Stacked);

    // 2. Metadata Map
    struct FileInfoMeta {
        QString originalName;
        QString alias;
        QString effectiveName;
    };

    QMap<QString, FileInfoMeta> fileMetaMap;
    for (int i = 0; i < activeFiles.size(); ++i)
    {
        const QString &fPath = activeFiles.at(i);
        QString orig = QFileInfo(fPath).fileName();
        QString aliasStr = "";

        if (mDisplayNames.contains(fPath) && mDisplayNames.value(fPath) != orig)
            aliasStr = mDisplayNames.value(fPath).trimmed();

        QString defaultId = QString("F%1").arg(i + 1);
        QString effective = aliasStr.isEmpty() ? defaultId : aliasStr;
        fileMetaMap.insert(fPath, FileInfoMeta{orig, aliasStr, effective});
    }

    // 3. Extract Busbar Names
    int nameLogicalCol = -1;
    for (int col = 0; col < table->columnCount(); ++col)
    {
        QTableWidgetItem *item = table->horizontalHeaderItem(col);
        if (item && item->text().compare("Name", Qt::CaseInsensitive) == 0)
        {
            nameLogicalCol = col;
            break;
        }
    }

    QStringList busbarNames;
    if (nameLogicalCol >= 0 && table->rowCount() > 0)
    {
        for (int r = 0; r < table->rowCount(); ++r)
        {
            QTableWidgetItem *nItem = table->item(r, nameLogicalCol);
            QString bName = (nItem && !nItem->text().trimmed().isEmpty())
                                ? nItem->text().trimmed()
                                : QString("Busbar %1").arg(r + 1);
            busbarNames.append(bName);
        }
    }
    else
    {
        for (int fileIdx = 0; fileIdx < mLoadedFiles.size(); ++fileIdx)
        {
            if (fileIdx >= mCsvHeaders.size() || fileIdx >= mCsvRows.size()) continue;
            const QStringList &headers = mCsvHeaders.at(fileIdx);
            int nIdx = headers.indexOf("Name");
            if (nIdx < 0) continue;
            for (const QStringList &row : mCsvRows.at(fileIdx))
            {
                if (nIdx < row.size())
                {
                    QString bName = row.at(nIdx).trimmed();
                    if (!bName.isEmpty() && !busbarNames.contains(bName))
                        busbarNames.append(bName);
                }
            }
        }
    }

    // =========================================================
    // 4. PRESERVE SCREEN ORDER: TABLES & COLUMNS
    // =========================================================
    struct ParamCol {
        QString fileDisplayName;
        int fileIndex;
    };

    struct ParamTableData {
        QString paramName;
        QList<ParamCol> columns;
        QStringList rowBusbarNames; // Preserves individual table row reorder
    };

    QList<ParamTableData> allParamTables;

    if (isStacked)
    {
        QScrollArea *scrollArea = mComparisonTable->findChild<QScrollArea*>("stackedScrollArea");
        QWidget *container = scrollArea ? scrollArea->widget() : nullptr;
        QLayout *stackedLayout = container ? container->layout() : nullptr;

        if (stackedLayout)
        {
            for (int i = 0; i < stackedLayout->count(); ++i)
            {
                QLayoutItem *it = stackedLayout->itemAt(i);
                if (!it || !it->widget()) continue;
                QWidget *w = it->widget();
                if (w->objectName() != "stackedBlockCard") continue;

                QLabel *lbl = w->findChild<QLabel*>();
                QTableWidget *miniTable = w->findChild<QTableWidget*>("stackedMiniTable");
                if (!lbl || !miniTable) continue;

                QString cardParam = lbl->property("paramName").toString();
                if (cardParam.isEmpty()) cardParam = lbl->text().remove(":::").trimmed();

                ParamTableData pData;
                pData.paramName = cardParam;

                // Capture each individual stacked card table's exact row order
                for (int r = 0; r < miniTable->rowCount(); ++r)
                {
                    QTableWidgetItem *nItem = miniTable->item(r, 1);
                    if (nItem && !nItem->text().trimmed().isEmpty())
                        pData.rowBusbarNames.append(nItem->text().trimmed());
                    else if (r < busbarNames.size())
                        pData.rowBusbarNames.append(busbarNames.at(r));
                }

                QHeaderView *miniHeader = miniTable->horizontalHeader();
                for (int v = 2; v < miniTable->columnCount(); ++v)
                {
                    int logicalCol = miniHeader ? miniHeader->logicalIndex(v) : v;
                    QTableWidgetItem *hItem = miniTable->horizontalHeaderItem(logicalCol);
                    if (!hItem) continue;

                    QString colTitle = hItem->text().trimmed();
                    for (int f = 0; f < mLoadedFiles.size(); ++f)
                    {
                        const QString &fPath = mLoadedFiles.at(f);
                        if (disabledPaths.contains(fPath)) continue;

                        const FileInfoMeta meta = fileMetaMap.value(fPath);
                        if (colTitle == meta.effectiveName || colTitle == meta.originalName || colTitle == QString("F%1").arg(f + 1))
                        {
                            pData.columns.append(ParamCol{colTitle, f});
                            break;
                        }
                    }
                }

                if (!pData.columns.isEmpty())
                    allParamTables.append(pData);
            }
        }
    }
    else
    {
        QHeaderView *hHeader = table->horizontalHeader();
        const QStringList tableGroups = mComparisonTable->groupNames();
        QStringList visualGroupSequence;

        for (int v = 0; v < table->columnCount(); ++v)
        {
            int lCol = hHeader ? hHeader->logicalIndex(v) : v;
            if (lCol == nameLogicalCol || lCol < 0 || lCol >= table->columnCount()) continue;

            QString gName = (lCol < tableGroups.size()) ? tableGroups.at(lCol).trimmed() : "";
            if (!gName.isEmpty() && !visualGroupSequence.contains(gName))
            {
                visualGroupSequence.append(gName);
            }
        }

        for (const QString &gName : visualGroupSequence)
        {
            ParamTableData pData;
            pData.paramName = gName;
            pData.rowBusbarNames = busbarNames;

            for (int v = 0; v < table->columnCount(); ++v)
            {
                int lCol = hHeader ? hHeader->logicalIndex(v) : v;
                if (lCol == nameLogicalCol || lCol < 0 || lCol >= table->columnCount()) continue;

                QString colGroup = (lCol < tableGroups.size()) ? tableGroups.at(lCol).trimmed() : "";
                if (colGroup != gName) continue;

                QTableWidgetItem *hItem = table->horizontalHeaderItem(lCol);
                if (!hItem) continue;

                QString colTitle = hItem->text().trimmed();
                for (int f = 0; f < mLoadedFiles.size(); ++f)
                {
                    const QString &fPath = mLoadedFiles.at(f);
                    if (disabledPaths.contains(fPath)) continue;

                    const FileInfoMeta meta = fileMetaMap.value(fPath);
                    if (meta.effectiveName == colTitle || meta.originalName == colTitle || colTitle == QString("F%1").arg(f + 1))
                    {
                        pData.columns.append(ParamCol{colTitle, f});
                        break;
                    }
                }
            }

            if (!pData.columns.isEmpty())
                allParamTables.append(pData);
        }
    }

    if (allParamTables.isEmpty())
    {
        QStringList selParams = mComparisonSelection->selectedColumns();
        selParams.removeAll("Name");
        for (const QString &pName : selParams)
        {
            ParamTableData pData;
            pData.paramName = pName;
            pData.rowBusbarNames = busbarNames;
            for (int f = 0; f < mLoadedFiles.size(); ++f)
            {
                const QString &fPath = mLoadedFiles.at(f);
                if (disabledPaths.contains(fPath)) continue;
                pData.columns.append(ParamCol{fileMetaMap.value(fPath).effectiveName, f});
            }
            if (!pData.columns.isEmpty())
                allParamTables.append(pData);
        }
    }

    if (allParamTables.isEmpty() || activeFiles.isEmpty())
    {
        QMessageBox::information(this, "Export PDF", "There is no comparison data or parameter selected to export.");
        return;
    }

    // 5. File Dialog
    QString defaultFileName = QString("Fault_Analysis_Report_%1.pdf")
                                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Fault Analysis as PDF",
        QDir::home().filePath(defaultFileName),
        "PDF Files (*.pdf)");

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
        fileName += ".pdf";

    // 6. Printer Setup (A4 Landscape, High-DPI 300)
    QPdfWriter pdfWriter(fileName);
    pdfWriter.setResolution(300);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Landscape);
    pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);

    QPainter painter(&pdfWriter);
    if (!painter.isActive())
    {
        QMessageBox::critical(this, "Export Failed", "Unable to create the PDF file.");
        return;
    }

    // Symmetrical Margins
    const int totalWidth = pdfWriter.width();
    const int totalHeight = pdfWriter.height();
    const int marginX = qRound(14.0 / 25.4 * 300.0);
    const int marginY = qRound(12.0 / 25.4 * 300.0);

    const int left = marginX;
    const int right = totalWidth - marginX;
    const int pageWidth = right - left;
    const int top = marginY;
    const int bottom = totalHeight - marginY;

    // Typography
    QFont titleFont("Segoe UI", 16, QFont::Bold);
    QFont sectionFont("Segoe UI", 11.5, QFont::Bold);
    QFont bannerFont("Segoe UI", 10.5, QFont::Bold);
    QFont colHeaderFont("Segoe UI", 9.5, QFont::Bold);
    QFont cellFont("Segoe UI", 8.8, QFont::Normal);
    QFont metaFont("Segoe UI", 9.5, QFont::Normal);
    QFont footerFont("Segoe UI", 8.5, QFont::Normal);

    QFontMetrics fmColHeader(colHeaderFont);
    QFontMetrics fmCell(cellFont);

    // Color Palette
    const QColor navyDark("#0F172A");
    const QColor brandBlue("#1769AA");
    const QColor bannerBg("#EAF3FF");
    const QColor bannerText("#0F3D64");
    const QColor headerBg("#F8FAFC");
    const QColor borderGray("#CBD5E1");
    const QColor borderDark("#94A3B8");
    const QColor textDark("#1E293B");
    const QColor textMuted("#64748B");
    const QColor altRowBg("#F8FAFC");

    int pageNumber = 1;

    auto drawFooter = [&]() {
        painter.save();
        painter.setFont(footerFont);
        painter.setPen(textMuted);
        painter.drawText(QRect(left + pageWidth / 2, bottom - 35, pageWidth / 2, 30),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString("%1").arg(pageNumber));
        painter.restore();
    };

    auto triggerNewPage = [&]() {
        drawFooter();
        ++pageNumber;
        pdfWriter.newPage();
    };

    int curY = top + 15;

    // =========================================================
    // PAGE 1: HEADER & ENLARGED IPSA BRAND LOGO
    // =========================================================
    QPixmap logoPixmap;
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList possibleLogoPaths = {
        "C:/Projects/TAComparator-main/TAComparator-main/resources/icons/ipsa.png",
        "C:\\Projects\\TAComparator-main\\TAComparator-main\\resources\\icons\\ipsa.png",
        "C:/Projects/TAComparator-main/TAComparator-main/resources/icons/ipsa.ico",
        "C:\\Projects\\TAComparator-main\\TAComparator-main\\resources\\icons\\ipsa.ico",
        appDir + "/../resources/icons/ipsa.png",
        appDir + "/resources/icons/ipsa.png",
        ":/icons/ipsa.png",
        ":/images/ipsa.png",
        ":/resources/icons/ipsa.png",
        ":/app_icon.png"
    };

    for (const QString &p : possibleLogoPaths)
    {
        if (QFile::exists(p) && logoPixmap.load(p))
            break;
    }

    if (logoPixmap.isNull())
    {
        QIcon appIcon = windowIcon();
        if (appIcon.isNull() && qApp)
            appIcon = qApp->windowIcon();

        if (!appIcon.isNull())
            logoPixmap = appIcon.pixmap(512, 512);
    }

    const int logoW = 280;
    const int logoH = 100;
    QRect logoRect(right - logoW, curY - 14, logoW, logoH);

    if (!logoPixmap.isNull())
    {
        painter.drawPixmap(logoRect, logoPixmap.scaled(logoRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor("#0F3D64"), 2.5));
        painter.setBrush(QColor("#EAF3FF"));
        painter.drawRoundedRect(logoRect, 8, 8);
        painter.setFont(QFont("Segoe UI", 24, QFont::Bold));
        painter.setPen(QColor("#0F3D64"));
        painter.drawText(logoRect, Qt::AlignCenter, "IPSA");
        painter.restore();
    }

    painter.setFont(titleFont);
    painter.setPen(navyDark);
    QFontMetrics fmTitle = painter.fontMetrics();
    painter.drawText(left, curY + fmTitle.ascent(), "Fault Analysis Comparison Report");
    curY += fmTitle.height() + 16;

    painter.setFont(metaFont);
    painter.setPen(textMuted);
    QString layoutDesc = isStacked ? "Stacked (Parameter Blocks)" : "Side-by-Side";
    QString metaString = QString("Generated: %1   |   Layout: %2   |   Datasets: %3   |   Busbars: %4")
                             .arg(QDateTime::currentDateTime().toString("dd MMM yyyy, hh:mm AP"))
                             .arg(layoutDesc)
                             .arg(activeFiles.size())
                             .arg(busbarNames.size());
    QFontMetrics fmMeta = painter.fontMetrics();
    painter.drawText(left, curY + fmMeta.ascent(), metaString);
    curY += fmMeta.height() + 16;

    painter.setPen(QPen(borderGray, 1.5));
    painter.drawLine(left, curY, right, curY);
    curY += 20;

    // =========================================================
    // LOADED DATASETS LIST: SPACED TEXT ROWS WITH FULLY VISIBLE DOTS
    // =========================================================
    painter.setFont(sectionFont);
    painter.setPen(brandBlue);
    QFontMetrics fmSec = painter.fontMetrics();
    painter.drawText(left, curY + fmSec.ascent(), "Loaded Datasets");
    curY += fmSec.height() + 14;

    const int vLineH = 48;

    for (int i = 0; i < activeFiles.size(); ++i)
    {
        if (curY + vLineH > bottom - 60)
        {
            triggerNewPage();
            curY = top + 20;
        }

        const FileInfoMeta meta = fileMetaMap.value(activeFiles.at(i));

        painter.setFont(colHeaderFont);
        painter.setPen(textMuted);
        painter.drawText(QRect(left + 8, curY, 55, vLineH), Qt::AlignLeft | Qt::AlignVCenter, QString("%1.").arg(i + 1));

        painter.setFont(colHeaderFont);
        painter.setPen(navyDark);
        QString idPart = meta.effectiveName + " ";
        int aWidth = painter.fontMetrics().horizontalAdvance(idPart);
        painter.drawText(QRect(left + 65, curY, aWidth, vLineH), Qt::AlignLeft | Qt::AlignVCenter, idPart);

        painter.setFont(cellFont);
        painter.setPen(textMuted);
        QRect origRect(left + 65 + aWidth, curY, pageWidth - (80 + aWidth), vLineH);
        painter.drawText(origRect, Qt::AlignLeft | Qt::AlignVCenter, QString("(%1)").arg(meta.originalName));

        curY += vLineH;
    }

    curY += 24;

    // =========================================================
    // CONFIGURED FAULT TYPE SETTINGS
    // =========================================================
    auto getEffectiveSettings = [&](const QString &file) -> FaultTypeSettings {
        if (liveCardSettings.contains(file))
            return liveCardSettings.value(file);
        if (mFaultSettings.contains(file))
            return mFaultSettings.value(file);
        return FaultTypeSettings();
    };

    auto isConfigured = [&](const QString &file) -> bool {
        FaultTypeSettings s = getEffectiveSettings(file);

        bool hasCalc   = (!s.calculateType.isEmpty() && s.calculateType != "None");
        bool hasFault  = (!s.faultType.isEmpty() && s.faultType != "None");
        bool hasResult = (!s.resultType.isEmpty() && s.resultType != "None");

        bool okRf = false, okXf = false, okTime = false;
        double dRf = s.faultResistance.toDouble(&okRf);
        double dXf = s.faultReactance.toDouble(&okXf);
        double dTime = s.faultTime.toDouble(&okTime);

        bool hasRf   = (okRf && std::abs(dRf) > 1e-6);
        bool hasXf   = (okXf && std::abs(dXf) > 1e-6);
        bool hasTime = (okTime && std::abs(dTime) > 1e-6);

        return hasCalc || hasFault || hasResult || hasRf || hasXf || hasTime || s.configured;
    };

    bool hasAnyConfigured = false;
    for (const QString &file : activeFiles)
    {
        if (isConfigured(file))
        {
            hasAnyConfigured = true;
            break;
        }
    }

    if (hasAnyConfigured)
    {
        if (curY + 180 > bottom - 60)
        {
            triggerNewPage();
            curY = top + 20;
        }

        painter.setFont(sectionFont);
        painter.setPen(brandBlue);
        QFontMetrics fmF = painter.fontMetrics();
        painter.drawText(left, curY + fmF.ascent(), "Configured Fault Type Settings");
        curY += fmF.height() + 12;

        QStringList sHeaders = {"Dataset / Alias", "Calculate Type", "Fault Type", "Result Type", "Fault Time (s)", "Rf (pu)", "Xf (pu)"};
        QList<int> sWidths = {240, 140, 150, 140, 105, 90, 90};

        int sumSW = 0;
        for (int w : sWidths) sumSW += w;
        for (int &w : sWidths) w = (w * pageWidth) / sumSW;

        const int minSettingsRowH = 60;
        const int sHeaderH = qMax(minSettingsRowH, fmColHeader.height() + 28);
        const int sRowH = qMax(minSettingsRowH, fmCell.height() + 28);

        int sx = left;
        for (int i = 0; i < sHeaders.size(); ++i)
        {
            int cellW = (i == sHeaders.size() - 1) ? (right - sx) : sWidths[i];
            QRect r(sx, curY, cellW, sHeaderH);
            painter.fillRect(r, headerBg);
            painter.setPen(borderGray);
            painter.drawRect(r);
            painter.setFont(colHeaderFont);
            painter.setPen(navyDark);
            painter.drawText(r.adjusted(4, 0, -4, 0), Qt::AlignCenter | Qt::TextWordWrap, sHeaders[i]);
            sx += cellW;
        }
        curY += sHeaderH;

        for (int idx = 0; idx < activeFiles.size(); ++idx)
        {
            const QString &file = activeFiles[idx];
            if (!isConfigured(file))
                continue;

            const FaultTypeSettings s = getEffectiveSettings(file);

            if (curY + sRowH > bottom - 60)
            {
                triggerNewPage();
                curY = top + 20;
            }

            const FileInfoMeta meta = fileMetaMap.value(file);

            QStringList sValues = {
                meta.effectiveName,
                s.calculateType.isEmpty() ? "None" : s.calculateType,
                s.faultType.isEmpty() ? "None" : s.faultType,
                s.resultType.isEmpty() ? "None" : s.resultType,
                s.faultTime.isEmpty() ? "0.0000" : s.faultTime,
                s.faultResistance.isEmpty() ? "0.0000" : s.faultResistance,
                s.faultReactance.isEmpty() ? "0.0000" : s.faultReactance
            };

            sx = left;
            for (int i = 0; i < sValues.size(); ++i)
            {
                int cellW = (i == sValues.size() - 1) ? (right - sx) : sWidths[i];
                QRect r(sx, curY, cellW, sRowH);
                painter.setPen(borderGray);
                painter.drawRect(r);
                painter.setFont(cellFont);
                painter.setPen(textDark);
                painter.drawText(r.adjusted(8, 0, -8, 0), (i == 0 ? Qt::AlignLeft : Qt::AlignCenter) | Qt::AlignVCenter, sValues[i]);
                sx += cellW;
            }
            curY += sRowH;
        }

        curY += 30;
    }

    // =========================================================
    // 7. CELL EXTRACTOR & COLOR MATCHING
    // =========================================================
    const QList<QMap<QString, QString>> allMappings = buildColumnMappings();

    auto getFileValue = [&](int fileIndex, const QString &bName, const QString &param) -> QString {
        if (fileIndex < 0 || fileIndex >= mCsvHeaders.size() || fileIndex >= mCsvRows.size())
            return "";

        QString rawColumn = param;
        if (fileIndex < allMappings.size())
        {
            const QMap<QString, QString> &mapping = allMappings.at(fileIndex);
            if (mapping.contains(param))
                rawColumn = mapping.value(param);
        }

        const QStringList &headers = mCsvHeaders.at(fileIndex);
        int colIdx = headers.indexOf(rawColumn);
        int nameIdx = headers.indexOf("Name");
        if (colIdx < 0 || nameIdx < 0)
            return "";

        for (const QStringList &row : mCsvRows.at(fileIndex))
        {
            if (nameIdx < row.size() && row.at(nameIdx).trimmed() == bName.trimmed())
            {
                if (colIdx < row.size())
                    return row.at(colIdx).trimmed();
                return "";
            }
        }
        return "";
    };

    struct CompColor { QColor bg; QColor fg; };
    const QVector<CompColor> compPalette = {
        { QColor("#DCFCE7"), QColor("#166534") }, // Green
        { QColor("#FEF3C7"), QColor("#92400E") }, // Yellow
        { QColor("#DBEAFE"), QColor("#1E40AF") }, // Blue
        { QColor("#EDE9FE"), QColor("#6D28D9") }, // Purple
        { QColor("#FFE4E6"), QColor("#9F1239") }, // Rose
        { QColor("#CFFAFE"), QColor("#155E75") }  // Cyan
    };

    auto computeRowColors = [&](const QStringList &values) -> QVector<CompColor> {
        QVector<CompColor> result(values.size(), { QColor(Qt::white), textDark });
        QVector<int> groupIndexes(values.size(), -1);
        QVector<QString> reps;
        QVector<int> sizes;

        for (int i = 0; i < values.size(); ++i)
        {
            const QString v = values.at(i).trimmed();
            if (v.isEmpty()) continue;

            int g = -1;
            bool ok1 = false;
            double d1 = v.toDouble(&ok1);

            for (int k = 0; k < reps.size(); ++k)
            {
                bool ok2 = false;
                double d2 = reps[k].toDouble(&ok2);
                if (ok1 && ok2 && std::abs(d1 - d2) < 1e-6)
                {
                    g = k;
                    break;
                }
                else if (v == reps[k])
                {
                    g = k;
                    break;
                }
            }

            if (g == -1)
            {
                g = reps.size();
                reps.append(v);
                sizes.append(0);
            }
            ++sizes[g];
            groupIndexes[i] = g;
        }

        for (int i = 0; i < values.size(); ++i)
        {
            if (values.at(i).trimmed().isEmpty())
            {
                result[i] = { QColor("#F3F4F6"), textMuted }; // Unavailable
            }
            else
            {
                int g = groupIndexes[i];
                if (g >= 0 && sizes[g] >= 2)
                {
                    result[i] = compPalette.at(g % compPalette.size());
                }
            }
        }
        return result;
    };

    const int numColWidth = 55;

    int maxBusbarTextW = fmColHeader.horizontalAdvance("Busbar Name");
    for (const QString &bName : busbarNames)
    {
        maxBusbarTextW = qMax(maxBusbarTextW, fmCell.horizontalAdvance(bName));
    }
    const int nameColWidth = qBound(310, maxBusbarTextW + 80, 440);

    const int minDefaultCellH = 60;
    const int cellRowH = qMax(minDefaultCellH, fmCell.height() + 28);
    const int gapAfterTable = 40;
    const int topLevelH = 92;
    const int subLevelH = 64;
    const int totalHeaderH = topLevelH + subLevelH;
    const int availableDataWidth = pageWidth - (numColWidth + nameColWidth);

    // Draw Section Header: Comparison Tables
    if (curY + 40 > bottom - 60)
    {
        triggerNewPage();
        curY = top + 20;
    }
    painter.setFont(sectionFont);
    painter.setPen(brandBlue);
    QFontMetrics fmSecComp = painter.fontMetrics();
    painter.drawText(left, curY + fmSecComp.ascent(), "Comparison Tables");
    curY += fmSecComp.height() + 16;

    // =========================================================
    // 8A. MODE: SIDE-BY-SIDE (SMART CHUNKING FOR MANY DATASETS)
    // =========================================================
    if (!isStacked)
    {
        const int maxSideColsPerChunk = 7;

        QList<ParamTableData> chunkedParamTables;
        for (const ParamTableData &p : allParamTables)
        {
            if (p.columns.size() <= maxSideColsPerChunk)
            {
                chunkedParamTables.append(p);
            }
            else
            {
                const int totalCols = p.columns.size();
                for (int start = 0; start < totalCols; start += maxSideColsPerChunk)
                {
                    int end = qMin(start + maxSideColsPerChunk, totalCols);

                    ParamTableData pPart;
                    pPart.paramName = p.paramName; // Keeps clean parameter title without part suffix
                    pPart.rowBusbarNames = p.rowBusbarNames;
                    pPart.columns = p.columns.mid(start, end - start);
                    chunkedParamTables.append(pPart);
                }
            }
        }

        int pIdx = 0;
        while (pIdx < chunkedParamTables.size())
        {
            QList<ParamTableData> rowParams;
            int totalColsOnRow = 0;

            while (pIdx < chunkedParamTables.size())
            {
                const ParamTableData &candidate = chunkedParamTables[pIdx];
                int nextCount = candidate.columns.size();

                if (!rowParams.isEmpty() && (totalColsOnRow + nextCount) > maxSideColsPerChunk)
                {
                    break;
                }

                rowParams.append(candidate);
                totalColsOnRow += nextCount;
                ++pIdx;

                if (totalColsOnRow >= maxSideColsPerChunk)
                    break;
            }

            int totalFiles = qMax(1, totalColsOnRow);
            int baseColW = availableDataWidth / totalFiles;

            auto drawSideBySideRowHeaders = [&]() {
                // #
                QRect numBox(left, curY, numColWidth, totalHeaderH);
                painter.fillRect(numBox, headerBg);
                painter.setPen(borderGray);
                painter.drawRect(numBox);
                painter.setFont(colHeaderFont);
                painter.setPen(navyDark);
                painter.drawText(numBox, Qt::AlignCenter, "#");

                // Busbar Name
                QRect nameBox(left + numColWidth, curY, nameColWidth, totalHeaderH);
                painter.fillRect(nameBox, headerBg);
                painter.setPen(borderGray);
                painter.drawRect(nameBox);
                painter.setFont(colHeaderFont);
                painter.setPen(navyDark);
                painter.drawText(nameBox.adjusted(16, 0, -16, 0), Qt::AlignLeft | Qt::AlignVCenter, "Busbar Name");

                painter.setPen(QPen(borderDark, 1.5));
                painter.drawLine(nameBox.right(), nameBox.top(), nameBox.right(), nameBox.bottom());

                // Parameter Banners and Column Headers
                int hx = left + numColWidth + nameColWidth;
                int globalColIndex = 0;

                for (int pi = 0; pi < rowParams.size(); ++pi)
                {
                    const ParamTableData &param = rowParams[pi];
                    int groupW = 0;
                    for (int c = 0; c < param.columns.size(); ++c)
                    {
                        if (globalColIndex + c == totalFiles - 1)
                            groupW += availableDataWidth - (baseColW * (totalFiles - 1));
                        else
                            groupW += baseColW;
                    }

                    QRect bannerR(hx, curY, groupW, topLevelH);
                    painter.fillRect(bannerR, bannerBg);
                    painter.setPen(QPen(borderDark, 1.0));
                    painter.drawRect(bannerR);

                    painter.setFont(bannerFont);
                    painter.setPen(bannerText);
                    QRect bannerTextR = bannerR.adjusted(6, 4, -6, -4);
                    painter.drawText(bannerTextR, Qt::AlignCenter | Qt::TextWordWrap, param.paramName);

                    int subX = hx;
                    for (int c = 0; c < param.columns.size(); ++c)
                    {
                        int thisW = (globalColIndex == totalFiles - 1)
                        ? (availableDataWidth - (baseColW * (totalFiles - 1)))
                        : baseColW;

                        QRect subH(subX, curY + topLevelH, thisW, subLevelH);
                        painter.fillRect(subH, headerBg);
                        painter.setPen(borderGray);
                        painter.drawRect(subH);

                        painter.setFont(colHeaderFont);
                        painter.setPen(navyDark);
                        QRect subHText = subH.adjusted(4, 2, -4, -2);
                        painter.drawText(subHText, Qt::AlignCenter | Qt::TextWordWrap, param.columns[c].fileDisplayName);

                        painter.setPen(QPen(borderDark, 1.0));
                        painter.drawLine(subH.right(), subH.top(), subH.right(), subH.bottom());

                        subX += thisW;
                        ++globalColIndex;
                    }

                    painter.setPen(QPen(borderDark, 2.0));
                    painter.drawLine(hx + groupW, curY, hx + groupW, curY + totalHeaderH);

                    hx += groupW;
                }

                curY += totalHeaderH;
            };

            if (curY + totalHeaderH + (cellRowH * 2) > bottom - 50)
            {
                triggerNewPage();
                curY = top + 20;
            }

            drawSideBySideRowHeaders();

            // Data Rows
            for (int r = 0; r < busbarNames.size(); ++r)
            {
                if (curY + cellRowH > bottom - 50)
                {
                    triggerNewPage();
                    curY = top + 20;
                    drawSideBySideRowHeaders();
                }

                const QString &bName = busbarNames.at(r);
                int rx = left;

                // #
                QRect nCell(rx, curY, numColWidth, cellRowH);
                painter.fillRect(nCell, altRowBg);
                painter.setPen(borderGray);
                painter.drawRect(nCell);
                painter.setFont(cellFont);
                painter.setPen(textMuted);
                painter.drawText(nCell, Qt::AlignCenter, QString::number(r + 1));
                rx += numColWidth;

                // Busbar Name
                QRect bCell(rx, curY, nameColWidth, cellRowH);
                painter.fillRect(bCell, Qt::white);
                painter.setPen(borderGray);
                painter.drawRect(bCell);
                painter.setFont(cellFont);
                painter.setPen(textDark);
                painter.drawText(bCell.adjusted(16, 0, -16, 0), Qt::AlignLeft | Qt::AlignVCenter, bName);

                painter.setPen(QPen(borderDark, 1.5));
                painter.drawLine(bCell.right(), bCell.top(), bCell.right(), bCell.bottom());
                rx += nameColWidth;

                int globalColIndex = 0;
                for (int pi = 0; pi < rowParams.size(); ++pi)
                {
                    const ParamTableData &param = rowParams[pi];

                    QStringList paramValues;
                    for (int c = 0; c < param.columns.size(); ++c)
                    {
                        paramValues.append(getFileValue(param.columns[c].fileIndex, bName, param.paramName));
                    }

                    QVector<CompColor> paramColors = computeRowColors(paramValues);

                    for (int c = 0; c < param.columns.size(); ++c)
                    {
                        int thisW = (globalColIndex == totalFiles - 1)
                        ? (availableDataWidth - (baseColW * (totalFiles - 1)))
                        : baseColW;

                        QRect dCell(rx, curY, thisW, cellRowH);
                        QString val = paramValues.at(c);

                        painter.fillRect(dCell, paramColors[c].bg);
                        painter.setPen(borderGray);
                        painter.drawRect(dCell);

                        painter.setFont(cellFont);
                        painter.setPen(paramColors[c].fg);
                        painter.drawText(dCell.adjusted(4, 0, -4, 0), Qt::AlignCenter | Qt::AlignVCenter, val);

                        bool isGroupEdge = (c == param.columns.size() - 1);
                        painter.setPen(QPen(borderDark, isGroupEdge ? 2.0 : 1.0));
                        painter.drawLine(dCell.right(), dCell.top(), dCell.right(), dCell.bottom());

                        rx += thisW;
                        ++globalColIndex;
                    }
                }

                painter.setPen(QPen(borderDark, 1.5));
                painter.drawLine(right, curY, right, curY + cellRowH);

                curY += cellRowH;
            }

            curY += gapAfterTable;
        }
    }
    // =========================================================
    // 8B. MODE: STACKED (SEPARATE STANDALONE FULL-WIDTH CARDS)
    // =========================================================
    else
    {
        for (const ParamTableData &param : allParamTables)
        {
            const int totalCols = param.columns.size();
            if (totalCols == 0) continue;

            const int maxColsPerChunk = 12;

            // Use the specific row order of this stacked card table
            const QStringList &cardBusbarNames = !param.rowBusbarNames.isEmpty() ? param.rowBusbarNames : busbarNames;

            for (int chunkStart = 0; chunkStart < totalCols; chunkStart += maxColsPerChunk)
            {
                int chunkEnd = qMin(chunkStart + maxColsPerChunk, totalCols);
                int colCount = chunkEnd - chunkStart;

                int baseColW = availableDataWidth / colCount;
                auto getColW = [&](int idx) -> int {
                    if (idx == colCount - 1)
                        return availableDataWidth - (baseColW * (colCount - 1));
                    return baseColW;
                };

                auto drawStackedHeaders = [&]() {
                    QRect bannerRect(left, curY, pageWidth, topLevelH);
                    painter.fillRect(bannerRect, bannerBg);
                    painter.setPen(QPen(borderDark, 1.5));
                    painter.drawRect(bannerRect);

                    painter.setFont(bannerFont);
                    painter.setPen(bannerText);

                    // Clean parameter title without Part suffix
                    QString titleText = param.paramName;
                    QRect bannerTextRect = bannerRect.adjusted(16, 4, -16, -4);
                    painter.drawText(bannerTextRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, titleText);
                    curY += topLevelH;

                    int hx = left;

                    QRect numH(hx, curY, numColWidth, subLevelH);
                    painter.fillRect(numH, headerBg);
                    painter.setPen(borderGray);
                    painter.drawRect(numH);
                    painter.setFont(colHeaderFont);
                    painter.setPen(navyDark);
                    painter.drawText(numH, Qt::AlignCenter, "#");
                    hx += numColWidth;

                    QRect nameH(hx, curY, nameColWidth, subLevelH);
                    painter.fillRect(nameH, headerBg);
                    painter.setPen(borderGray);
                    painter.drawRect(nameH);
                    painter.setFont(colHeaderFont);
                    painter.setPen(navyDark);
                    painter.drawText(nameH.adjusted(16, 0, -16, 0), Qt::AlignLeft | Qt::AlignVCenter, "Busbar Name");

                    painter.setPen(QPen(borderDark, 1.5));
                    painter.drawLine(nameH.right(), nameH.top(), nameH.right(), nameH.bottom());
                    hx += nameColWidth;

                    for (int i = 0; i < colCount; ++i)
                    {
                        int colW = getColW(i);
                        QRect fH(hx, curY, colW, subLevelH);
                        painter.fillRect(fH, headerBg);
                        painter.setPen(borderGray);
                        painter.drawRect(fH);

                        painter.setFont(colHeaderFont);
                        painter.setPen(navyDark);
                        QRect fHText = fH.adjusted(4, 2, -4, -2);
                        painter.drawText(fHText, Qt::AlignCenter | Qt::TextWordWrap, param.columns[chunkStart + i].fileDisplayName);

                        painter.setPen(QPen(borderDark, 1.0));
                        painter.drawLine(fH.right(), fH.top(), fH.right(), fH.bottom());

                        hx += colW;
                    }
                    curY += subLevelH;
                };

                if (curY + totalHeaderH + (cellRowH * 2) > bottom - 50)
                {
                    triggerNewPage();
                    curY = top + 20;
                }

                drawStackedHeaders();

                // Rows rendered using the specific order of this card
                for (int r = 0; r < cardBusbarNames.size(); ++r)
                {
                    if (curY + cellRowH > bottom - 50)
                    {
                        triggerNewPage();
                        curY = top + 20;
                        drawStackedHeaders();
                    }

                    const QString &bName = cardBusbarNames.at(r);
                    int rx = left;

                    QRect nCell(rx, curY, numColWidth, cellRowH);
                    painter.fillRect(nCell, altRowBg);
                    painter.setPen(borderGray);
                    painter.drawRect(nCell);
                    painter.setFont(cellFont);
                    painter.setPen(textMuted);
                    painter.drawText(nCell, Qt::AlignCenter, QString::number(r + 1));
                    rx += numColWidth;

                    QRect bCell(rx, curY, nameColWidth, cellRowH);
                    painter.fillRect(bCell, Qt::white);
                    painter.setPen(borderGray);
                    painter.drawRect(bCell);
                    painter.setFont(cellFont);
                    painter.setPen(textDark);
                    painter.drawText(bCell.adjusted(16, 0, -16, 0), Qt::AlignLeft | Qt::AlignVCenter, bName);

                    painter.setPen(QPen(borderDark, 1.5));
                    painter.drawLine(bCell.right(), bCell.top(), bCell.right(), bCell.bottom());
                    rx += nameColWidth;

                    QStringList rowValues;
                    for (int i = 0; i < colCount; ++i)
                    {
                        rowValues.append(getFileValue(param.columns[chunkStart + i].fileIndex, bName, param.paramName));
                    }

                    QVector<CompColor> rowColors = computeRowColors(rowValues);

                    for (int i = 0; i < colCount; ++i)
                    {
                        int colW = getColW(i);
                        QRect dCell(rx, curY, colW, cellRowH);
                        QString val = rowValues.at(i);

                        painter.fillRect(dCell, rowColors[i].bg);
                        painter.setPen(borderGray);
                        painter.drawRect(dCell);

                        painter.setFont(cellFont);
                        painter.setPen(rowColors[i].fg);
                        painter.drawText(dCell.adjusted(4, 0, -4, 0), Qt::AlignCenter | Qt::AlignVCenter, val);

                        bool isChunkEdge = (i == colCount - 1);
                        painter.setPen(QPen(borderDark, isChunkEdge ? 1.5 : 1.0));
                        painter.drawLine(dCell.right(), dCell.top(), dCell.right(), dCell.bottom());

                        rx += colW;
                    }

                    painter.setPen(QPen(borderDark, 1.5));
                    painter.drawLine(right, curY, right, curY + cellRowH);

                    curY += cellRowH;
                }

                curY += gapAfterTable;
            }
        }
    }

    drawFooter();
    painter.end();

    statusBar()->showMessage(QString("Successfully exported report: %1").arg(fileName), 6000);
    QMessageBox::information(this, "Export Complete",
                             QString("Comparison report exported successfully to:\n\n%1").arg(fileName));
}
// =============================================================
// EXPORT FAULT ANALYSIS AS WORD (.DOC) - PROPER LOGO & OVERWRITE
// =============================================================
void FaultAnalysisWindow::exportWord()
{
    if (!mComparisonTable || !mComparisonSelection)
    {
        QMessageBox::warning(this, "Export to Word", "Comparison data is not available.");
        return;
    }

    QTableWidget *table = mComparisonTable->tableWidget();
    if (!table)
    {
        QMessageBox::information(this, "Export to Word", "Comparison data is not available.");
        return;
    }

    // 1. Gather active files and settings
    const QList<FaultFileCard*> activeCards = findChildren<FaultFileCard*>();
    QSet<QString> disabledPaths;
    QMap<QString, FaultTypeSettings> liveCardSettings;

    for (FaultFileCard *c : activeCards)
    {
        if (!c) continue;
        if (!c->alias().isEmpty())
            mDisplayNames[c->filePath()] = c->alias();

        liveCardSettings[c->filePath()] = c->settings();

        QCheckBox *cb = c->findChild<QCheckBox*>();
        if (cb && !cb->isChecked())
            disabledPaths.insert(c->filePath());
    }

    QStringList activeFiles;
    for (const QString &fPath : mLoadedFiles)
    {
        if (!disabledPaths.contains(fPath))
            activeFiles.append(fPath);
    }
    if (activeFiles.isEmpty())
        activeFiles = mLoadedFiles;

    TableLayoutMode activeMode = mComparisonSelection->currentLayoutMode();
    const bool isStacked = (activeMode == TableLayoutMode::Stacked);

    // 2. Metadata
    struct FileInfoMeta {
        QString originalName;
        QString alias;
        QString effectiveName;
    };

    QMap<QString, FileInfoMeta> fileMetaMap;
    for (int i = 0; i < activeFiles.size(); ++i)
    {
        const QString &fPath = activeFiles.at(i);
        QString orig = QFileInfo(fPath).fileName();
        QString aliasStr = "";

        if (mDisplayNames.contains(fPath) && mDisplayNames.value(fPath) != orig)
            aliasStr = mDisplayNames.value(fPath).trimmed();

        QString defaultId = QString("F%1").arg(i + 1);
        QString effective = aliasStr.isEmpty() ? defaultId : aliasStr;
        fileMetaMap.insert(fPath, FileInfoMeta{orig, aliasStr, effective});
    }

    // 3. Extract Busbars
    int nameLogicalCol = -1;
    for (int col = 0; col < table->columnCount(); ++col)
    {
        QTableWidgetItem *item = table->horizontalHeaderItem(col);
        if (item && item->text().compare("Name", Qt::CaseInsensitive) == 0)
        {
            nameLogicalCol = col;
            break;
        }
    }

    QStringList busbarNames;
    if (nameLogicalCol >= 0 && table->rowCount() > 0)
    {
        for (int r = 0; r < table->rowCount(); ++r)
        {
            QTableWidgetItem *nItem = table->item(r, nameLogicalCol);
            QString bName = (nItem && !nItem->text().trimmed().isEmpty())
                                ? nItem->text().trimmed()
                                : QString("Busbar %1").arg(r + 1);
            busbarNames.append(bName);
        }
    }
    else
    {
        for (int fileIdx = 0; fileIdx < mLoadedFiles.size(); ++fileIdx)
        {
            if (fileIdx >= mCsvHeaders.size() || fileIdx >= mCsvRows.size()) continue;
            const QStringList &headers = mCsvHeaders.at(fileIdx);
            int nIdx = headers.indexOf("Name");
            if (nIdx < 0) continue;
            for (const QStringList &row : mCsvRows.at(fileIdx))
            {
                if (nIdx < row.size())
                {
                    QString bName = row.at(nIdx).trimmed();
                    if (!bName.isEmpty() && !busbarNames.contains(bName))
                        busbarNames.append(bName);
                }
            }
        }
    }

    // 4. Resolve Parameters & Column Structure in exact visual order
    struct ParamCol {
        QString fileDisplayName;
        int fileIndex;
    };

    struct ParamTableData {
        QString paramName;
        QList<ParamCol> columns;
        QStringList rowBusbarNames; // Preserves individual table row reorder
    };

    QList<ParamTableData> allParamTables;

    if (isStacked)
    {
        QScrollArea *scrollArea = mComparisonTable->findChild<QScrollArea*>("stackedScrollArea");
        QWidget *container = scrollArea ? scrollArea->widget() : nullptr;
        QLayout *stackedLayout = container ? container->layout() : nullptr;

        if (stackedLayout)
        {
            for (int i = 0; i < stackedLayout->count(); ++i)
            {
                QLayoutItem *it = stackedLayout->itemAt(i);
                if (!it || !it->widget()) continue;
                QWidget *w = it->widget();
                if (w->objectName() != "stackedBlockCard") continue;

                QLabel *lbl = w->findChild<QLabel*>();
                QTableWidget *miniTable = w->findChild<QTableWidget*>("stackedMiniTable");
                if (!lbl || !miniTable) continue;

                QString cardParam = lbl->property("paramName").toString();
                if (cardParam.isEmpty()) cardParam = lbl->text().remove(":::").trimmed();

                ParamTableData pData;
                pData.paramName = cardParam;

                // Capture each individual stacked card table's exact row order
                for (int r = 0; r < miniTable->rowCount(); ++r)
                {
                    QTableWidgetItem *nItem = miniTable->item(r, 1);
                    if (nItem && !nItem->text().trimmed().isEmpty())
                        pData.rowBusbarNames.append(nItem->text().trimmed());
                    else if (r < busbarNames.size())
                        pData.rowBusbarNames.append(busbarNames.at(r));
                }

                QHeaderView *miniHeader = miniTable->horizontalHeader();
                for (int v = 2; v < miniTable->columnCount(); ++v)
                {
                    int logicalCol = miniHeader ? miniHeader->logicalIndex(v) : v;
                    QTableWidgetItem *hItem = miniTable->horizontalHeaderItem(logicalCol);
                    if (!hItem) continue;

                    QString colTitle = hItem->text().trimmed();
                    for (int f = 0; f < mLoadedFiles.size(); ++f)
                    {
                        const QString &fPath = mLoadedFiles.at(f);
                        if (disabledPaths.contains(fPath)) continue;

                        const FileInfoMeta meta = fileMetaMap.value(fPath);
                        if (colTitle == meta.effectiveName || colTitle == meta.originalName || colTitle == QString("F%1").arg(f + 1))
                        {
                            pData.columns.append(ParamCol{colTitle, f});
                            break;
                        }
                    }
                }

                if (!pData.columns.isEmpty())
                    allParamTables.append(pData);
            }
        }
    }
    else
    {
        QHeaderView *hHeader = table->horizontalHeader();
        const QStringList tableGroups = mComparisonTable->groupNames();
        QStringList visualGroupSequence;

        for (int v = 0; v < table->columnCount(); ++v)
        {
            int lCol = hHeader ? hHeader->logicalIndex(v) : v;
            if (lCol == nameLogicalCol || lCol < 0 || lCol >= table->columnCount()) continue;

            QString gName = (lCol < tableGroups.size()) ? tableGroups.at(lCol).trimmed() : "";
            if (!gName.isEmpty() && !visualGroupSequence.contains(gName))
                visualGroupSequence.append(gName);
        }

        for (const QString &gName : visualGroupSequence)
        {
            ParamTableData pData;
            pData.paramName = gName;
            pData.rowBusbarNames = busbarNames;

            for (int v = 0; v < table->columnCount(); ++v)
            {
                int lCol = hHeader ? hHeader->logicalIndex(v) : v;
                if (lCol == nameLogicalCol || lCol < 0 || lCol >= table->columnCount()) continue;

                QString colGroup = (lCol < tableGroups.size()) ? tableGroups.at(lCol).trimmed() : "";
                if (colGroup != gName) continue;

                QTableWidgetItem *hItem = table->horizontalHeaderItem(lCol);
                if (!hItem) continue;

                QString colTitle = hItem->text().trimmed();
                for (int f = 0; f < mLoadedFiles.size(); ++f)
                {
                    const QString &fPath = mLoadedFiles.at(f);
                    if (disabledPaths.contains(fPath)) continue;

                    const FileInfoMeta meta = fileMetaMap.value(fPath);
                    if (meta.effectiveName == colTitle || meta.originalName == colTitle || colTitle == QString("F%1").arg(f + 1))
                    {
                        pData.columns.append(ParamCol{colTitle, f});
                        break;
                    }
                }
            }

            if (!pData.columns.isEmpty())
                allParamTables.append(pData);
        }
    }

    if (allParamTables.isEmpty())
    {
        QStringList selParams = mComparisonSelection->selectedColumns();
        selParams.removeAll("Name");
        for (const QString &pName : selParams)
        {
            ParamTableData pData;
            pData.paramName = pName;
            pData.rowBusbarNames = busbarNames;
            for (int f = 0; f < mLoadedFiles.size(); ++f)
            {
                const QString &fPath = mLoadedFiles.at(f);
                if (disabledPaths.contains(fPath)) continue;
                pData.columns.append(ParamCol{fileMetaMap.value(fPath).effectiveName, f});
            }
            if (!pData.columns.isEmpty())
                allParamTables.append(pData);
        }
    }

    if (allParamTables.isEmpty() || activeFiles.isEmpty())
    {
        QMessageBox::information(this, "Export to Word", "There is no comparison data selected to export.");
        return;
    }

    // 5. File Dialog with Default Name
    QString defaultFileName = QString("Fault_Analysis_Report_%1.doc")
                                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmm"));

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Fault Analysis to Word",
        QDir::home().filePath(defaultFileName),
        "Word Document (*.doc);;All Files (*.*)");

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".doc", Qt::CaseInsensitive) && !fileName.endsWith(".docx", Qt::CaseInsensitive))
        fileName += ".doc";

    // 6. Safe File Overwrite / Open Check
    QFile docFile(fileName);

    // If file exists, check whether Microsoft Word has locked it
    if (docFile.exists())
    {
        if (!docFile.open(QIODevice::ReadWrite))
        {
            QMessageBox::warning(this, "File In Use",
                                 QString("Unable to overwrite '%1'.\n\nIf the file is currently open in Microsoft Word, please close it and try exporting again.")
                                     .arg(QFileInfo(fileName).fileName()));
            return;
        }
        docFile.close();
    }

    if (!docFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QMessageBox::critical(this, "Export Failed",
                              QString("Failed to create file '%1': %2").arg(fileName, docFile.errorString()));
        return;
    }

    // 7. Value & Color Computation
    const QList<QMap<QString, QString>> allMappings = buildColumnMappings();

    auto getFileValue = [&](int fileIndex, const QString &bName, const QString &param) -> QString {
        if (fileIndex < 0 || fileIndex >= mCsvHeaders.size() || fileIndex >= mCsvRows.size())
            return "";

        QString rawColumn = param;
        if (fileIndex < allMappings.size())
        {
            const QMap<QString, QString> &mapping = allMappings.at(fileIndex);
            if (mapping.contains(param))
                rawColumn = mapping.value(param);
        }

        const QStringList &headers = mCsvHeaders.at(fileIndex);
        int colIdx = headers.indexOf(rawColumn);
        int nameIdx = headers.indexOf("Name");
        if (colIdx < 0 || nameIdx < 0)
            return "";

        for (const QStringList &row : mCsvRows.at(fileIndex))
        {
            if (nameIdx < row.size() && row.at(nameIdx).trimmed() == bName.trimmed())
            {
                if (colIdx < row.size())
                    return row.at(colIdx).trimmed();
                return "";
            }
        }
        return "";
    };

    struct CompColor { QString bg; QString fg; };
    const QVector<CompColor> compPalette = {
        { "#DCFCE7", "#166534" }, // Green
        { "#FEF3C7", "#92400E" }, // Yellow
        { "#DBEAFE", "#1E40AF" }, // Blue
        { "#EDE9FE", "#6D28D9" }, // Purple
        { "#FFE4E6", "#9F1239" }, // Rose
        { "#CFFAFE", "#155E75" }  // Cyan
    };

    auto computeRowColors = [&](const QStringList &values) -> QVector<CompColor> {
        QVector<CompColor> result(values.size(), { "#FFFFFF", "#1E293B" });
        QVector<int> groupIndexes(values.size(), -1);
        QVector<QString> reps;
        QVector<int> sizes;

        for (int i = 0; i < values.size(); ++i)
        {
            const QString v = values.at(i).trimmed();
            if (v.isEmpty()) continue;

            int g = -1;
            bool ok1 = false;
            double d1 = v.toDouble(&ok1);

            for (int k = 0; k < reps.size(); ++k)
            {
                bool ok2 = false;
                double d2 = reps[k].toDouble(&ok2);
                if (ok1 && ok2 && std::abs(d1 - d2) < 1e-6)
                {
                    g = k;
                    break;
                }
                else if (v == reps[k])
                {
                    g = k;
                    break;
                }
            }

            if (g == -1)
            {
                g = reps.size();
                reps.append(v);
                sizes.append(0);
            }
            ++sizes[g];
            groupIndexes[i] = g;
        }

        for (int i = 0; i < values.size(); ++i)
        {
            if (values.at(i).trimmed().isEmpty())
            {
                result[i] = { "#F3F4F6", "#64748B" };
            }
            else
            {
                int g = groupIndexes[i];
                if (g >= 0 && sizes[g] >= 2)
                    result[i] = compPalette.at(g % compPalette.size());
            }
        }
        return result;
    };

    // 8. Base64 Encode Logo
    QString logoBase64;
    const QStringList possibleLogoPaths = {
        "C:/Projects/TAComparator-main/TAComparator-main/resources/icons/ipsa.png",
        "C:\\Projects\\TAComparator-main\\TAComparator-main\\resources\\icons\\ipsa.png",
        "C:/Projects/TAComparator-main/TAComparator-main/resources/icons/ipsa.ico",
        "C:\\Projects\\TAComparator-main\\TAComparator-main\\resources\\icons\\ipsa.ico",
        "resources/icons/ipsa.png",
        ":/icons/ipsa.png",
        ":/images/ipsa.png",
        ":/resources/icons/ipsa.png"
    };

    for (const QString &p : possibleLogoPaths)
    {
        if (QFile::exists(p))
        {
            QFile imgFile(p);
            if (imgFile.open(QIODevice::ReadOnly))
            {
                logoBase64 = QString::fromLatin1(imgFile.readAll().toBase64());
                imgFile.close();
                break;
            }
        }
    }

    if (logoBase64.isEmpty())
    {
        QIcon appIcon = windowIcon();
        if (!appIcon.isNull())
        {
            QString tempPath = QDir::temp().filePath("ipsa_logo_temp.png");
            if (appIcon.pixmap(256, 256).save(tempPath, "PNG"))
            {
                QFile tf(tempPath);
                if (tf.open(QIODevice::ReadOnly))
                {
                    logoBase64 = QString::fromLatin1(tf.readAll().toBase64());
                    tf.close();
                }
                QFile::remove(tempPath);
            }
        }
    }

    // 9. Write HTML Payload
    QTextStream out(&docFile);
    out.setEncoding(QStringConverter::Utf8);

    out << "<!DOCTYPE html>\n<html xmlns:o='urn:schemas-microsoft-com:office:office' xmlns:w='urn:schemas-microsoft-com:office:word'>\n<head>\n<meta charset=\"utf-8\">\n";
    out << "<style>\n";
    out << "  @page WordSection1 { size: 11.0in 8.5in; mso-page-orientation: landscape; margin: 0.5in; }\n";
    out << "  div.WordSection1 { page: WordSection1; }\n";
    out << "  body { font-family: 'Segoe UI', Calibri, Arial, sans-serif; color: #1E293B; margin: 0; padding: 0; }\n";
    out << "  h1 { color: #0F3D64; font-size: 19pt; margin: 0; padding: 0; font-weight: bold; }\n";
    out << "  .meta-text { color: #64748B; font-size: 9.5pt; margin-top: 6px; }\n";
    out << "  .divider { border-bottom: 2px solid #CBD5E1; margin: 14px 0 20px 0; }\n";
    out << "  h2.section-title { color: #125B94; font-size: 12pt; font-weight: bold; margin: 20px 0 10px 0; }\n";
    out << "  table { border-collapse: collapse; width: 100%; margin-bottom: 22px; table-layout: auto; mso-table-lspace: 0pt; mso-table-rspace: 0pt; }\n";
    out << "  th, td { border: 1px solid #CBD5E1; padding: 6px 10px; font-size: 9.5pt; text-align: center; vertical-align: middle; }\n";
    out << "  th.header-bg { background-color: #F8FAFC; color: #0F172A; font-weight: bold; }\n";
    out << "  th.banner-bg { background-color: #EAF3FF; color: #0F3D64; font-size: 10.5pt; font-weight: bold; text-align: center; }\n";
    out << "  td.left-align { text-align: left; }\n";
    out << "  td.alt-bg { background-color: #F8FAFC; color: #64748B; font-weight: 500; }\n";
    out << "</style>\n</head>\n<body>\n<div class=\"WordSection1\">\n";

    // Header Table
    out << "<table style=\"border: none; width: 100%; margin-bottom: 0;\">\n<tr style=\"border: none;\">\n";
    out << "<td style=\"border: none; text-align: left; vertical-align: middle; padding: 0;\">\n";
    out << "  <h1>Fault Analysis Comparison Report</h1>\n";
    QString layoutDesc = isStacked ? "Stacked (Parameter Blocks)" : "Side-by-Side";
    out << QString("  <div class=\"meta-text\">Generated: %1 &nbsp;&nbsp;|&nbsp;&nbsp; Layout: %2 &nbsp;&nbsp;|&nbsp;&nbsp; Datasets: %3 &nbsp;&nbsp;|&nbsp;&nbsp; Busbars: %4</div>\n")
               .arg(QDateTime::currentDateTime().toString("dd MMM yyyy, hh:mm AP"))
               .arg(layoutDesc)
               .arg(activeFiles.size())
               .arg(busbarNames.size());
    out << "</td>\n";

    // Header Right: Scaled Logo (105px x 38px)
    out << "<td style=\"border: none; text-align: right; vertical-align: middle; width: 120px; padding: 0;\">\n";
    if (!logoBase64.isEmpty())
    {
        out << QString("  <img src=\"data:image/png;base64,%1\" width=\"105\" height=\"38\" style=\"width: 105px; height: 38px; max-width: 105px; max-height: 38px; display: inline-block;\" alt=\"IPSA\" />\n").arg(logoBase64);
    }
    else
    {
        out << "  <div style=\"background-color: #EAF3FF; border: 1.5px solid #0F3D64; border-radius: 4px; padding: 4px 12px; font-weight: bold; color: #0F3D64; font-size: 13pt; display: inline-block;\">IPSA</div>\n";
    }
    out << "</td>\n</tr>\n</table>\n";
    out << "<div class=\"divider\"></div>\n";

    // Loaded Datasets Table
    out << "<h2 class=\"section-title\">Loaded Datasets</h2>\n";
    out << "<table>\n";
    out << "<tr><th class=\"header-bg\" style=\"width: 45px;\">#</th><th class=\"header-bg\" style=\"width: 130px;\">ID / Alias</th><th class=\"header-bg left-align\">Original File Name</th></tr>\n";
    for (int i = 0; i < activeFiles.size(); ++i)
    {
        const FileInfoMeta meta = fileMetaMap.value(activeFiles.at(i));
        QString bgCol = (i % 2 == 0) ? "#FFFFFF" : "#F8FAFC";
        out << QString("<tr style=\"background-color: %1;\">").arg(bgCol);
        out << QString("<td class=\"alt-bg\"><b>%1.</b></td>").arg(i + 1);
        out << QString("<td><b style=\"color: #0F172A;\">%1</b></td>").arg(meta.effectiveName);
        out << QString("<td class=\"left-align\" style=\"color: #64748B;\">(%1)</td>").arg(meta.originalName);
        out << "</tr>\n";
    }
    out << "</table>\n";

    // Configured Fault Type Settings
    auto getEffectiveSettings = [&](const QString &file) -> FaultTypeSettings {
        if (liveCardSettings.contains(file))
            return liveCardSettings.value(file);
        if (mFaultSettings.contains(file))
            return mFaultSettings.value(file);
        return FaultTypeSettings();
    };

    auto isConfigured = [&](const QString &file) -> bool {
        FaultTypeSettings s = getEffectiveSettings(file);
        bool hasCalc   = (!s.calculateType.isEmpty() && s.calculateType != "None");
        bool hasFault  = (!s.faultType.isEmpty() && s.faultType != "None");
        bool hasResult = (!s.resultType.isEmpty() && s.resultType != "None");
        bool okRf = false, okXf = false, okTime = false;
        double dRf = s.faultResistance.toDouble(&okRf);
        double dXf = s.faultReactance.toDouble(&okXf);
        double dTime = s.faultTime.toDouble(&okTime);
        bool hasRf   = (okRf && std::abs(dRf) > 1e-6);
        bool hasXf   = (okXf && std::abs(dXf) > 1e-6);
        bool hasTime = (okTime && std::abs(dTime) > 1e-6);
        return hasCalc || hasFault || hasResult || hasRf || hasXf || hasTime || s.configured;
    };

    bool hasAnyConfigured = false;
    for (const QString &file : activeFiles)
    {
        if (isConfigured(file))
        {
            hasAnyConfigured = true;
            break;
        }
    }

    if (hasAnyConfigured)
    {
        out << "<h2 class=\"section-title\">Configured Fault Type Settings</h2>\n<table>\n";
        out << "<tr>";
        out << "<th class=\"header-bg\">Dataset / Alias</th>";
        out << "<th class=\"header-bg\">Calculate Type</th>";
        out << "<th class=\"header-bg\">Fault Type</th>";
        out << "<th class=\"header-bg\">Result Type</th>";
        out << "<th class=\"header-bg\">Fault Time (s)</th>";
        out << "<th class=\"header-bg\">Rf (pu)</th>";
        out << "<th class=\"header-bg\">Xf (pu)</th>";
        out << "</tr>\n";

        for (const QString &file : activeFiles)
        {
            if (!isConfigured(file)) continue;
            FaultTypeSettings s = getEffectiveSettings(file);
            const FileInfoMeta meta = fileMetaMap.value(file);
            out << "<tr>";
            out << QString("<td class=\"left-align\"><b>%1</b></td>").arg(meta.effectiveName);
            out << QString("<td>%1</td>").arg(s.calculateType.isEmpty() ? "None" : s.calculateType);
            out << QString("<td>%1</td>").arg(s.faultType.isEmpty() ? "None" : s.faultType);
            out << QString("<td>%1</td>").arg(s.resultType.isEmpty() ? "None" : s.resultType);
            out << QString("<td>%1</td>").arg(s.faultTime.isEmpty() ? "0.0000" : s.faultTime);
            out << QString("<td>%1</td>").arg(s.faultResistance.isEmpty() ? "0.0000" : s.faultResistance);
            out << QString("<td>%1</td>").arg(s.faultReactance.isEmpty() ? "0.0000" : s.faultReactance);
            out << "</tr>\n";
        }
        out << "</table>\n";
    }

    // Comparison Data Tables
    out << "<h2 class=\"section-title\">Comparison Tables</h2>\n";

    if (!isStacked)
    {
        int pIdx = 0;
        const int maxMergeCols = 6;

        while (pIdx < allParamTables.size())
        {
            QList<ParamTableData> rowParams;
            int totalColsOnRow = 0;

            while (pIdx < allParamTables.size())
            {
                const ParamTableData &candidate = allParamTables[pIdx];
                int nextCount = candidate.columns.size();

                if (!rowParams.isEmpty() && (totalColsOnRow + nextCount) > maxMergeCols)
                    break;

                rowParams.append(candidate);
                totalColsOnRow += nextCount;
                ++pIdx;

                if (totalColsOnRow >= maxMergeCols)
                    break;
            }

            out << "<table>\n";
            // Header Row 1: Parameter Banners
            out << "<tr>";
            out << "<th class=\"header-bg\" rowspan=\"2\" style=\"width: 45px;\">#</th>";
            out << "<th class=\"header-bg left-align\" rowspan=\"2\" style=\"width: 200px;\">Busbar Name</th>";
            for (const ParamTableData &param : rowParams)
            {
                out << QString("<th class=\"banner-bg\" colspan=\"%1\">%2</th>")
                .arg(param.columns.size())
                    .arg(param.paramName);
            }
            out << "</tr>\n";

            // Header Row 2: File Headers
            out << "<tr>";
            for (const ParamTableData &param : rowParams)
            {
                for (const ParamCol &col : param.columns)
                    out << QString("<th class=\"header-bg\">%1</th>").arg(col.fileDisplayName);
            }
            out << "</tr>\n";

            // Data Rows
            for (int r = 0; r < busbarNames.size(); ++r)
            {
                const QString &bName = busbarNames.at(r);
                out << "<tr>";
                out << QString("<td class=\"alt-bg\">%1</td>").arg(r + 1);
                out << QString("<td class=\"left-align\"><b>%1</b></td>").arg(bName);

                for (const ParamTableData &param : rowParams)
                {
                    QStringList paramValues;
                    for (const ParamCol &col : param.columns)
                        paramValues.append(getFileValue(col.fileIndex, bName, param.paramName));

                    QVector<CompColor> colors = computeRowColors(paramValues);

                    for (int c = 0; c < param.columns.size(); ++c)
                    {
                        out << QString("<td style=\"background-color: %1; color: %2;\">%3</td>")
                        .arg(colors[c].bg)
                            .arg(colors[c].fg)
                            .arg(paramValues.at(c));
                    }
                }
                out << "</tr>\n";
            }
            out << "</table>\n";
        }
    }
    else
    {
        for (const ParamTableData &param : allParamTables)
        {
            // Use the specific row order of this stacked card table
            const QStringList &cardBusbarNames = !param.rowBusbarNames.isEmpty() ? param.rowBusbarNames : busbarNames;

            out << "<table>\n";
            out << "<tr>";
            out << QString("<th class=\"banner-bg\" colspan=\"%1\">%2</th>")
                       .arg(2 + param.columns.size())
                       .arg(param.paramName);
            out << "</tr>\n";

            out << "<tr>";
            out << "<th class=\"header-bg\" style=\"width: 45px;\">#</th>";
            out << "<th class=\"header-bg left-align\" style=\"width: 200px;\">Busbar Name</th>";
            for (const ParamCol &col : param.columns)
                out << QString("<th class=\"header-bg\">%1</th>").arg(col.fileDisplayName);
            out << "</tr>\n";

            // Render rows in the exact local order of this card table
            for (int r = 0; r < cardBusbarNames.size(); ++r)
            {
                const QString &bName = cardBusbarNames.at(r);
                out << "<tr>";
                out << QString("<td class=\"alt-bg\">%1</td>").arg(r + 1);
                out << QString("<td class=\"left-align\"><b>%1</b></td>").arg(bName);

                QStringList paramValues;
                for (const ParamCol &col : param.columns)
                    paramValues.append(getFileValue(col.fileIndex, bName, param.paramName));

                QVector<CompColor> colors = computeRowColors(paramValues);

                for (int c = 0; c < param.columns.size(); ++c)
                {
                    out << QString("<td style=\"background-color: %1; color: %2;\">%3</td>")
                    .arg(colors[c].bg)
                        .arg(colors[c].fg)
                        .arg(paramValues.at(c));
                }
                out << "</tr>\n";
            }
            out << "</table>\n";
        }
    }

    out << "</div>\n</body>\n</html>\n";
    docFile.close();

    statusBar()->showMessage(QString("Successfully exported Word document: %1").arg(fileName), 6000);
    QMessageBox::information(this, "Export Complete",
                             QString("Comparison report exported successfully to Word:\n\n%1").arg(fileName));
}
// =============================================================
// SHOW ABOUT
// =============================================================

void FaultAnalysisWindow::showAbout()
{
    const QString appVersion = "1.0.0";
    const QString buildNumber = "2026.09.24";
    QMessageBox::about(
        this,
        "About Comparator Tool",
        "<h3>Comparator Tool</h3>"
        "<p><b>Fault Analysis Comparison</b></p>"
        "<p><b>Version:</b> " + appVersion + " (Build " + buildNumber + ")</p>"
                                                      "<p>A tool for comparing fault analysis results from CSV files.</p>"
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

// =============================================================
// UPDATE SELECTED FILE COUNT
// =============================================================

void FaultAnalysisWindow::updateSelectedFileCount()
{
    if (!mFilesHeadingLabel)
        return;

    int selectedCount = 0;
    for (const QString &filePath : mLoadedFiles)
    {
        if (mFileSelectionState.value(filePath, true))
            ++selectedCount;
    }

    mFilesHeadingLabel->setText(
        QString("Files to Compare & Fault Type Settings (%1 Selected)").arg(selectedCount)
        );
}

// =============================================================
// FILE SETTINGS CHANGED
// =============================================================

void FaultAnalysisWindow::handleFileSettingsChanged(
    const QString &filePath,
    const FaultTypeSettings &settings
    )
{
    mFaultSettings.insert(
        filePath,
        settings
        );

    rebuildComparisonSelection();
}

// =============================================================
// FILE RENAMED
// =============================================================

void FaultAnalysisWindow::handleFileRename(
    const QString &filePath,
    const QString &newName
    )
{
    const QString trimmedName =
        newName.trimmed();

    if (trimmedName.isEmpty())
        return;

    mDisplayNames.insert(
        filePath,
        trimmedName
        );

    updateComparisonTable();
}

// =============================================================
// CLOSE FILE BY PATH
// =============================================================

void FaultAnalysisWindow::closeFileByPath(
    const QString &filePath
    )
{
    const int index =
        mLoadedFiles.indexOf(
            filePath
            );

    if (index >= 0)
    {
        closeFile(index);
    }
}

// =============================================================
// UPDATE FILE SCROLL AREA HEIGHT
// =============================================================

void FaultAnalysisWindow::updateFileScrollAreaHeight()
{
    if (!mFileScrollArea ||
        !mFileContainer ||
        !mFileLayout)
    {
        return;
    }

    if (mFileCards.isEmpty())
    {
        mFileScrollArea->setFixedHeight(0);
        mFileScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        return;
    }

    constexpr int columns = 3;
    constexpr int visibleRows = 2;
    const int visibleCardCount = columns * visibleRows;

    mFileLayout->activate();

    int cardHeight = 0;
    for (FaultFileCard *card : mFileCards)
    {
        if (!card)
            continue;
        cardHeight = qMax(cardHeight, card->sizeHint().height());
        cardHeight = qMax(cardHeight, card->height());
    }

    if (cardHeight <= 0)
        cardHeight = 190;

    const QMargins margins = mFileLayout->contentsMargins();
    const int containerTopMargin = margins.top();
    const int containerBottomMargin = margins.bottom();
    const int gridVerticalSpacing = mFileLayout->verticalSpacing();

    const int expandedContainerHeight =
        (visibleRows * cardHeight) +
        ((visibleRows - 1) * gridVerticalSpacing) +
        containerTopMargin +
        containerBottomMargin;

    int maxAllowedHeight = qBound(180, this->height() / 3, 280);
    int finalHeight = qMin(expandedContainerHeight, maxAllowedHeight);

    mFileScrollArea->setFixedHeight(finalHeight);

    if (finalHeight < expandedContainerHeight || mFileCards.size() > visibleCardCount)
    {
        mFileScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    else
    {
        mFileScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    mFileScrollArea->updateGeometry();
}

// =============================================================
// REFRESH FILE CARDS
// =============================================================

void FaultAnalysisWindow::refreshFileCards()
{
    if (!mFileLayout ||
        !mFileContainer)
    {
        return;
    }

    while (QLayoutItem *item =
           mFileLayout->takeAt(0))
    {
        if (QWidget *widget =
            item->widget())
        {
            widget->deleteLater();
        }

        delete item;
    }

    mFileCards.clear();

    if (mLoadedFiles.isEmpty())
    {
        QLabel *emptyLabel =
            new QLabel(
                "No CSV files loaded. Click + Add CSV files to begin.",
                mFileContainer
                );

        emptyLabel->setAlignment(
            Qt::AlignCenter
            );

        emptyLabel->setStyleSheet(
            "color: #6B7280; padding: 25px;"
            );

        mFileLayout->addWidget(
            emptyLabel,
            0,
            0,
            1,
            3
            );

        updateSelectedFileCount();

        QTimer::singleShot(
            0,
            this,
            [this]()
            {
                updateFileScrollAreaHeight();
            }
            );
        return;
    }

    for (int i = 0;
         i < mLoadedFiles.size();
         ++i)
    {
        const QString filePath =
            mLoadedFiles.at(i);

        QFileInfo fileInfo(
            filePath
            );

        const int rowCount =
            (i < mCsvRows.size())
                ? mCsvRows.at(i).size()
                : 0;

        const int columnCount =
            (i < mCsvHeaders.size())
                ? mCsvHeaders.at(i).size()
                : 0;

        const FaultTypeSettings settings =
            mFaultSettings.value(
                filePath
                );

        FaultFileCard *card =
            new FaultFileCard(
                filePath,
                settings,
                rowCount,
                columnCount,
                fileInfo.size(),
                mFileContainer
                );

        // 1. Assign compact sequential ID (F1, F2, F3...)
        const QString shortId = QString("F%1").arg(i + 1);
        card->setShortId(shortId);

        // 2. Resolve display name: custom alias if set, otherwise default to F1, F2...
        QString displayName = mDisplayNames.value(filePath, QString());
        if (displayName.isEmpty() || displayName == fileInfo.fileName())
        {
            displayName = shortId;
            mDisplayNames[filePath] = shortId;
        }

        card->setDisplayName(displayName);

        card->setSelected(
            mFileSelectionState.value(
                filePath,
                true
                )
            );

        connect(
            card,
            &FaultFileCard::selectionChanged,
            this,
            [this, card]()
            {
                mFileSelectionState[
                    card->filePath()
                ] =
                    card->isSelected();

                updateSelectedFileCount();
                updateComparisonTable();
            }
            );

        connect(
            card,
            &FaultFileCard::settingsChanged,
            this,
            &FaultAnalysisWindow::
            handleFileSettingsChanged
            );

        connect(
            card,
            &FaultFileCard::removeRequested,
            this,
            &FaultAnalysisWindow::
            closeFileByPath
            );

        connect(
            card,
            &FaultFileCard::renameRequested,
            this,
            &FaultAnalysisWindow::
            handleFileRename
            );

        mFileCards.append(
            card
            );

        const int row =
            i / 3;

        const int column =
            i % 3;

        mFileLayout->addWidget(
            card,
            row,
            column
            );
    }

    updateSelectedFileCount();
    updateFileScrollAreaHeight();
}
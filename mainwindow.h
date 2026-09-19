#pragma once

#include <QMainWindow>
#include <QString>
#include <QVector>

#include "src/model/Study.h"
#include "src/ui/PlotDisplayMapper.h"

class QAction;
class QDockWidget;
class FileSelectionWidget;
class PlotBrowserWidget;
class QActionGroup;
class QToolBar;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void addStudyFiles();
    void clearStudyFiles();

    void addPlotWindow();
    void resetPlotLayout();

    void onFileSelectionSettingsChanged();

    void showQuickHelp();
    void showAbout();

	void exportAllPlotsAsPng();
	void exportAllPlotsAsPdf();
	void exportSelectedPlotAsPng();

	void useIpsaFriendlyNames();
	void useRawItfNames();

	void removeStudyFile(int studyIndex);

	void onPlotXRangeChanged(PlotBrowserWidget* sourcePlot,
                         double minX,
                         double maxX,
                         bool hasCustomRange);
	void chooseGlobalTextFont();

	void stackAllPlotViews();
	void tileAllPlotViews();
	void tabAllPlotViews();
	void setAdvancedSignalPlottingEnabled(bool enabled);
	void setAdvancedMultiAxisPlottingEnabled(bool enabled);

private:
    void createMenus();

    bool loadStudyFromFile(const QString& fileName,
                           Study& targetStudy);

    void refreshBrowser();

    void setupDockUi();

    void refreshWidgets();
    void refreshPlotFileSettings();

    QVector<const Study*> studyPointers() const;

    void arrangePlotDocks();
	void removePlotWindow(QDockWidget* plotDock);
	void renumberPlotDocks();

	void warnIfLoadedNetworksDiffer();

	void applyNameDisplayMode();

	void equalizePlotDockSizes();

	void dumpDockLayout(const QString& label) const;

	void finalizePlotDockLayout(const QRect& previousGeometry,
                            bool wasMaximized);

	void preparePlotDocksForLayout();
	void resizeStackedPlotDocks();
	void resizeTiledPlotDocks(int rowCount,
                          int columnCount);

	void setPlotSelectionPanelsVisible(bool visible);

	void clearTilePlaceholderDocks();
	QDockWidget* createTilePlaceholderDock(int cellNumber);
	QVector<QDockWidget*> currentTileDockList() const;

	void applyCurrentPlotLayout();
	void resizeCurrentPlotLayout();

	enum class PlotLayoutMode
	{
	    Stack,
	    Tile,
	    Tab
	};

private:
    QVector<Study> mStudies;

    QAction* mAddFilesAction = nullptr;
    QAction* mClearFilesAction = nullptr;

    QAction* mAddPlotWindowAction = nullptr;
    QAction* mResetPlotLayoutAction = nullptr;

    QAction* mQuickHelpAction = nullptr;
    QAction* mAboutAction = nullptr;

    FileSelectionWidget* mFileSelectionWidget = nullptr;
    QToolBar* mFileSelectionToolBar = nullptr;

	QAction* mExportAllPlotsAsPngAction = nullptr;
	QAction* mExportAllPlotsAsPdfAction = nullptr;
	QAction* mExportSelectedPlotAsPngAction = nullptr;

    QVector<PlotBrowserWidget*> mPlotBrowserWidgets;
    QVector<QDockWidget*> mPlotDocks;

	QActionGroup* mNameDisplayActionGroup = nullptr;
	QAction* mIpsaFriendlyNamesAction = nullptr;
	QAction* mRawItfNamesAction = nullptr;

	QAction* mFilesPanelAction = nullptr;

	QAction* mSyncPlotZoomAction = nullptr;

	QAction* mChooseGlobalTextFontAction = nullptr;

	QAction* mStackAllViewAction = nullptr;
	QAction* mTileAllViewAction = nullptr;
	QAction* mTabAllViewAction = nullptr;

	QAction* mAdvancedSignalPlottingAction = nullptr;
	QAction* mAdvancedMultiAxisPlottingAction = nullptr;

	QLabel* mAdvancedModeStatusLabel = nullptr;

	QVector<QDockWidget*> mTilePlaceholderDocks;
	
	int mTileRowCount = 0;
	int mTileColumnCount = 0;
	
	bool mSyncPlotZoom = false;
	bool mIsApplyingSynchronizedZoom = false;

	bool mAdvancedSignalPlottingEnabled = false;
	bool mAdvancedMultiAxisPlottingEnabled = false;

	PlotLayoutMode mCurrentPlotLayoutMode = PlotLayoutMode::Stack;
	
	NameDisplayMode mNameDisplayMode = NameDisplayMode::IpsaFriendly;

};

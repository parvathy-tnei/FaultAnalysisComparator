#pragma once

#include <QWidget>
#include <QVector>
#include <QPixmap>
#include <QString>
#include <QSize>
#include <QSet>
#include <QHash>
#include <QColor>

#include "FilePlotSettings.h"
#include "PlotWidget.h"

#include "PlotDisplayMapper.h"
#include "MultiAxisPlotTypes.h"

class Study;

class QComboBox;
class QLabel;
class QFrame;
class QListWidget;
class QSplitter;
class QListWidgetItem;
class QStackedWidget;
class MultiAxisPlotWidget;

enum class PlotMode
{
    TimePlot,
    XYPlot
};

enum class PlotExportRange
{
    CompletePlot,
    CurrentView
};

struct PlotExportOptions
{
    PlotExportRange mRange = PlotExportRange::CompletePlot;
    bool mIncludeLegend = true;
    int mFontHeight = 14;
    QString mOutputFilePath;
};

class PlotBrowserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlotBrowserWidget(QWidget* parent = nullptr);

    void setStudies(const QVector<const Study*>& studies);
    void setFilePlotSettings(const QVector<FilePlotSettings>& settings);	
	
	void setNameDisplayMode(NameDisplayMode mode);

	void dumpLayoutDebug(const QString& label,
                     int plotIndex) const;

	QSize sizeHint() const override;
	QSize minimumSizeHint() const override;

	QPixmap exportPlotPixmap(int targetWidth = 1600) const;
	
	QPixmap exportPlotPixmap(const QSize& exportSize,
                         const PlotExportOptions& exportOptions) const;
	QPixmap exportPlotPixmap(const QSize& exportSize = QSize(1600, 900)) const;
	QString exportPlotTitle() const;

	void applyExternalXRange(double minX,
                         double maxX,
                         bool hasCustomRange);
	void setPlotSelectionPanelVisible(bool visible);
	bool askExportOptions(PlotExportOptions& exportOptions);
	void setAdvancedSignalPlottingEnabled(bool enabled);
	void setAdvancedMultiAxisPlottingEnabled(bool enabled);

signals:
    void plotXRangeChanged(PlotBrowserWidget* sourcePlot,
                           double minX,
                           double maxX,
                           bool hasCustomRange);

private:
    void populateStudyTypes();
    void populateComponents();
    void populateSignals();

    void updatePlot();

    const Study* referenceStudy() const;

    QString studyDisplayName(const Study* study) const;

	PlotType currentPlotType() const;
	PlotMode currentPlotMode() const;
	
	QString currentRawStudyType() const;
	QString currentRawSignalName() const;
	QString currentRawXSignalName() const;

	void clearExportCache();
	void showPlotContextMenu(const QPoint& position);
	void exportCurrentPlotAsPng();
	QStringList currentRawYSignalNames() const;
	void updateAdvancedSignalControlVisibility();
	void updateAdvancedSignalPlot();
	QString advancedSignalColorOverrideKey(const QString& studyType,
		const QString& rawSignalName) const;

	QColor advancedSignalColorForSignal(const QString& studyType,
		const QString& rawSignalName,
		int fallbackIndex) const;

	void updateAdvancedSignalListItemColor(QListWidgetItem* item,
		const QColor& color);
	void updateAdvancedMultiAxisSignalPlot();


private slots:
    void onStudyTypeChanged();
    void onComponentChanged();
	void onSignalChanged();
	void onPlotTypeChanged();
	void onPlotModeChanged();
	void togglePlotSelectionPanel();
	void showAdvancedSignalColorMenu(const QPoint& position);
	void showMultiAxisPlotContextMenu(const QPoint& position);

private:
    QVector<const Study*> mStudies;
    QVector<FilePlotSettings> mFilePlotSettings;

	QComboBox* mStudyTypeCombo = nullptr;
	QComboBox* mComponentCombo = nullptr;
	QComboBox* mPlotModeCombo = nullptr;
	QComboBox* mXSignalCombo = nullptr;
	QComboBox* mSignalCombo = nullptr;
	QComboBox* mPlotTypeCombo = nullptr;

    QLabel* mSummaryLabel = nullptr;
    PlotWidget* mPlotWidget = nullptr;

	QLabel* mXSignalLabel = nullptr;

	QFrame* mControlPanel = nullptr;
	QSplitter* mMainSplitter = nullptr;
		
	QSet<QString> mHiddenSeriesNames;

	NameDisplayMode mNameDisplayMode = NameDisplayMode::IpsaFriendly;

	QVector<PlotSeries> mExportSeriesList;
	QString mExportTitle;
	QString mExportXAxisLabel;
	QString mExportYAxisLabel;
	PlotType mExportPlotType = PlotType::Line;

	QLabel* mSignalComboLabel = nullptr;
	QLabel* mSignalListLabel = nullptr;
	QListWidget* mSignalListWidget = nullptr;

	bool mAdvancedSignalPlottingEnabled = false;

	QLabel* mSignalUnitWarningLabel = nullptr;
	QHash<QString, QColor> mAdvancedSignalColorOverrides;

	QStackedWidget* mPlotStackWidget = nullptr;
	MultiAxisPlotWidget* mMultiAxisPlotWidget = nullptr;

	// New members alongside mExportSeriesList / mExportTitle / mExportPlotType:
	QVector<MultiAxisPlotSeries> mExportMultiAxisSeriesList;
	QVector<MultiAxisPlotAxis>   mExportMultiAxisAxes;
	bool                          mExportIsMultiAxis = false;

	bool mAdvancedMultiAxisPlottingEnabled = false;
};

#include "mainwindow.h"

#include "src/parser/ItfParser.h"
#include "src/ui/StudyBrowserWidget.h"

#include <cmath>

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStringList>

#include "src/ui/FileSelectionWidget.h"
#include "src/ui/PlotBrowserWidget.h"
#include "src/ui/PlotDockWidget.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QWidget> 

#include <QMessageBox>

#include <QEvent>
#include <QTimer>

#include <QDir>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPixmap>
#include <QRegularExpression>
#include <QInputDialog>

#include <QActionGroup>
#include <QApplication>
#include <QFontDialog>

#include <QIcon>

#include <QToolBar>
#include <QTabWidget>

#include <QRectF>
#include <QPen>
#include <QBrush>
#include <QPolygonF>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

namespace
{
    constexpr int SoftWarningComparisonFiles = 10;
	constexpr int SoftWarningPlotWindows = 6;
	const QSize ExportPlotImageSize(1600, 900);

	QRect centeredPixmapRect(const QRect& pageRect,
							 const QSize& pixmapSize)
	{
		if (pageRect.isEmpty() ||
			!pixmapSize.isValid() ||
			pixmapSize.width() <= 0 ||
			pixmapSize.height() <= 0)
		{
			return QRect();
		}
	
		/*
		 * Important:
		 * Use only the printable area's size, not its x/y offset.
		 *
		 * QPdfWriter painting is already inside the page coordinate system.
		 * Using paintRectPixels().x() / y() here can shift the image right/down.
		 */
		QRect availableRect(QPoint(0, 0),
							pageRect.size());
	
		const int horizontalPadding =
			qMax(40,
				 availableRect.width() / 35);
	
		const int verticalPadding =
			qMax(30,
				 availableRect.height() / 35);
	
		QRect fitRect =
			availableRect.adjusted(horizontalPadding,
								   verticalPadding,
								   -horizontalPadding,
								   -verticalPadding);
	
		QSize scaledSize =
			pixmapSize;
	
		scaledSize.scale(fitRect.size(),
						 Qt::KeepAspectRatio);
	
		const int x =
			fitRect.x() +
			(fitRect.width() - scaledSize.width()) / 2;
	
		const int y =
			fitRect.y() +
			(fitRect.height() - scaledSize.height()) / 2;
	
		return QRect(QPoint(x, y),
					 scaledSize);
	}

	QString safeExportFileName(const QString& text)
	{
	    QString safeName =
	        text.trimmed();

	    if (safeName.isEmpty())
	    {
	        safeName = "Plot";
	    }

	    safeName.replace(QRegularExpression("[\\\\/:*?\"<>|]+"),
	                     "_");

	    safeName.replace(QRegularExpression("\\s+"),
	                     "_");

	    while (safeName.contains("__"))
	    {
	        safeName.replace("__", "_");
	    }

	    if (safeName.size() > 120)
	    {
	        safeName = safeName.left(120);
	    }

	    return safeName;
	}

	enum class LayoutMenuIconType
	{
		Stack,
		Tile,
		Tab
	};
	
	QIcon makeLayoutMenuIcon(LayoutMenuIconType type)
	{
		QPixmap pixmap(32, 32);
		pixmap.fill(Qt::transparent);
	
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
	
		const QColor borderColor(80, 80, 80);
		const QColor fillColor(235, 238, 242);
		const QColor accentColor(45, 105, 190);
	
		QPen borderPen(borderColor, 1.4);
		QPen accentPen(accentColor, 1.8);
	
		auto drawTile =
			[&](const QRectF& rect, bool accent = false)
			{
				painter.setPen(accent ? accentPen : borderPen);
				painter.setBrush(accent ? QColor(220, 230, 250) : fillColor);
				painter.drawRoundedRect(rect, 2.0, 2.0);
			};
	
		switch (type)
		{
		case LayoutMenuIconType::Stack:
			drawTile(QRectF(5, 5, 22, 5), true);
			drawTile(QRectF(5, 13, 22, 5));
			drawTile(QRectF(5, 21, 22, 5));
			break;
	
		case LayoutMenuIconType::Tile:
			drawTile(QRectF(5, 5, 9, 9), true);
			drawTile(QRectF(18, 5, 9, 9));
			drawTile(QRectF(5, 18, 9, 9));
			drawTile(QRectF(18, 18, 9, 9));
			break;
	
		case LayoutMenuIconType::Tab:
			/*
			 * Draw top tabs and one active content area.
			 */
			drawTile(QRectF(5, 6, 8, 5), true);
			drawTile(QRectF(14, 6, 8, 5));
			drawTile(QRectF(23, 6, 5, 5));
	
			painter.setPen(accentPen);
			painter.setBrush(QColor(235, 238, 242));
			painter.drawRoundedRect(QRectF(5, 11, 23, 15), 2.0, 2.0);
			break;
		}
	
		return QIcon(pixmap);
	}

	enum class AppMenuIconType
	{
		AddFile,
		Clear,
		Exit,
		AddPlot,
		NameDisplay,
		Toolbar,
		SyncZoom,
		TextFont,
		ExportImage,
		ExportImageStack,
		ExportPdf,
		Help,
		About
	};
	
	QIcon makeAppMenuIcon(AppMenuIconType type)
	{
		QPixmap pixmap(32, 32);
		pixmap.fill(Qt::transparent);
	
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
	
		const QColor borderColor(80, 80, 80);
		const QColor fillColor(235, 238, 242);
		const QColor accentColor(45, 105, 190);
		const QColor mutedColor(150, 150, 150);
	
		QPen borderPen(borderColor, 1.5);
		QPen accentPen(accentColor, 2.0);
		QPen mutedPen(mutedColor, 1.5);
	
		painter.setPen(borderPen);
		painter.setBrush(fillColor);
	
		switch (type)
		{
		case AppMenuIconType::AddFile:
			painter.drawRoundedRect(QRectF(7, 5, 14, 20), 2, 2);
			painter.drawLine(QPointF(17, 5), QPointF(22, 10));
			painter.drawLine(QPointF(22, 10), QPointF(22, 25));
	
			painter.setPen(accentPen);
			painter.drawLine(QPointF(20, 18), QPointF(28, 18));
			painter.drawLine(QPointF(24, 14), QPointF(24, 22));
			break;
	
		case AppMenuIconType::Clear:
			painter.drawRoundedRect(QRectF(9, 10, 14, 15), 2, 2);
			painter.drawLine(QPointF(8, 9), QPointF(24, 9));
			painter.drawLine(QPointF(12, 6), QPointF(20, 6));
			painter.setPen(mutedPen);
			painter.drawLine(QPointF(13, 13), QPointF(13, 22));
			painter.drawLine(QPointF(18, 13), QPointF(18, 22));
			break;
	
		case AppMenuIconType::Exit:
			painter.drawRoundedRect(QRectF(7, 6, 12, 20), 2, 2);
			painter.setPen(accentPen);
			painter.drawLine(QPointF(16, 16), QPointF(27, 16));
			painter.drawLine(QPointF(23, 12), QPointF(27, 16));
			painter.drawLine(QPointF(23, 20), QPointF(27, 16));
			break;
	
		case AppMenuIconType::AddPlot:
			painter.drawRoundedRect(QRectF(5, 7, 18, 17), 2, 2);
			painter.setPen(accentPen);
			painter.drawPolyline(QPolygonF()
								 << QPointF(8, 20)
								 << QPointF(12, 15)
								 << QPointF(16, 17)
								 << QPointF(21, 11));
	
			painter.drawLine(QPointF(22, 22), QPointF(29, 22));
			painter.drawLine(QPointF(25.5, 18.5), QPointF(25.5, 25.5));
			break;
	
		case AppMenuIconType::NameDisplay:
			painter.drawRoundedRect(QRectF(5, 8, 22, 16), 2, 2);
			painter.setPen(accentPen);
			painter.drawLine(QPointF(9, 13), QPointF(23, 13));
			painter.drawLine(QPointF(9, 18), QPointF(19, 18));
			break;
	
		case AppMenuIconType::Toolbar:
			painter.drawRoundedRect(QRectF(5, 8, 22, 16), 2, 2);
			painter.setPen(accentPen);
			painter.drawLine(QPointF(7, 13), QPointF(25, 13));
			painter.drawRoundedRect(QRectF(8, 16, 5, 4), 1, 1);
			painter.drawRoundedRect(QRectF(15, 16, 5, 4), 1, 1);
			break;
	
		case AppMenuIconType::SyncZoom:
			painter.drawEllipse(QRectF(6, 7, 10, 10));
			painter.drawEllipse(QRectF(17, 15, 10, 10));
	
			painter.setPen(accentPen);
			painter.drawLine(QPointF(14, 16), QPointF(19, 18));
			painter.drawLine(QPointF(16, 12), QPointF(21, 14));
			break;
	
		case AppMenuIconType::TextFont:
		{
			QFont font =
				painter.font();
	
			font.setBold(true);
			font.setPointSize(17);
	
			painter.setFont(font);
			painter.setPen(accentPen);
			painter.drawText(QRectF(5, 3, 22, 25),
							 Qt::AlignCenter,
							 "A");
			break;
		}
	
		case AppMenuIconType::ExportImage:
			painter.drawRoundedRect(QRectF(6, 7, 20, 17), 2, 2);
	
			painter.setPen(accentPen);
			painter.drawEllipse(QRectF(10, 10, 3, 3));
			painter.drawPolyline(QPolygonF()
								 << QPointF(8, 22)
								 << QPointF(14, 16)
								 << QPointF(18, 19)
								 << QPointF(22, 14)
								 << QPointF(26, 22));
			break;
	
		case AppMenuIconType::ExportImageStack:
			painter.setPen(mutedPen);
			painter.drawRoundedRect(QRectF(4, 5, 18, 14), 2, 2);
	
			painter.setPen(borderPen);
			painter.setBrush(fillColor);
			painter.drawRoundedRect(QRectF(8, 9, 20, 17), 2, 2);
	
			painter.setPen(accentPen);
			painter.drawPolyline(QPolygonF()
								 << QPointF(10, 24)
								 << QPointF(15, 18)
								 << QPointF(20, 21)
								 << QPointF(25, 15));
			break;
	
		case AppMenuIconType::ExportPdf:
		{
			painter.drawRoundedRect(QRectF(7, 5, 18, 22), 2, 2);
	
			QFont font =
				painter.font();
	
			font.setBold(true);
			font.setPointSize(7);
	
			painter.setFont(font);
			painter.setPen(accentPen);
			painter.drawText(QRectF(7, 11, 18, 12),
							 Qt::AlignCenter,
							 "PDF");
			break;
		}
	
		case AppMenuIconType::Help:
		{
			QFont font =
				painter.font();
	
			font.setBold(true);
			font.setPointSize(17);
	
			painter.setFont(font);
			painter.setPen(accentPen);
			painter.drawText(QRectF(5, 3, 22, 25),
							 Qt::AlignCenter,
							 "?");
			break;
		}
	
		case AppMenuIconType::About:
		{
			painter.setPen(accentPen);
			painter.setBrush(QColor(220, 230, 250));
			painter.drawEllipse(QRectF(7, 7, 18, 18));
	
			QFont font =
				painter.font();
	
			font.setBold(true);
			font.setPointSize(14);
	
			painter.setFont(font);
			painter.drawText(QRectF(7, 6, 18, 18),
							 Qt::AlignCenter,
							 "i");
			break;
		}
		}
	
		return QIcon(pixmap);
	}

	bool askPdfExportOptions(QWidget* parent,
							 PlotExportOptions& exportOptions)
	{
		QDialog dialog(parent);
		dialog.setWindowTitle("Print");
		dialog.setModal(true);
	
		QVBoxLayout* mainLayout =
			new QVBoxLayout(&dialog);
	
		QHBoxLayout* topLayout =
			new QHBoxLayout;
	
		//
		// Layout group.
		//
		QGroupBox* layoutGroup =
			new QGroupBox("Layout", &dialog);
	
		QVBoxLayout* layoutGroupLayout =
			new QVBoxLayout(layoutGroup);
	
		QRadioButton* wholeDiagramRadio =
			new QRadioButton("Whole diagram", layoutGroup);
	
		QRadioButton* currentViewRadio =
			new QRadioButton("Current view", layoutGroup);
	
		wholeDiagramRadio->setChecked(
			exportOptions.mRange == PlotExportRange::CompletePlot);
	
		currentViewRadio->setChecked(
			exportOptions.mRange == PlotExportRange::CurrentView);
	
		layoutGroupLayout->addWidget(wholeDiagramRadio);
		layoutGroupLayout->addWidget(currentViewRadio);
	
		topLayout->addWidget(layoutGroup);
	
		//
		// Options group.
		//
		QGroupBox* optionsGroup =
			new QGroupBox("Options", &dialog);
	
		QGridLayout* optionsLayout =
			new QGridLayout(optionsGroup);
	
		QCheckBox* legendCheckBox =
			new QCheckBox("Legend", optionsGroup);
	
		legendCheckBox->setChecked(
			exportOptions.mIncludeLegend);
	
		QLabel* fontHeightLabel =
			new QLabel("Font height", optionsGroup);
	
		QSpinBox* fontHeightSpinBox =
			new QSpinBox(optionsGroup);
	
		fontHeightSpinBox->setRange(8, 32);
		fontHeightSpinBox->setValue(
			qBound(8,
				   exportOptions.mFontHeight,
				   32));
	
		optionsLayout->addWidget(legendCheckBox, 0, 0, 1, 2);
		optionsLayout->addWidget(fontHeightLabel, 1, 0);
		optionsLayout->addWidget(fontHeightSpinBox, 1, 1);
	
		topLayout->addWidget(optionsGroup);
	
		mainLayout->addLayout(topLayout);
	
		//
		// PDF output file.
		//
		QGridLayout* outputLayout =
			new QGridLayout;
	
		QLabel* outputFileLabel =
			new QLabel("PDF output file", &dialog);
	
		QLineEdit* outputFileEdit =
			new QLineEdit(&dialog);
	
		outputFileEdit->setText(
			exportOptions.mOutputFilePath);
	
		QPushButton* browseButton =
			new QPushButton("...", &dialog);
	
		browseButton->setFixedWidth(32);
	
		outputLayout->addWidget(outputFileLabel, 0, 0);
		outputLayout->addWidget(outputFileEdit, 0, 1);
		outputLayout->addWidget(browseButton, 0, 2);
	
		mainLayout->addLayout(outputLayout);
	
		//
		// Buttons.
		//
		QDialogButtonBox* buttonBox =
			new QDialogButtonBox(QDialogButtonBox::Ok |
								 QDialogButtonBox::Cancel |
								 QDialogButtonBox::Help,
								 &dialog);
	
		mainLayout->addWidget(buttonBox);
	
		QObject::connect(browseButton,
						 &QPushButton::clicked,
						 &dialog,
						 [&dialog, outputFileEdit]()
						 {
							 QString selectedFile =
								 QFileDialog::getSaveFileName(
									 &dialog,
									 "PDF output file",
									 outputFileEdit->text(),
									 "PDF Files (*.pdf)");
	
							 if (selectedFile.isEmpty())
							 {
								 return;
							 }
	
							 if (!selectedFile.endsWith(".pdf",
														Qt::CaseInsensitive))
							 {
								 selectedFile += ".pdf";
							 }
	
							 outputFileEdit->setText(selectedFile);
						 });
	
		QObject::connect(buttonBox,
						 &QDialogButtonBox::helpRequested,
						 &dialog,
						 [&dialog]()
						 {
							 QMessageBox::information(
								 &dialog,
								 "Export Help",
								 "Whole diagram exports the complete plot range.\n"
								 "Current view exports the currently visible or zoomed range.\n"
								 "Legend controls whether the legend is included in the export.\n"
								 "Font height controls exported plot text size.\n"
								 "PDF output file is the report file to create.");
						 });
	
		QObject::connect(buttonBox->button(QDialogButtonBox::Cancel),
						 &QPushButton::clicked,
						 &dialog,
						 &QDialog::reject);
	
		QObject::connect(buttonBox->button(QDialogButtonBox::Ok),
						 &QPushButton::clicked,
						 &dialog,
						 [&dialog, outputFileEdit]()
						 {
							 QString filePath =
								 outputFileEdit->text().trimmed();
	
							 if (filePath.isEmpty())
							 {
								 QMessageBox::warning(
									 &dialog,
									 "Missing file",
									 "Please select a PDF output file.");
	
								 return;
							 }
	
							 if (!filePath.endsWith(".pdf",
													Qt::CaseInsensitive))
							 {
								 filePath += ".pdf";
								 outputFileEdit->setText(filePath);
							 }
	
							 dialog.accept();
						 });
	
		if (dialog.exec() != QDialog::Accepted)
		{
			return false;
		}
	
		exportOptions.mRange =
			currentViewRadio->isChecked()
				? PlotExportRange::CurrentView
				: PlotExportRange::CompletePlot;
	
		exportOptions.mIncludeLegend =
			legendCheckBox->isChecked();
	
		exportOptions.mFontHeight =
			fontHeightSpinBox->value();
	
		exportOptions.mOutputFilePath =
			outputFileEdit->text().trimmed();
	
		return true;
	}

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{

    const QIcon appIcon(":/icons/ta_comparator.svg");

    qApp->setWindowIcon(appIcon);
    setWindowIcon(appIcon);

    QWidget* emptyCentralWidget = new QWidget(this);
    emptyCentralWidget->setMinimumSize(0, 0);
    emptyCentralWidget->setMaximumSize(0, 0);

    setCentralWidget(emptyCentralWidget);

    setDockOptions(QMainWindow::AllowNestedDocks |
                   QMainWindow::AllowTabbedDocks |
                   QMainWindow::AnimatedDocks);

    setupDockUi();

    createMenus();

    mAdvancedModeStatusLabel =
        new QLabel("ADVANCED SIGNAL PLOTTING", this);

    mAdvancedModeStatusLabel->setVisible(false);
    mAdvancedModeStatusLabel->setStyleSheet(
        "QLabel {"
        " color: #17406d;"
        " background-color: #dbeafe;"
        " border: 1px solid #7aa7d9;"
        " border-radius: 5px;"
        " padding: 3px 8px;"
        " font-weight: bold;"
        "}");

    statusBar()->addPermanentWidget(mAdvancedModeStatusLabel);

    addPlotWindow();

    setWindowTitle("TA Comparator");
}

void MainWindow::setupDockUi()
{
    mFileSelectionWidget =
        new FileSelectionWidget(this);

    mFileSelectionWidget->setMinimumHeight(40);
    mFileSelectionWidget->setMaximumHeight(56);
    mFileSelectionWidget->setSizePolicy(QSizePolicy::Expanding,
                                        QSizePolicy::Fixed);

    mFileSelectionToolBar =
        new QToolBar("Files to Plot", this);

    mFileSelectionToolBar->setObjectName("FilesToPlotToolBar");
    mFileSelectionToolBar->setMovable(false);
    mFileSelectionToolBar->setFloatable(false);
    mFileSelectionToolBar->setIconSize(QSize(42, 14));

    mFileSelectionToolBar->addWidget(mFileSelectionWidget);

    addToolBar(Qt::TopToolBarArea,
               mFileSelectionToolBar);

    connect(mFileSelectionWidget,
            &FileSelectionWidget::settingsChanged,
            this,
            &MainWindow::onFileSelectionSettingsChanged);

    connect(mFileSelectionWidget,
            &FileSelectionWidget::removeStudyRequested,
            this,
            &MainWindow::removeStudyFile);
}

void MainWindow::addPlotWindow()
{
	const bool shouldRestoreWindowGeometry =
    isVisible();

	const QRect previousGeometry =
	    geometry();

	const bool wasMaximized =
	    isMaximized();
	
	const int nextPlotCount =
		mPlotBrowserWidgets.size() + 1;
	
	if (nextPlotCount > SoftWarningPlotWindows)
	{
		const QMessageBox::StandardButton result =
			QMessageBox::question(
				this,
				"Many plot windows",
				QString("You are about to create %1 plot windows.\n\n"
						"This may reduce readability and make each plot smaller.\n\n"
						"Do you want to continue?")
					.arg(nextPlotCount),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No);
	
		if (result != QMessageBox::Yes)
		{
			return;
		}
	}


    const int plotNumber =
        mPlotBrowserWidgets.size() + 1;

    PlotBrowserWidget* plotWidget =
        new PlotBrowserWidget(this);
	
	plotWidget->setNameDisplayMode(mNameDisplayMode);
	plotWidget->setAdvancedSignalPlottingEnabled(
    mAdvancedSignalPlottingEnabled);
    plotWidget->setAdvancedMultiAxisPlottingEnabled(
        mAdvancedMultiAxisPlottingEnabled);

	connect(plotWidget,
			&PlotBrowserWidget::plotXRangeChanged,
			this,
			&MainWindow::onPlotXRangeChanged);


    PlotDockWidget* plotDock =
    new PlotDockWidget(
        QString("Plot %1").arg(plotNumber),
        this);

    plotDock->setObjectName(
        QString("PlotDock%1").arg(plotNumber));

    plotDock->setWidget(plotWidget);

    plotDock->setAllowedAreas(Qt::AllDockWidgetAreas);

    plotDock->setFeatures(
    QDockWidget::DockWidgetMovable |
    QDockWidget::DockWidgetFloatable |
    QDockWidget::DockWidgetClosable);

	connect(plotDock,
        &PlotDockWidget::closeRequested,
        this,
        [this](PlotDockWidget* dock)
        {
            removePlotWindow(dock);
        });

    mPlotBrowserWidgets.append(plotWidget);
    mPlotDocks.append(plotDock);

    addDockWidget(Qt::RightDockWidgetArea,
                  plotDock);

    const QVector<const Study*> pointers =
        studyPointers();

    plotWidget->setStudies(pointers);

    if (mFileSelectionWidget)
    {
        plotWidget->setFilePlotSettings(
            mFileSelectionWidget->filePlotSettings());
    }

	applyCurrentPlotLayout();
	
	if (shouldRestoreWindowGeometry)
	{
		finalizePlotDockLayout(previousGeometry,
							   wasMaximized);
	}
}

void MainWindow::setPlotSelectionPanelsVisible(bool visible)
{
    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

        plotWidget->setPlotSelectionPanelVisible(visible);
    }
}

void MainWindow::clearTilePlaceholderDocks()
{
    for (QDockWidget* dock : mTilePlaceholderDocks)
    {
        if (!dock)
        {
            continue;
        }

        removeDockWidget(dock);
        dock->deleteLater();
    }

    mTilePlaceholderDocks.clear();

    mTileRowCount = 0;
    mTileColumnCount = 0;
}

QDockWidget* MainWindow::createTilePlaceholderDock(int cellNumber)
{
    QDockWidget* placeholderDock =
        new QDockWidget(QString(), this);

    placeholderDock->setObjectName(
        QString("TilePlaceholderDock%1")
            .arg(cellNumber));

    placeholderDock->setAllowedAreas(Qt::AllDockWidgetAreas);

    placeholderDock->setFeatures(
        QDockWidget::NoDockWidgetFeatures);

    QWidget* titleBar =
        new QWidget(placeholderDock);

    titleBar->setFixedHeight(0);

    placeholderDock->setTitleBarWidget(titleBar);

    QWidget* placeholderWidget =
        new QWidget(placeholderDock);

    placeholderWidget->setMinimumSize(0, 0);
    placeholderWidget->setSizePolicy(QSizePolicy::Expanding,
                                     QSizePolicy::Expanding);

    placeholderWidget->setStyleSheet(
        "background-color: #f3f3f3;");

    placeholderDock->setWidget(placeholderWidget);

    mTilePlaceholderDocks.append(placeholderDock);

    return placeholderDock;
}

QVector<QDockWidget*> MainWindow::currentTileDockList() const
{
    QVector<QDockWidget*> docks =
        mPlotDocks;

    for (QDockWidget* placeholderDock : mTilePlaceholderDocks)
    {
        if (placeholderDock)
        {
            docks.append(placeholderDock);
        }
    }

    return docks;
}

void MainWindow::preparePlotDocksForLayout()
{
    clearTilePlaceholderDocks();

    if (mFileSelectionToolBar)
    {
        mFileSelectionToolBar->show();
    }

    for (QDockWidget* dock : mPlotDocks)
    {
        if (!dock)
        {
            continue;
        }

        dock->setFloating(false);
        dock->hide();

        removeDockWidget(dock);
    }
}

void MainWindow::resetPlotLayout()
{
    stackAllPlotViews();
}

void MainWindow::stackAllPlotViews()
{
    if (mPlotDocks.isEmpty())
    {
        return;
    }

	mCurrentPlotLayoutMode = PlotLayoutMode::Stack;
	
    setUpdatesEnabled(false);

    preparePlotDocksForLayout();

	setPlotSelectionPanelsVisible(true);
	
    QDockWidget* firstPlotDock =
        mPlotDocks.value(0, nullptr);

    if (!firstPlotDock)
    {
        setUpdatesEnabled(true);
        return;
    }

    firstPlotDock->show();

    addDockWidget(Qt::RightDockWidgetArea,
                  firstPlotDock);

    QDockWidget* previousPlotDock =
        firstPlotDock;

    for (int i = 1; i < mPlotDocks.size(); ++i)
    {
        QDockWidget* currentPlotDock =
            mPlotDocks[i];

        if (!currentPlotDock)
        {
            continue;
        }

        currentPlotDock->show();

        splitDockWidget(previousPlotDock,
                        currentPlotDock,
                        Qt::Vertical);

        previousPlotDock =
            currentPlotDock;
    }

    for (QDockWidget* dock : mPlotDocks)
    {
        if (dock)
        {
            dock->show();
            dock->raise();
        }
    }

    setUpdatesEnabled(true);

    QTimer::singleShot(
        0,
        this,
        &MainWindow::resizeStackedPlotDocks);
}

void MainWindow::tileAllPlotViews()
{
    if (mPlotDocks.isEmpty())
    {
        return;
    }

	mCurrentPlotLayoutMode =   PlotLayoutMode::Tile;
	
    const int plotCount =
        mPlotDocks.size();

    const int columnCount =
        static_cast<int>(
            std::ceil(std::sqrt(static_cast<double>(plotCount))));

    const int rowCount =
        static_cast<int>(
            std::ceil(static_cast<double>(plotCount) /
                      static_cast<double>(columnCount)));

    const int requiredCellCount =
        rowCount * columnCount;

    setUpdatesEnabled(false);

    preparePlotDocksForLayout();

    /*
     * MATLAB-like tiled view should focus on the plots.
     * Hide per-plot selection panels and their toggle buttons.
     */
	setPlotSelectionPanelsVisible(false);

    mTileRowCount =
        rowCount;

    mTileColumnCount =
        columnCount;

    QVector<QDockWidget*> tileDocks =
        mPlotDocks;

    for (int cellIndex = plotCount;
         cellIndex < requiredCellCount;
         ++cellIndex)
    {
        tileDocks.append(
            createTilePlaceholderDock(cellIndex + 1));
    }

    QDockWidget* firstDock =
        tileDocks.value(0, nullptr);

    if (!firstDock)
    {
        setUpdatesEnabled(true);
        return;
    }

    firstDock->show();

    addDockWidget(Qt::RightDockWidgetArea,
                  firstDock);

    QVector<QDockWidget*> rowAnchors;
    rowAnchors.append(firstDock);

    /*
     * Create row anchors vertically.
     */
    for (int row = 1; row < rowCount; ++row)
    {
        const int rowStartIndex =
            row * columnCount;

        QDockWidget* rowAnchorDock =
            tileDocks.value(rowStartIndex, nullptr);

        if (!rowAnchorDock)
        {
            continue;
        }

        rowAnchorDock->show();

        splitDockWidget(rowAnchors.last(),
                        rowAnchorDock,
                        Qt::Vertical);

        rowAnchors.append(rowAnchorDock);
    }

    /*
     * Create columns inside each row.
     */
    for (int row = 0; row < rowCount; ++row)
    {
        const int rowStartIndex =
            row * columnCount;

        QDockWidget* previousColumnDock =
            tileDocks.value(rowStartIndex, nullptr);

        if (!previousColumnDock)
        {
            continue;
        }

        for (int column = 1; column < columnCount; ++column)
        {
            const int dockIndex =
                rowStartIndex + column;

            QDockWidget* currentDock =
                tileDocks.value(dockIndex, nullptr);

            if (!currentDock)
            {
                continue;
            }

            currentDock->show();

            splitDockWidget(previousColumnDock,
                            currentDock,
                            Qt::Horizontal);

            previousColumnDock =
                currentDock;
        }
    }

    for (QDockWidget* dock : tileDocks)
    {
        if (dock)
        {
            dock->show();
        }
    }

    /*
     * Raise only real plot docks.
     * Placeholder docks should just occupy empty grid cells.
     */
    for (QDockWidget* dock : mPlotDocks)
    {
        if (dock)
        {
            dock->raise();
        }
    }

    setUpdatesEnabled(true);

    QTimer::singleShot(
        0,
        this,
        [this]()
        {
            resizeTiledPlotDocks(mTileRowCount,
                                 mTileColumnCount);
        });
}

void MainWindow::tabAllPlotViews()
{
    if (mPlotDocks.isEmpty())
    {
        return;
    }

	mCurrentPlotLayoutMode =   PlotLayoutMode::Tab;

	setTabPosition(Qt::AllDockWidgetAreas,
               QTabWidget::North);
	
    setUpdatesEnabled(false);

    preparePlotDocksForLayout();

	setPlotSelectionPanelsVisible(true);

    QDockWidget* firstPlotDock =
        mPlotDocks.value(0, nullptr);

    if (!firstPlotDock)
    {
        setUpdatesEnabled(true);
        return;
    }

    firstPlotDock->show();

    addDockWidget(Qt::RightDockWidgetArea,
                  firstPlotDock);

    for (int i = 1; i < mPlotDocks.size(); ++i)
    {
        QDockWidget* currentPlotDock =
            mPlotDocks[i];

        if (!currentPlotDock)
        {
            continue;
        }

        currentPlotDock->show();

        addDockWidget(Qt::RightDockWidgetArea,
                      currentPlotDock);

        tabifyDockWidget(firstPlotDock,
                         currentPlotDock);
    }

    firstPlotDock->raise();

    setUpdatesEnabled(true);
}

void MainWindow::resizeStackedPlotDocks()
{
    QVector<QDockWidget*> visiblePlotDocks;
    QList<int> sizes;

    for (QDockWidget* dock : mPlotDocks)
    {
        if (!dock)
        {
            continue;
        }

        visiblePlotDocks.append(dock);
        sizes.append(1);
    }

    if (visiblePlotDocks.size() <= 1)
    {
        return;
    }

    resizeDocks(visiblePlotDocks,
                sizes,
                Qt::Vertical);
}

void MainWindow::resizeTiledPlotDocks(int rowCount,
                                      int columnCount)
{
    if (rowCount <= 0 ||
        columnCount <= 0)
    {
        return;
    }

    const QVector<QDockWidget*> tileDocks =
        currentTileDockList();

    if (tileDocks.isEmpty())
    {
        return;
    }

    const int tileCount =
        tileDocks.size();

    /*
     * Equalize row heights.
     * Row anchor = first dock in each row.
     */
    QVector<QDockWidget*> rowAnchorDocks;
    QList<int> rowSizes;

    for (int row = 0; row < rowCount; ++row)
    {
        const int rowStartIndex =
            row * columnCount;

        if (rowStartIndex >= tileCount)
        {
            break;
        }

        QDockWidget* rowAnchorDock =
            tileDocks[rowStartIndex];

        if (!rowAnchorDock)
        {
            continue;
        }

        rowAnchorDocks.append(rowAnchorDock);
        rowSizes.append(1);
    }

    if (rowAnchorDocks.size() > 1)
    {
        resizeDocks(rowAnchorDocks,
                    rowSizes,
                    Qt::Vertical);
    }

    /*
     * Equalize column widths in each row.
     * Placeholder docks make incomplete rows keep the same grid width.
     */
    for (int row = 0; row < rowCount; ++row)
    {
        const int rowStartIndex =
            row * columnCount;

        if (rowStartIndex >= tileCount)
        {
            break;
        }

        QVector<QDockWidget*> rowDocks;
        QList<int> columnSizes;

        for (int column = 0; column < columnCount; ++column)
        {
            const int dockIndex =
                rowStartIndex + column;

            if (dockIndex >= tileCount)
            {
                break;
            }

            QDockWidget* dock =
                tileDocks[dockIndex];

            if (!dock)
            {
                continue;
            }

            rowDocks.append(dock);
            columnSizes.append(1);
        }

        if (rowDocks.size() > 1)
        {
            resizeDocks(rowDocks,
                        columnSizes,
                        Qt::Horizontal);
        }
    }
}

void MainWindow::equalizePlotDockSizes()
{
    if (mPlotDocks.isEmpty())
    {
        return;
    }

    QVector<QDockWidget*> visiblePlotDocks;
    QList<int> sizes;

    for (QDockWidget* dock : mPlotDocks)
    {
        if (!dock)
        {
            continue;
        }

        dock->show();
        dock->raise();

        visiblePlotDocks.append(dock);
        sizes.append(1);
    }

    if (visiblePlotDocks.size() <= 1)
    {
        return;
    }

    resizeDocks(visiblePlotDocks,
                sizes,
                Qt::Vertical);
}

void MainWindow::finalizePlotDockLayout(const QRect& previousGeometry,
                                        bool wasMaximized)
{
    /*
     * Qt dock layout recalculation can temporarily increase the MainWindow
     * minimum size. Let Qt complete one layout pass first, then restore the
     * user-visible window size.
     */
    QTimer::singleShot(
        0,
        this,
        [this, previousGeometry, wasMaximized]()
        {
            resizeCurrentPlotLayout();

            if (wasMaximized)
            {
                showMaximized();
            }
            else
            {
                setGeometry(previousGeometry);
            }

            /*
             * Run one more equalization after the window size has been restored.
             */
            QTimer::singleShot(
                0,
                this,
                [this]()
                {
                    resizeCurrentPlotLayout();
                });
        });
}

void MainWindow::arrangePlotDocks()
{
    if (mPlotDocks.isEmpty())
    {
        return;
    }

    for (QDockWidget* dock : mPlotDocks)
    {
        if (dock)
        {
            dock->show();
            dock->raise();
        }
    }

    if (mPlotDocks.size() == 1)
    {
        return;
    }

    QDockWidget* previousPlotDock =
        mPlotDocks[0];

    for (int i = 1; i < mPlotDocks.size(); ++i)
    {
        QDockWidget* currentPlotDock =
            mPlotDocks[i];

        if (!previousPlotDock ||
            !currentPlotDock)
        {
            continue;
        }

        splitDockWidget(previousPlotDock,
                        currentPlotDock,
                        Qt::Vertical);

        previousPlotDock =
            currentPlotDock;
    }

    equalizePlotDockSizes();
}

void MainWindow::removePlotWindow(QDockWidget* plotDock)
{
    if (!plotDock)
    {
        return;
    }

    if (mPlotDocks.size() <= 1)
    {
        QMessageBox::information(
            this,
            "Cannot close plot",
            "At least one plot window must remain.");

        plotDock->show();

        if (mFileSelectionToolBar)
        {
            mFileSelectionToolBar->show();
        }

        return;
    }

    const int index =
        mPlotDocks.indexOf(plotDock);

    if (index < 0)
    {
        return;
    }

    mPlotDocks.removeAt(index);

    if (index < mPlotBrowserWidgets.size())
    {
        mPlotBrowserWidgets.removeAt(index);
    }

    removeDockWidget(plotDock);

    plotDock->deleteLater();

    renumberPlotDocks();

    if (mFileSelectionToolBar)
    {
        mFileSelectionToolBar->show();
    }

	applyCurrentPlotLayout();
}

void MainWindow::renumberPlotDocks()
{
    for (int i = 0; i < mPlotDocks.size(); ++i)
    {
        QDockWidget* dock =
            mPlotDocks[i];

        if (!dock)
        {
            continue;
        }

        const int plotNumber =
            i + 1;

        dock->setWindowTitle(
            QString("Plot %1").arg(plotNumber));

        dock->setObjectName(
            QString("PlotDock%1").arg(plotNumber));
    }
}

void MainWindow::showQuickHelp()
{
    QMessageBox::information(
        this,
        "TA Comparator - Quick Help",
        "TA Comparator Quick Help\n\n"
        "File menu:\n"
        "- Add ITF File(s): Load one or more transient result files.\n"
        "- Clear Loaded Files: Remove all loaded files from the current session.\n\n"
        "Files panel:\n"
        "- Tick files to include them in all plot windows.\n"
        "- Choose color and line thickness per file.\n\n"
        "Plot menu:\n"
		"- Add Plot Window: Add another plot window.\n"
		"- Reset Plot Layout: Stack plot windows vertically.\n\n"
        "Each plot window:\n"
        "- Select Study Type, Component, Signal and Plot Type independently.\n"
        "- Use mouse wheel to zoom on the time axis.\n"
        "- Drag inside a zoomed plot to pan.\n"
        "- Double-click a plot to reset zoom.\n"
        "- Hover over a plot to compare values across selected files.");
}

void MainWindow::showAbout()
{
    QMessageBox::about(
        this,
        "About TA Comparator",
        "TA Comparator\n\n"
        "A Qt-based tool for comparing IPSA transient analysis ITF files.\n\n"
        "Features:\n"
        "- Multiple ITF file comparison\n"
        "- Independent dockable plot windows\n"
        "- Shared file selection, color and thickness settings\n"
        "- Line and bar plots\n"
        "- Hover comparison tooltip\n"
        "- Zoom and pan support\n"
		"- Resettable vertical plot layout");
}

void MainWindow::exportAllPlotsAsPng()
{
    if (mPlotBrowserWidgets.isEmpty())
    {
        QMessageBox::information(
            this,
            "No plots",
            "There are no plots to export.");

        return;
    }

	PlotExportOptions exportOptions;
	
	PlotBrowserWidget* optionsSourcePlot =
		nullptr;
	
	for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
	{
		if (plotWidget)
		{
			optionsSourcePlot = plotWidget;
			break;
		}
	}
	
	if (!optionsSourcePlot)
	{
		QMessageBox::information(
			this,
			"No plots",
			"There are no valid plots to export.");
	
		return;
	}
	
	if (!optionsSourcePlot->askExportOptions(exportOptions))
	{
		return;
	}

    const QString exportFolder =
        QFileDialog::getExistingDirectory(
            this,
            "Export All Plots as PNG Files");

    if (exportFolder.isEmpty())
    {
        return;
    }

    QDir directory(exportFolder);

    int exportedCount = 0;
    QStringList failedFiles;

    for (int i = 0; i < mPlotBrowserWidgets.size(); ++i)
    {
        PlotBrowserWidget* plotBrowser =
            mPlotBrowserWidgets[i];

        if (!plotBrowser)
        {
            continue;
        }

        const QString plotTitle =
            plotBrowser->exportPlotTitle();

        const QString fileName =
            QString("Plot_%1_%2.png")
                .arg(i + 1)
                .arg(safeExportFileName(plotTitle));

        const QString filePath =
            directory.filePath(fileName);

		const QSize exportPlotSize(1600, 900);
		
		QPixmap pixmap =
			plotBrowser->exportPlotPixmap(exportPlotSize,
										  exportOptions);

        if (pixmap.isNull())
        {
            failedFiles.append(fileName);
            continue;
        }

        if (!pixmap.save(filePath, "PNG"))
        {
            failedFiles.append(fileName);
            continue;
        }

        ++exportedCount;
    }

    QString message =
        QString("Exported %1 plot(s) as PNG files.")
            .arg(exportedCount);

    if (!failedFiles.isEmpty())
    {
        message += "\n\nFailed files:\n";

        for (const QString& failedFile : failedFiles)
        {
            message += "  - " + failedFile + "\n";
        }
    }

    QMessageBox::information(
        this,
        "Export complete",
        message);
}

void MainWindow::exportAllPlotsAsPdf()
{
    if (mPlotBrowserWidgets.isEmpty())
    {
        QMessageBox::information(this,
                                 "No plots",
                                 "There are no plots to export.");

        return;
    }

	PlotExportOptions exportOptions;
	
	PlotBrowserWidget* optionsSourcePlot =
		nullptr;
	
	for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
	{
		if (plotWidget)
		{
			optionsSourcePlot = plotWidget;
			break;
		}
	}
	
	if (!optionsSourcePlot)
	{
		QMessageBox::information(
			this,
			"No plots",
			"There are no valid plots to export.");
	
		return;
	}
	
	exportOptions.mOutputFilePath =
		QDir::home().filePath("TA_Comparator_Report.pdf");
	
	if (!askPdfExportOptions(this,
							 exportOptions))
	{
		return;
	}
	
	const QString fileName =
		exportOptions.mOutputFilePath;

    QPdfWriter pdfWriter(fileName);

    pdfWriter.setResolution(300);

    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Landscape);

    pdfWriter.setPageMargins(QMarginsF(12, 12, 12, 12),
                             QPageLayout::Millimeter);

    QPainter painter(&pdfWriter);

    if (!painter.isActive())
    {
        QMessageBox::critical(this,
                              "Export Failed",
                              "Unable to create PDF file.");

        return;
    }

    bool firstPage =
        true;

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

		const QPixmap plotPixmap =
			plotWidget->exportPlotPixmap(ExportPlotImageSize,
										 exportOptions);

        if (plotPixmap.isNull())
        {
            continue;
        }

        if (!firstPage)
        {
            pdfWriter.newPage();
        }

        firstPage =
            false;

        const QRect pageRect =
            pdfWriter.pageLayout()
                .paintRectPixels(pdfWriter.resolution());

        const QRect targetRect =
            centeredPixmapRect(pageRect,
                               plotPixmap.size());

        painter.drawPixmap(targetRect,
                           plotPixmap);
    }

    painter.end();

    statusBar()->showMessage(
        QString("Exported PDF: %1")
            .arg(fileName));
}

void MainWindow::exportSelectedPlotAsPng()
{
    if (mPlotBrowserWidgets.isEmpty())
    {
        QMessageBox::information(
            this,
            "No plots",
            "There are no plots to export.");

        return;
    }

    QStringList plotChoices;
    QVector<int> plotIndexes;

    for (int i = 0; i < mPlotBrowserWidgets.size(); ++i)
    {
        PlotBrowserWidget* plotBrowser =
            mPlotBrowserWidgets[i];

        if (!plotBrowser)
        {
            continue;
        }

        const QString choiceText =
            QString("Plot %1 - %2")
                .arg(i + 1)
                .arg(plotBrowser->exportPlotTitle());

        plotChoices.append(choiceText);
        plotIndexes.append(i);
    }

    if (plotChoices.isEmpty())
    {
        QMessageBox::information(
            this,
            "No plots",
            "There are no valid plots to export.");

        return;
    }

    bool ok = false;

    const QString selectedPlot =
        QInputDialog::getItem(
            this,
            "Export Selected Plot",
            "Select plot to export:",
            plotChoices,
            0,
            false,
            &ok);

    if (!ok || selectedPlot.isEmpty())
    {
        return;
    }

    const int selectedChoiceIndex =
        plotChoices.indexOf(selectedPlot);

    if (selectedChoiceIndex < 0 ||
        selectedChoiceIndex >= plotIndexes.size())
    {
        return;
    }

    const int plotIndex =
        plotIndexes[selectedChoiceIndex];

    PlotBrowserWidget* plotBrowser =
        mPlotBrowserWidgets[plotIndex];

    if (!plotBrowser)
    {
        return;
    }

	PlotExportOptions exportOptions;

	if (!plotBrowser->askExportOptions(exportOptions))
	{
	    return;
	}
	
    const QString defaultFileName =
        QString("Plot_%1_%2.png")
            .arg(plotIndex + 1)
            .arg(safeExportFileName(
                plotBrowser->exportPlotTitle()));

    QString filePath =
        QFileDialog::getSaveFileName(
            this,
            "Export Selected Plot as PNG",
            defaultFileName,
            "PNG Files (*.png)");

    if (filePath.isEmpty())
    {
        return;
    }

    if (!filePath.endsWith(".png",
                           Qt::CaseInsensitive))
    {
        filePath += ".png";
    }

	const QSize exportPlotSize(1600, 900);
	
	QPixmap pixmap =
		plotBrowser->exportPlotPixmap(exportPlotSize);


    if (pixmap.isNull())
    {
        QMessageBox::critical(
            this,
            "Export failed",
            "Could not render the selected plot.");

        return;
    }

    if (!pixmap.save(filePath, "PNG"))
    {
        QMessageBox::critical(
            this,
            "Export failed",
            "Could not save the selected plot as PNG.");

        return;
    }

    QMessageBox::information(
        this,
        "Export complete",
        QString("Exported Plot %1 as PNG.")
            .arg(plotIndex + 1));
}

void MainWindow::useIpsaFriendlyNames()
{
    mNameDisplayMode =
        NameDisplayMode::IpsaFriendly;

    applyNameDisplayMode();
}

void MainWindow::useRawItfNames()
{
    mNameDisplayMode =
        NameDisplayMode::Raw;

    applyNameDisplayMode();
}

void MainWindow::applyNameDisplayMode()
{
    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

        plotWidget->setNameDisplayMode(mNameDisplayMode);
    }
}

QVector<const Study*> MainWindow::studyPointers() const
{
    QVector<const Study*> pointers;

    for (const Study& study : mStudies)
    {
        pointers.append(&study);
    }

    return pointers;
}

void MainWindow::refreshWidgets()
{
    const QVector<const Study*> pointers =
        studyPointers();

    /*
     * Rebuild the Files panel first.
     * Block signals so it does not trigger plot refresh halfway through.
     */
    if (mFileSelectionWidget)
    {
        const bool previousSignalState =
            mFileSelectionWidget->blockSignals(true);

        mFileSelectionWidget->setStudies(pointers);

        mFileSelectionWidget->blockSignals(previousSignalState);
    }

    QVector<FilePlotSettings> settings;

    if (!pointers.isEmpty() &&
        mFileSelectionWidget)
    {
        settings =
            mFileSelectionWidget->filePlotSettings();
    }

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

        /*
         * Important:
         * Set the new Study pointers first.
         *
         * setFilePlotSettings() calls updatePlot().
         * If it is called before setStudies(), the plot may still contain
         * stale Study pointers from before mStudies.removeAt().
         */
        plotWidget->setStudies(pointers);
        plotWidget->setFilePlotSettings(settings);
    }
}

void MainWindow::refreshPlotFileSettings()
{
    if (!mFileSelectionWidget)
    {
        return;
    }

    const QVector<FilePlotSettings> settings =
        mFileSelectionWidget->filePlotSettings();

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (plotWidget)
        {
            plotWidget->setFilePlotSettings(settings);
        }
    }
}

void MainWindow::onFileSelectionSettingsChanged()
{
    refreshPlotFileSettings();
}

void MainWindow::createMenus()
{
    menuBar()->clear();

    //
    // File menu first.
    //
    QMenu* fileMenu =
        menuBar()->addMenu("File");

	mAddFilesAction =
		fileMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::AddFile),
			"Add ITF File(s)...");


    connect(mAddFilesAction,
            &QAction::triggered,
            this,
            &MainWindow::addStudyFiles);

	mClearFilesAction =
		fileMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::Clear),
			"Clear Loaded Files");


    connect(mClearFilesAction,
            &QAction::triggered,
            this,
            &MainWindow::clearStudyFiles);

    fileMenu->addSeparator();

	QAction* exitAction =
		fileMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::Exit),
			"Exit");


    connect(exitAction,
            &QAction::triggered,
            this,
            &QWidget::close);

    //
    // Plot menu second.
    //
    QMenu* plotMenu =
        menuBar()->addMenu("Plot");

	mAddPlotWindowAction =
		plotMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::AddPlot),
			"Add Plot Window");

	
	connect(mAddPlotWindowAction,
			&QAction::triggered,
			this,
			&MainWindow::addPlotWindow);
	
	//
	// Layout menu third.
	// This is a top-level menu, parallel to View.
	//
	QMenu* layoutMenu =
		menuBar()->addMenu("Layout");
	
	mStackAllViewAction =
		layoutMenu->addAction(
			makeLayoutMenuIcon(LayoutMenuIconType::Stack),
			"Stack All View");

	
	connect(mStackAllViewAction,
			&QAction::triggered,
			this,
			&MainWindow::stackAllPlotViews);
	
	mTileAllViewAction =
		layoutMenu->addAction(
			makeLayoutMenuIcon(LayoutMenuIconType::Tile),
			"Tile All View");

	
	connect(mTileAllViewAction,
			&QAction::triggered,
			this,
			&MainWindow::tileAllPlotViews);
	
	mTabAllViewAction =
		layoutMenu->addAction(
			makeLayoutMenuIcon(LayoutMenuIconType::Tab),
			"Tab All View");

	
	connect(mTabAllViewAction,
			&QAction::triggered,
			this,
			&MainWindow::tabAllPlotViews);
	
	/*
	 * Keep old action pointer valid if other code still references it.
	 * Stack view is the old/current reset layout behaviour.
	 */
	mResetPlotLayoutAction =
		mStackAllViewAction;



	//
	// View menu fourth.
	//
	QMenu* viewMenu =
		menuBar()->addMenu("View");

	
	QMenu* nameDisplayMenu =
		viewMenu->addMenu("Name Display");
	
	mNameDisplayActionGroup =
		new QActionGroup(this);
	
	mNameDisplayActionGroup->setExclusive(true);
	
	mIpsaFriendlyNamesAction =
		nameDisplayMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::NameDisplay),
			"IPSA Friendly Names");

	
	mIpsaFriendlyNamesAction->setCheckable(true);
	mIpsaFriendlyNamesAction->setChecked(
		mNameDisplayMode == NameDisplayMode::IpsaFriendly);
	
	mRawItfNamesAction =
		nameDisplayMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::NameDisplay),
			"Raw ITF Names");

	
	mRawItfNamesAction->setCheckable(true);
	mRawItfNamesAction->setChecked(
		mNameDisplayMode == NameDisplayMode::Raw);
	
	mNameDisplayActionGroup->addAction(mIpsaFriendlyNamesAction);
	mNameDisplayActionGroup->addAction(mRawItfNamesAction);
	
	connect(mIpsaFriendlyNamesAction,
			&QAction::triggered,
			this,
			&MainWindow::useIpsaFriendlyNames);
	
	connect(mRawItfNamesAction,
			&QAction::triggered,
			this,
			&MainWindow::useRawItfNames);

	// Hide/show file legend toolbar.
	viewMenu->addSeparator();
	
	mFilesPanelAction =
		viewMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::Toolbar),
			"Files to Plot Toolbar");

	
	mFilesPanelAction->setCheckable(true);
	mFilesPanelAction->setChecked(
		mFileSelectionToolBar &&
		mFileSelectionToolBar->isVisible());
	
	connect(mFilesPanelAction,
			&QAction::toggled,
			this,
			[this](bool checked)
			{
				if (!mFileSelectionToolBar)
				{
					return;
				}
	
				mFileSelectionToolBar->setVisible(checked);
	
				QTimer::singleShot(
					0,
					this,
					[this]()
					{
						equalizePlotDockSizes();
					});
			});
	
	if (mFileSelectionToolBar)
	{
		connect(mFileSelectionToolBar,
				&QToolBar::visibilityChanged,
				this,
				[this](bool visible)
				{
					if (mFilesPanelAction)
					{
						mFilesPanelAction->setChecked(visible);
					}
	
					QTimer::singleShot(
						0,
						this,
						[this]()
						{
							equalizePlotDockSizes();
						});
				});
	}

	//sync-zoom option
	viewMenu->addSeparator();
	
	mSyncPlotZoomAction =
		viewMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::SyncZoom),
			"Sync Plot Zoom");

	
	mSyncPlotZoomAction->setCheckable(true);
	mSyncPlotZoomAction->setChecked(mSyncPlotZoom);
	
	connect(mSyncPlotZoomAction,
			&QAction::toggled,
			this,
			[this](bool checked)
			{
				mSyncPlotZoom =
					checked;
			});

	
	//Text size/font
	viewMenu->addSeparator();
	
	mChooseGlobalTextFontAction =
		viewMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::TextFont),
			"Text Font / Size...");

	
	connect(mChooseGlobalTextFontAction,
			&QAction::triggered,
			this,
			&MainWindow::chooseGlobalTextFont);

	//
	// Advanced menu.
	//
	QMenu* advancedMenu =
		menuBar()->addMenu("Advanced");
	
	mAdvancedSignalPlottingAction =
		advancedMenu->addAction("Enable Advanced Signal Plotting");
	
	mAdvancedSignalPlottingAction->setCheckable(true);
	mAdvancedSignalPlottingAction->setChecked(
		mAdvancedSignalPlottingEnabled);
	
	connect(mAdvancedSignalPlottingAction,
			&QAction::toggled,
			this,
			&MainWindow::setAdvancedSignalPlottingEnabled);

    advancedMenu->addSeparator();

    mAdvancedMultiAxisPlottingAction =
        advancedMenu->addAction("Enable Multi-Axis Plots");

    mAdvancedMultiAxisPlottingAction->setCheckable(true);
    mAdvancedMultiAxisPlottingAction->setChecked(
        mAdvancedMultiAxisPlottingEnabled);

    mAdvancedMultiAxisPlottingAction->setEnabled(
        mAdvancedSignalPlottingEnabled);

    connect(mAdvancedMultiAxisPlottingAction,
        &QAction::toggled,
        this,
        &MainWindow::setAdvancedMultiAxisPlottingEnabled);


	//
	// Export menu fifth.
	//
	QMenu* exportMenu =
	    menuBar()->addMenu("Export");

	mExportSelectedPlotAsPngAction =
		exportMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::ExportImage),
			"Export Selected Plot as PNG...");


	connect(mExportSelectedPlotAsPngAction,
	        &QAction::triggered,
	        this,
	        &MainWindow::exportSelectedPlotAsPng);

	exportMenu->addSeparator();

	mExportAllPlotsAsPngAction =
		exportMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::ExportImageStack),
			"Export All Plots as PNG Files...");


	connect(mExportAllPlotsAsPngAction,
	        &QAction::triggered,
	        this,
	        &MainWindow::exportAllPlotsAsPng);

	mExportAllPlotsAsPdfAction =
		exportMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::ExportPdf),
			"Export All Plots as PDF Report...");


	connect(mExportAllPlotsAsPdfAction,
	        &QAction::triggered,
	        this,
	        &MainWindow::exportAllPlotsAsPdf);

    //
    // Help menu last.
    //
    QMenu* helpMenu =
        menuBar()->addMenu("Help");

	mQuickHelpAction =
		helpMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::Help),
			"Quick Help");


    connect(mQuickHelpAction,
            &QAction::triggered,
            this,
            &MainWindow::showQuickHelp);

	mAboutAction =
		helpMenu->addAction(
			makeAppMenuIcon(AppMenuIconType::About),
			"About");


    connect(mAboutAction,
            &QAction::triggered,
            this,
            &MainWindow::showAbout);
}

void MainWindow::chooseGlobalTextFont()
{
    bool ok = false;

    const QFont currentFont =
        qApp->font();

    const QFont selectedFont =
        QFontDialog::getFont(&ok,
                             currentFont,
                             this,
                             "Select Text Font and Size");

    if (!ok)
    {
        return;
    }

    /*
     * Apply globally to the whole application.
     *
     * This affects normal Qt widgets and also PlotWidget drawing,
     * because PlotWidget uses painter.font() for axis labels,
     * tick labels, title, legend and hover text.
     */
    qApp->setFont(selectedFont);

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

        plotWidget->updateGeometry();
        plotWidget->update();
    }

    if (mFileSelectionWidget)
    {
        mFileSelectionWidget->updateGeometry();
        mFileSelectionWidget->update();
    }

    statusBar()->showMessage(
        QString("Text font changed to %1, %2 pt")
            .arg(selectedFont.family())
            .arg(selectedFont.pointSize()));
}

void MainWindow::onPlotXRangeChanged(PlotBrowserWidget* sourcePlot,
                                     double minX,
                                     double maxX,
                                     bool hasCustomRange)
{
    if (!mSyncPlotZoom)
    {
        return;
    }

    if (mIsApplyingSynchronizedZoom)
    {
        return;
    }

    if (!sourcePlot)
    {
        return;
    }

    mIsApplyingSynchronizedZoom =
        true;

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget ||
            plotWidget == sourcePlot)
        {
            continue;
        }

        plotWidget->applyExternalXRange(minX,
                                        maxX,
                                        hasCustomRange);
    }

    mIsApplyingSynchronizedZoom =
        false;
}

void MainWindow::addStudyFiles()
{
    const QStringList selectedFiles =
        QFileDialog::getOpenFileNames(
            this,
            "Add ITF File(s)",
            QString(),
            "ITF Files (*.itf);;All Files (*.*)");

    if (selectedFiles.isEmpty())
    {
        return;
    }

    const int totalFileCountAfterLoad =
        mStudies.size() + selectedFiles.size();

    if (totalFileCountAfterLoad > SoftWarningComparisonFiles)
    {
        const QMessageBox::StandardButton result =
            QMessageBox::question(
                this,
                "Many files selected",
                QString("You are about to load %1 file(s) in total.\n\n"
                        "This may make plots crowded.\n\n"
                        "Do you want to continue?")
                    .arg(totalFileCountAfterLoad),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    const QStringList filesToLoad =
        selectedFiles;

    int loadedCount = 0;

    for (const QString& fileName : filesToLoad)
    {
        Study loadedStudy;

        if (!loadStudyFromFile(fileName, loadedStudy))
        {
            continue;
        }

        mStudies.append(loadedStudy);
        loadedCount++;
    }

    refreshBrowser();

    setWindowTitle(QString("TA Comparator - %1 file(s)")
                       .arg(mStudies.size()));

    statusBar()->showMessage(
        QString("Loaded %1 file(s). Total: %2.")
            .arg(loadedCount)
            .arg(mStudies.size()));

	if (loadedCount > 0)
	{
	    warnIfLoadedNetworksDiffer();
	}
}

void MainWindow::clearStudyFiles()
{
    /*
     * First detach all UI widgets from the current Study pointers.
     * This must happen before mStudies.clear().
     */
    const QVector<const Study*> emptyStudies;
    const QVector<FilePlotSettings> emptyFileSettings;

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

        plotWidget->setStudies(emptyStudies);
        plotWidget->setFilePlotSettings(emptyFileSettings);
    }

    if (mFileSelectionWidget)
    {
        const bool previousSignalState =
            mFileSelectionWidget->blockSignals(true);

        mFileSelectionWidget->setStudies(emptyStudies);

        mFileSelectionWidget->blockSignals(previousSignalState);
    }

    mStudies.clear();

    setWindowTitle("TA Comparator");

    statusBar()->showMessage("Loaded files cleared.");
}

void MainWindow::removeStudyFile(int studyIndex)
{
    if (studyIndex < 0 ||
        studyIndex >= mStudies.size())
    {
        return;
    }

    const QString fileName =
        QFileInfo(mStudies[studyIndex].getFileName()).fileName();

    const QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            "Remove file",
            QString("Remove this file from comparison?\n\n%1")
                .arg(fileName),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (result != QMessageBox::Yes)
    {
        return;
    }

	QStringList selectedFilesAfterRemove;
	
	if (mFileSelectionWidget)
	{
		const QVector<FilePlotSettings> currentSettings =
			mFileSelectionWidget->filePlotSettings();
	
		for (int i = 0; i < mStudies.size(); ++i)
		{
			if (i == studyIndex)
			{
				continue;
			}
	
			if (i >= currentSettings.size())
			{
				continue;
			}
	
			if (currentSettings[i].mEnabled)
			{
				selectedFilesAfterRemove.append(
					mStudies[i].getFileName());
			}
		}
	
		mFileSelectionWidget->setPendingSelectedFileNames(
			selectedFilesAfterRemove);
	}


    /*
     * Remove the actual loaded Study from MainWindow.
     * FileSelectionWidget only requested the removal;
     * it does not own the Study data.
     */
    mStudies.removeAt(studyIndex);

    refreshWidgets();

    if (mStudies.isEmpty())
    {
        setWindowTitle("TA Comparator");
    }
    else
    {
        setWindowTitle(
            QString("TA Comparator - %1 file(s)")
                .arg(mStudies.size()));
    }

    statusBar()->showMessage(
        QString("Removed file: %1")
            .arg(fileName));
}

bool MainWindow::loadStudyFromFile(const QString& fileName,
                                   Study& targetStudy)
{
    ItfParser parser;

    if (!parser.parse(fileName, targetStudy))
    {
        QMessageBox::critical(
            this,
            "Parse Failed",
            QString("Failed to parse ITF file:\n%1")
                .arg(fileName));

        statusBar()->showMessage("Failed to parse ITF file.");

        return false;
    }

    return true;
}

void MainWindow::refreshBrowser()
{
    refreshWidgets();
}

void MainWindow::warnIfLoadedNetworksDiffer()
{
    QStringList uniqueNetworkNames;
    QStringList fileNetworkLines;

    for (const Study& study : mStudies)
    {
        const QString networkName =
            study.getNetworkName().trimmed();

        QString fileName;

        fileName = QFileInfo(study.getFileName()).fileName();

        const QString displayNetworkName =
            networkName.isEmpty() ? "(Unknown Network)" : networkName;

        fileNetworkLines.append(
            QString("  - %1  ->  %2")
                .arg(fileName, displayNetworkName));

        if (networkName.isEmpty())
        {
            continue;
        }

        bool alreadyKnown = false;

        for (const QString& knownNetworkName : uniqueNetworkNames)
        {
            if (QString::compare(knownNetworkName,
                                 networkName,
                                 Qt::CaseInsensitive) == 0)
            {
                alreadyKnown = true;
                break;
            }
        }

        if (!alreadyKnown)
        {
            uniqueNetworkNames.append(networkName);
        }
    }

    if (uniqueNetworkNames.size() <= 1)
    {
        return;
    }

    QString message;

    message += "The loaded ITF files appear to belong to different networks.\n\n";
    message += "File to network mapping:\n";

    for (const QString& line : fileNetworkLines)
    {
        message += line + "\n";
    }

    message += "\nYou can continue, but comparisons between different networks may not be meaningful.";

    QMessageBox::warning(
        this,
        "Different Networks Detected",
        message);
}

void MainWindow::applyCurrentPlotLayout()
{
    switch (mCurrentPlotLayoutMode)
    {
    case PlotLayoutMode::Stack:
        stackAllPlotViews();
        break;

    case PlotLayoutMode::Tile:
        tileAllPlotViews();
        break;

    case PlotLayoutMode::Tab:
        tabAllPlotViews();
        break;
    }
}

void MainWindow::resizeCurrentPlotLayout()
{
    switch (mCurrentPlotLayoutMode)
    {
    case PlotLayoutMode::Stack:
        resizeStackedPlotDocks();
        break;

    case PlotLayoutMode::Tile:
        resizeTiledPlotDocks(mTileRowCount,
                             mTileColumnCount);
        break;

    case PlotLayoutMode::Tab:
        break;
    }
}

void MainWindow::setAdvancedMultiAxisPlottingEnabled(bool enabled)
{
    if (mAdvancedMultiAxisPlottingEnabled == enabled)
    {
        return;
    }

    if (enabled &&
        !mAdvancedSignalPlottingEnabled)
    {
        /*
         * Multi-axis plotting is only meaningful inside
         * Advanced Signal Plotting mode.
         */
        if (mAdvancedMultiAxisPlottingAction)
        {
            mAdvancedMultiAxisPlottingAction->blockSignals(true);
            mAdvancedMultiAxisPlottingAction->setChecked(false);
            mAdvancedMultiAxisPlottingAction->blockSignals(false);
        }

        statusBar()->showMessage(
            "Enable Advanced Signal Plotting before enabling Multi-Axis Plots.");

        return;
    }

    mAdvancedMultiAxisPlottingEnabled =
        enabled;

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

        plotWidget->setAdvancedMultiAxisPlottingEnabled(enabled);
    }

    refreshPlotFileSettings();

    statusBar()->showMessage(
        enabled
        ? "Advanced multi-axis plots enabled."
        : "Advanced multi-axis plots disabled.");
}

void MainWindow::setAdvancedSignalPlottingEnabled(bool enabled)
{
    if (mAdvancedSignalPlottingEnabled == enabled)
    {
        return;
    }

    mAdvancedSignalPlottingEnabled =
        enabled;

    if (mAdvancedModeStatusLabel)
    {
        mAdvancedModeStatusLabel->setVisible(enabled);
    }

    if (mFileSelectionWidget)
    {
        mFileSelectionWidget->setAdvancedSignalPlottingModeEnabled(enabled);
    }

    if (mAdvancedMultiAxisPlottingAction)
    {
        mAdvancedMultiAxisPlottingAction->setEnabled(enabled);
    }

    if (!enabled &&
        mAdvancedMultiAxisPlottingEnabled)
    {
        if (mAdvancedMultiAxisPlottingAction)
        {
            mAdvancedMultiAxisPlottingAction->blockSignals(true);
            mAdvancedMultiAxisPlottingAction->setChecked(false);
            mAdvancedMultiAxisPlottingAction->blockSignals(false);
        }

        setAdvancedMultiAxisPlottingEnabled(false);
    }

    for (PlotBrowserWidget* plotWidget : mPlotBrowserWidgets)
    {
        if (!plotWidget)
        {
            continue;
        }

        plotWidget->setAdvancedSignalPlottingEnabled(enabled);
        plotWidget->setAdvancedMultiAxisPlottingEnabled(
            mAdvancedMultiAxisPlottingEnabled);
    }

    refreshPlotFileSettings();

    statusBar()->showMessage(
        enabled
            ? "Advanced signal plotting enabled."
            : "Advanced signal plotting disabled.");
}

void MainWindow::dumpDockLayout(const QString& label) const
{
    qDebug().noquote()
        << "\n========== DOCK LAYOUT DUMP:"
        << label
        << "==========";

    qDebug().noquote()
        << "MainWindow geometry:"
        << geometry()
        << "size:"
        << size();

	qDebug().noquote()
	    << "MainWindow minimumSize:" << minimumSize()
	    << "minimumSizeHint:" << minimumSizeHint()
	    << "maximumSize:" << maximumSize()
	    << "isMaximized:" << isMaximized();

    if (centralWidget())
    {
        qDebug().noquote()
            << "Central widget geometry:"
            << centralWidget()->geometry()
            << "size:"
            << centralWidget()->size()
            << "min:"
            << centralWidget()->minimumSize()
            << "max:"
            << centralWidget()->maximumSize();
    }

	if (mFileSelectionToolBar)
	{
		qDebug().noquote()
			<< "FilesToPlotToolBar"
			<< "visible:" << mFileSelectionToolBar->isVisible()
			<< "geometry:" << mFileSelectionToolBar->geometry()
			<< "size:" << mFileSelectionToolBar->size()
			<< "min:" << mFileSelectionToolBar->minimumSize()
			<< "max:" << mFileSelectionToolBar->maximumSize();
	
		if (mFileSelectionWidget)
		{
			qDebug().noquote()
				<< "  FilesToPlot widget geometry:"
				<< mFileSelectionWidget->geometry()
				<< "size:"
				<< mFileSelectionWidget->size()
				<< "min:"
				<< mFileSelectionWidget->minimumSize()
				<< "max:"
				<< mFileSelectionWidget->maximumSize();
		}
	}

    for (int i = 0; i < mPlotDocks.size(); ++i)
    {
        QDockWidget* dock =
            mPlotDocks[i];

        if (!dock)
        {
            qDebug().noquote()
                << "PlotDock" << i + 1 << ": null";
            continue;
        }

        qDebug().noquote()
            << "PlotDock" << i + 1
            << "objectName:" << dock->objectName()
            << "visible:" << dock->isVisible()
            << "floating:" << dock->isFloating()
            << "area:" << dockWidgetArea(dock)
            << "geometry:" << dock->geometry()
            << "size:" << dock->size()
            << "min:" << dock->minimumSize()
            << "max:" << dock->maximumSize();

        if (dock->widget())
        {
            qDebug().noquote()
                << "  PlotDock" << i + 1
                << "widget geometry:"
                << dock->widget()->geometry()
                << "size:"
                << dock->widget()->size()
                << "min:"
                << dock->widget()->minimumSize()
                << "max:"
                << dock->widget()->maximumSize();
        }
		if (i < mPlotBrowserWidgets.size() &&
		    mPlotBrowserWidgets[i])
		{
		    mPlotBrowserWidgets[i]->dumpLayoutDebug(label,
		                                            i + 1);
		}
    }

    qDebug().noquote()
        << "========== END DOCK LAYOUT DUMP ==========\n";
}


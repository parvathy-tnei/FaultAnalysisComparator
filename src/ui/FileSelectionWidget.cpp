#include "FileSelectionWidget.h"

#include "../model/Study.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QColorDialog>
#include <QSignalBlocker>
#include <QToolButton>
#include <QStringList>

#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QPixmap>

#include <QActionGroup>
#include <QSpacerItem>
#include <QWidget>
#include <QTimer>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QFontMetrics>

#include <QPen>
#include <QBrush>
#include <QRectF>
#include <QPolygonF>

#include <QVariant>

namespace
{
    struct ColorOption
    {
        QString mName;
        QColor mColor;
    };

    QVector<ColorOption> comparisonColorOptions()
    {
        return
        {
            { "Blue", QColor(0, 90, 180) },
            { "Red", QColor(190, 60, 50) },
            { "Green", QColor(40, 140, 80) },
            { "Purple", QColor(120, 80, 170) },
            { "Orange", QColor(200, 130, 30) },
            { "Cyan", QColor(0, 150, 170) },
            { "Magenta", QColor(180, 70, 140) },
            { "Brown", QColor(130, 90, 50) },
            { "Dark Gray", QColor(80, 80, 80) },
            { "Black", QColor(0, 0, 0) }
        };
    }

	enum class FileMenuIconType
	{
		SelectFile,
		UnselectFile,
		Colour,
		Thickness,
		LineType,
		CloseFile
	};
	
	QIcon makeFileMenuIcon(FileMenuIconType type,
						   const QColor& lineColor = QColor(0, 90, 180),
						   int lineThickness = 2,
						   Qt::PenStyle lineStyle = Qt::SolidLine)
	{
		QPixmap pixmap(32, 32);
		pixmap.fill(Qt::transparent);
	
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
	
		const QColor borderColor(80, 80, 80);
		const QColor fillColor(235, 238, 242);
		const QColor accentColor =
			lineColor.isValid()
				? lineColor
				: QColor(45, 105, 190);
	
		QPen borderPen(borderColor, 1.5);
		QPen accentPen(accentColor, 2.0);
	
		painter.setPen(borderPen);
		painter.setBrush(fillColor);
	
		switch (type)
		{
		case FileMenuIconType::SelectFile:
			painter.drawRoundedRect(QRectF(6, 7, 20, 18), 3, 3);
	
			painter.setPen(accentPen);
			painter.drawLine(QPointF(11, 16), QPointF(15, 20));
			painter.drawLine(QPointF(15, 20), QPointF(23, 11));
			break;
	
		case FileMenuIconType::UnselectFile:
			painter.drawRoundedRect(QRectF(6, 7, 20, 18), 3, 3);
	
			painter.setPen(accentPen);
			painter.drawLine(QPointF(11, 12), QPointF(22, 23));
			painter.drawLine(QPointF(22, 12), QPointF(11, 23));
			break;
	
		case FileMenuIconType::Colour:
			painter.setPen(borderPen);
			painter.setBrush(accentColor);
			painter.drawEllipse(QRectF(7, 7, 18, 18));
	
			painter.setPen(QPen(borderColor, 1.2));
			painter.setBrush(Qt::NoBrush);
			painter.drawEllipse(QRectF(7, 7, 18, 18));
			break;
	
		case FileMenuIconType::Thickness:
		{
			QPen thinPen(accentColor, 1);
			QPen mediumPen(accentColor, 2);
			QPen thickPen(accentColor, 4);
	
			thinPen.setCapStyle(Qt::RoundCap);
			mediumPen.setCapStyle(Qt::RoundCap);
			thickPen.setCapStyle(Qt::RoundCap);
	
			painter.setPen(thinPen);
			painter.drawLine(QPointF(7, 9), QPointF(25, 9));
	
			painter.setPen(mediumPen);
			painter.drawLine(QPointF(7, 16), QPointF(25, 16));
	
			painter.setPen(thickPen);
			painter.drawLine(QPointF(7, 24), QPointF(25, 24));
			break;
		}
	
		case FileMenuIconType::LineType:
		{
			QPen pen(accentColor,
					 qMax(2, lineThickness));
	
			pen.setStyle(lineStyle);
			pen.setCapStyle(Qt::RoundCap);
	
			painter.setPen(pen);
	
			painter.drawLine(QPointF(6, 10), QPointF(26, 10));
			painter.drawLine(QPointF(6, 17), QPointF(26, 17));
			painter.drawLine(QPointF(6, 24), QPointF(26, 24));
			break;
		}
	
		case FileMenuIconType::CloseFile:
			painter.drawRoundedRect(QRectF(7, 6, 18, 20), 2, 2);
	
			painter.setPen(accentPen);
			painter.drawLine(QPointF(12, 12), QPointF(20, 20));
			painter.drawLine(QPointF(20, 12), QPointF(12, 20));
			break;
		}
	
		return QIcon(pixmap);
	}

	constexpr int CustomColorRole = Qt::UserRole + 1;
	constexpr int FileChipWidth = 180;
	constexpr int FileButtonWidth = 151;
	constexpr int FileNameTextWidth = 95;

}

FileSelectionWidget::FileSelectionWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* mainLayout =
        new QHBoxLayout(this);

	mainLayout->setContentsMargins(8, 0, 8, 0);
	mainLayout->setSpacing(8);
	mainLayout->setAlignment(Qt::AlignVCenter);


    QLabel* titleLabel =
        new QLabel("Files to Plot", this);

    QFont titleFont =
        titleLabel->font();

    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

	mainLayout->addWidget(titleLabel,
						  0,
						  Qt::AlignVCenter);


    mScrollLeftButton =
        new QToolButton(this);

    mScrollLeftButton->setText("<");
    mScrollLeftButton->setToolTip("Scroll files left");
    mScrollLeftButton->setAutoRaise(true);
    mScrollLeftButton->setFixedSize(20, 24);
    mScrollLeftButton->setVisible(false);

	mainLayout->addWidget(mScrollLeftButton,
						  0,
						  Qt::AlignVCenter);


    mFileScrollArea =
        new QScrollArea(this);

    mFileScrollArea->setFrameShape(QFrame::NoFrame);
    mFileScrollArea->setWidgetResizable(true);
    mFileScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mFileScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mFileScrollArea->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Fixed);
	mFileScrollArea->setMinimumHeight(28);
	mFileScrollArea->setMaximumHeight(34);
	mFileScrollArea->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);


    mFileSelectionFrame =
        new QFrame;

    mFileSelectionFrame->setFrameShape(QFrame::NoFrame);
    mFileSelectionFrame->setSizePolicy(QSizePolicy::Minimum,
                                       QSizePolicy::Fixed);

    mFileSelectionLayout =
        new QHBoxLayout(mFileSelectionFrame);

    mFileSelectionLayout->setContentsMargins(0, 0, 0, 0);
    mFileSelectionLayout->setSpacing(6);
	mFileSelectionLayout->setAlignment(Qt::AlignVCenter);
    mFileSelectionLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    mFileScrollArea->setWidget(mFileSelectionFrame);

	mainLayout->addWidget(mFileScrollArea,
						  1,
						  Qt::AlignVCenter);


    mScrollRightButton =
        new QToolButton(this);

    mScrollRightButton->setText(">");
    mScrollRightButton->setToolTip("Scroll files right");
    mScrollRightButton->setAutoRaise(true);
    mScrollRightButton->setFixedSize(20, 24);
    mScrollRightButton->setVisible(false);

	mainLayout->addWidget(mScrollRightButton,
						  0,
						  Qt::AlignVCenter);


    connect(mScrollLeftButton,
            &QToolButton::clicked,
            this,
            &FileSelectionWidget::scrollFilesLeft);

    connect(mScrollRightButton,
            &QToolButton::clicked,
            this,
            &FileSelectionWidget::scrollFilesRight);

    if (mFileScrollArea->horizontalScrollBar())
    {
        connect(mFileScrollArea->horizontalScrollBar(),
                &QScrollBar::rangeChanged,
                this,
                [this]()
                {
                    updateOverflowButtons();
                });

        connect(mFileScrollArea->horizontalScrollBar(),
                &QScrollBar::valueChanged,
                this,
                [this]()
                {
                    updateOverflowButtons();
                });
    }
}

void FileSelectionWidget::setStudies(const QVector<const Study*>& studies)
{
    /*
     * Important:
     * Do not read file names from the old mStudies pointers here.
     *
     * MainWindow owns the Study objects. After a file is removed,
     * those old pointers may already be invalid.
     *
     * Therefore, preserve previous settings using mStudyFileNames,
     * which is a stable QStringList copy.
     */
	QVector<FilePlotSettings> previousSettings =
		mSettings;
	
	QStringList previousFileNames =
		mStudyFileNames;
	
	/*
	 * If a file chip requested removal, remember which previous entry
	 * was removed. This is important when the same file path is loaded
	 * more than once.
	 */
	const QVariant pendingRemovedIndexValue =
		property("pendingRemovedStudyIndex");
	
	setProperty("pendingRemovedStudyIndex",
				QVariant());
	
	if (pendingRemovedIndexValue.isValid() &&
		previousSettings.size() == studies.size() + 1 &&
		previousFileNames.size() == studies.size() + 1)
	{
		const int removedIndex =
			pendingRemovedIndexValue.toInt();
	
		if (removedIndex >= 0 &&
			removedIndex < previousSettings.size() &&
			removedIndex < previousFileNames.size())
		{
			previousSettings.removeAt(removedIndex);
			previousFileNames.removeAt(removedIndex);
		}
	}

    mStudies =
        studies;

    mStudyFileNames.clear();
    mSettings.clear();

	const QVector<ColorOption> colors =
		comparisonColorOptions();
	
	QVector<bool> previousSettingsUsed(
		previousSettings.size(),
		false);
	
	for (int i = 0; i < mStudies.size(); ++i)
	{

        FilePlotSettings item;

        const QString currentFileName =
            mStudies[i]
                ? mStudies[i]->getFileName()
                : QString();

        mStudyFileNames.append(currentFileName);

		int previousIndex =
			-1;
		
		const int previousCount =
			qMin(previousFileNames.size(),
				 previousSettings.size());
		
		for (int previousFileIndex = 0;
			 previousFileIndex < previousCount;
			 ++previousFileIndex)
		{
			if (previousSettingsUsed[previousFileIndex])
			{
				continue;
			}
		
			if (previousFileNames[previousFileIndex] == currentFileName)
			{
				previousIndex =
					previousFileIndex;
		
				previousSettingsUsed[previousFileIndex] =
					true;
		
				break;
			}
		}
		
		if (previousIndex >= 0 &&
			previousIndex < previousSettings.size())
		{
			item =
				previousSettings[previousIndex];
		}
		else
		{
			item.mEnabled =
				(i == 0);
		
			if (!colors.isEmpty())
			{
				item.mColor =
					colors[i % colors.size()].mColor;
			}
		
			item.mLineThickness =
				2;
		
			item.mLineStyle =
				Qt::SolidLine;
		}

        mSettings.append(item);
    }

	if (mSingleSelectionModeEnabled &&
		!mAdvancedSignalPlottingModeEnabled)
	{
		enforceSingleSelection(0);
	}
	
	updateFileInfo();
	rebuildFileSelection();
	
	emit settingsChanged();
}

void FileSelectionWidget::setSingleSelectionModeEnabled(bool enabled)
{
    if (mSingleSelectionModeEnabled == enabled)
    {
        return;
    }

    mSingleSelectionModeEnabled =
        enabled;

    if (mSingleSelectionModeEnabled)
    {
        enforceSingleSelection(0);
        rebuildFileSelection();

        emit settingsChanged();
    }
}

void FileSelectionWidget::setAdvancedSignalPlottingModeEnabled(bool enabled)
{
	if (mAdvancedSignalPlottingModeEnabled == enabled)
	{
		return;
	}

	mAdvancedSignalPlottingModeEnabled =
		enabled;

	/*
	 * Advanced signal plotting now supports multiple files.
	 * So do not force single-selection mode here.
	 */
	mSingleSelectionModeEnabled =
		false;

	/*
	 * Keep at least one file selected when advanced mode is enabled,
	 * otherwise the plot immediately becomes empty.
	 */
	if (mAdvancedSignalPlottingModeEnabled &&
		!mSettings.isEmpty())
	{
		bool hasSelectedFile = false;

		for (const FilePlotSettings& settings : mSettings)
		{
			if (settings.mEnabled)
			{
				hasSelectedFile = true;
				break;
			}
		}

		if (!hasSelectedFile)
		{
			mSettings[0].mEnabled = true;
		}
	}

	rebuildFileSelection();

	emit settingsChanged();
}

void FileSelectionWidget::enforceSingleSelection(int preferredIndex)
{
    if (mSettings.isEmpty())
    {
        return;
    }

    int selectedIndex =
        preferredIndex;

    if (selectedIndex < 0 ||
        selectedIndex >= mSettings.size())
    {
        selectedIndex = -1;
    }

    if (selectedIndex < 0)
    {
        for (int i = 0; i < mSettings.size(); ++i)
        {
            if (mSettings[i].mEnabled)
            {
                selectedIndex = i;
                break;
            }
        }
    }

    if (selectedIndex < 0)
    {
        selectedIndex = 0;
    }

    for (int i = 0; i < mSettings.size(); ++i)
    {
        mSettings[i].mEnabled =
            (i == selectedIndex);
    }
}

void FileSelectionWidget::updateFileInfo()
{
    /*
     * No separate Current Study card anymore.
     * File count is visually represented by the number of file chips.
     */
}

void FileSelectionWidget::rebuildFileSelection()
{
    if (!mFileSelectionLayout)
    {
        return;
    }

    for (QWidget* chipWidget : mFileChipWidgets)
    {
        mFileSelectionLayout->removeWidget(chipWidget);
        chipWidget->deleteLater();
    }

    mFileChipWidgets.clear();
    mFileButtons.clear();

    if (mEmptyFilesLabel)
    {
        mFileSelectionLayout->removeWidget(mEmptyFilesLabel);
        mEmptyFilesLabel->deleteLater();
        mEmptyFilesLabel = nullptr;
    }

    if (mTrailingStretch)
    {
        mFileSelectionLayout->removeItem(mTrailingStretch);
        delete mTrailingStretch;
        mTrailingStretch = nullptr;
    }

    if (mStudies.isEmpty())
    {
        mEmptyFilesLabel =
            new QLabel("No files loaded", mFileSelectionFrame);

        mEmptyFilesLabel->setObjectName("EmptyFilesLabel");

		mFileSelectionLayout->addWidget(mEmptyFilesLabel,
										0,
										Qt::AlignVCenter);


        mTrailingStretch =
            new QSpacerItem(0,
                            0,
                            QSizePolicy::Expanding,
                            QSizePolicy::Minimum);

        mFileSelectionLayout->addItem(mTrailingStretch);

		QTimer::singleShot(0,
						   this,
						   &FileSelectionWidget::updateOverflowButtons);

        return;
    }

    for (int i = 0; i < mStudies.size(); ++i)
    {
        const Study* study =
            mStudies[i];

        const bool enabled =
            i < mSettings.size()
                ? mSettings[i].mEnabled
                : false;

		const QColor color =
			mAdvancedSignalPlottingModeEnabled
			? QColor(0, 0, 0)
			: (i < mSettings.size()
				? mSettings[i].mColor
				: QColor(0, 90, 180));

        const int thickness =
            i < mSettings.size()
                ? mSettings[i].mLineThickness
                : 2;

        const Qt::PenStyle lineStyle =
            i < mSettings.size()
                ? mSettings[i].mLineStyle
                : Qt::SolidLine;

        QWidget* chipWidget =
            new QWidget(mFileSelectionFrame);

        chipWidget->setObjectName("FileChipWidget");
		chipWidget->setFixedSize(FileChipWidth,
								 26);
		
		chipWidget->setSizePolicy(QSizePolicy::Fixed,
								  QSizePolicy::Fixed);


        QHBoxLayout* chipLayout =
            new QHBoxLayout(chipWidget);

		chipLayout->setContentsMargins(5, 0, 3, 0);
		chipLayout->setSpacing(3);
		chipLayout->setAlignment(Qt::AlignVCenter);


        QToolButton* closeButton =
            new QToolButton(chipWidget);

        closeButton->setText(QString::fromUtf8("×"));
        closeButton->setToolTip("Close file");
        closeButton->setAutoRaise(true);
        closeButton->setFixedSize(18, 18);

        closeButton->setStyleSheet(
            "QToolButton {"
            " border: none;"
            " background: transparent;"
            " color: #666666;"
            " font-weight: bold;"
            "}"
            "QToolButton:hover {"
            " background-color: rgba(180, 0, 0, 35);"
            " border-radius: 3px;"
            " color: #a00000;"
            "}");

        QToolButton* fileButton =
            new QToolButton(chipWidget);

		const QString baseDisplayName =
			studyDisplayName(study).isEmpty()
				? QString("Unnamed file")
				: studyDisplayName(study);
		
		int sameNameCount =
			0;
		
		int duplicateOccurrence =
			0;
		
		for (int studyIndex = 0;
			 studyIndex < mStudies.size();
			 ++studyIndex)
		{
			const Study* otherStudy =
				mStudies[studyIndex];
		
			const QString otherDisplayName =
				studyDisplayName(otherStudy).isEmpty()
					? QString("Unnamed file")
					: studyDisplayName(otherStudy);
		
			if (otherDisplayName != baseDisplayName)
			{
				continue;
			}
		
			++sameNameCount;
		
			if (studyIndex <= i)
			{
				++duplicateOccurrence;
			}
		}
		
		const QString fullDisplayName =
			sameNameCount > 1
				? QString("%1 [%2]")
					  .arg(baseDisplayName)
					  .arg(duplicateOccurrence)
				: baseDisplayName;
		
		const QString fullFilePath =
			study
				? study->getFileName()
				: QString();
		
		const QFontMetrics fileNameMetrics(fileButton->font());
		
		const QString visibleDisplayName =
			fileNameMetrics.elidedText(fullDisplayName,
									   Qt::ElideRight,
									   FileNameTextWidth);
		
		fileButton->setText(visibleDisplayName);

        fileButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        fileButton->setAutoRaise(true);
        fileButton->setCheckable(false);
		fileButton->setFixedSize(FileButtonWidth,
								 24);
		
		fileButton->setSizePolicy(QSizePolicy::Fixed,
								  QSizePolicy::Fixed);



        fileButton->setIcon(
            makeLineSampleIcon(color,
                               thickness,
                               lineStyle));

        fileButton->setIconSize(QSize(42, 14));

		fileButton->setToolTip(
			QString("%1\n%2\n%3")
				.arg(fullDisplayName,
					 fullFilePath,
					 enabled
						 ? "Click to change file plotting options"
						 : "Click to select this file for plotting"));
		
		chipWidget->setToolTip(
			QString("%1\n%2")
				.arg(fullDisplayName,
					 fullFilePath));
		
		closeButton->setToolTip(
			QString("Close %1")
				.arg(fullDisplayName));
		

        fileButton->setStyleSheet(
			"QToolButton {"
			" border: none;"
			" background: transparent;"
			" padding: 2px 5px;"
			" text-align: left;"
			"}"
            "QToolButton:hover {"
            " background-color: rgba(0, 0, 0, 18);"
            " border-radius: 3px;"
            "}");

        if (enabled)
        {
            chipWidget->setStyleSheet(
                QString(
                    "QWidget#FileChipWidget {"
                    " background-color: rgba(%1, %2, %3, 35);"
                    " border: 1px solid rgba(%1, %2, %3, 150);"
                    " border-radius: 4px;"
                    "}")
                    .arg(color.red())
                    .arg(color.green())
                    .arg(color.blue()));
        }
        else
        {
            chipWidget->setStyleSheet(
                "QWidget#FileChipWidget {"
                " background-color: transparent;"
                " border: 1px solid rgba(120, 120, 120, 80);"
                " border-radius: 4px;"
                "}");
        }

		connect(closeButton,
				&QToolButton::clicked,
				this,
				[this, i]()
				{
					const int fileIndex =
						i;
					
					/*
					 * Remember the exact previous entry being removed.
					 * This helps preserve settings correctly when duplicate
					 * file names or duplicate file paths exist.
					 */
					setProperty("pendingRemovedStudyIndex",
								fileIndex);
					
					/*
					 * Queue the remove request so Qt finishes handling the
					 * close-button click before MainWindow refreshes/rebuilds
					 * the toolbar widgets.
					 */
					QTimer::singleShot(

						0,
						this,
						[this, fileIndex]()
						{
							if (fileIndex < 0 ||
								fileIndex >= mStudies.size())
							{
								return;
							}
		
							emit removeStudyRequested(fileIndex);
						});
				});


		connect(fileButton,
				&QToolButton::clicked,
				this,
				[this, i, fileButton]()
				{
					if (i < 0 ||
						i >= mSettings.size())
					{
						return;
					}
		

					if (mSingleSelectionModeEnabled &&
						!mAdvancedSignalPlottingModeEnabled)
					{
						enforceSingleSelection(i);

						rebuildFileSelection();

						emit settingsChanged();

						return;
					}
		
					/*
					 * Normal mode:
					 * If file is not selected, first click only selects it.
					 * Do not open the options menu.
					 */
					if (!mSettings[i].mEnabled)
					{
						mSettings[i].mEnabled = true;
		
						rebuildFileSelection();
		
						emit settingsChanged();
		
						return;
					}
		
					/*
					 * If file is already selected, clicking opens options.
					 */
					showFileMenu(i,
								 fileButton);
				});


		chipLayout->addWidget(fileButton);
		chipLayout->addWidget(closeButton);

        mFileChipWidgets.append(chipWidget);
        mFileButtons.append(fileButton);

		mFileSelectionLayout->addWidget(chipWidget,
										0,
										Qt::AlignVCenter);

    }

    mTrailingStretch =
        new QSpacerItem(0,
                        0,
                        QSizePolicy::Expanding,
                        QSizePolicy::Minimum);

    mFileSelectionLayout->addItem(mTrailingStretch);

	QTimer::singleShot(0,
					   this,
					   &FileSelectionWidget::updateOverflowButtons);

}

void FileSelectionWidget::showFileMenu(int fileIndex,
                                       QToolButton* button)
{
    if (fileIndex < 0 ||
        fileIndex >= mSettings.size() ||
        !button)
    {
        return;
    }

    FilePlotSettings& settings =
        mSettings[fileIndex];

    QMenu menu(this);

	QAction* unselectAction =
		menu.addAction("Unselect file");


    menu.addSeparator();

	QAction* colourAction = nullptr;

	if (!mAdvancedSignalPlottingModeEnabled)
	{
		colourAction =
			menu.addAction(
				makeFileMenuIcon(FileMenuIconType::Colour,
					settings.mColor,
					settings.mLineThickness),
				"Colour...");
	}
	else
	{
		QAction* colourInfoAction =
			menu.addAction(
				makeFileMenuIcon(FileMenuIconType::Colour,
					settings.mColor,
					settings.mLineThickness),
				"Colour is controlled per signal in Advanced mode");

		colourInfoAction->setEnabled(false);
	}


	QMenu* thicknessMenu =
		menu.addMenu(
			makeFileMenuIcon(FileMenuIconType::Thickness,
							 settings.mColor,
							 settings.mLineThickness),
			"Thickness");


	QAction* thickness1 =
		thicknessMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::Thickness,
							 settings.mColor,
							 1),
			"1 px");
	
	QAction* thickness2 =
		thicknessMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::Thickness,
							 settings.mColor,
							 2),
			"2 px");
	
	QAction* thickness3 =
		thicknessMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::Thickness,
							 settings.mColor,
							 3),
			"3 px");
	
	QAction* thickness4 =
		thicknessMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::Thickness,
							 settings.mColor,
							 4),
			"4 px");
	
	QAction* thickness5 =
		thicknessMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::Thickness,
							 settings.mColor,
							 5),
			"5 px");


	QMenu* lineTypeMenu =
		menu.addMenu(
			makeFileMenuIcon(FileMenuIconType::LineType,
							 settings.mColor,
							 settings.mLineThickness,
							 settings.mLineStyle),
			"Line Type");

	
	QActionGroup* lineTypeGroup =
		new QActionGroup(&menu);
	
	lineTypeGroup->setExclusive(true);
	
	QAction* solidLineAction =
		lineTypeMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::LineType,
							 settings.mColor,
							 settings.mLineThickness,
							 Qt::SolidLine),
			"Solid");
	
	QAction* dashedLineAction =
		lineTypeMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::LineType,
							 settings.mColor,
							 settings.mLineThickness,
							 Qt::DashLine),
			"Dashed");
	
	QAction* dottedLineAction =
		lineTypeMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::LineType,
							 settings.mColor,
							 settings.mLineThickness,
							 Qt::DotLine),
			"Dotted");
	
	QAction* dashDotLineAction =
		lineTypeMenu->addAction(
			makeFileMenuIcon(FileMenuIconType::LineType,
							 settings.mColor,
							 settings.mLineThickness,
							 Qt::DashDotLine),
			"Dash-dot");

	solidLineAction->setCheckable(true);
	solidLineAction->setData(static_cast<int>(Qt::SolidLine));
	lineTypeGroup->addAction(solidLineAction);
	
	dashedLineAction->setCheckable(true);
	dashedLineAction->setData(static_cast<int>(Qt::DashLine));
	lineTypeGroup->addAction(dashedLineAction);
	
	dottedLineAction->setCheckable(true);
	dottedLineAction->setData(static_cast<int>(Qt::DotLine));
	lineTypeGroup->addAction(dottedLineAction);
	
	dashDotLineAction->setCheckable(true);
	dashDotLineAction->setData(static_cast<int>(Qt::DashDotLine));
	lineTypeGroup->addAction(dashDotLineAction);
	
	
	switch (settings.mLineStyle)
	{
	case Qt::DashLine:
		dashedLineAction->setChecked(true);
		break;
	
	case Qt::DotLine:
		dottedLineAction->setChecked(true);
		break;
	
	case Qt::DashDotLine:
		dashDotLineAction->setChecked(true);
		break;
	
	case Qt::SolidLine:
	default:
		solidLineAction->setChecked(true);
		break;
	}

    menu.addSeparator();

    QAction* selectedAction =
        menu.exec(button->mapToGlobal(
            QPoint(0, button->height())));

    if (!selectedAction)
    {
        return;
    }

	if (selectedAction == unselectAction)
	{
		settings.mEnabled = false;
	}

	else if (colourAction &&
		selectedAction == colourAction)
    {
        QColor initialColor =
            settings.mColor;

        if (!initialColor.isValid())
        {
            initialColor = QColor(0, 90, 180);
        }

        const QColor chosenColor =
            QColorDialog::getColor(initialColor,
                                   this,
                                   "Select Plot Colour");

        if (!chosenColor.isValid())
        {
            return;
        }

        settings.mColor =
            chosenColor;
    }
    else if (selectedAction == thickness1)
    {
        settings.mLineThickness = 1;
    }
    else if (selectedAction == thickness2)
    {
        settings.mLineThickness = 2;
    }
    else if (selectedAction == thickness3)
    {
        settings.mLineThickness = 3;
    }
    else if (selectedAction == thickness4)
    {
        settings.mLineThickness = 4;
    }
    else if (selectedAction == thickness5)
    {
        settings.mLineThickness = 5;
    }
	else if (selectedAction == solidLineAction)
	{
		settings.mLineStyle = Qt::SolidLine;
	}
	else if (selectedAction == dashedLineAction)
	{
		settings.mLineStyle = Qt::DashLine;
	}
	else if (selectedAction == dottedLineAction)
	{
		settings.mLineStyle = Qt::DotLine;
	}
	else if (selectedAction == dashDotLineAction)
	{
		settings.mLineStyle = Qt::DashDotLine;
	}


    rebuildFileSelection();

    emit settingsChanged();
}

QIcon FileSelectionWidget::makeLineSampleIcon(const QColor& color,
                                              int thickness,
                                              Qt::PenStyle lineStyle) const
{
    QPixmap pixmap(42, 14);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color.isValid() ? color : QColor(0, 90, 180),
             qMax(1, thickness));

    pen.setStyle(lineStyle);
    pen.setCapStyle(Qt::RoundCap);

    painter.setPen(pen);

    painter.drawLine(4,
                     pixmap.height() / 2,
                     pixmap.width() - 4,
                     pixmap.height() / 2);

    return QIcon(pixmap);
}

QVector<int> FileSelectionWidget::selectedStudyIndexes() const
{
    QVector<int> indexes;

    for (int i = 0; i < mSettings.size(); ++i)
    {
        if (mSettings[i].mEnabled)
        {
            indexes.append(i);
        }
    }

    return indexes;
}

QVector<FilePlotSettings> FileSelectionWidget::filePlotSettings() const
{
    return mSettings;
}

#if 0
void FileSelectionWidget::configureColorCombo(QComboBox* combo,
                                              int fileIndex) const
{
    if (!combo)
    {
        return;
    }

    combo->clear();

    /*
     * Custom colour appears first in the dropdown.
     * But it is NOT selected by default.
     */
    combo->addItem("Custom...",
                   QColor());

    combo->setItemData(combo->count() - 1,
                       true,
                       CustomColorRole);

    const QVector<ColorOption> colors =
        comparisonColorOptions();

    for (const ColorOption& option : colors)
    {
        combo->addItem(option.mName,
                       option.mColor);

        combo->setItemData(combo->count() - 1,
                           false,
                           CustomColorRole);
    }

    if (!colors.isEmpty())
    {
        const int defaultColorIndex =
            fileIndex % colors.size();

        /*
         * +1 because index 0 is now Custom...
         */
        const int comboIndex =
            defaultColorIndex + 1;

        combo->setCurrentIndex(comboIndex);

        combo->setProperty("previousColorIndex",
                           comboIndex);

        combo->setProperty("selectedColor",
                           colors[defaultColorIndex].mColor);
    }
}
#endif
QColor FileSelectionWidget::colorForFileIndex(int fileIndex) const
{
    if (fileIndex < 0 ||
        fileIndex >= mSettings.size())
    {
        return QColor(0, 90, 180);
    }

    const QColor color =
        mSettings[fileIndex].mColor;

    if (!color.isValid())
    {
        return QColor(0, 90, 180);
    }

    return color;
}

int FileSelectionWidget::thicknessForFileIndex(int fileIndex) const
{
    if (fileIndex < 0 ||
        fileIndex >= mSettings.size())
    {
        return 2;
    }

    return qMax(1,
                mSettings[fileIndex].mLineThickness);
}

QString FileSelectionWidget::studyDisplayName(const Study* study) const
{
    if (!study)
    {
        return QString();
    }

    const QFileInfo fileInfo(study->getFileName());

    if (!fileInfo.fileName().isEmpty())
    {
        return fileInfo.fileName();
    }

    return study->getFileName();
}

void FileSelectionWidget::setPendingSelectedFileNames(const QStringList& selectedFileNames)
{
    Q_UNUSED(selectedFileNames);

    /*
     * No longer needed after converting the file list into
     * the top toolbar/legend chip UI.
     *
     * Kept as a no-op so MainWindow does not need to change.
     */
}

void FileSelectionWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    QTimer::singleShot(0,
                       this,
                       &FileSelectionWidget::updateOverflowButtons);
}

void FileSelectionWidget::updateOverflowButtons()
{
    if (!mFileScrollArea ||
        !mScrollLeftButton ||
        !mScrollRightButton)
    {
        return;
    }

    QScrollBar* horizontalScrollBar =
        mFileScrollArea->horizontalScrollBar();

    if (!horizontalScrollBar)
    {
        return;
    }

    const bool hasOverflow =
        horizontalScrollBar->maximum() > 0;

    mScrollLeftButton->setVisible(hasOverflow);
    mScrollRightButton->setVisible(hasOverflow);

    if (!hasOverflow)
    {
        return;
    }

    mScrollLeftButton->setEnabled(
        horizontalScrollBar->value() >
        horizontalScrollBar->minimum());

    mScrollRightButton->setEnabled(
        horizontalScrollBar->value() <
        horizontalScrollBar->maximum());
}

void FileSelectionWidget::scrollFilesLeft()
{
    if (!mFileScrollArea)
    {
        return;
    }

    QScrollBar* horizontalScrollBar =
        mFileScrollArea->horizontalScrollBar();

    if (!horizontalScrollBar)
    {
        return;
    }

    const int step =
        qMax(120,
             mFileScrollArea->viewport()->width() / 2);

    horizontalScrollBar->setValue(
        horizontalScrollBar->value() - step);

    updateOverflowButtons();
}

void FileSelectionWidget::scrollFilesRight()
{
    if (!mFileScrollArea)
    {
        return;
    }

    QScrollBar* horizontalScrollBar =
        mFileScrollArea->horizontalScrollBar();

    if (!horizontalScrollBar)
    {
        return;
    }

    const int step =
        qMax(120,
             mFileScrollArea->viewport()->width() / 2);

    horizontalScrollBar->setValue(
        horizontalScrollBar->value() + step);

    updateOverflowButtons();
}


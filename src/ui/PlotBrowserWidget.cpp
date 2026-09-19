#include "PlotBrowserWidget.h"
#include "MultiAxisPlotWidget.h"
#include "MultiAxisPlotTypes.h"

#include "../model/Study.h"

#include <QComboBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <QPainter>
#include <QPixmap>

#include <QPushButton>

#include <QDebug>
#include <QLayout>
#include <QPoint>

//right click
#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QIcon>
#include <QHash>

#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QSpinBox>
#include <QCheckBox>

#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QSplitter>
#include <QColorDialog>
#include <QStackedWidget>

namespace
{
    const QSize ContextExportPlotSize(1600, 900);

    QString safePlotExportFileName(const QString& text)
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

	QIcon makeSeriesMenuIcon(const PlotSeries& series,
							 PlotType plotType)
	{
		QPixmap pixmap(72, 18);
		pixmap.fill(Qt::transparent);
	
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
	
		const QColor color =
			series.mColor.isValid()
				? series.mColor
				: QColor(0, 90, 180);
	
		const int thickness =
			qMax(2,
				 series.mLineThickness);
	
		QPen pen(color,
				 thickness);
	
		if (plotType == PlotType::Bar)
		{
			pen.setStyle(Qt::SolidLine);
		}
		else
		{
			pen.setStyle(series.mLineStyle);
		}
	
		pen.setCapStyle(Qt::RoundCap);
	
		painter.setPen(pen);
	
		painter.drawLine(6,
						 pixmap.height() / 2,
						 pixmap.width() - 6,
						 pixmap.height() / 2);
	
		return QIcon(pixmap);
	}

	const int AdvancedSignalColorRole =
		Qt::UserRole + 100;
	
	QColor advancedSignalColor(int signalIndex)
	{
		static const QVector<QColor> colors =
		{
			QColor(0, 90, 180), 	// blue
			QColor(190, 60, 50),	// red
			QColor(40, 140, 80),	// green
			QColor(120, 80, 170),	// purple
			QColor(200, 130, 30),	// orange
			QColor(0, 150, 170),	// teal
			QColor(180, 80, 140),	// magenta
			QColor(90, 90, 90)		// grey
		};
	
		return colors[signalIndex % colors.size()];
	}
	
	QIcon makeAdvancedSignalColorIcon(const QColor& color)
	{
		QPixmap pixmap(42, 14);
		pixmap.fill(Qt::transparent);
	
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
	
		QPen pen(color.isValid() ? color : QColor(0, 90, 180),
				 3);
	
		pen.setCapStyle(Qt::RoundCap);
	
		painter.setPen(pen);
	
		painter.drawLine(4,
						 pixmap.height() / 2,
						 pixmap.width() - 4,
						 pixmap.height() / 2);
	
		return QIcon(pixmap);
	}

	int exportLegendHeightForSeriesCount(int seriesCount)
	{
		if (seriesCount <= 0)
		{
			return 0;
		}
	
		/*
		 * Larger legend area for export.
		 * Export images are high resolution, so 90 px was too small.
		 */
		if (seriesCount <= 4)
		{
			return 90;
		}
		
		return 130;
	}
	
	void drawExportLegend(QPainter& painter,
						  const QRect& legendRect,
						  const QVector<PlotSeries>& seriesList,
						  PlotType plotType,
						  int fontHeight)
	{
		if (seriesList.isEmpty() ||
			legendRect.isEmpty())
		{
			return;
		}
	
		painter.save();
	
		painter.setRenderHint(QPainter::Antialiasing, true);
	
		QFont legendFont =
			painter.font();
		
		legendFont.setPointSize(
			qMax(fontHeight, 8));
		
		legendFont.setBold(false);
		
		painter.setFont(legendFont);
		
		QFontMetrics metrics(legendFont);
		
		painter.setPen(QPen(Qt::black, 1));
		
		const int itemTop =
			legendRect.top() + 12;
		
		const int rowHeight =
			metrics.height() + 18;
		
		const int lineWidth =
			58;
		
		const int lineToTextGap =
			12;
		
		const int itemGap =
			36;
		
		const int maxItemWidth =
			380;
		
		int x =
			legendRect.left() + 18;
		
		int y =
			itemTop;
		
		const int rightLimit =
			legendRect.right() - 18;

	
		for (const PlotSeries& series : seriesList)
		{
			QString displayName =
				series.mName;
	
			if (displayName.trimmed().isEmpty())
			{
				displayName = "Unnamed file";
			}
	
			displayName =
				metrics.elidedText(displayName,
								   Qt::ElideRight,
								   maxItemWidth - lineWidth - lineToTextGap);
	
			const int textWidth =
				metrics.horizontalAdvance(displayName);
	
			const int itemWidth =
				lineWidth + lineToTextGap + textWidth + itemGap;
	
			if (x + itemWidth > rightLimit &&
				x > legendRect.left() + 12)
			{
				x = legendRect.left() + 12;
				y += rowHeight;
			}
	
			if (y + rowHeight > legendRect.bottom())
			{
				break;
			}
	
			const QColor color =
				series.mColor.isValid()
					? series.mColor
					: QColor(0, 90, 180);
	
			const int thickness =
				qMax(1,
					 series.mLineThickness);
	
			QPen samplePen(color,
						   thickness);
	
			if (plotType == PlotType::Bar)
			{
				samplePen.setStyle(Qt::SolidLine);
			}
			else
			{
				samplePen.setStyle(series.mLineStyle);
			}
	
			samplePen.setCapStyle(Qt::RoundCap);
	
			painter.setPen(samplePen);
	
			const int lineY =
				y + rowHeight / 2;
	
			if (plotType == PlotType::Bar)
			{
				painter.setBrush(color);
			
				painter.drawRect(QRect(x,
									   lineY - 8,
									   36,
									   16));
			
				painter.setBrush(Qt::NoBrush);
			}
			else
			{
				painter.drawLine(x,
								 lineY,
								 x + lineWidth,
								 lineY);
			}
	
			painter.setPen(QPen(Qt::black, 1));
	
			painter.drawText(x + lineWidth + lineToTextGap,
							 y + metrics.ascent() + 5,
							 displayName);
	
			x += itemWidth;
		}
	
		painter.restore();
	}

	QString signalUnitForYAxis(const QString& rawStudyType,
							   const QString& rawSignalName)
	{
		if (rawStudyType == "BusbarOutput")
		{
			static const QHash<QString, QString> units =
			{
				{ "VMag",			"PU" },
				{ "VAngle", 		"deg" },
				{ "UnwrappedAngle", "deg" },
				{ "Frequency",		"PU" }
			};
	
			return units.value(rawSignalName);
		}
	
		if (rawStudyType == "MonBranchOutput")
		{
			static const QHash<QString, QString> units =
			{
				{ "SndVoltage", 	   "PU" },
				{ "SndAng", 		   "deg" },
				{ "SndCurrent", 	   "PU" },
				{ "SndRealPow", 	   "MW" },
				{ "SndReactPow",	   "MVar" },
				{ "SndRealCurr",	   "PU" },
				{ "SndReactCurr",	   "PU" },
				{ "SndReactCurrRatio", "" },
	
				{ "RcvVoltage", 	   "PU" },
				{ "RecvVoltage",	   "PU" },
	
				{ "RcvAng", 		   "deg" },
				{ "RecvAng",		   "deg" },
	
				{ "RcvCurrent", 	   "PU" },
				{ "RecvCurrent",	   "PU" },
	
				{ "RcvRealPow", 	   "MW" },
				{ "RecvRealPow",	   "MW" },
	
				{ "RcvReactPow",	   "MVar" },
				{ "RecvReactPow",	   "MVar" },
	
				{ "RcvRealCurr",	   "PU" },
				{ "RecvRealCurr",	   "PU" },
	
				{ "RcvReactCurr",	   "PU" },
				{ "RecvReactCurr",	   "PU" },
	
				{ "RcvReactCurrRatio", "" },
				{ "RecvReactCurrRatio","" }
			};
	
			return units.value(rawSignalName);
		}
	
		if (rawStudyType == "GeneratorOutput")
		{
			static const QHash<QString, QString> units =
			{
				{ "Angle",	   "deg" },
				{ "FDev",	   "PU" },
				{ "Slip",	   "PU" },
				{ "RealPow",   "MW" },
				{ "ReactPow",  "MVar" },
				{ "PMech",	   "MW" },
				{ "FieldV",    "PU" },
				{ "FieldCurr", "PU" },
				{ "TermV",	   "PU" },
				{ "TermCurr",  "PU" },
				{ "PowFac",    "" }
			};
	
			return units.value(rawSignalName);
		}
	
		if (rawStudyType == "IndMachOutput")
		{
			static const QHash<QString, QString> units =
			{
				{ "Slip",			   "%" },
				{ "InPowMW",		   "MW" },
				{ "InPowMVAR",		   "MVar" },
				{ "TermV",			   "PU" },
				{ "TermI",			   "PU" },
				{ "LoadTQ", 		   "" },
				{ "MotorTQ",		   "" },
				{ "PowFac", 		   "" },
				{ "RotorCurrMag",	   "PU" },
				{ "RotorDAxCurr",	   "PU" },
				{ "RotorQAxCurr",	   "PU" },
				{ "RotorDAxVInject",   "PU" },
				{ "RotorQAxVInject",   "PU" }
			};
	
			return units.value(rawSignalName);
		}
	
		return QString();
	}
	
	QString yAxisLabelWithUnit(const QString& label,
							   const QString& rawStudyType,
							   const QString& rawSignalName)
	{
		const QString trimmedLabel =
			label.trimmed();
	
		if (trimmedLabel.isEmpty())
		{
			return label;
		}
	
		/*
		 * Already has unit, for example:
		 *	 Voltage (PU)
		 *	 Unwrapped Angle (deg)
		 */
		if (trimmedLabel.endsWith(')') &&
			trimmedLabel.contains('('))
		{
			return trimmedLabel;
		}
	
		const QString unit =
			signalUnitForYAxis(rawStudyType,
							   rawSignalName);
	
		if (unit.trimmed().isEmpty())
		{
			return trimmedLabel;
		}
	
		return QString("%1 (%2)")
			.arg(trimmedLabel,
				 unit);
	}

	QString unitDisplayNameForSummary(const QString& unit)
	{
		const QString trimmedUnit =
			unit.trimmed();
	
		if (trimmedUnit.isEmpty())
		{
			return "unitless / unknown";
		}
	
		return trimmedUnit;
	}
	
	QString normalizedUnitText(const QString& unit)
	{
		QString normalized =
			unit.trimmed().toLower();

		normalized.remove(" ");
		normalized.remove(".");

		return normalized;
	}

	QString unitCompatibilityKey(const QString& unit)
	{
		const QString normalized =
			normalizedUnitText(unit);

		/*
		 * Empty unit means unitless / unknown.
		 * We treat it as compatible with any real unit.
		 */
		if (normalized.isEmpty())
		{
			return QString();
		}

		/*
		 * Power-system plotting rule:
		 * Active power, reactive power, and apparent power can be plotted
		 * together because they are all power quantities.
		 */
		 /*
		  * Keep same engineering scale together.
		  * Do not group MW with kW unless we later add unit conversion.
		  */
		if (normalized == "mw" ||
			normalized == "mvar" ||
			normalized == "mva")
		{
			return "power:mega";
		}

		if (normalized == "kw" ||
			normalized == "kvar" ||
			normalized == "kva")
		{
			return "power:kilo";
		}

		if (normalized == "w" ||
			normalized == "var" ||
			normalized == "va")
		{
			return "power:base";
		}

		if (normalized == "pu" ||
			normalized == "perunit")
		{
			return "pu";
		}

		if (normalized == "deg" ||
			normalized == "degree" ||
			normalized == "degrees")
		{
			return "angle";
		}

		if (normalized == "a" ||
			normalized == "amp" ||
			normalized == "amps" ||
			normalized == "ampere" ||
			normalized == "amperes")
		{
			return "current";
		}

		if (normalized == "hz")
		{
			return "frequency";
		}

		if (normalized == "%" ||
			normalized == "percent" ||
			normalized == "percentage")
		{
			return "percentage";
		}

		/*
		 * Default:
		 * unknown non-empty units must match exactly after normalization.
		 */
		return normalized;
	}

	QString unitFamilyDisplayName(const QString& unitKey,
		const QString& fallbackUnit)
	{
		if (unitKey.isEmpty())
		{
			return "unitless / unknown";
		}

		if (unitKey == "power" ||
			unitKey.startsWith("power:"))
		{
			return "Power";
		}

		if (unitKey == "pu")
		{
			return "PU";
		}

		if (unitKey == "angle")
		{
			return "deg";
		}

		if (unitKey == "current")
		{
			return "A";
		}

		if (unitKey == "frequency")
		{
			return "Hz";
		}

		if (unitKey == "percentage")
		{
			return "%";
		}

		return unitDisplayNameForSummary(fallbackUnit);
	}

	QString canonicalUnitDisplayName(const QString& rawUnit)
	{
		const QString normalized =
			normalizedUnitText(rawUnit);

		if (normalized == "mw")
		{
			return "MW";
		}

		if (normalized == "mvar")
		{
			return "MVar";
		}

		if (normalized == "mva")
		{
			return "MVA";
		}

		if (normalized == "kw")
		{
			return "kW";
		}

		if (normalized == "kvar")
		{
			return "kVar";
		}

		if (normalized == "kva")
		{
			return "kVA";
		}

		if (normalized == "w")
		{
			return "W";
		}

		if (normalized == "var")
		{
			return "Var";
		}

		if (normalized == "va")
		{
			return "VA";
		}

		if (normalized == "pu" ||
			normalized == "perunit")
		{
			return "PU";
		}

		if (normalized == "deg" ||
			normalized == "degree" ||
			normalized == "degrees")
		{
			return "deg";
		}

		if (normalized == "a" ||
			normalized == "amp" ||
			normalized == "amps" ||
			normalized == "ampere" ||
			normalized == "amperes")
		{
			return "A";
		}

		if (normalized == "hz")
		{
			return "Hz";
		}

		if (normalized == "%" ||
			normalized == "percent" ||
			normalized == "percentage")
		{
			return "%";
		}

		return rawUnit.trimmed();
	}

	QString actualUnitTextForUnitKey(
		const QString& rawStudyType,
		const QStringList& rawSignalNames,
		const QString& unitKey)
	{
		QStringList units;

		for (const QString& rawSignalName : rawSignalNames)
		{
			const QString rawUnit =
				signalUnitForYAxis(rawStudyType,
					rawSignalName).trimmed();

			if (rawUnit.isEmpty())
			{
				continue;
			}

			if (unitCompatibilityKey(rawUnit) != unitKey)
			{
				continue;
			}

			const QString displayUnit =
				canonicalUnitDisplayName(rawUnit);

			if (!displayUnit.isEmpty() &&
				!units.contains(displayUnit,
					Qt::CaseInsensitive))
			{
				units.append(displayUnit);
			}
		}

		if (units.isEmpty())
		{
			return unitFamilyDisplayName(unitKey,
				QString());
		}

		return units.join("/");
	}

	QString axisDisplayLabelForUnitKey(
		const QString& rawStudyType,
		const QStringList& rawSignalNames,
		const QString& unitKey)
	{
		const QString unitText =
			actualUnitTextForUnitKey(rawStudyType,
				rawSignalNames,
				unitKey);

		if (unitKey == "power" ||
			unitKey.startsWith("power:"))
		{
			if (unitText.isEmpty() ||
				unitText == "Power")
			{
				return "Power";
			}

			return QString("Power (%1)")
				.arg(unitText);
		}

		if (unitKey == "angle")
		{
			return QString("Angle (%1)")
				.arg(unitText.isEmpty() ? QString("deg") : unitText);
		}

		if (unitKey == "current")
		{
			return QString("Current (%1)")
				.arg(unitText.isEmpty() ? QString("A") : unitText);
		}

		if (unitKey == "frequency")
		{
			return QString("Frequency (%1)")
				.arg(unitText.isEmpty() ? QString("Hz") : unitText);
		}

		if (unitKey == "percentage")
		{
			return "Percent (%)";
		}

		if (unitKey == "pu")
		{
			return "PU";
		}

		return unitText;
	}

	QStringList selectedRealUnitKeys(const QString& rawStudyType,
		const QStringList& rawSignalNames)
	{
		QStringList unitKeys;

		for (const QString& rawSignalName : rawSignalNames)
		{
			const QString rawUnit =
				signalUnitForYAxis(rawStudyType,
					rawSignalName).trimmed();

			const QString unitKey =
				unitCompatibilityKey(rawUnit);

			/*
			 * Unitless signals do not create a new axis.
			 * They will be attached to the first real axis later.
			 */
			if (unitKey.isEmpty())
			{
				continue;
			}

			if (!unitKeys.contains(unitKey))
			{
				unitKeys.append(unitKey);
			}
		}

		return unitKeys;
	}

	bool selectedSignalUnitsAreCompatible(
		const QString& rawStudyType,
		const QStringList& rawSignalNames,
		NameDisplayMode nameDisplayMode,
		QString& commonUnit,
		QString& errorMessage)
	{
		commonUnit.clear();
		errorMessage.clear();

		if (rawSignalNames.isEmpty())
		{
			return true;
		}

		QString referenceUnitKey;
		QString referenceRawUnit;

		QStringList selectedSignalDetails;
		QStringList selectedUnitDisplays;

		bool hasMismatch = false;

		for (const QString& rawSignalName : rawSignalNames)
		{
			const QString rawUnit =
				signalUnitForYAxis(rawStudyType,
					rawSignalName).trimmed();

			const QString unitKey =
				unitCompatibilityKey(rawUnit);

			const QString displaySignalName =
				PlotDisplayMapper::displaySignalName(
					rawStudyType,
					rawSignalName,
					nameDisplayMode);

			const QString displayUnit =
				unitFamilyDisplayName(unitKey,
					rawUnit);

			selectedSignalDetails.append(
				QString("  - %1 : %2")
				.arg(displaySignalName,
					displayUnit));

			if (!selectedUnitDisplays.contains(displayUnit,
				Qt::CaseInsensitive))
			{
				selectedUnitDisplays.append(displayUnit);
			}

			/*
			 * Unitless / unknown signals are allowed with any selected unit.
			 */
			if (unitKey.isEmpty())
			{
				continue;
			}

			if (referenceUnitKey.isEmpty())
			{
				referenceUnitKey =
					unitKey;

				referenceRawUnit =
					rawUnit;

				continue;
			}

			if (unitKey != referenceUnitKey)
			{
				hasMismatch =
					true;
			}
		}

		commonUnit =
			unitFamilyDisplayName(referenceUnitKey,
				referenceRawUnit);

		if (!hasMismatch)
		{
			return true;
		}

		errorMessage =
			QString("Different unit types selected: %1.\n"
				"Select signals with compatible units to plot together.")
			.arg(selectedUnitDisplays.join(", "));

		return false;
	}
	void setComboToPlotType(QComboBox* combo,
	                        PlotType plotType)
	{
	    if (!combo)
	    {
	        return;
	    }

	    const int index =
	        combo->findData(static_cast<int>(plotType));

	    if (index >= 0)
	    {
	        combo->setCurrentIndex(index);
	    }
	}							   

	QString duplicateAwareStudyDisplayName(
		const QVector<const Study*>& studies,
		int currentIndex,
		const QString& baseDisplayName)
	{
		int sameNameCount = 0;
		int duplicateOccurrence = 0;
	
		for (int studyIndex = 0;
			 studyIndex < studies.size();
			 ++studyIndex)
		{
			const Study* otherStudy =
				studies[studyIndex];
	
			if (!otherStudy)
			{
				continue;
			}
	
			const QFileInfo otherFileInfo(otherStudy->getFileName());
	
			QString otherDisplayName =
				otherFileInfo.completeBaseName();
	
			if (otherDisplayName.isEmpty())
			{
				otherDisplayName =
					otherFileInfo.fileName();
			}
	
			if (otherDisplayName.isEmpty())
			{
				otherDisplayName =
					otherStudy->getFileName();
			}
	
			if (otherDisplayName != baseDisplayName)
			{
				continue;
			}
	
			++sameNameCount;
	
			if (studyIndex <= currentIndex)
			{
				++duplicateOccurrence;
			}
		}
	
		if (sameNameCount <= 1)
		{
			return baseDisplayName;
		}
	
		return QString("%1 [%2]")
			.arg(baseDisplayName)
			.arg(duplicateOccurrence);
	}

	struct PlotModeMenuActions
	{
		QAction* mTimePlotAction = nullptr;
		QAction* mXYPlotAction = nullptr;
	};
	
	PlotModeMenuActions addPlotModeMenu(QMenu& menu,
										PlotMode currentPlotMode)
	{
		PlotModeMenuActions actions;
	
		QMenu* plotModeMenu =
			menu.addMenu("Plot Mode");
	
		actions.mTimePlotAction =
			plotModeMenu->addAction("Time Plot");
	
		actions.mTimePlotAction->setCheckable(true);
		actions.mTimePlotAction->setChecked(
			currentPlotMode == PlotMode::TimePlot);
	
		actions.mXYPlotAction =
			plotModeMenu->addAction("X-Y Plot");
	
		actions.mXYPlotAction->setCheckable(true);
		actions.mXYPlotAction->setChecked(
			currentPlotMode == PlotMode::XYPlot);
	
		return actions;
	}
	
	void setComboToPlotMode(QComboBox* combo,
							PlotMode plotMode)
	{
		if (!combo)
		{
			return;
		}
	
		const int index =
			combo->findData(static_cast<int>(plotMode));
	
		if (index >= 0)
		{
			combo->setCurrentIndex(index);
		}
	}

	QString compactSignalNamesLabel(const QStringList& signalNames)
	{
		QStringList cleanedNames;
	
		for (const QString& signalName : signalNames)
		{
			const QString cleanedName =
				signalName.trimmed();
	
			if (!cleanedName.isEmpty())
			{
				cleanedNames.append(cleanedName);
			}
		}
	
		if (cleanedNames.isEmpty())
		{
			return "Signal";
		}
	
		if (cleanedNames.size() <= 3)
		{
			return cleanedNames.join(", ");
		}
	
		QStringList visibleNames =
			cleanedNames.mid(0, 3);
	
		return QString("%1, +%2 more")
			.arg(visibleNames.join(", "))
			.arg(cleanedNames.size() - visibleNames.size());
	}

	QString compactAdvancedYAxisLabel(const QStringList& signalDisplayNames,
									  const QString& commonUnit)
	{
		if (signalDisplayNames.isEmpty())
		{
			return commonUnit.trimmed().isEmpty()
				? QString("Signal")
				: QString("Signal (%1)").arg(commonUnit.trimmed());
		}
	
		QString firstSignalName =
			signalDisplayNames.first().trimmed();
	
		const QString trimmedUnit =
			commonUnit.trimmed();
	
		/*
		 * Friendly names often already contain the unit, for example:
		 * Voltage (PU)
		 *
		 * If the display name already has the unit, do not append it again.
		 */
		const bool firstSignalAlreadyHasUnit =
			firstSignalName.contains('(') &&
			firstSignalName.contains(')');

		if (!trimmedUnit.isEmpty() &&
			trimmedUnit != "unitless / unknown" &&
			!firstSignalAlreadyHasUnit)
		{
			firstSignalName =
				QString("%1 (%2)")
				.arg(firstSignalName,
					trimmedUnit);
		}
	
		const int remainingSignalCount =
			signalDisplayNames.size() - 1;
	
		if (remainingSignalCount <= 0)
		{
			return firstSignalName;
		}
	
		return QString("%1, +%2 more")
			.arg(firstSignalName)
			.arg(remainingSignalCount);
	}
}

PlotBrowserWidget::PlotBrowserWidget(QWidget* parent)
    : QWidget(parent)
{
	setMinimumSize(0, 0);
	setSizePolicy(QSizePolicy::Ignored,
				  QSizePolicy::Ignored);

	mStudyTypeCombo = new QComboBox(this);
	mComponentCombo = new QComboBox(this);
	mPlotModeCombo = new QComboBox(this);
	
	mXSignalLabel = new QLabel("X Signal", this);
	mXSignalCombo = new QComboBox(this);
	
	mSignalComboLabel = new QLabel("Y Signal", this);
	mSignalCombo = new QComboBox(this);

	mSignalListLabel = new QLabel("Y Signals", this);
	mSignalListWidget = new QListWidget(this);
	mSignalListWidget->setMaximumHeight(220);
	mSignalListWidget->setMinimumHeight(80);

	mSignalListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(mSignalListWidget,
		&QListWidget::customContextMenuRequested,
		this,
		&PlotBrowserWidget::showAdvancedSignalColorMenu);

	
	mSignalUnitWarningLabel = new QLabel(this);
	mSignalUnitWarningLabel->setWordWrap(true);
	mSignalUnitWarningLabel->setVisible(false);
	mSignalUnitWarningLabel->setStyleSheet(
		"QLabel {"
		" color: #8a4b00;"
		" background-color: #fff4d6;"
		" border: 1px solid #e0b75a;"
		" border-radius: 4px;"
		" padding: 5px;"
		"}");


	const int comboMinimumWidth = 150;
	
	mStudyTypeCombo->setMinimumWidth(comboMinimumWidth);
	mComponentCombo->setMinimumWidth(comboMinimumWidth);
	mPlotModeCombo->setMinimumWidth(comboMinimumWidth);
	mXSignalCombo->setMinimumWidth(comboMinimumWidth);
	mSignalCombo->setMinimumWidth(comboMinimumWidth);

	mPlotTypeCombo = new QComboBox(this);
	
	mPlotTypeCombo->addItem("Line",
							static_cast<int>(PlotType::Line));
	
	mPlotTypeCombo->addItem("Bar",
							static_cast<int>(PlotType::Bar));

	mPlotModeCombo->addItem("Time Plot",
							static_cast<int>(PlotMode::TimePlot));
	
	mPlotModeCombo->addItem("X-Y Plot",
							static_cast<int>(PlotMode::XYPlot));
	
	mPlotModeCombo->setVisible(false);
	
	mXSignalLabel->setVisible(false);
	mXSignalCombo->setVisible(false);
	mXSignalCombo->setEnabled(false);
	
	mSignalListLabel->setVisible(false);
	mSignalListWidget->setVisible(false);


    mSummaryLabel = new QLabel(this);
    mSummaryLabel->setWordWrap(true);
    mSummaryLabel->setText("No signal selected.");
	mSummaryLabel->setMaximumHeight(90);
	mSummaryLabel->setSizePolicy(QSizePolicy::Expanding,
                             QSizePolicy::Maximum);

    mPlotWidget = new PlotWidget(this);
    mPlotWidget->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);

	mPlotWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	mMultiAxisPlotWidget =
		new MultiAxisPlotWidget(this);

	mPlotStackWidget =
		new QStackedWidget(this);

	mPlotStackWidget->addWidget(mPlotWidget);
	mPlotStackWidget->addWidget(mMultiAxisPlotWidget);

	mPlotStackWidget->setCurrentWidget(mPlotWidget);
	
	connect(mPlotWidget,
			&QWidget::customContextMenuRequested,
			this,
			&PlotBrowserWidget::showPlotContextMenu);

	mMultiAxisPlotWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(mMultiAxisPlotWidget,
		&QWidget::customContextMenuRequested,
		this,
		&PlotBrowserWidget::showMultiAxisPlotContextMenu);

	connect(mPlotWidget,
			&PlotWidget::xRangeChanged,
			this,
			[this](double minX,
				   double maxX,
				   bool hasCustomRange)
			{
				if (currentPlotMode() == PlotMode::XYPlot)
				{
					return;
				}
	
				emit plotXRangeChanged(this,
									   minX,
									   maxX,
									   hasCustomRange);
			});

	connect(mMultiAxisPlotWidget,
		&MultiAxisPlotWidget::xRangeChanged,
		this,
		[this](double minX,
			double maxX,
			bool hasCustomRange)
		{
			if (currentPlotMode() == PlotMode::XYPlot)
			{
				return;
			}

			emit plotXRangeChanged(this,
				minX,
				maxX,
				hasCustomRange);
		});


    QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    //
	// Left control panel.
	//
	mControlPanel = new QFrame(this);
	mControlPanel->setFrameShape(QFrame::StyledPanel);
	mControlPanel->setMinimumWidth(250);
	mControlPanel->setMaximumWidth(QWIDGETSIZE_MAX);

	QVBoxLayout* leftLayout = new QVBoxLayout(mControlPanel);
	leftLayout->setContentsMargins(10, 10, 10, 10);
	leftLayout->setSpacing(10);

    //
    // Selection card.
    //
    QFrame* selectionFrame = new QFrame(mControlPanel);
    selectionFrame->setFrameShape(QFrame::StyledPanel);

    QVBoxLayout* selectionLayout =
        new QVBoxLayout(selectionFrame);

    selectionLayout->setContentsMargins(10, 10, 10, 10);
    selectionLayout->setSpacing(8);

    QLabel* selectionTitleLabel =
        new QLabel("Plot Selection", selectionFrame);

    QFont selectionTitleFont =
        selectionTitleLabel->font();

    selectionTitleFont.setBold(true);
    selectionTitleLabel->setFont(selectionTitleFont);

    QFormLayout* selectionFormLayout =
        new QFormLayout;

    selectionFormLayout->setLabelAlignment(Qt::AlignLeft);
    selectionFormLayout->setFormAlignment(Qt::AlignTop);
	selectionFormLayout->setHorizontalSpacing(8);
	selectionFormLayout->setVerticalSpacing(6);
    selectionFormLayout->setFieldGrowthPolicy(
        QFormLayout::AllNonFixedFieldsGrow);

	selectionFormLayout->addRow("Study Type", mStudyTypeCombo);
	selectionFormLayout->addRow("Component", mComponentCombo);
	selectionFormLayout->addRow(mXSignalLabel, mXSignalCombo);
	selectionFormLayout->addRow(mSignalComboLabel, mSignalCombo);
	selectionFormLayout->addRow(mSignalListLabel, mSignalListWidget);
	selectionFormLayout->addRow("Plot Type", mPlotTypeCombo);

	selectionLayout->addWidget(selectionTitleLabel);
	selectionLayout->addLayout(selectionFormLayout);
	selectionLayout->addWidget(mSignalUnitWarningLabel);

    leftLayout->addWidget(selectionFrame);

    //
    // Plot status card.
    //
    QFrame* statusFrame = new QFrame(mControlPanel);
    statusFrame->setFrameShape(QFrame::StyledPanel);

    QVBoxLayout* statusLayout =
        new QVBoxLayout(statusFrame);

    statusLayout->setContentsMargins(10, 10, 10, 10);
    statusLayout->setSpacing(8);

    QLabel* statusTitleLabel =
        new QLabel("Plot Status", statusFrame);

    QFont statusTitleFont =
        statusTitleLabel->font();

    statusTitleFont.setBold(true);
    statusTitleLabel->setFont(statusTitleFont);

    statusLayout->addWidget(statusTitleLabel);
    statusLayout->addWidget(mSummaryLabel);

    leftLayout->addWidget(statusFrame);
	leftLayout->addStretch();

	//
	// Right plot panel.
	//
	QWidget* rightPanel = new QWidget(this);
	
	QVBoxLayout* rightLayout =
		new QVBoxLayout(rightPanel);
	
	rightLayout->setContentsMargins(0, 0, 0, 0);
	rightLayout->setSpacing(0);
	
	rightLayout->addWidget(mPlotStackWidget, 1);
	
	mMainSplitter = new QSplitter(Qt::Horizontal, this);
	mMainSplitter->setChildrenCollapsible(false);

	mControlPanel->setMinimumWidth(220);
	mControlPanel->setMaximumWidth(QWIDGETSIZE_MAX);

	rightPanel->setMinimumWidth(300);

	mMainSplitter->addWidget(mControlPanel);
	mMainSplitter->addWidget(rightPanel);

	mMainSplitter->setStretchFactor(0, 0);
	mMainSplitter->setStretchFactor(1, 1);

	/*
	 * Initial width:
	 * Plot Selection = around 280 px
	 * Plot area gets remaining space.
	 */
	mMainSplitter->setSizes({ 280, 900 });

	mainLayout->addWidget(mMainSplitter, 1);


    connect(mStudyTypeCombo,
            &QComboBox::currentIndexChanged,
            this,
            &PlotBrowserWidget::onStudyTypeChanged);

    connect(mComponentCombo,
            &QComboBox::currentIndexChanged,
            this,
            &PlotBrowserWidget::onComponentChanged);

    connect(mSignalCombo,
            &QComboBox::currentIndexChanged,
            this,
            &PlotBrowserWidget::onSignalChanged);

    connect(mPlotTypeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PlotBrowserWidget::onPlotTypeChanged);
	
	connect(mPlotModeCombo,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&PlotBrowserWidget::onPlotModeChanged);
	
	connect(mXSignalCombo,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			this,
			&PlotBrowserWidget::onSignalChanged);

	connect(mSignalListWidget,
			&QListWidget::itemChanged,
			this,
			[this](QListWidgetItem*)
			{
				updatePlot();
			});
}

void PlotBrowserWidget::applyExternalXRange(double minX,
	double maxX,
	bool hasCustomRange)
{
	if (currentPlotMode() == PlotMode::XYPlot)
	{
		return;
	}

	if (mPlotWidget)
	{
		mPlotWidget->applyExternalXRange(minX,
			maxX,
			hasCustomRange);
	}

	if (mMultiAxisPlotWidget)
	{
		mMultiAxisPlotWidget->applyExternalXRange(minX,
			maxX,
			hasCustomRange);
	}
}

void PlotBrowserWidget::showMultiAxisPlotContextMenu(
	const QPoint& position)
{
	if (!mMultiAxisPlotWidget)
	{
		return;
	}

	QMenu menu(this);

	const int clickedSeriesIndex =
		mMultiAxisPlotWidget->seriesIndexAtPosition(position);

	QAction* plotSelectionAction =
		menu.addAction(
			mControlPanel && mControlPanel->isVisible()
			? "Hide Plot Selection"
			: "Show Plot Selection");

	menu.addSeparator();

	QAction* hideClickedSeriesAction = nullptr;

	if (clickedSeriesIndex >= 0)
	{
		hideClickedSeriesAction =
			menu.addAction("Hide this line");
	}

	QAction* showAllHiddenSeriesAction = nullptr;

	if (!mHiddenSeriesNames.isEmpty())
	{
		showAllHiddenSeriesAction =
			menu.addAction("Show all hidden lines");
	}

	menu.addSeparator();

	QAction* resetZoomAction =
		menu.addAction("Reset Zoom");

	menu.addSeparator();

	QAction* showMajorGridAction =
		menu.addAction("Show Major Grid");

	showMajorGridAction->setCheckable(true);
	showMajorGridAction->setChecked(
		mMultiAxisPlotWidget->showMajorGrid());

	QAction* showMinorGridAction =
		menu.addAction("Show Minor Grid");

	showMinorGridAction->setCheckable(true);
	showMinorGridAction->setChecked(
		mMultiAxisPlotWidget->showMinorGrid());

	menu.addSeparator();

	QAction* exportAction =
		menu.addAction("Export Plot as PNG...");

	exportAction->setEnabled(!mExportMultiAxisSeriesList.isEmpty());

	QAction* selectedAction =
		menu.exec(
			mMultiAxisPlotWidget->mapToGlobal(position));

	if (!selectedAction)
	{
		return;
	}

	if (selectedAction == plotSelectionAction)
	{
		togglePlotSelectionPanel();
		return;
	}

	if (selectedAction == exportAction)
	{
		exportCurrentPlotAsPng();
		return;
	}

	if (hideClickedSeriesAction &&
		selectedAction == hideClickedSeriesAction)
	{
		/*
		 * For multi-axis mode, mExportSeriesList is intentionally empty.
		 * So we cannot use it here.
		 *
		 * Instead, ask the widget for the clicked series name.
		 * If your MultiAxisPlotWidget does not expose that yet,
		 * add seriesNameAtIndex(index).
		 */
		const QString seriesName =
			mMultiAxisPlotWidget->seriesNameAtIndex(
				clickedSeriesIndex);

		if (!seriesName.isEmpty())
		{
			mHiddenSeriesNames.insert(seriesName);
			updatePlot();
		}

		return;
	}

	if (showAllHiddenSeriesAction &&
		selectedAction == showAllHiddenSeriesAction)
	{
		mHiddenSeriesNames.clear();
		updatePlot();
		return;
	}

	if (selectedAction == resetZoomAction)
	{
		mMultiAxisPlotWidget->resetZoom();
		return;
	}

	if (selectedAction == showMajorGridAction)
	{
		mMultiAxisPlotWidget->setShowMajorGrid(
			showMajorGridAction->isChecked());

		return;
	}

	if (selectedAction == showMinorGridAction)
	{
		mMultiAxisPlotWidget->setShowMinorGrid(
			showMinorGridAction->isChecked());

		return;
	}
}

void PlotBrowserWidget::showPlotContextMenu(const QPoint& position)
{
    if (!mPlotWidget)
    {
        return;
    }

    QMenu menu(this);

	const int clickedSeriesIndex =
		mPlotWidget
			? mPlotWidget->seriesIndexAtPosition(position)
			: -1;
	
	if (clickedSeriesIndex >= 0 &&
		clickedSeriesIndex < mExportSeriesList.size())
	{
		PlotModeMenuActions plotModeActions;

		if (!mAdvancedSignalPlottingEnabled)
		{
			plotModeActions =
				addPlotModeMenu(menu,
					currentPlotMode());

			menu.addSeparator();
		}

		QAction* hideClickedSeriesAction =
			menu.addAction(QString("Hide this line: %1")
				.arg(mExportSeriesList[clickedSeriesIndex].mName));
	
		QAction* selectedAction =
			menu.exec(mPlotWidget->mapToGlobal(position));
	
		if (plotModeActions.mTimePlotAction &&
			selectedAction == plotModeActions.mTimePlotAction)
		{
			setComboToPlotMode(mPlotModeCombo,
							   PlotMode::TimePlot);
	
			onPlotModeChanged();
	
			return;
		}
	
		if (plotModeActions.mXYPlotAction &&
			selectedAction == plotModeActions.mXYPlotAction)
		{
			setComboToPlotMode(mPlotModeCombo,
							   PlotMode::XYPlot);
	
			onPlotModeChanged();
	
			return;
		}
	
		if (selectedAction == hideClickedSeriesAction)
		{
			mHiddenSeriesNames.insert(
				mExportSeriesList[clickedSeriesIndex].mName);
	
			updatePlot();
	
			return;
		}
	
		return;
	}

	QAction* plotSelectionAction =
		menu.addAction(
			mControlPanel && mControlPanel->isVisible()
				? "Hide Plot Selection"
				: "Show Plot Selection");
	
	menu.addSeparator();


	QAction* linePlotAction = nullptr;
	QAction* barPlotAction = nullptr;

	PlotModeMenuActions plotModeActions;

	if (!mAdvancedSignalPlottingEnabled)
	{
		QMenu* plotTypeMenu =
			menu.addMenu("Plot Type");

		plotTypeMenu->setEnabled(
			currentPlotMode() == PlotMode::TimePlot);

		linePlotAction =
			plotTypeMenu->addAction("Line");

		linePlotAction->setCheckable(true);
		linePlotAction->setChecked(
			currentPlotType() == PlotType::Line);

		barPlotAction =
			plotTypeMenu->addAction("Bar");

		barPlotAction->setCheckable(true);
		barPlotAction->setChecked(
			currentPlotType() == PlotType::Bar);

		menu.addSeparator();

		plotModeActions =
			addPlotModeMenu(menu,
				currentPlotMode());

		menu.addSeparator();
	}
	
	QAction* exportAction =
		menu.addAction("Export Plot as PNG...");
	
	exportAction->setEnabled(!mExportSeriesList.isEmpty());
	
	menu.addSeparator();

	QAction* showMajorGridAction =
		menu.addAction("Show Major Grid");
	
	showMajorGridAction->setCheckable(true);
	showMajorGridAction->setChecked(mPlotWidget->showMajorGrid());
	
	QAction* showMinorGridAction =
		menu.addAction("Show Minor Grid");
	
	showMinorGridAction->setCheckable(true);
	showMinorGridAction->setChecked(mPlotWidget->showMinorGrid());

	QAction* showAllHiddenSeriesAction =
		nullptr;
	
	if (!mHiddenSeriesNames.isEmpty())
	{
		showAllHiddenSeriesAction =
			menu.addAction("Show all hidden lines");
	
		menu.addSeparator();
	}


    QAction* selectedAction =
        menu.exec(mPlotWidget->mapToGlobal(position));
	
	if (plotModeActions.mTimePlotAction &&
		selectedAction == plotModeActions.mTimePlotAction)
	{
		setComboToPlotMode(mPlotModeCombo,
						   PlotMode::TimePlot);
	
		onPlotModeChanged();
	
		return;
	}
	
	if (plotModeActions.mXYPlotAction &&
		selectedAction == plotModeActions.mXYPlotAction)
	{
		setComboToPlotMode(mPlotModeCombo,
						   PlotMode::XYPlot);
	
		onPlotModeChanged();
	
		return;
	}


	if (linePlotAction &&
		selectedAction == linePlotAction)
	{
		setComboToPlotType(mPlotTypeCombo,
						   PlotType::Line);
	
		updatePlot();
	
		return;
	}
	
	if (barPlotAction &&
		selectedAction == barPlotAction)
	{
		setComboToPlotType(mPlotTypeCombo,
						   PlotType::Bar);
	
		updatePlot();
	
		return;
	}

	if (showAllHiddenSeriesAction &&
		selectedAction == showAllHiddenSeriesAction)
	{
		mHiddenSeriesNames.clear();
	
		updatePlot();
	
		return;
	}

	if (selectedAction == plotSelectionAction)
	{
		togglePlotSelectionPanel();
		return;
	}


    if (selectedAction == exportAction)
    {
        exportCurrentPlotAsPng();
        return;
    }

	if (selectedAction == showMajorGridAction)
	{
		mPlotWidget->setShowMajorGrid(
			showMajorGridAction->isChecked());
	
		return;
	}
	
	if (selectedAction == showMinorGridAction)
	{
		mPlotWidget->setShowMinorGrid(
			showMinorGridAction->isChecked());
	
		return;
	}

}

bool PlotBrowserWidget::askExportOptions(
    PlotExportOptions& exportOptions)
{
    QDialog dialog(this);
    dialog.setWindowTitle("Export Options");

    QVBoxLayout* mainLayout =
        new QVBoxLayout(&dialog);

    //
    // Layout group.
    //
    QGroupBox* layoutGroup =
        new QGroupBox("Layout", &dialog);

    QVBoxLayout* layoutLayout =
        new QVBoxLayout(layoutGroup);

    QRadioButton* wholeDiagramRadio =
        new QRadioButton("Whole diagram", layoutGroup);

    QRadioButton* currentViewRadio =
        new QRadioButton("Current view", layoutGroup);

    wholeDiagramRadio->setChecked(
        exportOptions.mRange == PlotExportRange::CompletePlot);

    currentViewRadio->setChecked(
        exportOptions.mRange == PlotExportRange::CurrentView);

    layoutLayout->addWidget(wholeDiagramRadio);
    layoutLayout->addWidget(currentViewRadio);

    mainLayout->addWidget(layoutGroup);

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

    mainLayout->addWidget(optionsGroup);

    //
    // Buttons.
    //
    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok |
                             QDialogButtonBox::Cancel |
                             QDialogButtonBox::Help,
                             &dialog);

    mainLayout->addWidget(buttonBox);

    connect(buttonBox,
            &QDialogButtonBox::accepted,
            &dialog,
            &QDialog::accept);

    connect(buttonBox,
            &QDialogButtonBox::rejected,
            &dialog,
            &QDialog::reject);

    connect(buttonBox,
            &QDialogButtonBox::helpRequested,
            &dialog,
            [&dialog]()
            {
                QMessageBox::information(
                    &dialog,
                    "Export Help",
                    "Whole diagram exports the complete plot range.\n"
                    "Current view exports the currently visible or zoomed range.\n"
                    "Legend controls whether the legend is included.\n"
                    "Font height controls exported plot text size.");
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

    return true;
}

void PlotBrowserWidget::exportCurrentPlotAsPng()
{
	if (mExportSeriesList.isEmpty() &&
		mExportMultiAxisSeriesList.isEmpty())        // was: mExportSeriesList.isEmpty()
	{
		QMessageBox::information(this, "No plot", "There is no plot data to export.");
		return;
	}

	PlotExportOptions exportOptions;
	
	if (!askExportOptions(exportOptions))
	{
		return;
	}

    QString defaultFileName =
        safePlotExportFileName(exportPlotTitle());

    if (!defaultFileName.endsWith(".png",
                                  Qt::CaseInsensitive))
    {
        defaultFileName += ".png";
    }

    QString filePath =
        QFileDialog::getSaveFileName(
            this,
            "Export Plot as PNG",
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

	const QPixmap pixmap =
		exportPlotPixmap(ContextExportPlotSize,
						 exportOptions);

    if (pixmap.isNull())
    {
        QMessageBox::critical(
            this,
            "Export failed",
            "Could not render the plot.");

        return;
    }

    if (!pixmap.save(filePath, "PNG"))
    {
        QMessageBox::critical(
            this,
            "Export failed",
            "Could not save the plot as PNG.");

        return;
    }
}

void PlotBrowserWidget::setStudies(
    const QVector<const Study*>& studies)
{
    mStudies = studies;
	mHiddenSeriesNames.clear();

    populateStudyTypes();
}

void PlotBrowserWidget::setFilePlotSettings(
    const QVector<FilePlotSettings>& settings)
{
    mFilePlotSettings = settings;

    updatePlot();
}

void PlotBrowserWidget::setAdvancedSignalPlottingEnabled(bool enabled)
{
	if (mAdvancedSignalPlottingEnabled == enabled)
	{
		return;
	}

	mAdvancedSignalPlottingEnabled = enabled;
	mHiddenSeriesNames.clear();

	if (mAdvancedSignalPlottingEnabled)
	{
		setComboToPlotMode(mPlotModeCombo, PlotMode::TimePlot);
		setComboToPlotType(mPlotTypeCombo, PlotType::Line);
	}

	if (mPlotWidget)
	{
		mPlotWidget->setShowLegend(mAdvancedSignalPlottingEnabled);
	}

	if (mMultiAxisPlotWidget)
	{
		mMultiAxisPlotWidget->setShowLegend(mAdvancedSignalPlottingEnabled);
	}

	updateAdvancedSignalControlVisibility();
	updatePlot();
}

void PlotBrowserWidget::updateAdvancedSignalControlVisibility()
{
    const bool isAdvanced =
        mAdvancedSignalPlottingEnabled;

    const bool isXYPlot =
        !isAdvanced &&
        currentPlotMode() == PlotMode::XYPlot;

    if (mXSignalLabel)
    {
        mXSignalLabel->setVisible(isXYPlot);
    }

    if (mXSignalCombo)
    {
        mXSignalCombo->setVisible(isXYPlot);
        mXSignalCombo->setEnabled(isXYPlot);
    }

    if (mSignalComboLabel)
    {
        mSignalComboLabel->setVisible(!isAdvanced);
    }

    if (mSignalCombo)
    {
        mSignalCombo->setVisible(!isAdvanced);
        mSignalCombo->setEnabled(!isAdvanced);
    }

    if (mSignalListLabel)
    {
        mSignalListLabel->setVisible(isAdvanced);
    }

    if (mSignalListWidget)
    {
        mSignalListWidget->setVisible(isAdvanced);
        mSignalListWidget->setEnabled(isAdvanced);
    }

    if (mPlotTypeCombo)
    {
        mPlotTypeCombo->setEnabled(!isAdvanced && !isXYPlot);
    }

	if (mSignalUnitWarningLabel)
	{
		mSignalUnitWarningLabel->setVisible(false);
	}

}

void PlotBrowserWidget::populateStudyTypes()
{
    /*
     * Capture current selections before clearing any combo box.
     * This preserves each plot window selection when files are added/removed.
     */
    const QString previouslySelectedStudyType =
        currentRawStudyType();

    const QVariant previouslySelectedComponent =
        mComponentCombo
            ? mComponentCombo->currentData()
            : QVariant();

    const QString previouslySelectedSignal =
        currentRawSignalName();

    mStudyTypeCombo->blockSignals(true);
    mComponentCombo->blockSignals(true);
    mSignalCombo->blockSignals(true);

    mStudyTypeCombo->clear();
    mComponentCombo->clear();
    mSignalCombo->clear();

    mSummaryLabel->clear();

    if (mPlotWidget)
    {
        mPlotWidget->clear();
    }

    const Study* study =
        referenceStudy();

    if (!study)
    {
        mStudyTypeCombo->blockSignals(false);
        mComponentCombo->blockSignals(false);
        mSignalCombo->blockSignals(false);
        return;
    }

    const QStringList studyTypes =
        study->getStudyTypeNames();

    for (const QString& rawStudyType : studyTypes)
    {
        const QString displayStudyType =
            PlotDisplayMapper::displayStudyTypeName(
                rawStudyType,
                mNameDisplayMode);

        mStudyTypeCombo->addItem(displayStudyType,
                                 rawStudyType);
    }

    const int previousStudyTypeIndex =
        mStudyTypeCombo->findData(previouslySelectedStudyType);

    if (previousStudyTypeIndex >= 0)
    {
        mStudyTypeCombo->setCurrentIndex(previousStudyTypeIndex);
    }
    else if (mStudyTypeCombo->count() > 0)
    {
        mStudyTypeCombo->setCurrentIndex(0);
    }

    mStudyTypeCombo->blockSignals(false);
    mComponentCombo->blockSignals(false);
    mSignalCombo->blockSignals(false);

    populateComponents();

    /*
     * populateComponents() and populateSignals() already try to preserve
     * their own current values, but during a full refresh the combo boxes
     * were cleared above. Restore component/signal explicitly here.
     */
    const int previousComponentIndex =
        mComponentCombo->findData(previouslySelectedComponent);

    if (previousComponentIndex >= 0)
    {
        mComponentCombo->setCurrentIndex(previousComponentIndex);
    }

    populateSignals();

    const int previousSignalIndex =
        mSignalCombo->findData(previouslySelectedSignal);

    if (previousSignalIndex >= 0)
    {
        mSignalCombo->setCurrentIndex(previousSignalIndex);
    }

    updatePlot();
}

void PlotBrowserWidget::populateComponents()
{
    const QVariant previouslySelectedComponent =
        mComponentCombo->currentData();

    mComponentCombo->blockSignals(true);
    mComponentCombo->clear();

    const Study* study =
        referenceStudy();

    if (!study)
    {
        mComponentCombo->blockSignals(false);
        populateSignals();
        return;
    }

    const QString studyType =
    currentRawStudyType();

    const QString targetGroup =
        study->getTargetGroupForStudyType(studyType);

    if (targetGroup.isEmpty())
    {
        mComponentCombo->blockSignals(false);
        populateSignals();
        return;
    }

    const QList<int> componentIds =
        study->getComponentIds(targetGroup);

    for (int componentId : componentIds)
    {
        const QString componentName =
            study->getComponentName(targetGroup,
                                    componentId);

        const QString displayText =
            QString("[%1] %2")
                .arg(componentId)
                .arg(componentName);

        mComponentCombo->addItem(displayText,
                                 componentId);
    }

    const int previousIndex =
        mComponentCombo->findData(previouslySelectedComponent);

    if (previousIndex >= 0)
    {
        mComponentCombo->setCurrentIndex(previousIndex);
    }
    else if (mComponentCombo->count() > 0)
    {
        mComponentCombo->setCurrentIndex(0);
    }

    mComponentCombo->blockSignals(false);

    populateSignals();
}

void PlotBrowserWidget::populateSignals()
{
	const QString previouslySelectedSignal =
		currentRawSignalName();
	
	const QStringList previouslySelectedSignals =
		currentRawYSignalNames();
	
	const QString previouslySelectedXSignal =
		currentRawXSignalName();
	
	mXSignalCombo->blockSignals(true);
	mSignalCombo->blockSignals(true);
	
	if (mSignalListWidget)
	{
		mSignalListWidget->blockSignals(true);
	}
	
	mXSignalCombo->clear();
	mSignalCombo->clear();
	
	if (mSignalListWidget)
	{
		mSignalListWidget->clear();
	}

    const Study* study =
        referenceStudy();

    if (!study)
    {
		mXSignalCombo->blockSignals(false);
		mSignalCombo->blockSignals(false);
		
		if (mSignalListWidget)
		{
			mSignalListWidget->blockSignals(false);
		}

        updatePlot();

        return;
    }

    const QString studyType =
        currentRawStudyType();

    if (studyType.isEmpty())
    {
		mXSignalCombo->blockSignals(false);
		mSignalCombo->blockSignals(false);
		
		if (mSignalListWidget)
		{
			mSignalListWidget->blockSignals(false);
		}

        updatePlot();

        return;
    }

    const QStringList signalNames =
        study->getSignalSchemaNames(studyType,
                                    false);

	for (int signalIndex = 0;
		 signalIndex < signalNames.size();
		 ++signalIndex)
	{
		const QString rawSignalName =
			signalNames[signalIndex];
	
		const QString displaySignalName =
			PlotDisplayMapper::displaySignalName(
				studyType,
				rawSignalName,
				mNameDisplayMode);
	
		const QColor signalColor =
			advancedSignalColorForSignal(studyType,
				rawSignalName,
				signalIndex);
	
		mXSignalCombo->addItem(displaySignalName,
							   rawSignalName);
	
		mSignalCombo->addItem(displaySignalName,
							  rawSignalName);
	
		if (mSignalListWidget)
		{
			QListWidgetItem* item =
				new QListWidgetItem(displaySignalName,
									mSignalListWidget);
	
			item->setData(Qt::UserRole,
				rawSignalName);

			updateAdvancedSignalListItemColor(item,
				signalColor);
	
			item->setFlags(item->flags() |
						   Qt::ItemIsUserCheckable);
	
			item->setCheckState(
				previouslySelectedSignals.contains(rawSignalName)
					? Qt::Checked
					: Qt::Unchecked);
		}
	}

    const int previousXSignalIndex =
        mXSignalCombo->findData(previouslySelectedXSignal);

    if (previousXSignalIndex >= 0)
    {
        mXSignalCombo->setCurrentIndex(previousXSignalIndex);
    }
    else if (mXSignalCombo->count() > 0)
    {
        mXSignalCombo->setCurrentIndex(0);
    }

    const int previousSignalIndex =
        mSignalCombo->findData(previouslySelectedSignal);

    if (previousSignalIndex >= 0)
    {
        mSignalCombo->setCurrentIndex(previousSignalIndex);
    }
    else if (mSignalCombo->count() > 0)
    {
        if (mSignalCombo->count() > 1)
        {
            mSignalCombo->setCurrentIndex(1);
        }
        else
        {
            mSignalCombo->setCurrentIndex(0);
        }
    }

	if (mSignalListWidget)
	{
		bool hasCheckedSignal = false;
	
		for (int i = 0; i < mSignalListWidget->count(); ++i)
		{
			QListWidgetItem* item =
				mSignalListWidget->item(i);
	
			if (item &&
				item->checkState() == Qt::Checked)
			{
				hasCheckedSignal = true;
				break;
			}
		}
	
		if (!hasCheckedSignal &&
			mSignalListWidget->count() > 0)
		{
			const int signalIndex =
				qBound(0,
					   mSignalCombo ? mSignalCombo->currentIndex() : 0,
					   mSignalListWidget->count() - 1);
	
			QListWidgetItem* item =
				mSignalListWidget->item(signalIndex);
	
			if (item)
			{
				item->setCheckState(Qt::Checked);
			}
		}
	}
	
	updateAdvancedSignalControlVisibility();

	mXSignalCombo->blockSignals(false);
	mSignalCombo->blockSignals(false);
	
	if (mSignalListWidget)
	{
		mSignalListWidget->blockSignals(false);
	}

    updatePlot();
}

void PlotBrowserWidget::updatePlot()
{
    const Study* reference =
        referenceStudy();

    if (!reference)
    {
        mSummaryLabel->setText("No study loaded.");
		clearExportCache();

		if (mPlotStackWidget &&
			mPlotWidget)
		{
			mPlotStackWidget->setCurrentWidget(mPlotWidget);
		}

		if (mPlotWidget)
		{
			mPlotWidget->clear();
		}

		if (mMultiAxisPlotWidget)
		{
			mMultiAxisPlotWidget->clear();
		}

        return;
    }

	if (mAdvancedSignalPlottingEnabled)
	{
		updateAdvancedSignalPlot();
		return;
	}

	const QString studyType =
		currentRawStudyType();

    const QString targetGroup =
        reference->getTargetGroupForStudyType(studyType);

	const PlotMode plotMode =
		currentPlotMode();
	
	const QString xSignalName =
		currentRawXSignalName();
	
	const QString ySignalName =
		currentRawSignalName();
	
	const int componentId =
		mComponentCombo->currentData().toInt();
	
	if (studyType.isEmpty() ||
		targetGroup.isEmpty() ||
		ySignalName.isEmpty() ||
		(plotMode == PlotMode::XYPlot && xSignalName.isEmpty()))

    {
        mSummaryLabel->setText("No signal selected.");
		clearExportCache();

        if (mPlotWidget)
        {
            mPlotWidget->clear();
        }

        return;
    }

    QVector<PlotSeries> seriesList;
    QStringList unavailableFiles;

    int enabledFileCount = 0;
    int plottedFileCount = 0;

    for (int i = 0; i < mStudies.size(); ++i)
    {
        if (i >= mFilePlotSettings.size())
        {
            continue;
        }

        const FilePlotSettings& settings =
            mFilePlotSettings[i];

        if (!settings.mEnabled)
        {
            continue;
        }

        ++enabledFileCount;

        const Study* study =
            mStudies[i];

        if (!study)
        {
            continue;
        }

        const QString fileTargetGroup =
            study->getTargetGroupForStudyType(studyType);

		QVector<double> xValues;
		
		if (plotMode == PlotMode::TimePlot)
		{
			xValues =
				study->getTimeValues();
		}
		else
		{
			xValues =
				study->getSignalValues(fileTargetGroup,
									   componentId,
									   xSignalName);
		}
		
		const QVector<double> yValues =
			study->getSignalValues(fileTargetGroup,
								   componentId,
								   ySignalName);


		const QString baseFileName =
			studyDisplayName(study);
		
		const QString fileName =
			duplicateAwareStudyDisplayName(mStudies,
										   i,
										   baseFileName);

		if (!xValues.isEmpty() &&
			!yValues.isEmpty() &&
			xValues.size() == yValues.size())
		{
			PlotSeries series;
			series.mName = fileName;
			series.mXValues = xValues;
			series.mYValues = yValues;
			series.mColor = settings.mColor;
			series.mLineThickness = settings.mLineThickness;
			series.mLineStyle = settings.mLineStyle;

			if (!mHiddenSeriesNames.contains(series.mName))
			{
				seriesList.append(series);
			}

            ++plottedFileCount;
        }
        else
        {
            unavailableFiles.append(fileName);
        }
    }

    if (enabledFileCount == 0)
    {
        mSummaryLabel->setText("No files selected for plotting.");
		clearExportCache();

        if (mPlotWidget)
        {
            mPlotWidget->clear();
        }

        return;
    }

    QString summary;

    summary += QString("Plotted: %1 / %2 selected file(s)\n")
                   .arg(plottedFileCount)
                   .arg(enabledFileCount);

    if (!unavailableFiles.isEmpty())
    {
        summary += "\nUnavailable in:\n";

        for (const QString& fileName : unavailableFiles)
        {
            summary += QString("  - %1\n")
                           .arg(fileName);
        }
    }

	if (!mHiddenSeriesNames.isEmpty())
	{
		summary += QString("\nHidden in this plot: %1 line(s)\n")
					   .arg(mHiddenSeriesNames.size());
	}

    mSummaryLabel->setText(summary);

	const QString displayStudyType =
	    mStudyTypeCombo
	        ? mStudyTypeCombo->currentText()
	        : studyType;

	const QString displayXSignalName =
		mXSignalCombo
			? mXSignalCombo->currentText()
			: xSignalName;
	
	const QString displayYSignalName =
		mSignalCombo
			? mSignalCombo->currentText()
			: ySignalName;
	
	const QString title =
		plotMode == PlotMode::TimePlot
			? QString("%1 [%2] - %3")
				  .arg(displayStudyType)
				  .arg(componentId)
				  .arg(displayYSignalName)
			: QString("%1 [%2] - %3 vs %4")
				  .arg(displayStudyType)
				  .arg(componentId)
				  .arg(displayYSignalName)
				  .arg(displayXSignalName);
	
	const QString xAxisLabel =
		plotMode == PlotMode::TimePlot
			? QString("Time (s)")
			: displayXSignalName;
	
	const QString yAxisLabel =
		displayYSignalName;
	
	const PlotType effectivePlotType =
		plotMode == PlotMode::XYPlot
			? PlotType::Line
			: currentPlotType();

	
	mExportSeriesList =
		seriesList;
	
	mExportTitle =
		title;
	
	mExportXAxisLabel =
		xAxisLabel;
	
	mExportYAxisLabel =
		yAxisLabel;
	
	mExportPlotType =
		effectivePlotType;

	mExportIsMultiAxis = false;
	
	if (mPlotStackWidget &&
		mPlotWidget)
	{
		mPlotStackWidget->setCurrentWidget(mPlotWidget);
	}

	if (mMultiAxisPlotWidget)
	{
		mMultiAxisPlotWidget->clear();
	}

	if (mPlotWidget)
	{
		mPlotWidget->setPlotType(effectivePlotType);

		mPlotWidget->setSeries(mExportSeriesList,
			mExportTitle,
			mExportXAxisLabel,
			mExportYAxisLabel);
	}

}

QString PlotBrowserWidget::advancedSignalColorOverrideKey(
	const QString& studyType,
	const QString& rawSignalName) const
{
	return QString("%1::%2")
		.arg(studyType,
			rawSignalName);
}

QColor PlotBrowserWidget::advancedSignalColorForSignal(
	const QString& studyType,
	const QString& rawSignalName,
	int fallbackIndex) const
{
	/*
	 * 1. User override has highest priority.
	 */
	const QString key =
		advancedSignalColorOverrideKey(studyType,
			rawSignalName);

	if (mAdvancedSignalColorOverrides.contains(key))
	{
		const QColor overrideColor =
			mAdvancedSignalColorOverrides.value(key);

		if (overrideColor.isValid())
		{
			return overrideColor;
		}
	}

	/*
	 * 2. If the signal checklist already has this signal,
	 * use the colour stored in the list item.
	 *
	 * This keeps the colour shown beside the signal exactly
	 * same as the colour used by the plotted line.
	 */
	if (mSignalListWidget)
	{
		for (int i = 0;
			i < mSignalListWidget->count();
			++i)
		{
			const QListWidgetItem* item =
				mSignalListWidget->item(i);

			if (!item)
			{
				continue;
			}

			const QString itemRawSignalName =
				item->data(Qt::UserRole).toString();

			if (itemRawSignalName != rawSignalName)
			{
				continue;
			}

			const QColor itemColor(
				item->data(AdvancedSignalColorRole).toString());

			if (itemColor.isValid())
			{
				return itemColor;
			}
		}
	}

	/*
	 * 3. Final fallback.
	 */
	return advancedSignalColor(fallbackIndex);
}

void PlotBrowserWidget::updateAdvancedSignalListItemColor(
	QListWidgetItem* item,
	const QColor& color)
{
	if (!item)
	{
		return;
	}

	item->setIcon(
		makeAdvancedSignalColorIcon(color));

	item->setData(AdvancedSignalColorRole,
		color.name());
}

void PlotBrowserWidget::showAdvancedSignalColorMenu(
	const QPoint& position)
{
	if (!mAdvancedSignalPlottingEnabled ||
		!mSignalListWidget)
	{
		return;
	}

	QListWidgetItem* item =
		mSignalListWidget->itemAt(position);

	if (!item)
	{
		return;
	}

	const QString studyType =
		currentRawStudyType();

	const QString rawSignalName =
		item->data(Qt::UserRole).toString();

	if (studyType.isEmpty() ||
		rawSignalName.isEmpty())
	{
		return;
	}

	QMenu menu(this);

	QAction* changeColourAction =
		menu.addAction("Change Signal Colour...");

	QAction* resetColourAction =
		menu.addAction("Reset Signal Colour");

	QAction* selectedAction =
		menu.exec(
			mSignalListWidget->viewport()->mapToGlobal(position));

	if (!selectedAction)
	{
		return;
	}

	const QString key =
		advancedSignalColorOverrideKey(studyType,
			rawSignalName);

	if (selectedAction == changeColourAction)
	{
		QColor currentColor(
			item->data(AdvancedSignalColorRole).toString());

		if (!currentColor.isValid())
		{
			currentColor =
				advancedSignalColor(mSignalListWidget->row(item));
		}

		const QColor chosenColor =
			QColorDialog::getColor(currentColor,
				this,
				"Select Signal Colour");

		if (!chosenColor.isValid())
		{
			return;
		}

		mAdvancedSignalColorOverrides.insert(key,
			chosenColor);

		updateAdvancedSignalListItemColor(item,
			chosenColor);

		updatePlot();

		return;
	}

	if (selectedAction == resetColourAction)
	{
		mAdvancedSignalColorOverrides.remove(key);

		const QColor defaultColor =
			advancedSignalColor(mSignalListWidget->row(item));

		updateAdvancedSignalListItemColor(item,
			defaultColor);

		updatePlot();

		return;
	}
}

void PlotBrowserWidget::updateAdvancedMultiAxisSignalPlot()
{
	const Study* reference =
		referenceStudy();

	if (!reference)
	{
		mSummaryLabel->setText("No study loaded.");

		if (mMultiAxisPlotWidget)
		{
			mMultiAxisPlotWidget->clear();
		}

		clearExportCache();
		return;
	}

	const QString studyType =
		currentRawStudyType();

	const QString targetGroup =
		reference->getTargetGroupForStudyType(studyType);

	const int componentId =
		mComponentCombo
		? mComponentCombo->currentData().toInt()
		: 0;

	const QStringList selectedSignals =
		currentRawYSignalNames();

	if (studyType.isEmpty() ||
		targetGroup.isEmpty() ||
		selectedSignals.isEmpty())
	{
		mSummaryLabel->setText(
			"Multi-axis mode: select one or more Y signals.");

		if (mMultiAxisPlotWidget)
		{
			mMultiAxisPlotWidget->clear();
		}

		clearExportCache();
		return;
	}

	QStringList unitKeys =
		selectedRealUnitKeys(studyType,
			selectedSignals);

	if (unitKeys.isEmpty())
	{
		/*
		 * Only unitless signals selected.
		 * There is no need for multi-axis.
		 */
		if (mPlotStackWidget &&
			mPlotWidget)
		{
			mPlotStackWidget->setCurrentWidget(mPlotWidget);
		}

		updateAdvancedSignalPlot();
		return;
	}

	if (unitKeys.size() == 1)
	{
		/*
		 * Single real unit family.
		 * Existing advanced plot is better.
		 */
		if (mPlotStackWidget &&
			mPlotWidget)
		{
			mPlotStackWidget->setCurrentWidget(mPlotWidget);
		}

		updateAdvancedSignalPlot();
		return;
	}

	const bool showManyAxisWarning =
		unitKeys.size() >= 4;

	if (mSignalUnitWarningLabel)
	{
		mSignalUnitWarningLabel->clear();
		mSignalUnitWarningLabel->setVisible(false);
	}

	QVector<int> selectedFileIndexes;

	const int fileCount =
		qMin(mStudies.size(),
			mFilePlotSettings.size());

	for (int fileIndex = 0;
		fileIndex < fileCount;
		++fileIndex)
	{
		if (mFilePlotSettings[fileIndex].mEnabled)
		{
			selectedFileIndexes.append(fileIndex);
		}
	}

	if (selectedFileIndexes.isEmpty())
	{
		mSummaryLabel->setText(
			"Multi-axis mode: select one or more files.");

		if (mMultiAxisPlotWidget)
		{
			mMultiAxisPlotWidget->clear();
		}

		clearExportCache();
		return;
	}

	QVector<MultiAxisPlotAxis> axes;

	for (int i = 0;
		i < unitKeys.size();
		++i)
	{
		const QString unitKey =
			unitKeys[i];

		MultiAxisPlotAxis axis;
		axis.mKey =
			unitKey;

		axis.mUseRightSide =
			(i % 2 == 1);

		axis.mLabel =
			axisDisplayLabelForUnitKey(studyType,
				selectedSignals,
				unitKey);

		/*
		 * Colour the axis using the first selected signal that belongs
		 * to this unit family.
		 *
		 * If multiple signals share the same axis, the axis colour follows
		 * the first one in the selected signal list.
		 */
		for (int signalIndex = 0;
			signalIndex < selectedSignals.size();
			++signalIndex)
		{
			const QString rawSignalName =
				selectedSignals[signalIndex];

			const QString rawUnit =
				signalUnitForYAxis(studyType,
					rawSignalName).trimmed();

			const QString signalUnitKey =
				unitCompatibilityKey(rawUnit);

			if (signalUnitKey != unitKey)
			{
				continue;
			}

			axis.mColor =
				advancedSignalColorForSignal(studyType,
					rawSignalName,
					signalIndex);

			break;
		}

		axes.append(axis);
	}

	QVector<MultiAxisPlotSeries> multiAxisSeriesList;
	QStringList unavailableLines;
	QStringList plottedSignalDisplayNames;

	int selectedSignalIndex = 0;

	for (const QString& rawSignalName : selectedSignals)
	{
		QString rawUnit =
			signalUnitForYAxis(studyType,
				rawSignalName).trimmed();

		QString axisKey =
			unitCompatibilityKey(rawUnit);

		/*
		 * Unitless / unknown signal goes with the first real axis.
		 */
		if (axisKey.isEmpty())
		{
			axisKey =
				unitKeys.first();
		}

		const QColor signalColor =
			advancedSignalColorForSignal(studyType,
				rawSignalName,
				selectedSignalIndex);

		const QString displaySignalName =
			PlotDisplayMapper::displaySignalName(
				studyType,
				rawSignalName,
				mNameDisplayMode);

		if (!plottedSignalDisplayNames.contains(displaySignalName))
		{
			plottedSignalDisplayNames.append(displaySignalName);
		}

		for (int selectedFileIndex : selectedFileIndexes)
		{
			if (selectedFileIndex < 0 ||
				selectedFileIndex >= mStudies.size() ||
				selectedFileIndex >= mFilePlotSettings.size())
			{
				continue;
			}

			const Study* study =
				mStudies[selectedFileIndex];

			if (!study)
			{
				continue;
			}

			const FilePlotSettings& settings =
				mFilePlotSettings[selectedFileIndex];

			const QString fileTargetGroup =
				study->getTargetGroupForStudyType(studyType);

			const QVector<double> timeValues =
				study->getTimeValues();

			const QVector<double> signalValues =
				study->getSignalValues(fileTargetGroup,
					componentId,
					rawSignalName);

			const QString fileName =
				studyDisplayName(study);

			if (!timeValues.isEmpty() &&
				!signalValues.isEmpty() &&
				timeValues.size() == signalValues.size())
			{
				MultiAxisPlotSeries series;

				series.mName =
					QString("%1 - %2")
					.arg(fileName,
						displaySignalName);

				series.mXValues =
					timeValues;

				series.mYValues =
					signalValues;

				series.mAxisKey =
					axisKey;

				/*
				 * Advanced multi-axis visual encoding:
				 * colour      = signal
				 * line style  = file
				 * thickness   = file
				 */
				series.mColor =
					signalColor;

				series.mLineThickness =
					settings.mLineThickness;

				series.mLineStyle =
					settings.mLineStyle;

				if (!mHiddenSeriesNames.contains(series.mName))
				{
					multiAxisSeriesList.append(series);
				}
			}
			else
			{
				unavailableLines.append(
					QString("%1 - %2")
					.arg(fileName,
						displaySignalName));
			}
		}

		++selectedSignalIndex;
	}

	const QString displayStudyType =
		mStudyTypeCombo
		? mStudyTypeCombo->currentText()
		: studyType;

	const QString title =
		QString("%1 [%2] - %3 (%4 files, multi-axis)")
		.arg(displayStudyType)
		.arg(componentId)
		.arg(compactSignalNamesLabel(plottedSignalDisplayNames))
		.arg(selectedFileIndexes.size());

	if (mPlotStackWidget &&
		mMultiAxisPlotWidget)
	{
		mPlotStackWidget->setCurrentWidget(mMultiAxisPlotWidget);
	}

	if (mMultiAxisPlotWidget)
	{
		mMultiAxisPlotWidget->setSeries(multiAxisSeriesList,
			axes,
			title,
			"Time (s)");
	}

	if (mPlotWidget)
	{
		mPlotWidget->clear();
	}

	clearExportCache();

	mExportMultiAxisSeriesList = multiAxisSeriesList;
	mExportMultiAxisAxes = axes;
	mExportTitle = title;
	mExportXAxisLabel = "Time (s)";
	mExportIsMultiAxis = true;

	QString summary;

	summary += "Advanced multi-axis plotting\n";
	summary += QString("Files selected: %1\n")
		.arg(selectedFileIndexes.size());
	summary += QString("Signals selected: %1\n")
		.arg(selectedSignals.size());
	summary += QString("Lines plotted: %1\n")
		.arg(multiAxisSeriesList.size());
	summary += "\nAxes:\n";

	int leftAxisNumber = 1;
	int rightAxisNumber = 1;

	for (const MultiAxisPlotAxis& axis : axes)
	{
		if (axis.mUseRightSide)
		{
			summary += QString("  Right axis %1: %2\n")
				.arg(rightAxisNumber)
				.arg(axis.mLabel);

			++rightAxisNumber;
		}
		else
		{
			summary += QString("  Left axis %1: %2\n")
				.arg(leftAxisNumber)
				.arg(axis.mLabel);

			++leftAxisNumber;
		}
	}

	summary += "Colour: signal\n";
	summary += "Line type/thickness: file\n";
	if (showManyAxisWarning)
	{
		summary += "\nWarning: 4 or more Y axes are selected. "
			"The plot may become crowded, but all axes will still be shown.\n";
	}
	if (!mHiddenSeriesNames.isEmpty())
	{
		summary += QString("\nHidden in this plot: %1 line(s)\n")
			.arg(mHiddenSeriesNames.size());
	}
	//summary += "\nExport: not supported for multi-axis plots yet.\n";

	if (!unavailableLines.isEmpty())
	{
		summary += "\nUnavailable lines:\n";

		const int maxLinesToShow =
			qMin(6,
				unavailableLines.size());

		for (int i = 0;
			i < maxLinesToShow;
			++i)
		{
			summary += QString("  - %1\n")
				.arg(unavailableLines[i]);
		}

		if (unavailableLines.size() > maxLinesToShow)
		{
			summary += QString("  - +%1 more\n")
				.arg(unavailableLines.size() -
					maxLinesToShow);
		}
	}

	mSummaryLabel->setText(summary);
}

void PlotBrowserWidget::updateAdvancedSignalPlot()
{
    const Study* reference =
        referenceStudy();

    if (!reference)
    {
        mSummaryLabel->setText("No study loaded.");
        clearExportCache();

        if (mPlotWidget)
        {
            mPlotWidget->clear();
        }

        return;
    }

    const QString studyType =
        currentRawStudyType();

    const QString targetGroup =
        reference->getTargetGroupForStudyType(studyType);

    const int componentId =
        mComponentCombo
            ? mComponentCombo->currentData().toInt()
            : 0;

    const QStringList selectedSignals =
        currentRawYSignalNames();

    if (studyType.isEmpty() ||
        targetGroup.isEmpty() ||
        selectedSignals.isEmpty())
    {
        mSummaryLabel->setText(
            "Advanced mode: select one or more Y signals.");

        clearExportCache();

        if (mPlotWidget)
        {
            mPlotWidget->clear();
        }

        return;
    }

	if (mAdvancedMultiAxisPlottingEnabled)
	{
		const QStringList unitKeys =
			selectedRealUnitKeys(studyType,
				selectedSignals);

		if (unitKeys.size() > 1)
		{
			updateAdvancedMultiAxisSignalPlot();
			return;
		}
	}

	QString commonSignalUnit;
	QString unitCompatibilityError;
	
	if (!selectedSignalUnitsAreCompatible(
			studyType,
			selectedSignals,
			mNameDisplayMode,
			commonSignalUnit,
			unitCompatibilityError))
	{
		if (mSignalUnitWarningLabel)
		{
			mSignalUnitWarningLabel->setText(
				unitCompatibilityError);
	
			mSignalUnitWarningLabel->setVisible(true);
		}
	
		mSummaryLabel->setText(
			"Advanced mode: incompatible signal units.");
	
		clearExportCache();
	
		if (mPlotWidget)
		{
			mPlotWidget->clear();
		}
	
		return;
	}
	
	if (mSignalUnitWarningLabel)
	{
		mSignalUnitWarningLabel->clear();
		mSignalUnitWarningLabel->setVisible(false);
	}

	const QStringList singleAxisUnitKeys =
		selectedRealUnitKeys(studyType,
			selectedSignals);

	if (!singleAxisUnitKeys.isEmpty())
	{
		commonSignalUnit =
			actualUnitTextForUnitKey(studyType,
				selectedSignals,
				singleAxisUnitKeys.first());
	}


	QVector<int> selectedFileIndexes;

	const int fileCount =
		qMin(mStudies.size(),
			mFilePlotSettings.size());

	for (int fileIndex = 0;
		fileIndex < fileCount;
		++fileIndex)
	{
		if (mFilePlotSettings[fileIndex].mEnabled)
		{
			selectedFileIndexes.append(fileIndex);
		}
	}

	if (selectedFileIndexes.isEmpty())
	{
		mSummaryLabel->setText(
			"Advanced mode: select one or more files.");

		clearExportCache();

		if (mPlotWidget)
		{
			mPlotWidget->clear();
		}

		return;
	}

	QVector<PlotSeries> seriesList;
	QStringList unavailableSignals;
	QStringList plottedSignalDisplayNames;

	const int expectedLineCount =
		selectedFileIndexes.size() * selectedSignals.size();

	if (expectedLineCount > 12)
	{
		/*
		 * Do not block. Just mention it in the summary later.
		 */
	}

	int selectedSignalIndex = 0;

	for (const QString& rawSignalName : selectedSignals)
	{
		const QColor signalColor =
			advancedSignalColorForSignal(studyType,
				rawSignalName,
				selectedSignalIndex);

		const QString displaySignalName =
			PlotDisplayMapper::displaySignalName(
				studyType,
				rawSignalName,
				mNameDisplayMode);

		if (!plottedSignalDisplayNames.contains(displaySignalName))
		{
			plottedSignalDisplayNames.append(displaySignalName);
		}

		for (int selectedFileIndex : selectedFileIndexes)
		{
			if (selectedFileIndex < 0 ||
				selectedFileIndex >= mStudies.size() ||
				selectedFileIndex >= mFilePlotSettings.size())
			{
				continue;
			}

			const Study* study =
				mStudies[selectedFileIndex];

			if (!study)
			{
				continue;
			}

			const FilePlotSettings& settings =
				mFilePlotSettings[selectedFileIndex];

			const QString fileTargetGroup =
				study->getTargetGroupForStudyType(studyType);

			const QVector<double> timeValues =
				study->getTimeValues();

			const QVector<double> signalValues =
				study->getSignalValues(fileTargetGroup,
					componentId,
					rawSignalName);

			const QString fileName =
				studyDisplayName(study);

			if (!timeValues.isEmpty() &&
				!signalValues.isEmpty() &&
				timeValues.size() == signalValues.size())
			{
				PlotSeries series;

				/*
				 * Important:
				 * In multi-file + multi-signal mode, the series name must
				 * identify both dimensions.
				 */
				series.mName =
					QString("%1 - %2")
					.arg(fileName,
						displaySignalName);

				series.mXValues =
					timeValues;

				series.mYValues =
					signalValues;

				/*
				 * Advanced visual encoding:
				 * colour      = signal
				 * line style  = file
				 * thickness   = file
				 */
				series.mColor =
					signalColor;

				series.mLineThickness =
					settings.mLineThickness;

				series.mLineStyle =
					settings.mLineStyle;

				if (!mHiddenSeriesNames.contains(series.mName))
				{
					seriesList.append(series);
				}
			}
			else
			{
				unavailableSignals.append(
					QString("%1 - %2")
					.arg(fileName,
						displaySignalName));
			}
		}

		++selectedSignalIndex;
	}

	QString summary;

	summary += "Advanced signal plotting\n";

	summary += QString("Files selected: %1\n")
		.arg(selectedFileIndexes.size());

	summary += QString("Signals selected: %1\n")
		.arg(selectedSignals.size());

	summary += QString("Lines plotted: %1 / %2\n")
		.arg(seriesList.size())
		.arg(expectedLineCount);

	summary += QString("Unit: %1\n")
		.arg(unitDisplayNameForSummary(
			commonSignalUnit));

	summary += "Colour: signal\n";
	summary += "Line type/thickness: file\n";

	if (expectedLineCount > 12)
	{
		summary += "\nWarning: many lines selected. "
			"Use hover or hide lines if the plot becomes crowded.\n";
	}

	if (!unavailableSignals.isEmpty())
	{
		summary += "\nUnavailable lines:\n";

		const int maxUnavailableToShow =
			qMin(6,
				unavailableSignals.size());

		for (int i = 0;
			i < maxUnavailableToShow;
			++i)
		{
			summary += QString("  - %1\n")
				.arg(unavailableSignals[i]);
		}

		if (unavailableSignals.size() > maxUnavailableToShow)
		{
			summary += QString("  - +%1 more\n")
				.arg(unavailableSignals.size() -
					maxUnavailableToShow);
		}
	}

	if (!mHiddenSeriesNames.isEmpty())
	{
		summary += QString("\nHidden in this plot: %1 line(s)\n")
			.arg(mHiddenSeriesNames.size());
	}

	mSummaryLabel->setText(summary);

    const QString displayStudyType =
        mStudyTypeCombo
            ? mStudyTypeCombo->currentText()
            : studyType;

	const QString signalNamesLabel =
		compactSignalNamesLabel(plottedSignalDisplayNames);
	
	QString title =
		QString("%1 [%2] - %3")
		.arg(displayStudyType)
		.arg(componentId)
		.arg(signalNamesLabel);

	if (selectedFileIndexes.size() > 1)
	{
		title += QString(" (%1 files)")
			.arg(selectedFileIndexes.size());
	}


    mExportSeriesList =
        seriesList;

    mExportTitle =
        title;

    mExportXAxisLabel =
        "Time (s)";

	mExportYAxisLabel =
		compactAdvancedYAxisLabel(plottedSignalDisplayNames,
								  commonSignalUnit);

    mExportPlotType =
        PlotType::Line;

	mExportIsMultiAxis = false;

	if (mPlotStackWidget &&
		mPlotWidget)
	{
		mPlotStackWidget->setCurrentWidget(mPlotWidget);
	}

	if (mMultiAxisPlotWidget)
	{
		mMultiAxisPlotWidget->clear();
	}

	if (mPlotWidget)
	{
		mPlotWidget->setPlotType(PlotType::Line);

		mPlotWidget->setSeries(mExportSeriesList,
			mExportTitle,
			mExportXAxisLabel,
			mExportYAxisLabel);
	}
}

const Study* PlotBrowserWidget::referenceStudy() const
{
    if (mStudies.isEmpty())
    {
        return nullptr;
    }

    return mStudies.first();
}

QString PlotBrowserWidget::studyDisplayName(
    const Study* study) const
{
    if (!study)
    {
        return QString();
    }

    const QFileInfo fileInfo(study->getFileName());

    /*
     * Show file name without .itf extension in plot legend / hover details.
     *
     * Example:
     *   first.itf -> first
     *   DynamicPluginProf.itf -> DynamicPluginProf
     */
    const QString nameWithoutExtension =
        fileInfo.completeBaseName();

    if (!nameWithoutExtension.isEmpty())
    {
        return nameWithoutExtension;
    }

    if (!fileInfo.fileName().isEmpty())
    {
        return fileInfo.fileName();
    }

    return study->getFileName();
}

PlotType PlotBrowserWidget::currentPlotType() const
{
    if (!mPlotTypeCombo)
    {
        return PlotType::Line;
    }

    return static_cast<PlotType>(
        mPlotTypeCombo->currentData().toInt());
}

PlotMode PlotBrowserWidget::currentPlotMode() const
{
    if (!mPlotModeCombo)
    {
        return PlotMode::TimePlot;
    }

    return static_cast<PlotMode>(
        mPlotModeCombo->currentData().toInt());
}

QString PlotBrowserWidget::currentRawXSignalName() const
{
    if (!mXSignalCombo)
    {
        return QString();
    }

    const QString rawSignalName =
        mXSignalCombo->currentData().toString();

    if (!rawSignalName.isEmpty())
    {
        return rawSignalName;
    }

    return mXSignalCombo->currentText();
}

QString PlotBrowserWidget::currentRawStudyType() const
{
    if (!mStudyTypeCombo)
    {
        return QString();
    }

    const QString rawStudyType =
        mStudyTypeCombo->currentData().toString();

    if (!rawStudyType.isEmpty())
    {
        return rawStudyType;
    }

    return mStudyTypeCombo->currentText();
}

QString PlotBrowserWidget::currentRawSignalName() const
{
    if (!mSignalCombo)
    {
        return QString();
    }

    const QString rawSignalName =
        mSignalCombo->currentData().toString();

    if (!rawSignalName.isEmpty())
    {
        return rawSignalName;
    }

    return mSignalCombo->currentText();
}

QStringList PlotBrowserWidget::currentRawYSignalNames() const
{
    QStringList signalNames;

    if (!mAdvancedSignalPlottingEnabled)
    {
        const QString signalName =
            currentRawSignalName();

        if (!signalName.isEmpty())
        {
            signalNames.append(signalName);
        }

        return signalNames;
    }

    if (!mSignalListWidget)
    {
        return signalNames;
    }

    for (int i = 0; i < mSignalListWidget->count(); ++i)
    {
        const QListWidgetItem* item =
            mSignalListWidget->item(i);

        if (!item ||
            item->checkState() != Qt::Checked)
        {
            continue;
        }

        const QString rawSignalName =
            item->data(Qt::UserRole).toString();

        if (!rawSignalName.isEmpty())
        {
            signalNames.append(rawSignalName);
        }
    }

    return signalNames;
}

QPixmap PlotBrowserWidget::exportPlotPixmap(int targetWidth) const
{
    /*
     * Backward-compatible wrapper.
     * Route the old API through the fixed-size export path.
     */
    if (targetWidth <= 0)
    {
        targetWidth = 1600;
    }

    const int targetHeight =
        qMax(1,
             static_cast<int>(
                 static_cast<double>(targetWidth) * 9.0 / 16.0));

    return exportPlotPixmap(QSize(targetWidth,
                                  targetHeight));
}

void PlotBrowserWidget::onStudyTypeChanged()
{
    populateComponents();
}

void PlotBrowserWidget::onComponentChanged()
{
    populateSignals();
}

void PlotBrowserWidget::onSignalChanged()
{
    updatePlot();
}

void PlotBrowserWidget::onPlotTypeChanged()
{
	if (mAdvancedSignalPlottingEnabled)
	{
		setComboToPlotType(mPlotTypeCombo,
						   PlotType::Line);
	
		updatePlot();
	
		return;
	}

    updatePlot();
}

void PlotBrowserWidget::onPlotModeChanged()
{
	if (mAdvancedSignalPlottingEnabled)
	{
		setComboToPlotMode(mPlotModeCombo,
						   PlotMode::TimePlot);
	
		updateAdvancedSignalControlVisibility();
		updatePlot();
	
		return;
	}
	
    const bool isXYPlot =
        currentPlotMode() == PlotMode::XYPlot;

    if (mXSignalLabel)
    {
        mXSignalLabel->setVisible(isXYPlot);
    }

    if (mXSignalCombo)
    {
        mXSignalCombo->setVisible(isXYPlot);
        mXSignalCombo->setEnabled(isXYPlot);
    }

    if (mPlotTypeCombo)
    {
        mPlotTypeCombo->setEnabled(!isXYPlot);
    }

    /*
     * X-Y plot changes the meaning of the curve.
     * Hidden line state should not carry across plot modes.
     */
    mHiddenSeriesNames.clear();

    updatePlot();
}

void PlotBrowserWidget::setPlotSelectionPanelVisible(bool visible)
{
    if (!mControlPanel)
    {
        return;
    }

    mControlPanel->setVisible(visible);

    if (mPlotWidget)
    {
        mPlotWidget->updateGeometry();
        mPlotWidget->update();
    }

    updateGeometry();
}

void PlotBrowserWidget::togglePlotSelectionPanel()
{
    if (!mControlPanel)
    {
        return;
    }

    setPlotSelectionPanelVisible(
        !mControlPanel->isVisible());
}

void PlotBrowserWidget::setNameDisplayMode(NameDisplayMode mode)
{
    if (mNameDisplayMode == mode)
    {
        return;
    }

    mNameDisplayMode = mode;

    populateStudyTypes();
}

QSize PlotBrowserWidget::sizeHint() const
{
    /*
     * Preferred size when space is available.
     * This is not a hard constraint.
     */
    return QSize(760, 420);
}

QSize PlotBrowserWidget::minimumSizeHint() const
{
	const int controlPanelMinHeight =
		mControlPanel
		? mControlPanel->minimumSizeHint().height()
		: 180;

	return QSize(320,
		qMax(180, controlPanelMinHeight));
}

void PlotBrowserWidget::clearExportCache()
{
	mExportSeriesList.clear();
	mExportMultiAxisSeriesList.clear();   // new
	mExportMultiAxisAxes.clear();         // new
	mExportIsMultiAxis = false;           // new
	mExportTitle.clear();
	mExportXAxisLabel.clear();
	mExportYAxisLabel.clear();
	mExportPlotType = PlotType::Line;
}

QPixmap PlotBrowserWidget::exportPlotPixmap(
	const QSize& exportSize,
	const PlotExportOptions& exportOptions) const
{
	QSize finalExportSize = exportSize;

	if (!finalExportSize.isValid() ||
		finalExportSize.width() <= 0 ||
		finalExportSize.height() <= 0)
	{
		finalExportSize = QSize(1600, 900);
	}

	// Build a legend-compatible series list regardless of mode.
	QVector<PlotSeries> legendSeriesList;

	if (mExportIsMultiAxis)
	{
		for (const MultiAxisPlotSeries& series : mExportMultiAxisSeriesList)
		{
			PlotSeries legendSeries;
			legendSeries.mName = series.mName;
			legendSeries.mColor = series.mColor;
			legendSeries.mLineThickness = series.mLineThickness;
			legendSeries.mLineStyle = series.mLineStyle;

			legendSeriesList.append(legendSeries);
		}
	}
	else
	{
		legendSeriesList = mExportSeriesList;
	}

	const int legendHeight =
		exportOptions.mIncludeLegend
		? exportLegendHeightForSeriesCount(legendSeriesList.size())
		: 0;

	QSize plotOnlySize = finalExportSize;

	if (legendHeight > 0 &&
		finalExportSize.height() > legendHeight + 200)
	{
		plotOnlySize.setHeight(finalExportSize.height() - legendHeight);
	}

	QFont exportFont;
	exportFont.setPointSize(qMax(exportOptions.mFontHeight, 8));

	QPixmap pixmap(finalExportSize);
	pixmap.fill(Qt::white);

	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);

	if (mExportIsMultiAxis)
	{
		MultiAxisPlotWidget exportWidget;

		exportWidget.resize(plotOnlySize);
		exportWidget.setMinimumSize(plotOnlySize);
		exportWidget.setMaximumSize(plotOnlySize);
		exportWidget.setFont(exportFont);
		exportWidget.setTextScale(1.0);

		exportWidget.setSeries(mExportMultiAxisSeriesList,
			mExportMultiAxisAxes,
			mExportTitle,
			mExportXAxisLabel);

		if (exportOptions.mRange == PlotExportRange::CurrentView &&
			mMultiAxisPlotWidget)
		{
			double minX = 0.0, maxX = 0.0;
			bool hasCustomRange = false;

			if (mMultiAxisPlotWidget->currentVisibleXRange(minX, maxX, hasCustomRange))
			{
				exportWidget.applyExternalXRange(minX, maxX, true);
			}
		}

		exportWidget.render(&painter, QPoint(0, 0));
	}
	else
	{
		PlotWidget exportWidget;

		exportWidget.resize(plotOnlySize);
		exportWidget.setMinimumSize(plotOnlySize);
		exportWidget.setMaximumSize(plotOnlySize);
		exportWidget.setPlotType(mExportPlotType);
		exportWidget.setFont(exportFont);
		exportWidget.setTextScale(1.0);

		exportWidget.setSeries(mExportSeriesList,
			mExportTitle,
			mExportXAxisLabel,
			mExportYAxisLabel);

		if (exportOptions.mRange == PlotExportRange::CurrentView &&
			mPlotWidget)
		{
			double minX = 0.0, maxX = 0.0;
			bool hasCustomRange = false;

			if (mPlotWidget->currentVisibleXRange(minX, maxX, hasCustomRange))
			{
				exportWidget.applyExternalXRange(minX, maxX, true);
			}
		}

		exportWidget.render(&painter, QPoint(0, 0));
	}

	if (legendHeight > 0)
	{
		QRect legendRect(0,
			plotOnlySize.height(),
			finalExportSize.width(),
			finalExportSize.height() - plotOnlySize.height());

		painter.fillRect(legendRect, Qt::white);

		painter.setPen(QPen(QColor(210, 210, 210), 1));
		painter.drawLine(legendRect.left() + 12, legendRect.top(),
			legendRect.right() - 12, legendRect.top());

		drawExportLegend(painter,
			legendRect,
			legendSeriesList,
			PlotType::Line,          // multi-axis series are always line-drawn
			exportOptions.mFontHeight);
	}

	return pixmap;
}

QPixmap PlotBrowserWidget::exportPlotPixmap(
    const QSize& exportSize) const
{
    PlotExportOptions defaultOptions;

    return exportPlotPixmap(exportSize,
                            defaultOptions);
}

QString PlotBrowserWidget::exportPlotTitle() const
{
    if (!mExportTitle.isEmpty())
    {
        return mExportTitle;
    }

    return "Plot";
}
 
void PlotBrowserWidget::setAdvancedMultiAxisPlottingEnabled(bool enabled)
{
	if (mAdvancedMultiAxisPlottingEnabled == enabled)
	{
		return;
	}

	mAdvancedMultiAxisPlottingEnabled =
		enabled;

	updatePlot();
}

void PlotBrowserWidget::dumpLayoutDebug(const QString& label,
                                        int plotIndex) const
{
    qDebug().noquote()
        << "\n----- PlotBrowserWidget Debug:"
        << label
        << "Plot"
        << plotIndex
        << "-----";

    qDebug().noquote()
        << "PlotBrowserWidget"
        << "geometry:" << geometry()
        << "size:" << size()
        << "minimumSize:" << minimumSize()
        << "maximumSize:" << maximumSize()
        << "sizeHint:" << sizeHint()
        << "minimumSizeHint:" << minimumSizeHint();

    if (layout())
    {
        qDebug().noquote()
            << "Main layout"
            << "sizeConstraint:" << layout()->sizeConstraint()
            << "contentsMargins:" << layout()->contentsMargins()
            << "spacing:" << layout()->spacing();
    }

    if (mControlPanel)
    {
        qDebug().noquote()
            << "mControlPanel"
            << "visible:" << mControlPanel->isVisible()
            << "geometry:" << mControlPanel->geometry()
            << "size:" << mControlPanel->size()
            << "minimumSize:" << mControlPanel->minimumSize()
            << "maximumSize:" << mControlPanel->maximumSize()
            << "sizeHint:" << mControlPanel->sizeHint()
            << "minimumSizeHint:" << mControlPanel->minimumSizeHint();

        if (mControlPanel->layout())
        {
            qDebug().noquote()
                << "mControlPanel layout"
                << "sizeConstraint:" << mControlPanel->layout()->sizeConstraint()
                << "contentsMargins:" << mControlPanel->layout()->contentsMargins()
                << "spacing:" << mControlPanel->layout()->spacing();
        }
    }

    if (mPlotWidget)
    {
        qDebug().noquote()
            << "mPlotWidget"
            << "geometry:" << mPlotWidget->geometry()
            << "size:" << mPlotWidget->size()
            << "minimumSize:" << mPlotWidget->minimumSize()
            << "maximumSize:" << mPlotWidget->maximumSize()
            << "sizeHint:" << mPlotWidget->sizeHint()
            << "minimumSizeHint:" << mPlotWidget->minimumSizeHint();

        QWidget* rightPanel =
            mPlotWidget->parentWidget();

        if (rightPanel)
        {
            qDebug().noquote()
                << "rightPanel"
                << "geometry:" << rightPanel->geometry()
                << "size:" << rightPanel->size()
                << "minimumSize:" << rightPanel->minimumSize()
                << "maximumSize:" << rightPanel->maximumSize()
                << "sizeHint:" << rightPanel->sizeHint()
                << "minimumSizeHint:" << rightPanel->minimumSizeHint();

            if (rightPanel->layout())
            {
                qDebug().noquote()
                    << "rightPanel layout"
                    << "sizeConstraint:" << rightPanel->layout()->sizeConstraint()
                    << "contentsMargins:" << rightPanel->layout()->contentsMargins()
                    << "spacing:" << rightPanel->layout()->spacing();
            }
        }
    }

    if (mStudyTypeCombo)
    {
        qDebug().noquote()
            << "mStudyTypeCombo"
            << "text:" << mStudyTypeCombo->currentText()
            << "geometry:" << mStudyTypeCombo->geometry()
            << "minimumSize:" << mStudyTypeCombo->minimumSize()
            << "sizeHint:" << mStudyTypeCombo->sizeHint()
            << "minimumSizeHint:" << mStudyTypeCombo->minimumSizeHint();
    }

    if (mComponentCombo)
    {
        qDebug().noquote()
            << "mComponentCombo"
            << "text:" << mComponentCombo->currentText()
            << "geometry:" << mComponentCombo->geometry()
            << "minimumSize:" << mComponentCombo->minimumSize()
            << "sizeHint:" << mComponentCombo->sizeHint()
            << "minimumSizeHint:" << mComponentCombo->minimumSizeHint();
    }

    if (mSignalCombo)
    {
        qDebug().noquote()
            << "mSignalCombo"
            << "text:" << mSignalCombo->currentText()
            << "geometry:" << mSignalCombo->geometry()
            << "minimumSize:" << mSignalCombo->minimumSize()
            << "sizeHint:" << mSignalCombo->sizeHint()
            << "minimumSizeHint:" << mSignalCombo->minimumSizeHint();
    }

    if (mSummaryLabel)
    {
        qDebug().noquote()
            << "mSummaryLabel"
            << "text:" << mSummaryLabel->text().replace("\n", " | ")
            << "geometry:" << mSummaryLabel->geometry()
            << "minimumSize:" << mSummaryLabel->minimumSize()
            << "maximumSize:" << mSummaryLabel->maximumSize()
            << "sizeHint:" << mSummaryLabel->sizeHint()
            << "minimumSizeHint:" << mSummaryLabel->minimumSizeHint();
    }

    qDebug().noquote()
        << "----- End PlotBrowserWidget Debug -----\n";
}


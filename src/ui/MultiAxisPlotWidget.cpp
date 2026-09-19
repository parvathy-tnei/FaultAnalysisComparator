#include "MultiAxisPlotWidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QFont>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPolygonF>
#include <QSizePolicy>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    QString formatAxisValue(double value)
    {
        return QString::number(value, 'g', 6);
    }

    struct AxisScaleInfo
    {
        bool mUseScale = false;
        double mScaleFactor = 1.0;
        int mExponent = 0;
    };

    struct AxisRuntime
    {
        MultiAxisPlotAxis mAxis;
        double mMinY = 0.0;
        double mMaxY = 0.0;
        QVector<double> mMajorTicks;
        QVector<double> mMinorTicks;
        AxisScaleInfo mScaleInfo;

        /*
         * Axis slot on its side.
         *
         * left slot 0  -> main left axis
         * left slot 1  -> outer left axis
         * left slot 2  -> further outer left axis
         *
         * right slot 0 -> main right axis
         * right slot 1 -> outer right axis
         * right slot 2 -> further outer right axis
         */
        int mSideSlot = 0;
    };

    QColor seriesColor(int index)
    {
        static const QVector<QColor> colors =
        {
            QColor(0, 90, 180),
            QColor(190, 60, 50),
            QColor(40, 140, 80),
            QColor(120, 80, 170),
            QColor(200, 130, 30),
            QColor(0, 150, 170),
            QColor(180, 80, 140),
            QColor(90, 90, 90)
        };

        return colors[index % colors.size()];
    }

    AxisScaleInfo calculateYAxisScale(double minY,
        double maxY)
    {
        AxisScaleInfo scaleInfo;

        const double maxAbsY =
            std::max(std::abs(minY),
                std::abs(maxY));

        if (maxAbsY <= 0.0 ||
            !std::isfinite(maxAbsY))
        {
            return scaleInfo;
        }

        const int exponent =
            static_cast<int>(std::floor(std::log10(maxAbsY)));

        if (exponent <= -2 ||
            exponent >= 2)
        {
            scaleInfo.mUseScale = true;
            scaleInfo.mExponent = exponent;
            scaleInfo.mScaleFactor = std::pow(10.0,
                exponent);
        }

        return scaleInfo;
    }

    QString formatScaledAxisValue(double value,
        const AxisScaleInfo& scaleInfo)
    {
        double displayValue =
            scaleInfo.mUseScale
            ? value / scaleInfo.mScaleFactor
            : value;

        if (std::abs(displayValue) < 1e-12)
        {
            displayValue = 0.0;
        }

        return formatAxisValue(displayValue);
    }

    QString toSuperscriptText(int value)
    {
        QString result;

        if (value < 0)
        {
            result.append(QChar(0x207B));
        }

        const QString digits =
            QString::number(std::abs(value));

        for (const QChar& digit : digits)
        {
            switch (digit.unicode())
            {
            case '0': result.append(QChar(0x2070)); break;
            case '1': result.append(QChar(0x00B9)); break;
            case '2': result.append(QChar(0x00B2)); break;
            case '3': result.append(QChar(0x00B3)); break;
            case '4': result.append(QChar(0x2074)); break;
            case '5': result.append(QChar(0x2075)); break;
            case '6': result.append(QChar(0x2076)); break;
            case '7': result.append(QChar(0x2077)); break;
            case '8': result.append(QChar(0x2078)); break;
            case '9': result.append(QChar(0x2079)); break;
            default: break;
            }
        }

        return result;
    }

    QString scaleTextForAxis(const AxisScaleInfo& scaleInfo)
    {
        return QString("10%1")
            .arg(toSuperscriptText(scaleInfo.mExponent));
    }

    QString scaledAxisLabel(const QString& originalLabel,
        const AxisScaleInfo& scaleInfo)
    {
        if (!scaleInfo.mUseScale)
        {
            return originalLabel;
        }

        const QString scaleText =
            scaleTextForAxis(scaleInfo);

        const QString trimmedLabel =
            originalLabel.trimmed();

        if (trimmedLabel.isEmpty())
        {
            return QString("Y (%1)")
                .arg(scaleText);
        }

        const int openParenIndex =
            trimmedLabel.lastIndexOf('(');

        const bool hasTrailingUnit =
            openParenIndex >= 0 &&
            trimmedLabel.endsWith(')') &&
            openParenIndex < trimmedLabel.size() - 1;

        if (hasTrailingUnit)
        {
            const QString labelPart =
                trimmedLabel.left(openParenIndex).trimmed();

            const QString unitPart =
                trimmedLabel.mid(openParenIndex + 1,
                    trimmedLabel.size() - openParenIndex - 2)
                .trimmed();

            if (!unitPart.isEmpty())
            {
                if (!labelPart.isEmpty())
                {
                    return QString("%1 (%2 %3)")
                        .arg(labelPart,
                            scaleText,
                            unitPart);
                }

                return QString("(%1 %2)")
                    .arg(scaleText,
                        unitPart);
            }
        }

        return QString("%1 (x %2)")
            .arg(trimmedLabel,
                scaleText);
    }

    double niceNumber(double value,
        bool round)
    {
        if (value <= 0.0)
        {
            return 1.0;
        }

        const double exponent =
            std::floor(std::log10(value));

        const double fraction =
            value / std::pow(10.0, exponent);

        double niceFraction = 1.0;

        if (round)
        {
            if (fraction < 1.5)
            {
                niceFraction = 1.0;
            }
            else if (fraction < 3.0)
            {
                niceFraction = 2.0;
            }
            else if (fraction < 7.0)
            {
                niceFraction = 5.0;
            }
            else
            {
                niceFraction = 10.0;
            }
        }
        else
        {
            if (fraction <= 1.0)
            {
                niceFraction = 1.0;
            }
            else if (fraction <= 2.0)
            {
                niceFraction = 2.0;
            }
            else if (fraction <= 5.0)
            {
                niceFraction = 5.0;
            }
            else
            {
                niceFraction = 10.0;
            }
        }

        return niceFraction * std::pow(10.0, exponent);
    }

    void applyAutomaticYAxisRange(double& minY,
        double& maxY,
        int tickCount)
    {
        if (minY == maxY)
        {
            const double magnitude =
                std::abs(minY);

            const double padding =
                magnitude > 0.0
                ? magnitude * 0.10
                : 1.0;

            minY -= padding;
            maxY += padding;
        }
        else
        {
            const double range =
                maxY - minY;

            const double padding =
                range * 0.08;

            minY -= padding;
            maxY += padding;
        }

        const double range =
            maxY - minY;

        const double tickStep =
            niceNumber(range / tickCount,
                true);

        if (tickStep > 0.0)
        {
            minY =
                std::floor(minY / tickStep) * tickStep;

            maxY =
                std::ceil(maxY / tickStep) * tickStep;
        }

        if (minY == maxY)
        {
            minY -= 1.0;
            maxY += 1.0;
        }
    }

    QVector<double> generateNiceTicks(double minValue,
        double maxValue,
        int targetTickCount)
    {
        QVector<double> ticks;

        if (targetTickCount <= 0)
        {
            return ticks;
        }

        if (maxValue <= minValue)
        {
            ticks.append(minValue);
            return ticks;
        }

        const double range =
            maxValue - minValue;

        const double rawStep =
            range / static_cast<double>(targetTickCount);

        const double step =
            niceNumber(rawStep,
                true);

        if (step <= 0.0)
        {
            return ticks;
        }

        const double firstTick =
            std::ceil(minValue / step) * step;

        for (double value = firstTick;
            value <= maxValue + step * 0.5;
            value += step)
        {
            if (value >= minValue - step * 0.5 &&
                value <= maxValue + step * 0.5)
            {
                ticks.append(value);
            }
        }

        return ticks;
    }

    QVector<double> generateMinorTicks(double minValue,
        double maxValue,
        const QVector<double>& majorTicks,
        int subdivisions)
    {
        QVector<double> minorTicks;

        if (subdivisions <= 1 ||
            majorTicks.size() < 2 ||
            maxValue <= minValue)
        {
            return minorTicks;
        }

        const double majorStep =
            majorTicks[1] - majorTicks[0];

        if (majorStep <= 0.0)
        {
            return minorTicks;
        }

        const double minorStep =
            majorStep / static_cast<double>(subdivisions);

        if (minorStep <= 0.0)
        {
            return minorTicks;
        }

        const double firstTick =
            std::ceil(minValue / minorStep) * minorStep;

        for (double value = firstTick;
            value <= maxValue + minorStep * 0.5;
            value += minorStep)
        {
            bool isMajorTick = false;

            for (double majorTick : majorTicks)
            {
                if (std::abs(value - majorTick) < minorStep * 0.25)
                {
                    isMajorTick = true;
                    break;
                }
            }

            if (!isMajorTick &&
                value >= minValue - minorStep * 0.5 &&
                value <= maxValue + minorStep * 0.5)
            {
                minorTicks.append(value);
            }
        }

        return minorTicks;
    }

    void drawZoomHintBadge(QPainter& painter,
        const QRect& plotRect,
        bool isZoomed)
    {
        if (!isZoomed)
        {
            return;
        }

        if (plotRect.width() < 140 ||
            plotRect.height() < 45)
        {
            return;
        }

        QString hintText =
            QString("Zoomed %1 drag to pan %1 double-click to reset")
            .arg(QChar(0x2022));

        painter.save();

        QFont badgeFont =
            painter.font();

        if (badgeFont.pointSizeF() > 0.0)
        {
            badgeFont.setPointSizeF(
                std::max(7.0,
                    badgeFont.pointSizeF() - 1.0));
        }
        else if (badgeFont.pixelSize() > 0)
        {
            badgeFont.setPixelSize(
                std::max(8,
                    badgeFont.pixelSize() - 1));
        }

        painter.setFont(badgeFont);

        QFontMetrics metrics(badgeFont);

        const int horizontalPadding = 8;
        const int verticalPadding = 4;
        const int outerMargin = 8;

        auto makeBadgeRect =
            [&](const QString& text) -> QRect
            {
                const int textWidth =
                    metrics.horizontalAdvance(text);

                const int badgeWidth =
                    textWidth + horizontalPadding * 2;

                const int badgeHeight =
                    metrics.height() + verticalPadding * 2;

                QRect rect(0,
                    0,
                    badgeWidth,
                    badgeHeight);

                rect.moveTopRight(
                    QPoint(plotRect.right() - outerMargin,
                        plotRect.top() + outerMargin));

                return rect;
            };

        QRect badgeRect =
            makeBadgeRect(hintText);

        if (badgeRect.width() > plotRect.width() - outerMargin * 2)
        {
            hintText =
                QString("Zoomed %1 drag to pan")
                .arg(QChar(0x2022));

            badgeRect =
                makeBadgeRect(hintText);
        }

        if (badgeRect.width() > plotRect.width() - outerMargin * 2)
        {
            painter.restore();
            return;
        }

        painter.setRenderHint(QPainter::Antialiasing,
            true);

        painter.setPen(QPen(QColor(160, 160, 160, 180),
            1));
        painter.setBrush(QColor(255, 255, 255, 225));

        painter.drawRoundedRect(badgeRect,
            6,
            6);

        painter.setPen(QColor(60, 60, 60));

        painter.drawText(badgeRect.adjusted(horizontalPadding,
            0,
            -horizontalPadding,
            0),
            Qt::AlignCenter,
            hintText);

        painter.restore();
    }

    double distanceToLineSegment(const QPointF& point,
        const QPointF& start,
        const QPointF& end)
    {
        const double dx =
            end.x() - start.x();

        const double dy =
            end.y() - start.y();

        if (dx == 0.0 &&
            dy == 0.0)
        {
            const double pointDx =
                point.x() - start.x();

            const double pointDy =
                point.y() - start.y();

            return std::sqrt(pointDx * pointDx +
                pointDy * pointDy);
        }

        const double t =
            qBound(0.0,
                ((point.x() - start.x()) * dx +
                    (point.y() - start.y()) * dy) /
                (dx * dx + dy * dy),
                1.0);

        const QPointF projection(start.x() + t * dx,
            start.y() + t * dy);

        const double pointDx =
            point.x() - projection.x();

        const double pointDy =
            point.y() - projection.y();

        return std::sqrt(pointDx * pointDx +
            pointDy * pointDy);
    }

    bool isValidSeries(const MultiAxisPlotSeries& series)
    {
        return !series.mXValues.isEmpty() &&
            !series.mYValues.isEmpty() &&
            series.mXValues.size() == series.mYValues.size();
    }

    QVector<MultiAxisPlotSeries> visibleValidSeries(
        const QVector<MultiAxisPlotSeries>& seriesList,
        const QSet<QString>& hiddenSeriesNames)
    {
        QVector<MultiAxisPlotSeries> result;

        for (const MultiAxisPlotSeries& series : seriesList)
        {
            if (!isValidSeries(series))
            {
                continue;
            }

            if (hiddenSeriesNames.contains(series.mName))
            {
                continue;
            }

            result.append(series);
        }

        return result;
    }

    const AxisRuntime* axisForKey(const QVector<AxisRuntime>& axes,
        const QString& axisKey)
    {
        for (const AxisRuntime& axis : axes)
        {
            if (axis.mAxis.mKey == axisKey)
            {
                return &axis;
            }
        }

        return nullptr;
    }

    QPointF mapPoint(double xValue,
        double yValue,
        const QRect& plotRect,
        double minX,
        double maxX,
        const AxisRuntime& axis)
    {
        const double xRatio =
            (xValue - minX) / (maxX - minX);

        const double yRatio =
            (yValue - axis.mMinY) / (axis.mMaxY - axis.mMinY);

        const double px =
            plotRect.left() +
            xRatio * plotRect.width();

        const double py =
            plotRect.bottom() -
            yRatio * plotRect.height();

        return QPointF(px,
            py);
    }

    QVector<AxisRuntime> buildAxisRuntimes(
        const QVector<MultiAxisPlotSeries>& seriesList,
        const QVector<MultiAxisPlotAxis>& axes,
        int tickCount)
    {
        QVector<AxisRuntime> runtimeAxes;

        int leftSideAxisCount = 0;
        int rightSideAxisCount = 0;

        for (const MultiAxisPlotAxis& axis : axes)
        {
            bool foundValue = false;
            double minY = std::numeric_limits<double>::max();
            double maxY = std::numeric_limits<double>::lowest();

            for (const MultiAxisPlotSeries& series : seriesList)
            {
                if (series.mAxisKey != axis.mKey)
                {
                    continue;
                }

                for (double value : series.mYValues)
                {
                    if (!std::isfinite(value))
                    {
                        continue;
                    }

                    minY = std::min(minY,
                        value);

                    maxY = std::max(maxY,
                        value);

                    foundValue = true;
                }
            }

            if (!foundValue)
            {
                continue;
            }

            applyAutomaticYAxisRange(minY,
                maxY,
                tickCount);

            AxisRuntime runtime;
            runtime.mAxis = axis;
            runtime.mMinY = minY;
            runtime.mMaxY = maxY;
            runtime.mMajorTicks =
                generateNiceTicks(minY,
                    maxY,
                    6);
            runtime.mMinorTicks =
                generateMinorTicks(minY,
                    maxY,
                    runtime.mMajorTicks,
                    5);
            runtime.mScaleInfo =
                calculateYAxisScale(minY,
                    maxY);

            if (runtime.mAxis.mUseRightSide)
            {
                runtime.mSideSlot =
                    rightSideAxisCount;

                ++rightSideAxisCount;
            }
            else
            {
                runtime.mSideSlot =
                    leftSideAxisCount;

                ++leftSideAxisCount;
            }

            runtimeAxes.append(runtime);
        }

        return runtimeAxes;
    }
}

MultiAxisPlotWidget::MultiAxisPlotWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(0, 0);

    setSizePolicy(QSizePolicy::Ignored,
        QSizePolicy::Ignored);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void MultiAxisPlotWidget::setSeries(
    const QVector<MultiAxisPlotSeries>& series,
    const QVector<MultiAxisPlotAxis>& axes,
    const QString& title,
    const QString& xAxisLabel)
{
    mSeries = series;
    mAxes = axes;
    mTitle = title;
    mXAxisLabel = xAxisLabel;

    mHasCustomXRange = false;
    mCustomMinX = 0.0;
    mCustomMaxX = 0.0;

    mIsPanning = false;
    mIsSelectingZoomArea = false;

    unsetCursor();

    update();
}

void MultiAxisPlotWidget::clear()
{
    mSeries.clear();
    mAxes.clear();
    mTitle.clear();
    mXAxisLabel.clear();
    mHiddenSeriesNames.clear();

    mHasCustomXRange = false;
    mCustomMinX = 0.0;
    mCustomMaxX = 0.0;

    mIsPanning = false;
    mIsSelectingZoomArea = false;

    mLegendOffset = QPointF(0, 0);   // new
    mIsDraggingLegend = false;        // new
    mLegendRect = QRect();            // new

    unsetCursor();

    update();
}

void MultiAxisPlotWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,
        true);

    painter.fillRect(rect(),
        Qt::white);

    if (mTextScale != 1.0)
    {
        QFont exportFont =
            painter.font();

        if (exportFont.pointSizeF() > 0.0)
        {
            exportFont.setPointSizeF(
                exportFont.pointSizeF() * mTextScale);
        }
        else if (exportFont.pixelSize() > 0)
        {
            const int scaledPixelSize =
                std::max(1,
                    static_cast<int>(
                        std::round(exportFont.pixelSize() * mTextScale)));

            exportFont.setPixelSize(scaledPixelSize);
        }

        painter.setFont(exportFont);
    }

    const QRect plotRect =
        plotAreaRect();

    if (plotRect.width() <= 0 ||
        plotRect.height() <= 0)
    {
        return;
    }

    painter.setPen(QPen(palette().text().color(),
        1));

    if (!mTitle.isEmpty())
    {
        painter.drawText(QRect(0,
            5,
            width(),
            25),
            Qt::AlignCenter,
            mTitle);
    }

    painter.drawRect(plotRect);

    const QVector<MultiAxisPlotSeries> validSeries =
        visibleValidSeries(mSeries,
            mHiddenSeriesNames);

    if (validSeries.isEmpty() ||
        mAxes.isEmpty())
    {
        painter.drawText(plotRect,
            Qt::AlignCenter,
            "No data to plot");
        return;
    }

    double minX = 0.0;
    double maxX = 0.0;

    if (!dataXRange(minX,
        maxX))
    {
        painter.drawText(plotRect,
            Qt::AlignCenter,
            "No data to plot");
        return;
    }

    if (minX == maxX)
    {
        minX -= 1.0;
        maxX += 1.0;
    }

    if (mHasCustomXRange)
    {
        minX = mCustomMinX;
        maxX = mCustomMaxX;
    }

    const double axisMinX = minX;
    const double axisMaxX = maxX;

    const int tickSize = 5;
    const int tickCount = 5;

    const QVector<AxisRuntime> runtimeAxes =
        buildAxisRuntimes(validSeries,
            mAxes,
            tickCount);

    if (runtimeAxes.isEmpty())
    {
        painter.drawText(plotRect,
            Qt::AlignCenter,
            "No data to plot");
        return;
    }

    QFontMetrics metrics(painter.font());

    QPen axisPen(palette().text().color(),
        1);

    axisPen.setCosmetic(true);

    QPen gridPen(palette().mid().color(),
        1,
        Qt::DotLine);
    gridPen.setCosmetic(true);

    QColor minorGridColor =
        palette().mid().color();

    minorGridColor.setAlpha(80);

    QPen minorGridPen(minorGridColor,
        1,
        Qt::DotLine);
    minorGridPen.setCosmetic(true);

    const QVector<double> xTicks =
        generateNiceTicks(axisMinX,
            axisMaxX,
            8);

    const QVector<double> minorXTicks =
        generateMinorTicks(axisMinX,
            axisMaxX,
            xTicks,
            5);

    const AxisRuntime& gridAxis =
        runtimeAxes.first();

    if (mShowMinorGrid)
    {
        painter.setPen(minorGridPen);

        for (double value : minorXTicks)
        {
            const double ratio =
                (value - minX) / (maxX - minX);

            const int x =
                plotRect.left() +
                static_cast<int>(ratio * plotRect.width());

            if (x <= plotRect.left() ||
                x >= plotRect.right())
            {
                continue;
            }

            painter.drawLine(x,
                plotRect.top(),
                x,
                plotRect.bottom());
        }

        for (double value : gridAxis.mMinorTicks)
        {
            const double ratio =
                (value - gridAxis.mMinY) /
                (gridAxis.mMaxY - gridAxis.mMinY);

            const int y =
                plotRect.bottom() -
                static_cast<int>(ratio * plotRect.height());

            if (y <= plotRect.top() ||
                y >= plotRect.bottom())
            {
                continue;
            }

            painter.drawLine(plotRect.left(),
                y,
                plotRect.right(),
                y);
        }
    }

    for (double value : xTicks)
    {
        const double ratio =
            (value - minX) / (maxX - minX);

        const int x =
            plotRect.left() +
            static_cast<int>(ratio * plotRect.width());

        if (x < plotRect.left() ||
            x > plotRect.right())
        {
            continue;
        }

        if (mShowMajorGrid &&
            x > plotRect.left() &&
            x < plotRect.right())
        {
            painter.setPen(gridPen);

            painter.drawLine(x,
                plotRect.top(),
                x,
                plotRect.bottom());
        }

        painter.setPen(axisPen);

        painter.drawLine(x,
            plotRect.bottom(),
            x,
            plotRect.bottom() + tickSize);

        const QString text =
            formatAxisValue(value);

        const int textWidth =
            metrics.horizontalAdvance(text);

        painter.drawText(x - textWidth / 2,
            plotRect.bottom() + tickSize + metrics.height(),
            text);
    }

    for (double value : gridAxis.mMajorTicks)
    {
        const double ratio =
            (value - gridAxis.mMinY) /
            (gridAxis.mMaxY - gridAxis.mMinY);

        const int y =
            plotRect.bottom() -
            static_cast<int>(ratio * plotRect.height());

        if (y < plotRect.top() ||
            y > plotRect.bottom())
        {
            continue;
        }

        if (mShowMajorGrid &&
            y > plotRect.top() &&
            y < plotRect.bottom())
        {
            painter.setPen(gridPen);

            painter.drawLine(plotRect.left(),
                y,
                plotRect.right(),
                y);
        }
    }

    painter.setPen(axisPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(plotRect);

    const int axisSpacing = 72;
    const int axisInsetFromFrame = 16;

    for (const AxisRuntime& axis : runtimeAxes)
    {
        const bool rightSide =
            axis.mAxis.mUseRightSide;

        const int axisX =
            rightSide
            ? plotRect.right() + axisInsetFromFrame + axis.mSideSlot * axisSpacing
            : plotRect.left() - axisInsetFromFrame - axis.mSideSlot * axisSpacing;

        const QColor axisColor =
            axis.mAxis.mColor.isValid()
            ? axis.mAxis.mColor
            : palette().text().color();

        QPen coloredAxisPen(axisColor, 1);
        coloredAxisPen.setCosmetic(true);

        painter.setPen(coloredAxisPen);
        painter.drawLine(axisX,
            plotRect.top(),
            axisX,
            plotRect.bottom());

        int maxTickLabelWidth = 0;

        for (double value : axis.mMajorTicks)
        {
            const QString text =
                formatScaledAxisValue(value,
                    axis.mScaleInfo);

            maxTickLabelWidth =
                std::max(maxTickLabelWidth,
                    metrics.horizontalAdvance(text));

            const double ratio =
                (value - axis.mMinY) /
                (axis.mMaxY - axis.mMinY);

            const int y =
                plotRect.bottom() -
                static_cast<int>(ratio * plotRect.height());

            if (y < plotRect.top() ||
                y > plotRect.bottom())
            {
                continue;
            }

            painter.setPen(coloredAxisPen);

            if (rightSide)
            {
                painter.drawLine(axisX - tickSize,
                    y,
                    axisX,
                    y);
            }
            else
            {
                painter.drawLine(axisX,
                    y,
                    axisX + tickSize,
                    y);
            }

            if (rightSide)
            {
                painter.drawText(axisX + tickSize + 6,
                    y + metrics.height() / 3,
                    text);
            }
            else
            {
                painter.drawText(axisX - tickSize - metrics.horizontalAdvance(text) - 6,
                    y + metrics.height() / 3,
                    text);
            }
        }

        const QString displayAxisLabel =
            scaledAxisLabel(axis.mAxis.mLabel,
                axis.mScaleInfo);

        if (!displayAxisLabel.isEmpty())
        {
            painter.save();
            painter.setPen(coloredAxisPen);

            const int labelGapFromAxis =
                maxTickLabelWidth + 26;

            if (rightSide)
            {
                painter.translate(axisX + labelGapFromAxis,
                    plotRect.top() + plotRect.height() / 2);

                painter.rotate(90);
            }
            else
            {
                painter.translate(axisX - labelGapFromAxis,
                    plotRect.top() + plotRect.height() / 2);

                painter.rotate(-90);
            }

            painter.drawText(QRect(-plotRect.height() / 2,
                -10,
                plotRect.height(),
                20),
                Qt::AlignCenter,
                displayAxisLabel);

            painter.restore();
        }
    }

    if (!mXAxisLabel.isEmpty())
    {
        painter.setPen(QPen(palette().text().color(), 1));

        painter.drawText(QRect(plotRect.left(),
            height() - 45,
            plotRect.width(),
            25),
            Qt::AlignCenter,
            mXAxisLabel);
    }

    painter.save();
    painter.setClipRect(plotRect.adjusted(1,
        1,
        -1,
        -1));

    for (int seriesIndex = 0;
        seriesIndex < validSeries.size();
        ++seriesIndex)
    {
        const MultiAxisPlotSeries& series =
            validSeries[seriesIndex];

        const AxisRuntime* axis =
            axisForKey(runtimeAxes,
                series.mAxisKey);

        if (!axis)
        {
            continue;
        }

        QPolygonF polyline;

        for (int i = 0;
            i < series.mXValues.size();
            ++i)
        {
            const double xValue =
                series.mXValues[i];

            const double yValue =
                series.mYValues[i];

            if (!std::isfinite(xValue) ||
                !std::isfinite(yValue))
            {
                continue;
            }

            polyline.append(mapPoint(xValue,
                yValue,
                plotRect,
                minX,
                maxX,
                *axis));
        }

        const QColor color =
            series.mColor.isValid()
            ? series.mColor
            : seriesColor(seriesIndex);

        const int thickness =
            std::max(1,
                series.mLineThickness);

        QPen linePen(color,
            thickness);
        linePen.setStyle(series.mLineStyle);
        linePen.setCapStyle(Qt::RoundCap);
        linePen.setJoinStyle(Qt::RoundJoin);

        painter.setPen(linePen);
        painter.drawPolyline(polyline);
    }

    painter.restore();

    //
    // Legend drawn inside plot area.
    // Translucent, and draggable, matching PlotWidget's behaviour.
    //
    mLegendRect = QRect();

    if (mShowLegend && !validSeries.isEmpty())
    {
        const int legendPadding = 6;
        const int legendRowHeight = 18;
        const int legendLineWidth = 22;
        const int legendTextGap = 6;
        const int legendOuterMargin = 8;

        if (plotRect.width() >= 90 &&
            plotRect.height() >= 45)
        {
            const int maxLegendBoxWidth =
                qMax(70, qMin(240, plotRect.width() / 2));

            const int maxLegendBoxHeight =
                qMax(32, qMin(120, plotRect.height() / 2));

            const int maxLegendTextWidth =
                qMax(20,
                    maxLegendBoxWidth
                    - legendPadding * 2
                    - legendLineWidth
                    - legendTextGap);

            int maxRows =
                (maxLegendBoxHeight - legendPadding * 2) / legendRowHeight;

            maxRows = qMax(1, maxRows);

            int visibleSeriesCount =
                qMin(validSeries.size(), maxRows);

            const bool needsMoreRow =
                visibleSeriesCount < validSeries.size();

            if (needsMoreRow && visibleSeriesCount > 1)
            {
                visibleSeriesCount -= 1;
            }

            QVector<QString> legendTexts;
            int actualTextWidth = 0;

            for (int seriesIndex = 0;
                seriesIndex < visibleSeriesCount;
                ++seriesIndex)
            {
                const MultiAxisPlotSeries& series = validSeries[seriesIndex];

                const QString legendText =
                    metrics.elidedText(series.mName,
                        Qt::ElideRight,
                        maxLegendTextWidth);

                legendTexts.append(legendText);

                actualTextWidth =
                    qMax(actualTextWidth,
                        metrics.horizontalAdvance(legendText));
            }

            if (needsMoreRow)
            {
                const QString moreText =
                    QString("+ %1 more")
                    .arg(validSeries.size() - visibleSeriesCount);

                const QString elidedMoreText =
                    metrics.elidedText(moreText,
                        Qt::ElideRight,
                        maxLegendTextWidth);

                legendTexts.append(elidedMoreText);

                actualTextWidth =
                    qMax(actualTextWidth,
                        metrics.horizontalAdvance(elidedMoreText));
            }

            const int legendBoxWidth =
                qMin(maxLegendBoxWidth,
                    legendPadding * 2
                    + legendLineWidth
                    + legendTextGap
                    + actualTextWidth);

            const int legendBoxHeight =
                qMin(maxLegendBoxHeight,
                    legendPadding * 2
                    + legendRowHeight * legendTexts.size());

            QRect legendRect(plotRect.right()
                - legendBoxWidth
                - legendOuterMargin
                + static_cast<int>(mLegendOffset.x()),
                plotRect.top()
                + legendOuterMargin
                + static_cast<int>(mLegendOffset.y()),
                legendBoxWidth,
                legendBoxHeight);

            if (legendRect.left() < plotRect.left() + legendOuterMargin)
            {
                legendRect.moveLeft(plotRect.left() + legendOuterMargin);
            }

            if (legendRect.top() < plotRect.top() + legendOuterMargin)
            {
                legendRect.moveTop(plotRect.top() + legendOuterMargin);
            }

            if (legendRect.right() > plotRect.right() - legendOuterMargin)
            {
                legendRect.moveRight(plotRect.right() - legendOuterMargin);
            }

            if (legendRect.bottom() > plotRect.bottom() - legendOuterMargin)
            {
                legendRect.moveBottom(plotRect.bottom() - legendOuterMargin);
            }

            painter.save();

            painter.setClipRect(legendRect);

            painter.setPen(QPen(QColor(120, 120, 120, 150), 1));
            painter.setBrush(QColor(255, 255, 255, 90));
            painter.drawRoundedRect(legendRect, 5, 5);

            int rowY = legendRect.top() + legendPadding;

            for (int rowIndex = 0;
                rowIndex < legendTexts.size();
                ++rowIndex)
            {
                const bool isMoreRow =
                    needsMoreRow && rowIndex == legendTexts.size() - 1;

                const int lineY = rowY + legendRowHeight / 2;
                const int lineStartX = legendRect.left() + legendPadding;
                const int lineEndX = lineStartX + legendLineWidth;

                if (!isMoreRow)
                {
                    const MultiAxisPlotSeries& series = validSeries[rowIndex];

                    const QColor color =
                        series.mColor.isValid()
                        ? series.mColor
                        : seriesColor(rowIndex);

                    QPen legendPen(color, 2);
                    legendPen.setStyle(series.mLineStyle);
                    legendPen.setCapStyle(Qt::RoundCap);

                    painter.setPen(legendPen);

                    painter.drawLine(lineStartX, lineY, lineEndX, lineY);
                }

                const QRect textRect(lineEndX + legendTextGap,
                    rowY,
                    legendRect.right()
                    - lineEndX
                    - legendTextGap
                    - legendPadding,
                    legendRowHeight);

                painter.setPen(QPen(palette().text().color(), 1));

                painter.drawText(textRect,
                    Qt::AlignLeft | Qt::AlignVCenter,
                    legendTexts[rowIndex]);

                rowY += legendRowHeight;
            }

            painter.restore();

            mLegendRect = legendRect;
        }
    }

    if (mIsSelectingZoomArea)
    {
        const int leftX =
            std::max(plotRect.left(),
                std::min(mZoomSelectionStart.x(),
                    mZoomSelectionEnd.x()));

        const int rightX =
            std::min(plotRect.right(),
                std::max(mZoomSelectionStart.x(),
                    mZoomSelectionEnd.x()));

        QRect selectionRect(QPoint(leftX,
            plotRect.top()),
            QPoint(rightX,
                plotRect.bottom()));

        QColor highlightColor =
            palette().highlight().color();

        highlightColor.setAlpha(60);

        painter.setPen(QPen(palette().highlight().color(),
            1,
            Qt::DashLine));
        painter.setBrush(highlightColor);
        painter.drawRect(selectionRect.normalized());
    }

    drawZoomHintBadge(painter,
        plotRect,
        mHasCustomXRange);

    if (mHasMousePosition &&
        plotRect.contains(mMousePosition))
    {
        bool foundTime = false;
        double nearestTime = 0.0;
        double nearestXDistanceSquared = 0.0;
        double guideX = 0.0;

        for (const MultiAxisPlotSeries& series : validSeries)
        {
            const AxisRuntime* axis =
                axisForKey(runtimeAxes,
                    series.mAxisKey);

            if (!axis)
            {
                continue;
            }

            for (int pointIndex = 0;
                pointIndex < series.mXValues.size();
                ++pointIndex)
            {
                const QPointF point =
                    mapPoint(series.mXValues[pointIndex],
                        series.mYValues[pointIndex],
                        plotRect,
                        minX,
                        maxX,
                        *axis);

                const double dx =
                    point.x() - mMousePosition.x();

                const double distanceSquared =
                    dx * dx;

                if (!foundTime ||
                    distanceSquared < nearestXDistanceSquared)
                {
                    foundTime = true;
                    nearestXDistanceSquared = distanceSquared;
                    nearestTime = series.mXValues[pointIndex];
                    guideX = point.x();
                }
            }
        }

        if (foundTime)
        {
            struct HoverValue
            {
                QString mName;
                QString mAxisLabel;
                double mTime = 0.0;
                double mValue = 0.0;
                QPointF mPoint;
                QColor mColor;
                bool mValid = false;
            };

            QVector<HoverValue> hoverValues;

            for (int seriesIndex = 0;
                seriesIndex < validSeries.size();
                ++seriesIndex)
            {
                const MultiAxisPlotSeries& series =
                    validSeries[seriesIndex];

                const AxisRuntime* axis =
                    axisForKey(runtimeAxes,
                        series.mAxisKey);

                if (!axis)
                {
                    continue;
                }

                if (series.mXValues.isEmpty() ||
                    series.mYValues.isEmpty() ||
                    series.mXValues.size() != series.mYValues.size())
                {
                    continue;
                }

                const auto minMaxTime =
                    std::minmax_element(series.mXValues.begin(),
                        series.mXValues.end());

                const double seriesMinTime =
                    *minMaxTime.first;

                const double seriesMaxTime =
                    *minMaxTime.second;

                const double tolerance =
                    std::max(1e-9,
                        (seriesMaxTime - seriesMinTime) * 1e-9);

                if (nearestTime < seriesMinTime - tolerance ||
                    nearestTime > seriesMaxTime + tolerance)
                {
                    continue;
                }

                int nearestIndex = -1;
                double nearestTimeDelta = 0.0;
                bool foundPointForSeries = false;

                for (int pointIndex = 0;
                    pointIndex < series.mXValues.size();
                    ++pointIndex)
                {
                    const double timeDelta =
                        std::abs(series.mXValues[pointIndex] - nearestTime);

                    if (!foundPointForSeries ||
                        timeDelta < nearestTimeDelta)
                    {
                        foundPointForSeries = true;
                        nearestTimeDelta = timeDelta;
                        nearestIndex = pointIndex;
                    }
                }

                if (nearestIndex < 0)
                {
                    continue;
                }

                HoverValue hoverValue;
                hoverValue.mName = series.mName;
                hoverValue.mAxisLabel = axis->mAxis.mLabel;
                hoverValue.mTime = series.mXValues[nearestIndex];
                hoverValue.mValue = series.mYValues[nearestIndex];
                hoverValue.mPoint =
                    mapPoint(hoverValue.mTime,
                        hoverValue.mValue,
                        plotRect,
                        minX,
                        maxX,
                        *axis);
                hoverValue.mColor =
                    series.mColor.isValid()
                    ? series.mColor
                    : seriesColor(seriesIndex);
                hoverValue.mValid = true;

                hoverValues.append(hoverValue);
            }

            if (!hoverValues.isEmpty())
            {
                painter.setPen(QPen(palette().mid().color(),
                    1,
                    Qt::DashLine));

                painter.drawLine(QPointF(guideX,
                    plotRect.top()),
                    QPointF(guideX,
                        plotRect.bottom()));

                for (const HoverValue& hoverValue : hoverValues)
                {
                    if (!hoverValue.mValid)
                    {
                        continue;
                    }

                    painter.setPen(QPen(hoverValue.mColor,
                        2));
                    painter.setBrush(palette().window());

                    painter.drawEllipse(hoverValue.mPoint,
                        5,
                        5);
                }

                struct HoverDisplayRow
                {
                    QString mName;
                    QString mValue;
                    QColor mColor;
                };

                QVector<HoverDisplayRow> displayRows;

                for (const HoverValue& hoverValue : hoverValues)
                {
                    HoverDisplayRow row;
                    row.mName = hoverValue.mName;

                    const QString axisSuffix =
                        hoverValue.mAxisLabel.trimmed().isEmpty()
                        ? QString()
                        : QString(" [%1]").arg(hoverValue.mAxisLabel.trimmed());

                    row.mValue =
                        QString("%1%2")
                        .arg(QString::number(hoverValue.mValue, 'g', 10),
                            axisSuffix);
                    row.mColor = hoverValue.mColor;

                    displayRows.append(row);
                }

                const QString xHoverLabel =
                    mXAxisLabel.trimmed().isEmpty()
                    ? QString("X")
                    : mXAxisLabel.trimmed();

                const QString timeLine =
                    QString("%1: %2")
                    .arg(xHoverLabel,
                        QString::number(nearestTime, 'g', 10));

                const QFont originalFont = painter.font();

                QFont tooltipFont = painter.font();
                tooltipFont.setPointSize(
                    qMax(7,
                        tooltipFont.pointSize() - 1));
                painter.setFont(tooltipFont);

                QFontMetrics tooltipMetrics(painter.font());

                const int padding = 8;
                const int rowGap = 6;
                const int colorMarkerWidth = 12;
                const int markerToNameGap = 6;
                const int nameToValueGap = 12;

                int maxNameWidth = 0;
                int maxValueWidth = 0;

                for (const HoverDisplayRow& row : displayRows)
                {
                    maxNameWidth =
                        std::max(maxNameWidth,
                            tooltipMetrics.horizontalAdvance(row.mName));

                    maxValueWidth =
                        std::max(maxValueWidth,
                            tooltipMetrics.horizontalAdvance(row.mValue));
                }

                const int valueColumnWidth =
                    maxValueWidth + 6;

                const int timeWidth =
                    tooltipMetrics.horizontalAdvance(timeLine);

                int naturalContentWidth =
                    colorMarkerWidth +
                    markerToNameGap +
                    maxNameWidth +
                    nameToValueGap +
                    valueColumnWidth;

                naturalContentWidth =
                    std::max(naturalContentWidth,
                        timeWidth);

                int tooltipWidth =
                    naturalContentWidth + padding * 2;

                const int maxTooltipWidth =
                    qMax(120,
                        width() - 16);

                tooltipWidth =
                    std::min(tooltipWidth,
                        maxTooltipWidth);

                const int contentWidth =
                    tooltipWidth - padding * 2;

                const int rowHeight =
                    tooltipMetrics.height() + rowGap;

                const int tooltipHeight =
                    padding * 2 +
                    tooltipMetrics.height() +
                    4 +
                    displayRows.size() * rowHeight;

                QPoint tooltipTopLeft =
                    mMousePosition + QPoint(14,
                        14);

                if (tooltipTopLeft.x() + tooltipWidth > width())
                {
                    tooltipTopLeft.setX(
                        mMousePosition.x() - tooltipWidth - 14);
                }

                if (tooltipTopLeft.y() + tooltipHeight > height())
                {
                    tooltipTopLeft.setY(
                        mMousePosition.y() - tooltipHeight - 14);
                }

                if (tooltipTopLeft.x() < 4)
                {
                    tooltipTopLeft.setX(4);
                }

                if (tooltipTopLeft.y() < 4)
                {
                    tooltipTopLeft.setY(4);
                }

                QRect tooltipRect(tooltipTopLeft,
                    QSize(tooltipWidth,
                        tooltipHeight));

                painter.setPen(QPen(QColor(80, 80, 80),
                    1));
                painter.setBrush(QColor(255, 255, 255, 235));
                painter.drawRoundedRect(tooltipRect,
                    6,
                    6);

                painter.setPen(palette().text().color());
                painter.drawText(tooltipRect.adjusted(padding,
                    padding,
                    -padding,
                    -padding),
                    Qt::AlignTop | Qt::AlignLeft,
                    timeLine);

                int rowY =
                    tooltipRect.top() +
                    padding +
                    tooltipMetrics.height() +
                    8;

                painter.save();
                painter.setClipRect(tooltipRect.adjusted(1,
                    1,
                    -1,
                    -1));

                for (const HoverDisplayRow& row : displayRows)
                {
                    const int markerX =
                        tooltipRect.left() + padding;

                    const int markerY =
                        rowY + tooltipMetrics.ascent() / 2;

                    painter.setPen(QPen(row.mColor,
                        2));
                    painter.drawLine(markerX,
                        markerY,
                        markerX + colorMarkerWidth,
                        markerY);

                    const int nameX =
                        markerX + colorMarkerWidth + markerToNameGap;

                    const int valueX =
                        tooltipRect.left() +
                        padding +
                        contentWidth - valueColumnWidth;

                    const int availableNameWidth =
                        std::max(20,
                            valueX - nameToValueGap - nameX);

                    const QString elidedName =
                        tooltipMetrics.elidedText(row.mName,
                            Qt::ElideRight,
                            availableNameWidth);

                    painter.setPen(palette().text().color());
                    painter.drawText(QRect(nameX,
                        rowY,
                        availableNameWidth,
                        tooltipMetrics.height()),
                        Qt::AlignLeft | Qt::AlignVCenter,
                        elidedName);

                    painter.drawText(QRect(valueX,
                        rowY,
                        valueColumnWidth,
                        tooltipMetrics.height()),
                        Qt::AlignRight | Qt::AlignVCenter,
                        row.mValue);

                    rowY += rowHeight;
                }

                painter.restore();
                painter.setFont(originalFont);
            }
        }
    }
}

void MultiAxisPlotWidget::mouseMoveEvent(QMouseEvent* event)
{
    mMousePosition = event->position().toPoint();
    mHasMousePosition = true;

    if (mIsDraggingLegend)
    {
        const QPoint delta =
            mMousePosition - mLegendDragStartMousePos;

        mLegendOffset =
            mLegendDragStartOffset + QPointF(delta);

        update();

        event->accept();
        return;
    }

    if (mIsSelectingZoomArea)
    {
        const QRect plotRect =
            plotAreaRect();

        const QPoint mousePosition =
            event->position().toPoint();

        const int clampedX =
            std::max(plotRect.left(),
                std::min(mousePosition.x(),
                    plotRect.right()));

        const int clampedY =
            std::max(plotRect.top(),
                std::min(mousePosition.y(),
                    plotRect.bottom()));

        mZoomSelectionEnd =
            QPoint(clampedX,
                clampedY);

        update();

        event->accept();
        return;
    }

    if (mIsPanning)
    {
        const QRect plotRect =
            plotAreaRect();

        if (plotRect.width() <= 0)
        {
            event->ignore();
            return;
        }

        double dataMinX = 0.0;
        double dataMaxX = 0.0;

        if (!dataXRange(dataMinX,
            dataMaxX))
        {
            event->ignore();
            return;
        }

        const double visibleRange =
            mPanStartMaxX - mPanStartMinX;

        if (visibleRange <= 0.0)
        {
            event->ignore();
            return;
        }

        const int dxPixels =
            event->position().toPoint().x() -
            mPanStartMousePosition.x();

        const double dxData =
            static_cast<double>(dxPixels) /
            static_cast<double>(plotRect.width()) *
            visibleRange;

        double newMinX =
            mPanStartMinX - dxData;

        double newMaxX =
            mPanStartMaxX - dxData;

        if (newMinX < dataMinX)
        {
            newMinX = dataMinX;
            newMaxX = dataMinX + visibleRange;
        }

        if (newMaxX > dataMaxX)
        {
            newMaxX = dataMaxX;
            newMinX = dataMaxX - visibleRange;
        }

        mHasCustomXRange = true;
        mCustomMinX = newMinX;
        mCustomMaxX = newMaxX;

        update();

        emit xRangeChanged(mCustomMinX,
            mCustomMaxX,
            true);

        event->accept();
        return;
    }

    update();

    event->accept();
}

void MultiAxisPlotWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton &&
        mIsDraggingLegend)
    {
        mIsDraggingLegend = false;

        unsetCursor();

        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton &&
        mIsSelectingZoomArea)
    {
        const QRect plotRect =
            plotAreaRect();

        mIsSelectingZoomArea = false;

        const int selectionLeftX =
            std::max(plotRect.left(),
                std::min(mZoomSelectionStart.x(),
                    mZoomSelectionEnd.x()));

        const int selectionRightX =
            std::min(plotRect.right(),
                std::max(mZoomSelectionStart.x(),
                    mZoomSelectionEnd.x()));

        const int selectionWidth =
            selectionRightX - selectionLeftX;

        if (selectionWidth < 10 ||
            plotRect.width() <= 0)
        {
            unsetCursor();
            update();

            event->accept();
            return;
        }

        double dataMinX = 0.0;
        double dataMaxX = 0.0;

        if (!dataXRange(dataMinX,
            dataMaxX))
        {
            unsetCursor();
            update();

            event->accept();
            return;
        }

        const double currentMinX =
            mHasCustomXRange
            ? mCustomMinX
            : dataMinX;

        const double currentMaxX =
            mHasCustomXRange
            ? mCustomMaxX
            : dataMaxX;

        const double currentRange =
            currentMaxX - currentMinX;

        const double fullRange =
            dataMaxX - dataMinX;

        if (currentRange <= 0.0 ||
            fullRange <= 0.0)
        {
            unsetCursor();
            update();

            event->accept();
            return;
        }

        const double leftRatio =
            static_cast<double>(selectionLeftX - plotRect.left()) /
            static_cast<double>(plotRect.width());

        const double rightRatio =
            static_cast<double>(selectionRightX - plotRect.left()) /
            static_cast<double>(plotRect.width());

        double newMinX =
            currentMinX + leftRatio * currentRange;

        double newMaxX =
            currentMinX + rightRatio * currentRange;

        if (newMinX > newMaxX)
        {
            std::swap(newMinX,
                newMaxX);
        }

        const double selectedRange =
            newMaxX - newMinX;

        const double minimumAllowedRange =
            fullRange * 0.001;

        if (selectedRange < minimumAllowedRange)
        {
            unsetCursor();
            update();

            event->accept();
            return;
        }

        if (newMinX < dataMinX)
        {
            newMinX = dataMinX;
        }

        if (newMaxX > dataMaxX)
        {
            newMaxX = dataMaxX;
        }

        if (newMinX <= dataMinX &&
            newMaxX >= dataMaxX)
        {
            mHasCustomXRange = false;
            mCustomMinX = 0.0;
            mCustomMaxX = 0.0;
        }
        else
        {
            mHasCustomXRange = true;
            mCustomMinX = newMinX;
            mCustomMaxX = newMaxX;
        }

        unsetCursor();
        update();

        emit xRangeChanged(mCustomMinX,
            mCustomMaxX,
            mHasCustomXRange);

        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton &&
        mIsPanning)
    {
        mIsPanning = false;

        unsetCursor();
        update();

        event->accept();
        return;
    }

    event->ignore();
}

void MultiAxisPlotWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    mHasMousePosition = false;

    if (!mIsPanning &&
        !mIsSelectingZoomArea)
    {
        unsetCursor();
    }

    update();
}

QRect MultiAxisPlotWidget::plotAreaRect() const
{
    int leftAxisCount = 0;
    int rightAxisCount = 0;

    for (const MultiAxisPlotAxis& axis : mAxes)
    {
        if (axis.mUseRightSide)
        {
            ++rightAxisCount;
        }
        else
        {
            ++leftAxisCount;
        }
    }

    const int baseLeftMargin = 95;
    const int baseRightMargin = 95;
    const int extraAxisSpacing = 72;

    const int leftMargin =
        baseLeftMargin +
        qMax(0, leftAxisCount - 1) * extraAxisSpacing;

    const int rightMargin =
        baseRightMargin +
        qMax(0, rightAxisCount - 1) * extraAxisSpacing;

    return rect().adjusted(leftMargin,
        48,
        -rightMargin,
        -58);
}

bool MultiAxisPlotWidget::dataXRange(double& minX,
    double& maxX) const
{
    bool firstValue = true;

    for (const MultiAxisPlotSeries& series : mSeries)
    {
        if (series.mXValues.isEmpty())
        {
            continue;
        }

        auto minMaxX =
            std::minmax_element(series.mXValues.begin(),
                series.mXValues.end());

        if (firstValue)
        {
            minX = *minMaxX.first;
            maxX = *minMaxX.second;
            firstValue = false;
        }
        else
        {
            minX = std::min(minX,
                *minMaxX.first);
            maxX = std::max(maxX,
                *minMaxX.second);
        }
    }

    return !firstValue;
}

void MultiAxisPlotWidget::resetZoom()
{
    mHasCustomXRange = false;
    mCustomMinX = 0.0;
    mCustomMaxX = 0.0;

    mIsPanning = false;
    mIsSelectingZoomArea = false;

    unsetCursor();
    update();

    emit xRangeChanged(0.0,
        0.0,
        false);
}

void MultiAxisPlotWidget::applyExternalXRange(double minX,
    double maxX,
    bool hasCustomRange)
{
    if (!hasCustomRange)
    {
        mHasCustomXRange = false;
        mCustomMinX = 0.0;
        mCustomMaxX = 0.0;

        mIsPanning = false;
        mIsSelectingZoomArea = false;

        unsetCursor();
        update();
        return;
    }

    if (maxX <= minX)
    {
        return;
    }

    double dataMinX = 0.0;
    double dataMaxX = 0.0;

    if (!dataXRange(dataMinX,
        dataMaxX))
    {
        return;
    }

    double clampedMinX =
        std::max(minX,
            dataMinX);

    double clampedMaxX =
        std::min(maxX,
            dataMaxX);

    if (clampedMaxX <= clampedMinX)
    {
        return;
    }

    if (clampedMinX <= dataMinX &&
        clampedMaxX >= dataMaxX)
    {
        mHasCustomXRange = false;
        mCustomMinX = 0.0;
        mCustomMaxX = 0.0;
    }
    else
    {
        mHasCustomXRange = true;
        mCustomMinX = clampedMinX;
        mCustomMaxX = clampedMaxX;
    }

    mIsPanning = false;
    mIsSelectingZoomArea = false;

    unsetCursor();
    update();
}

bool MultiAxisPlotWidget::currentVisibleXRange(double& minX,
    double& maxX,
    bool& hasCustomRange) const
{
    hasCustomRange = false;

    double dataMinX = 0.0;
    double dataMaxX = 0.0;

    if (!dataXRange(dataMinX,
        dataMaxX))
    {
        return false;
    }

    if (mHasCustomXRange &&
        mCustomMaxX > mCustomMinX)
    {
        minX = mCustomMinX;
        maxX = mCustomMaxX;
        hasCustomRange = true;
        return true;
    }

    minX = dataMinX;
    maxX = dataMaxX;
    return true;
}

void MultiAxisPlotWidget::setShowMajorGrid(bool show)
{
    if (mShowMajorGrid == show)
    {
        return;
    }

    mShowMajorGrid = show;
    update();
}

void MultiAxisPlotWidget::setShowMinorGrid(bool show)
{
    if (mShowMinorGrid == show)
    {
        return;
    }

    mShowMinorGrid = show;
    update();
}

bool MultiAxisPlotWidget::showMajorGrid() const
{
    return mShowMajorGrid;
}

bool MultiAxisPlotWidget::showMinorGrid() const
{
    return mShowMinorGrid;
}

void MultiAxisPlotWidget::setTextScale(double textScale)
{
    if (textScale <= 0.0)
    {
        textScale = 1.0;
    }

    mTextScale = textScale;
    update();
}

int MultiAxisPlotWidget::seriesIndexAtPosition(const QPoint& position) const
{
    const QRect plotRect =
        plotAreaRect();

    if (!plotRect.contains(position))
    {
        return -1;
    }

    QVector<int> validSeriesIndexes;
    QVector<MultiAxisPlotSeries> validSeries;

    for (int i = 0;
        i < mSeries.size();
        ++i)
    {
        const MultiAxisPlotSeries& series =
            mSeries[i];

        if (!isValidSeries(series))
        {
            continue;
        }

        if (mHiddenSeriesNames.contains(series.mName))
        {
            continue;
        }

        validSeriesIndexes.append(i);
        validSeries.append(series);
    }

    if (validSeriesIndexes.isEmpty())
    {
        return -1;
    }

    double minX = 0.0;
    double maxX = 0.0;

    if (!dataXRange(minX,
        maxX))
    {
        return -1;
    }

    if (minX == maxX)
    {
        minX -= 1.0;
        maxX += 1.0;
    }

    if (mHasCustomXRange)
    {
        minX = mCustomMinX;
        maxX = mCustomMaxX;
    }

    if (maxX <= minX)
    {
        return -1;
    }

    const QVector<AxisRuntime> runtimeAxes =
        buildAxisRuntimes(validSeries,
            mAxes,
            5);

    if (runtimeAxes.isEmpty())
    {
        return -1;
    }

    const QPointF clickPoint(position);

    int nearestSeriesIndex = -1;
    double nearestDistance = std::numeric_limits<double>::max();

    for (int validIndex = validSeriesIndexes.size() - 1;
        validIndex >= 0;
        --validIndex)
    {
        const int seriesIndex =
            validSeriesIndexes[validIndex];

        const MultiAxisPlotSeries& series =
            mSeries[seriesIndex];

        const AxisRuntime* axis =
            axisForKey(runtimeAxes,
                series.mAxisKey);

        if (!axis)
        {
            continue;
        }

        for (int i = 1;
            i < series.mXValues.size();
            ++i)
        {
            const QPointF previousPoint =
                mapPoint(series.mXValues[i - 1],
                    series.mYValues[i - 1],
                    plotRect,
                    minX,
                    maxX,
                    *axis);

            const QPointF currentPoint =
                mapPoint(series.mXValues[i],
                    series.mYValues[i],
                    plotRect,
                    minX,
                    maxX,
                    *axis);

            const double distance =
                distanceToLineSegment(clickPoint,
                    previousPoint,
                    currentPoint);

            constexpr double SamePositionTolerancePixels = 1.0;

            if (distance < nearestDistance - SamePositionTolerancePixels)
            {
                nearestDistance = distance;
                nearestSeriesIndex = seriesIndex;
            }
        }
    }

    constexpr double HitTolerancePixels = 18.0;

    if (nearestSeriesIndex >= 0 &&
        nearestSeriesIndex < mSeries.size())
    {
        const int lineThickness =
            qMax(1,
                mSeries[nearestSeriesIndex].mLineThickness);

        const double effectiveTolerance =
            HitTolerancePixels + lineThickness;

        if (nearestDistance <= effectiveTolerance)
        {
            return nearestSeriesIndex;
        }
    }

    return -1;
}

void MultiAxisPlotWidget::wheelEvent(QWheelEvent* event)
{
    const QRect plotRect =
        plotAreaRect();

    if (!plotRect.contains(event->position().toPoint()))
    {
        event->ignore();
        return;
    }

    double dataMinX = 0.0;
    double dataMaxX = 0.0;

    if (!dataXRange(dataMinX,
        dataMaxX))
    {
        event->ignore();
        return;
    }

    if (dataMinX == dataMaxX)
    {
        event->ignore();
        return;
    }

    double currentMinX =
        mHasCustomXRange ? mCustomMinX : dataMinX;

    double currentMaxX =
        mHasCustomXRange ? mCustomMaxX : dataMaxX;

    const double currentRange =
        currentMaxX - currentMinX;

    if (currentRange <= 0.0)
    {
        event->ignore();
        return;
    }

    const double mouseRatio =
        static_cast<double>(event->position().x() - plotRect.left()) /
        static_cast<double>(plotRect.width());

    const double mouseDataX =
        currentMinX + mouseRatio * currentRange;

    const bool zoomIn =
        event->angleDelta().y() > 0;

    const double zoomFactor =
        zoomIn ? 0.80 : 1.25;

    double newRange =
        currentRange * zoomFactor;

    const double fullRange =
        dataMaxX - dataMinX;

    const double minAllowedRange =
        fullRange * 0.02;

    if (newRange < minAllowedRange)
    {
        newRange = minAllowedRange;
    }

    if (newRange > fullRange)
    {
        newRange = fullRange;
    }

    double newMinX =
        mouseDataX - mouseRatio * newRange;

    double newMaxX =
        newMinX + newRange;

    if (newMinX < dataMinX)
    {
        newMinX = dataMinX;
        newMaxX = dataMinX + newRange;
    }

    if (newMaxX > dataMaxX)
    {
        newMaxX = dataMaxX;
        newMinX = dataMaxX - newRange;
    }

    if (newMinX <= dataMinX &&
        newMaxX >= dataMaxX)
    {
        mHasCustomXRange = false;
        mCustomMinX = 0.0;
        mCustomMaxX = 0.0;
    }
    else
    {
        mHasCustomXRange = true;
        mCustomMinX = newMinX;
        mCustomMaxX = newMaxX;
    }

    update();

    emit xRangeChanged(mCustomMinX,
        mCustomMaxX,
        mHasCustomXRange);

    event->accept();
}

void MultiAxisPlotWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    const QRect plotRect =
        plotAreaRect();

    if (plotRect.contains(event->pos()))
    {
        resetZoom();
        event->accept();
        return;
    }

    event->ignore();
}

void MultiAxisPlotWidget::mousePressEvent(QMouseEvent* event)
{
    setFocus(Qt::MouseFocusReason);

    if (event->button() != Qt::LeftButton)
    {
        event->ignore();
        return;
    }

    const QPoint mousePosition =
        event->position().toPoint();

    const QRect plotRect =
        plotAreaRect();

    if (mShowLegend &&
        mLegendRect.contains(mousePosition))
    {
        mIsDraggingLegend = true;
        mLegendDragStartMousePos = mousePosition;
        mLegendDragStartOffset = mLegendOffset;

        setCursor(Qt::ClosedHandCursor);

        event->accept();
        return;
    }

    if (!plotRect.contains(mousePosition))
    {
        event->ignore();
        return;
    }

    double dataMinX = 0.0;
    double dataMaxX = 0.0;

    if (!dataXRange(dataMinX,
        dataMaxX))
    {
        event->ignore();
        return;
    }

    if (dataMinX == dataMaxX)
    {
        event->ignore();
        return;
    }

    const bool forceSelectZoom =
        event->modifiers().testFlag(Qt::ShiftModifier);

    const bool shouldSelectZoom =
        !mHasCustomXRange ||
        forceSelectZoom;

    if (shouldSelectZoom)
    {
        mIsSelectingZoomArea = true;
        mIsPanning = false;

        mZoomSelectionStart = mousePosition;
        mZoomSelectionEnd = mousePosition;

        mHasMousePosition = false;

        setCursor(Qt::CrossCursor);
        update();

        event->accept();
        return;
    }

    mIsPanning = true;
    mIsSelectingZoomArea = false;

    mPanStartMousePosition = mousePosition;

    mPanStartMinX =
        mHasCustomXRange
        ? mCustomMinX
        : dataMinX;

    mPanStartMaxX =
        mHasCustomXRange
        ? mCustomMaxX
        : dataMaxX;

    setCursor(Qt::ClosedHandCursor);

    event->accept();
}

void MultiAxisPlotWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        resetZoom();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void MultiAxisPlotWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);

    const int clickedSeriesIndex =
        seriesIndexAtPosition(event->pos());

    QAction* hideClickedSeriesAction = nullptr;

    if (clickedSeriesIndex >= 0 &&
        clickedSeriesIndex < mSeries.size())
    {
        hideClickedSeriesAction =
            menu.addAction(QString("Hide this line: %1")
                .arg(mSeries[clickedSeriesIndex].mName));

        menu.addSeparator();
    }

    QAction* resetZoomAction =
        menu.addAction("Reset Zoom");

    resetZoomAction->setEnabled(mHasCustomXRange);

    menu.addSeparator();

    QAction* showMajorGridAction =
        menu.addAction("Show Major Grid");
    showMajorGridAction->setCheckable(true);
    showMajorGridAction->setChecked(mShowMajorGrid);

    QAction* showMinorGridAction =
        menu.addAction("Show Minor Grid");
    showMinorGridAction->setCheckable(true);
    showMinorGridAction->setChecked(mShowMinorGrid);

    QAction* showAllHiddenSeriesAction = nullptr;

    if (!mHiddenSeriesNames.isEmpty())
    {
        menu.addSeparator();

        showAllHiddenSeriesAction =
            menu.addAction("Show all hidden lines");
    }

    QAction* selectedAction =
        menu.exec(event->globalPos());

    if (!selectedAction)
    {
        return;
    }

    if (hideClickedSeriesAction &&
        selectedAction == hideClickedSeriesAction)
    {
        mHiddenSeriesNames.insert(mSeries[clickedSeriesIndex].mName);
        update();
        return;
    }

    if (selectedAction == resetZoomAction)
    {
        resetZoom();
        return;
    }

    if (selectedAction == showMajorGridAction)
    {
        setShowMajorGrid(showMajorGridAction->isChecked());
        return;
    }

    if (selectedAction == showMinorGridAction)
    {
        setShowMinorGrid(showMinorGridAction->isChecked());
        return;
    }

    if (showAllHiddenSeriesAction &&
        selectedAction == showAllHiddenSeriesAction)
    {
        mHiddenSeriesNames.clear();
        update();
        return;
    }
}

QString MultiAxisPlotWidget::seriesNameAtIndex(int index) const
{
    if (index < 0 ||
        index >= mSeries.size())
    {
        return QString();
    }

    return mSeries[index].mName;
}

void MultiAxisPlotWidget::setShowLegend(bool show)
{
    if (mShowLegend == show)
    {
        return;
    }

    mShowLegend = show;
    update();
}

bool MultiAxisPlotWidget::showLegend() const
{
    return mShowLegend;
}
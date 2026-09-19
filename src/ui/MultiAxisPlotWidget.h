#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include <QColor>
#include <QPoint>
#include <QSet>

#include "MultiAxisPlotTypes.h"

class QEvent;
class QWheelEvent;
class QMouseEvent;
class QKeyEvent;
class QContextMenuEvent;

class MultiAxisPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MultiAxisPlotWidget(QWidget* parent = nullptr);

    void setSeries(const QVector<MultiAxisPlotSeries>& series,
        const QVector<MultiAxisPlotAxis>& axes,
        const QString& title,
        const QString& xAxisLabel);

    void clear();

    void setShowMajorGrid(bool show);
    void setShowMinorGrid(bool show);

    bool showMajorGrid() const;
    bool showMinorGrid() const;

    void setShowLegend(bool show);
    bool showLegend() const;

    void applyExternalXRange(double minX,
        double maxX,
        bool hasCustomRange);

    void setTextScale(double textScale);

    int seriesIndexAtPosition(const QPoint& position) const;

    bool currentVisibleXRange(double& minX,
        double& maxX,
        bool& hasCustomRange) const;

    void resetZoom();
    QString seriesNameAtIndex(int index) const;

signals:
    void xRangeChanged(double minX,
        double maxX,
        bool hasCustomRange);

protected:
    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QRect plotAreaRect() const;

    bool dataXRange(double& minX,
        double& maxX) const;

private:
    QVector<MultiAxisPlotSeries> mSeries;
    QVector<MultiAxisPlotAxis> mAxes;

    QString mTitle;
    QString mXAxisLabel;

    QPoint mMousePosition;
    bool mHasMousePosition = false;

    bool mHasCustomXRange = false;
    double mCustomMinX = 0.0;
    double mCustomMaxX = 0.0;

    bool mIsSelectingZoomArea = false;
    QPoint mZoomSelectionStart;
    QPoint mZoomSelectionEnd;

    bool mIsPanning = false;
    QPoint mPanStartMousePosition;
    double mPanStartMinX = 0.0;
    double mPanStartMaxX = 0.0;

    bool mShowMajorGrid = true;
    bool mShowMinorGrid = true;

    bool mShowLegend = false;

    QPointF mLegendOffset = QPointF(0, 0);
    bool mIsDraggingLegend = false;
    QPoint mLegendDragStartMousePos;
    QPointF mLegendDragStartOffset;
    QRect mLegendRect;

    double mTextScale = 1.0;

    QSet<QString> mHiddenSeriesNames;
};
#pragma once

#include <QColor>
#include <QString>
#include <QVector>
#include <Qt>

struct MultiAxisPlotAxis
{
    QString mKey;
    QString mLabel;
    bool mUseRightSide = false;
    QColor mColor;
};

struct MultiAxisPlotSeries
{
    QString mName;

    QVector<double> mXValues;
    QVector<double> mYValues;

    QString mAxisKey;

    QColor mColor;
    int mLineThickness = 2;
    Qt::PenStyle mLineStyle = Qt::SolidLine;
};
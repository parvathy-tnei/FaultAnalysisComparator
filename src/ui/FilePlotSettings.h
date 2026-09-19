#pragma once

#include <QColor>
#include <Qt>

struct FilePlotSettings
{
    bool mEnabled = false;
    QColor mColor;
    int mLineThickness = 2;
    Qt::PenStyle mLineStyle = Qt::SolidLine;
};

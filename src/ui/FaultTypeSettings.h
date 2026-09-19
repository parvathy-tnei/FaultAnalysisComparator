#pragma once

#include <QString>


struct FaultTypeSettings
{
    QString calculateType;
    QString faultType;
    QString resultType;

    QString faultTime;
    QString faultResistance;
    QString faultReactance;

    bool configured = false;
};
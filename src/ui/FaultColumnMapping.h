#pragma once

#include <QString>
#include <QMap>

#include "FaultTypeSettings.h"

struct FaultColumnMapping
{
    QString displayName;
    QString rawColumnName;
};

class FaultColumnMapper
{
public:
    static QMap<QString, FaultColumnMapping> createMapping(
        const FaultTypeSettings &settings);

    static QString displayNameForRawColumn(
        const FaultTypeSettings &settings,
        const QString &rawColumnName);

    static QString rawColumnForDisplayName(
        const FaultTypeSettings &settings,
        const QString &displayName);
};
#include "FaultColumnMapping.h"


QMap<QString, FaultColumnMapping>
FaultColumnMapper::createMapping(
    const FaultTypeSettings &settings)
{
    QMap<QString, FaultColumnMapping> mapping;

    // ---------------------------------------------------------
    // No configuration
    // ---------------------------------------------------------

    if (!settings.configured)
    {
        return mapping;
    }


    // =========================================================
    // LLL - SYMMETRIC RMS
    // =========================================================

    if (settings.faultType ==
            "Line-line-line (three phase)" &&
        settings.resultType ==
            "Symmetric RMS")
    {
        FaultColumnMapping current;

        current.displayName =
            "Symmetric RMS Current (kA)";

        current.rawColumnName =
            "AC Mag. (kA)";

        mapping.insert(
            "symmetric_rms_current",
            current
            );
    }


    // =========================================================
    // LLL - ASYMMETRIC RMS
    // =========================================================

    else if (settings.faultType ==
                 "Line-line-line (three phase)" &&
             settings.resultType ==
                 "Asymmetric RMS")
    {
        FaultColumnMapping current;

        current.displayName =
            "Asymmetric RMS Current (kA)";

        current.rawColumnName =
            "Red Phase Mag. (kA)";

        mapping.insert(
            "asymmetric_rms_current",
            current
            );
    }


    // =========================================================
    // LLL - PEAK
    // =========================================================

    else if (settings.faultType ==
                 "Line-line-line (three phase)" &&
             settings.resultType ==
                 "Peak")
    {
        FaultColumnMapping current;

        current.displayName =
            "Peak Current (kA)";

        current.rawColumnName =
            "Red Phase Mag. (kA)";

        mapping.insert(
            "peak_current",
            current
            );
    }


    return mapping;
}


QString FaultColumnMapper::displayNameForRawColumn(
    const FaultTypeSettings &settings,
    const QString &rawColumnName)
{
    QMap<QString, FaultColumnMapping> mapping =
        createMapping(settings);

    for (const FaultColumnMapping &column :
         mapping)
    {
        if (column.rawColumnName == rawColumnName)
        {
            return column.displayName;
        }
    }

    return rawColumnName;
}


QString FaultColumnMapper::rawColumnForDisplayName(
    const FaultTypeSettings &settings,
    const QString &displayName)
{
    QMap<QString, FaultColumnMapping> mapping =
        createMapping(settings);

    for (const FaultColumnMapping &column :
         mapping)
    {
        if (column.displayName == displayName)
        {
            return column.rawColumnName;
        }
    }

    return QString();
}
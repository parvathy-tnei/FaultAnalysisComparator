#pragma once

#include <QString>
#include <QStringList>
#include <QList>

class CsvReader
{
public:
    static bool readHeaders(const QString &filePath, QStringList &headers, QString &errorMessage);
    static bool readData(const QString &filePath, QStringList &headers, QList<QStringList> &rows, QString &errorMessage);
};
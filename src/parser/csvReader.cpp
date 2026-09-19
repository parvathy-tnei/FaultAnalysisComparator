#include "CsvReader.h"

#include <QFile>
#include <QTextStream>


bool CsvReader::readHeaders(const QString &filePath, QStringList &headers, QString &errorMessage)
{
    headers.clear();
    errorMessage.clear();

    //open csv file
    QFile file(filePath);
    if (!file.open(
            QIODevice::ReadOnly |
            QIODevice::Text))
    {
        errorMessage = QString("Unable to open CSV file:\n%1").arg(filePath);
        return false;
    }

     // Read first line
    QTextStream stream(&file);

    if (stream.atEnd())
    {
        errorMessage = "The CSV file is empty.";
        return false;
    }


    QString headerLine =stream.readLine();

    // =========================================================
    // REMOVE BOM IF PRESENT
    // =========================================================

    if (!headerLine.isEmpty() &&
        headerLine.at(0) == QChar(0xFEFF))
    {
        headerLine.remove(0, 1);
    }

    // =========================================================
    // DETERMINE SEPARATOR
    // =========================================================

    QChar separator = ',';

    if (headerLine.contains(';'))
    {
        separator = ';';
    }
    else if (headerLine.contains('\t'))
    {
        separator = '\t';
    }


    // split header
   headers = headerLine.split(separator,Qt::KeepEmptyParts);

    //remove spaces around headers
    for(QString &header:headers)
    {

        header = header.trimmed();
        if (header.startsWith('"') &&
            header.endsWith('"') &&
            header.length() >= 2)
        {
            header =
                header.mid(
                    1,
                    header.length() - 2
                    );
        }
    }
    if (!headers.isEmpty() &&
        headers.first().isEmpty())
    {
        headers.removeFirst();
    }



    if(headers.isEmpty()){
        errorMessage ="No CSV columns were found.";
        return false;
    }

    return true;
}

bool CsvReader::readData(const QString &filePath, QStringList &headers, QList<QStringList> &rows, QString &errorMessage)
{
    headers.clear();
    rows.clear();
    errorMessage.clear();

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        errorMessage= QString("Unable to open CSV file:\n%1").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    if (stream.atEnd()){
        errorMessage = "The CSV file is empty.";
        return false;
    }

    //Read header line
    QString headerLine = stream.readLine();

    // remove BOM if present

    if(!headerLine.isEmpty() && headerLine.at(0) == QChar(0xFEFF)){
        headerLine.remove(0,1);
    }

    QChar separator = ',';

    if (headerLine.contains(';')){
        separator = ';';
    }

    else if (headerLine.contains('\t')){
            separator = '\t';
        }

    //Read headers

    headers = headerLine.split(separator,Qt::KeepEmptyParts);

    //Clean Headers

    for(QString &header : headers)
    {
        header = header.trimmed();
        if(header.startsWith('"') && header.endsWith('"') && header.length()>=2)
        {
            header = header.mid(1,header.length()-2);
        }
    }

    bool hasUnnamedFirstColumn =!headers.isEmpty() && headers.first().isEmpty();

    if (hasUnnamedFirstColumn)
    {
        headers.removeFirst();
    }
    if (headers.isEmpty()){
        errorMessage = "No CSV columns were found";
        return false;
    }

    //Read data rows

    while (!stream.atEnd()){

        QString line = stream.readLine();
        if (line.trimmed().isEmpty())
        {
            continue;
        }

        QStringList row = line.split(separator,Qt::KeepEmptyParts);
        // ----------------------------------------------------
        // IMPORTANT FIX
        //
        // If the header had an unnamed first column,
        // remove the corresponding first value from EVERY row.
        //
        // Example:
        //
        // Header:
        // ,Name,AC Mag.,DC Mag.
        //
        // Row:
        // 1,Busbar1,1.73222,0
        //
        // After removing:
        //
        // Name,AC Mag.,DC Mag.
        //
        // Busbar1,1.73222,0
        // ----------------------------------------------------

        if (hasUnnamedFirstColumn &&
            !row.isEmpty())
        {
            row.removeFirst();
        }


        // ----------------------------------------------------
        // Clean data values
        // ----------------------------------------------------

        for (QString &value : row)
        {
            value = value.trimmed();

            if (value.startsWith('"') &&
                value.endsWith('"') &&
                value.length() >= 2)
            {
                value =
                    value.mid(
                        1,
                        value.length() - 2
                        );
            }
        }

        // Keep the same number of columns as the header

        while (row.size() < headers.size())
        {
            row.append("");
        }

        if (row.size() > headers.size())
        {
            row = row.mid(0, headers.size());
        }



        rows.append(row);
    }
    return true;

}
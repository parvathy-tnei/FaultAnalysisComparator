#pragma once

#include <QWidget>
#include <QStringList>
#include <QList>
#include <QMap>

class QTableWidget;
class QLabel;

class FaultComparisonTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FaultComparisonTableWidget(QWidget *parent = nullptr);

    QTableWidget *tableWidget() const;

    void setData(
        const QStringList &fileNames,
        const QList<QStringList> &headersPerFile,
        const QList<QList<QStringList>> &rowsPerFile,
        const QStringList &selectedColumns,
        const QList<QMap<QString, QString>> &columnMappingsPerFile,
        bool differenceEnabled
        );
    QStringList groupNames() const;


public slots:
    void setSelectedColumns(
        const QStringList &columns
        );

    void removeFileFromSavedOrder(
        const QString &fileName
        );

private:
    void RebuildTable();

    void saveColumnOrder();
    void restoreColumnOrder();
    void saveFileOrder();
    void restoreFileOrder();
    //void removeFileFromSavedOrder(const QString &fileName);
    bool columnAvailableForFile(int fileIndex, const QString &column) const;

    QString valueFor(
        int fileIndex,
        const QString &name,
        const QString &column
        ) const;

    double numericValueFor(
        int fileIndex,
        const QString &name,
        const QString &column
        ) const;


private:

    QTableWidget *mTable;
    QLabel *mEmptyLabel;
    QStringList mFileNames;
    QList<QStringList> mHeadersPerFile;
    QList<QList<QStringList>> mRowsPerFile;
    QStringList mSelectedColumns;
    QStringList mSavedColumnOrder;
    QStringList mSavedFileOrder;


    // Mapping for each individual CSV file.
    //
    // Example:
    //
    // File 0:
    // "Symmetric RMS Current" -> "AC Mag. (kA)"
    //
    // File 1:
    // "Asymmetric RMS Current" -> "Red Phase Mag. (kA)"
    //
    QList<QMap<QString, QString>> mColumnMappingsPerFile;

    // Global Difference option.
    bool mDifferenceEnabled = false;

    QStringList mGroupNames;
};
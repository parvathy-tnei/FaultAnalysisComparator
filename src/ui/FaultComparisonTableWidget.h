#pragma once

#include <QWidget>
#include <QStringList>
#include <QList>
#include <QMap>

#include "FaultComparisonSelectionWidget.h" // Provides TableLayoutMode

class QTableWidget;
class QTableView;
class QLabel;
class QScrollArea;
class QVBoxLayout;

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
        const QList<QMap<QString, QString>> &columnMappingsPerFile
        );

    QStringList groupNames() const;

public slots:
    void setSelectedColumns(const QStringList &columns);
    void removeFileFromSavedOrder(const QString &fileName);
    void setLayoutMode(TableLayoutMode mode);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void RebuildTable();
    void buildStackedView();
    void updateFrozenGeometry();

    void saveColumnOrder();
    void restoreColumnOrder();
    void saveFileOrder();
    void restoreFileOrder();

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
    QStringList                     mFileNames;
    QList<QStringList>              mHeadersPerFile;
    QList<QList<QStringList>>       mRowsPerFile;
    QStringList                     mSelectedColumns;
    QStringList                     mSavedColumnOrder;
    QStringList                     mSavedFileOrder;
    QList<QMap<QString, QString>>   mColumnMappingsPerFile;
    QStringList                     mGroupNames;
    QStringList                     mCustomRowOrder;

    bool                            mDifferenceEnabled = false;
    TableLayoutMode                 mLayoutMode = TableLayoutMode::SideBySide;

    QLabel                         *mComparisonTitle = nullptr;
    QLabel                         *mComparisonSummary = nullptr;
    QWidget                        *mComparisonLegend = nullptr;
    QLabel                         *mEmptyLabel = nullptr;

    // Mode A (Side by Side)
    QTableWidget                   *mTable = nullptr;
    QTableView                     *mFrozenView = nullptr; // Anchored overlay for # and Name

    // Mode B (Stacked)
    QScrollArea                    *mStackedScrollArea = nullptr;
    QWidget                        *mStackedContainer = nullptr;
    QVBoxLayout                    *mStackedLayout = nullptr;
};
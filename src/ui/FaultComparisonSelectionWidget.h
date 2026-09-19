#pragma once

#include <QWidget>
#include <QStringList>

//class QComboBox;
class QCheckBox;
class QGroupBox;
class QGridLayout;
class QScrollArea;
class QVBoxLayout;
class QPushButton;
class QLabel;

class FaultComparisonSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FaultComparisonSelectionWidget(QWidget *parent = nullptr);

    QStringList selectedColumns() const;
    bool differenceEnabled() const;

public slots:
    void setAvailableColumns(const QStringList &columns);
    void setSelectedColumns(const QStringList &columns);
    void setEmptyState(bool empty);


signals:
    void selectionChanged();

private slots:
    void selectAllColumns();
    void clearAllColumns();

private:
    void createColumnCheckboxes(const QStringList &columns);

private:

    //QGroupBox *mColumnGroup;
    //QGridLayout *mColumnLayout;
    QScrollArea *mColumnScrollArea;
    QWidget *mColumnContainer;
    QVBoxLayout *mColumnLayout;

    QPushButton *mSelectAllButton;
    QPushButton *mClearAllButton;
    QCheckBox *mDifferenceCheckBox;
    QLabel *mColumnsLabel;


};

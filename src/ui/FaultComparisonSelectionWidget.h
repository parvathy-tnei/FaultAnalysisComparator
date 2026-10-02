#pragma once

#include <QWidget>
#include <QStringList>
#include <QList>

class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QLabel;
class QLineEdit;
class QScrollArea;
class QPushButton;
class QButtonGroup;
class QRadioButton;
class QCheckBox;
class QFrame;

// ------------------------------------------------------------
// Layout Mode Enum (Defined here and used across comparison UI)
// ------------------------------------------------------------
enum class TableLayoutMode
{
    SideBySide = 0,
    Stacked    = 1
};

class FaultComparisonSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FaultComparisonSelectionWidget(QWidget *parent = nullptr);

    QStringList selectedColumns() const;
    void setSelectedColumns(const QStringList &columns);
    void setAvailableColumns(const QStringList &columns);

    TableLayoutMode currentLayoutMode() const;

signals:
    void selectionChanged();
    void layoutModeChanged(TableLayoutMode mode);

public slots:
    void setTopSectionCollapsed(bool collapsed);
    void toggleCollapsed(bool collapsed);
    void setEmptyState(bool empty);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void filterColumns(const QString &text);
    void selectAllColumns();
    void clearAllColumns();
    void onLayoutCardClicked(int id);

private:
    void createLayoutSection();
    void createColumnCheckboxes(const QStringList &columns);
    bool isNameColumn(QCheckBox *checkBox) const;

private:
    TableLayoutMode         mCurrentMode;

    QVBoxLayout            *mMainLayout;
    QWidget                *mHeaderWidget;
    QHBoxLayout            *mHeaderLayout;
    QToolButton            *mCollapseButton;
    QLabel                 *mTitleLabel;
    QLabel                 *mSubtitleLabel;

    QWidget                *mContentWidget;
    QVBoxLayout            *mContentLayout;
    QLineEdit              *mSearchEdit;
    QScrollArea            *mColumnScrollArea;
    QWidget                *mColumnContainer;
    QVBoxLayout            *mColumnLayout;

    QWidget                *mActionWidget;
    QHBoxLayout            *mActionLayout;
    QPushButton            *mSelectAllButton;
    QPushButton            *mClearAllButton;

    QButtonGroup           *mLayoutGroup;
    QFrame                 *mSideBySideCard;
    QFrame                 *mStackedCard;
    QRadioButton           *mSideBySideRadio;
    QRadioButton           *mStackedRadio;
};
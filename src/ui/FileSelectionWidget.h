#pragma once

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QStringList>
#include <Qt>

#include "FilePlotSettings.h"

class Study;

class QLabel;
class QFrame;
class QHBoxLayout;
class QToolButton;
class QCheckBox;
class QComboBox;
class QWidget;
class QSpacerItem;
class QScrollArea;
class QResizeEvent;

class FileSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileSelectionWidget(QWidget* parent = nullptr);

    void setStudies(const QVector<const Study*>& studies);

    QVector<int> selectedStudyIndexes() const;
    QVector<FilePlotSettings> filePlotSettings() const;

    QColor colorForFileIndex(int fileIndex) const;
    int thicknessForFileIndex(int fileIndex) const;
	void setPendingSelectedFileNames(
    const QStringList& selectedFileNames);
	void setSingleSelectionModeEnabled(bool enabled);
    void setAdvancedSignalPlottingModeEnabled(bool enabled);
	
protected:
    void resizeEvent(QResizeEvent* event) override;
	
signals:
    void settingsChanged();
	void removeStudyRequested(int studyIndex);

private:
    void updateFileInfo();
    void rebuildFileSelection();

    QString studyDisplayName(const Study* study) const;

	QIcon makeLineSampleIcon(const QColor& color,
							 int thickness,
							 Qt::PenStyle lineStyle) const;


    void showFileMenu(int fileIndex,
                      QToolButton* button);
    void updateOverflowButtons();
    void scrollFilesLeft();
    void scrollFilesRight();
	void enforceSingleSelection(int preferredIndex);

private:
    QVector<const Study*> mStudies;
    QVector<FilePlotSettings> mSettings;

    QFrame* mFileSelectionFrame = nullptr;
    QHBoxLayout* mFileSelectionLayout = nullptr;

    QVector<QToolButton*> mFileButtons;
	
	QVector<QWidget*> mFileChipWidgets;
	
	QLabel* mEmptyFilesLabel = nullptr;
	QSpacerItem* mTrailingStretch = nullptr;

	QStringList mStudyFileNames;

    QScrollArea* mFileScrollArea = nullptr;
    QToolButton* mScrollLeftButton = nullptr;
    QToolButton* mScrollRightButton = nullptr;

	bool mSingleSelectionModeEnabled = false;
    bool mAdvancedSignalPlottingModeEnabled = false;
};

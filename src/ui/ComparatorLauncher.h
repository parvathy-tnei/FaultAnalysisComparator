#pragma once

#include <QWidget>

class QPushButton;

class ComparatorLauncher : public QWidget   //our launcher is a qt window
{
    Q_OBJECT

public:

    explicit ComparatorLauncher(QWidget *parent = nullptr);

private slots:

    void OpenTransientAnalysis();
    void OpenFaultAnalysis();

private:

    QPushButton *mTransientButton;
    QPushButton *mFaultButton;


};

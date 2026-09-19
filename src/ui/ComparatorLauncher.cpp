#include "ComparatorLauncher.h"
#include "FaultAnalysisWindow.h"

#include "../../mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>



ComparatorLauncher::ComparatorLauncher(QWidget *parent):QWidget(parent) //this is the constructor, inside this we will create all UI elements
{

    setWindowTitle("ComparatorTool");
    resize(800, 500);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(20,20,20,20);


    //QLabel *title = new QLabel("Comparator Tool", this);
    //title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Select Comparison Tool",this);
    subtitle->setAlignment(Qt::AlignCenter);

    QFont subtitleFont;
    subtitleFont.setPointSize(12);
    subtitle->setFont(subtitleFont);



    mTransientButton = new QPushButton("Transient Analysis Comparison",this);
    mFaultButton = new QPushButton("Fault Anlaysis Comparison", this);

    mTransientButton->setMinimumHeight(50);
    mFaultButton->setMinimumHeight(50);

    layout->addStretch(2);
    layout->addWidget(subtitle);

    layout->addSpacing(40);
    layout->addWidget(mTransientButton);
    layout->addSpacing(10);
    layout->addWidget(mFaultButton);
    layout->addStretch(2);

    connect(mTransientButton, &QPushButton::clicked, this, &ComparatorLauncher::OpenTransientAnalysis);
    connect(mFaultButton, &QPushButton::clicked, this, &ComparatorLauncher::OpenFaultAnalysis);

}

void ComparatorLauncher:: OpenTransientAnalysis()
{
    MainWindow *window = new MainWindow();
    window->showMaximized();
    close();
}
void  ComparatorLauncher::OpenFaultAnalysis(){

    FaultAnalysisWindow *window = new FaultAnalysisWindow();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->showMaximized();
    close();

}























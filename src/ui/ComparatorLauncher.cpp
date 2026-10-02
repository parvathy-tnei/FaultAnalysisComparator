#include "ComparatorLauncher.h"
#include "FaultAnalysisWindow.h"
#include "../../mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QEvent>
#include <QGraphicsDropShadowEffect>

ComparatorLauncher::ComparatorLauncher(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Comparator Suite");
    resize(860, 520);
    setMinimumSize(760, 440);
    setObjectName("launcherRoot");

    setStyleSheet(
        "QWidget#launcherRoot {"
        "    background-color: #F8FAFC;"
        "}"
        "QFrame#hubCard {"
        "    background-color: #FFFFFF;"
        "    border: 1px solid #E2E8F0;"
        "    border-radius: 12px;"
        "}"
        "QFrame#hubCard:hover {"
        "    border: 1.5px solid #2563EB;"
        "}"
        "QLabel#cardTitle {"
        "    font-size: 17px;"
        "    font-weight: 700;"
        "    color: #0F172A;"
        "}"
        "QPushButton#hubCardAction {"
        "    background-color: #FFFFFF;"
        "    color: #1769AA;"
        "    border: 1px solid #1769AA;"
        "    border-radius: 6px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton#hubCardAction:hover {"
        "    background-color: #F1F7FC;"
        "    color: #1769AA;"
        "}"
        "QPushButton#hubCardAction:pressed {"
        "    background-color: #E2EFF9;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 30);
    mainLayout->setSpacing(0);

    // =========================================================
    // HEADER AREA (Power Systems Analysis text removed)
    // =========================================================
    mainLayout->addStretch(1);

    QLabel *mainTitle = new QLabel("Comparator Suite", this);
    mainTitle->setStyleSheet("font-size: 28px; font-weight: 800; color: #0F172A;");
    mainTitle->setAlignment(Qt::AlignCenter);

    QLabel *mainSubtitle = new QLabel("Select an analysis module to get started", this);
    mainSubtitle->setStyleSheet("font-size: 13px; color: #64748B; margin-top: 4px;");
    mainSubtitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(mainTitle);
    mainLayout->addWidget(mainSubtitle);
    mainLayout->addSpacing(32);

    // =========================================================
    // SIDE-BY-SIDE CARDS (Badges removed)
    // =========================================================
    QHBoxLayout *cardRow = new QHBoxLayout();
    cardRow->setSpacing(20);
    cardRow->setAlignment(Qt::AlignCenter);

    mTransientCard = createHubCard(
        "Transient Analysis",
        "Open Module  →",
        "transient"
        );

    mFaultCard = createHubCard(
        "Fault Analysis",
        "Open Module  →",
        "fault"
        );

    cardRow->addWidget(mTransientCard);
    cardRow->addWidget(mFaultCard);
    mainLayout->addLayout(cardRow);

    mainLayout->addStretch(2);



    // Make entire cards clickable
    mTransientCard->installEventFilter(this);
    mFaultCard->installEventFilter(this);
}

QFrame *ComparatorLauncher::createHubCard(
    const QString &title,
    const QString &actionText,
    const QString &cardId)
{
    QFrame *card = new QFrame(this);
    card->setObjectName("hubCard");
    card->setFixedSize(290, 136);
    card->setCursor(Qt::PointingHandCursor);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(15, 23, 42, 10));
    shadow->setOffset(0, 5);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 22, 22, 18);
    cardLayout->setSpacing(0);

    // Title
    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("cardTitle");
    cardLayout->addWidget(titleLabel);

    cardLayout->addStretch(1);

    // Button
    QPushButton *actionBtn = new QPushButton(actionText, card);
    actionBtn->setObjectName("hubCardAction");
    actionBtn->setCursor(Qt::PointingHandCursor);

    if (cardId == "transient")
    {
        mTransientBtn = actionBtn;
        connect(mTransientBtn, &QPushButton::clicked, this, &ComparatorLauncher::OpenTransientAnalysis);
    }
    else
    {
        mFaultBtn = actionBtn;
        connect(mFaultBtn, &QPushButton::clicked, this, &ComparatorLauncher::OpenFaultAnalysis);
    }

    cardLayout->addWidget(actionBtn);

    return card;
}

bool ComparatorLauncher::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        if (watched == mTransientCard)
        {
            OpenTransientAnalysis();
            return true;
        }
        else if (watched == mFaultCard)
        {
            OpenFaultAnalysis();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ComparatorLauncher::OpenTransientAnalysis()
{
    MainWindow *window = new MainWindow();
    window->showMaximized();
    close();
}

void ComparatorLauncher::OpenFaultAnalysis()
{
    FaultAnalysisWindow *window = new FaultAnalysisWindow();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->showMaximized();
    close();
}
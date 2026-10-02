#include <QWidget>

class QFrame;
class QPushButton;

class ComparatorLauncher : public QWidget
{
    Q_OBJECT

public:
    explicit ComparatorLauncher(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void OpenTransientAnalysis();
    void OpenFaultAnalysis();

private:
    QFrame *createHubCard(
        const QString &title,
        const QString &actionText,
        const QString &cardId
        );

    QFrame *mTransientCard = nullptr;
    QFrame *mFaultCard = nullptr;
    QPushButton *mTransientBtn = nullptr;
    QPushButton *mFaultBtn = nullptr;
};
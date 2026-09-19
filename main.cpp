#include "mainwindow.h"
#include "src/ui/ComparatorLauncher.h" //PR

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "src/model/Study.h"
#include "src/parser/ItfParser.h"

#include "src/core/Logger.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	Logger::instance().setMinimumLevel(Logger::Level::Debug);
    Logger::instance().setLogToConsole(true);
    Logger::instance().setLogToFile(true);
    Logger::instance().setLogFilePath("TAComparator.log");
    Logger::instance().install();

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "TAComparator_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    //MainWindow w; //PR
    //w.showMaximized(); //PR

    ComparatorLauncher launcher; //PR
    launcher.showMaximized(); //PR

    return QApplication::exec();
}

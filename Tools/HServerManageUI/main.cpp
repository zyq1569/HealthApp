#include "mainwindow.h"


///---------log file inc
#include "logging.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON
#include <QMessageBox>

///https://build-qt.fsu0413.me/?file=005-5.15%20Series/001-5.15.2%20for%20Windows
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(systray);

    QApplication a(argc, argv);
    HMainWindow w;
    w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
    w.show();
    udg::beginLogging();

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(nullptr, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);
    return a.exec();
}

#else

#include <QLabel>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HMainWindow w;
    w.show();

    return a.exec();
}
#endif

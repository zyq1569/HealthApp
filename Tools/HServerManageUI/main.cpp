#include "mainwindow.h"
#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON
#include <QMessageBox>
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(systray);

    QApplication a(argc, argv);
    HMainWindow w;
    w.show();

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

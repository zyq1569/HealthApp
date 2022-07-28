#include "mainapp.h"
#include "logging.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP


#include <QApplication>
#include <QFile>


#define LOADQSS(qssFile)                      \
{                                             \
    QString strQss;                           \
    QFile file(qssFile);                      \
    file.open(QFile::ReadOnly);               \
    if(file.isOpen())                         \
{                                         \
    strQss=QLatin1String(file.readAll()); \
    qApp->setStyleSheet(strQss);          \
    file.close();                         \
    }                                         \
    }


///
///need  office:
///
/// to do: https://curl.se/windows/
///        https://curl.se/download.html
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainApp MainWindow;

    udg::beginLogging();
    MainWindow.setWindowFlags(MainWindow.windowFlags()&  ~Qt::WindowMinimizeButtonHint);
    //MainWindow.show();///~Qt::WindowMaximizeButtonHint&
    MainWindow.showMaximized();// Max size show
    //MainWindow.showFullScreen();

    LOADQSS(":/qss/qss/style.qss");
    //a.setFont(QFont("ZYSong", 10));

    return a.exec();
}

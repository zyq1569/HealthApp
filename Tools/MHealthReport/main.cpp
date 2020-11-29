#include "mainapp.h"

#include "patientdata.h"


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
/// 红旗office、中标office、一铭office、极速office、泰山office等等
///
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainApp MainWindow;

    QString HttpUrl = "http://127.0.0.1:8080";
    QString Dir ="F:/log/down";
    setServerHttpUrl(HttpUrl);

    setDownDir(Dir);


    MainWindow.setWindowFlags(MainWindow.windowFlags()&  ~Qt::WindowMinimizeButtonHint);
    //MainWindow.show();///~Qt::WindowMaximizeButtonHint&
    MainWindow.showMaximized();// Max size show
    //MainWindow.showFullScreen();

    LOADQSS(":/qss/qss/style.qss");
    //a.setFont(QFont("ZYSong", 10));

    return a.exec();
}

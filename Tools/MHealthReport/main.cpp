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




static void *pull_one_url(void *url)
{
//    CURL *curl;

//    curl = curl_easy_init();
//    curl_easy_setopt(curl, CURLOPT_URL, url);
//    curl_easy_perform(curl); /* ignores error */
//    curl_easy_cleanup(curl);

    return NULL;
}

///
///need  office:
///
/// to do: https://curl.se/windows/
///        https://curl.se/download.html
/// 使用浏览器页面功能.暂时需要QWebEngineView. to do....???
///
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

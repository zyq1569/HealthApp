#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>
#include "Windows.h"
int main(int argc, char *argv[])
{
//    QString dll = "./DLL";
//    LPCSTR path =dll.toStdWString().c_str();
//    SetDllDirectoryA(path);
//    QCoreApplication::addLibraryPath("./DLL");

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

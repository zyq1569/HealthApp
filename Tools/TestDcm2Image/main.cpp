#include "mainwindow.h"
#include "logging.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //udg::beginLogging();
    w.show();
    return a.exec();
}

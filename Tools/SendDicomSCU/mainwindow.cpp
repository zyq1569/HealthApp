#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)//, /*scanner(patientdata),*/ sender(patientdata)
{
    ui->setupUi(this);

//    QString Dir = QDir::currentPath();
//    QString iniDir = Dir+"/config";
//    if (!isDirExist( iniDir))
//    {
//        CreatDir(iniDir);
//    }
//    QString configfilename = iniDir+"/dicomSCU.ini";
//#if defined(Q_OS_LINUX)
//    configfilename = iniDir+"/dicomSCU_linux.ini";
//#endif

//    QSettings configini(configfilename,QSettings::IniFormat);

}

MainWindow::~MainWindow()
{
//    QString Dir = QDir::currentPath();
//    QString iniDir = Dir+"/config";
//    if (!isDirExist( iniDir))
//    {
//        CreatDir(iniDir);
//    }
//    QString configfilename = iniDir+"/dicomSCU.ini";
//#if defined(Q_OS_LINUX)
//    configfilename = iniDir+"/dicomSCU_linux.ini";
//#endif

//    QSettings configini(configfilename,QSettings::IniFormat);

//    unsigned int size = destinations.size();
//    for (unsigned int i = 0; i < size; i++ )
//    {
//        std::stringstream stream;
//        stream << destinations[i].name << ","
//            << destinations[i].destinationHost << ","
//            << destinations[i].destinationPort << ","
//            << destinations[i].destinationAETitle << ","
//            << destinations[i].ourAETitle;
//        //QString sec ="/Destinations"
//       // configini.setValue("/Destinations", "pacs");
//        //configini.setValue("/",);
//    }
//    configini.setValue("/dir", "");

    delete ui;
}

void MainWindow::on_pbUpdate_clicked()
{
    QString dir = ui->cbDcmDir->currentText();

    startScan(dir);
//    scanner.DoScanAsync(dir);
}

void MainWindow::stopSend()
{
    //sender.Cancel();
}

void MainWindow::stopScan()
{
    //sender.Cancel();
}

void MainWindow::startSend(int destination, int threads)
{

}


void MainWindow::loadDestinationList()
{

}

void MainWindow::loadGlobalDestinationList()
{

}

void MainWindow::saveDestinationList()
{

}

void MainWindow::startScan(QString &path)
{
    sender.ScanPatient(path);

}

void MainWindow::on_pBSend_clicked()
{

}


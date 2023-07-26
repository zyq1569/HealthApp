#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QStandardItemModel>
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

    m_pMOdel = new QStandardItemModel(ui->tableView);
    m_pMOdel->setColumnCount(6);
    m_pMOdel->setHeaderData(0,Qt::Horizontal,QString(" "));
    m_pMOdel->setHeaderData(1,Qt::Horizontal,QString("姓名"));
    m_pMOdel->setHeaderData(2,Qt::Horizontal,QString("PatientID"));
    m_pMOdel->setHeaderData(3,Qt::Horizontal,QString("日期"));
    m_pMOdel->setHeaderData(4,Qt::Horizontal,QString("描述"));
    m_pMOdel->setHeaderData(5,Qt::Horizontal,QString("路径"));
    ui->tableView->setModel(m_pMOdel);

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
    QString dir = ui->cbDcmDir->toPlainText();

    startScan(dir);

    //show row col
    //ui->tableView;
    int rows = sender.listpatient.size();
    for (int i=0; i<rows; i++)
    {
        Patient pt = sender.listpatient[i];
        m_pMOdel->setItem(i,1,new QStandardItem(pt.patientname.c_str()));
        m_pMOdel->setItem(i,2,new QStandardItem(pt.patientid.c_str()));
        Study st = pt.studydatas[0];

        m_pMOdel->setItem(i,3,new QStandardItem(st.studydate.c_str()));
        QString str = st.studydesc.c_str();

        if (!st.filespath.empty())
        {
            str += "|images:";
            int size = st.filespath.size();
            str += QString::number(size);
        }
        m_pMOdel->setItem(i,4,new QStandardItem(str));
        m_pMOdel->setItem(i,5,new QStandardItem(st.dir.c_str()));

        QStandardItem *item = new QStandardItem();
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        m_pMOdel->setItem(i,0,item);
    }
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


void MainWindow::on_pBDir_clicked()
{
    QString  path = QFileDialog::getExistingDirectory(this,"select dicom dir...","./");
    //     ui->cbDcmDir->setCurrentText(path);
    ui->cbDcmDir->setText(path);
    ui->cbDcmDir->update();

}


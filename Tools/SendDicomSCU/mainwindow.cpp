#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QThreadPool>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)//, /*scanner(patientdata),*/ sender(patientdata)
{
    ui->setupUi(this);

    QThreadPool::globalInstance()->setMaxThreadCount(10);///???

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

    connect(&m_sender, &DicomSender::finishscandicomfile, this, &MainWindow::updatePatientList);



    m_pMOdel = new QStandardItemModel(ui->tableView);

    m_pMOdel->setColumnCount(7);
    m_pMOdel->setHeaderData(0,Qt::Horizontal,QString(""));
    //m_pMOdel->setHeaderData(1,Qt::Horizontal,QString("����"));
    //m_pMOdel->setHeaderData(2,Qt::Horizontal,QString("PatientID"));
    //m_pMOdel->setHeaderData(3,Qt::Horizontal,QString("����"));
    //m_pMOdel->setHeaderData(4,Qt::Horizontal,QString("����"));
    //m_pMOdel->setHeaderData(5,Qt::Horizontal,QString("·��"));
    m_pMOdel->setHeaderData(1,Qt::Horizontal,QString("Name"));
    m_pMOdel->setHeaderData(2,Qt::Horizontal,QString("PatientID"));
    m_pMOdel->setHeaderData(3,Qt::Horizontal,QString("Date"));
    m_pMOdel->setHeaderData(4,Qt::Horizontal,QString("Dec"));
    m_pMOdel->setHeaderData(5,Qt::Horizontal,QString("SOPUID"));
    m_pMOdel->setHeaderData(6,Qt::Horizontal,QString("Path"));
    ui->tableView->setModel(m_pMOdel);
    ui->tableView->setColumnWidth(0,1);
    ui->tableView->setColumnWidth(1,90);
    ui->tableView->setColumnWidth(6,300);
    //ui->tableView->resizeColumnToContents(5);
    //ui->tableView->resizeColumnToContents(1);
    //ui->tableView->resizeColumnToContents(2);
    //ui->tableView->resizeColumnsToContents();

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


}


void MainWindow::updatePatientList()
{
    ////show row col
    ////ui->tableView;
    int rows = m_sender.m_listpatient.size();
    for (int i=0; i<rows; i++)
    {
        Patient pt = m_sender.m_listpatient[i];
        QString name = pt.patientname.c_str();//^
        name.replace("^","");
        m_pMOdel->setItem(i,1,new QStandardItem(name.toStdString().c_str()/*pt.patientname.c_str()*/));
        m_pMOdel->setItem(i,2,new QStandardItem(pt.patientid.c_str()));
        Study st = pt.studydatas[0];

        m_pMOdel->setItem(i,3,new QStandardItem(st.studydate.c_str()));
        QString str = st.studydesc.c_str();

        if (!st.filespath.empty())
        {
            str += "|";
            int size = st.filespath.size();
            str += QString::number(size);
        }
        m_pMOdel->setItem(i,4,new QStandardItem(str));
        m_pMOdel->setItem(i,5,new QStandardItem(st.sopclassuid.c_str()));
        m_pMOdel->setItem(i,6,new QStandardItem(st.dir.c_str()));

        QStandardItem *item = new QStandardItem();
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        m_pMOdel->setItem(i,0,item);
    }
    ui->tableView->setColumnWidth(0,1);
    ui->tableView->setColumnWidth(1,90);
    ui->tableView->setColumnWidth(5,150);
    ui->tableView->setColumnWidth(6,600);
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
    m_sender.ScanPatient(path);

}

void MainWindow::on_pBSend_clicked()
{

}


void MainWindow::on_pBDir_clicked()
{
    QString dlg;
    dlg = ui->cbDcmDir->toPlainText();
    //���������� �����"./" ������ǰӦ�õ�Ŀ¼. QString()��Ϊ�ϴδ򿪵�Ŀ¼
    QString  path = QFileDialog::getExistingDirectory(this,"select dicom dir...",dlg/*QString()*/,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    //QString path(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = ShowDirsOnly);
    //ui->cbDcmDir->setCurrentText(path);
    ui->cbDcmDir->setText(path);
    ui->cbDcmDir->update();

}

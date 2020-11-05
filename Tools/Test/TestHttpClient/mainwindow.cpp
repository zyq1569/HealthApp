#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpclient.h"

#include "QNetworkAccessManager"
#include "QNetworkReply"
#include "QTextCodec"
#include "QFile"
#include "QFileInfo"

#include <QSocketNotifier>
#include <QLocalServer>
//#include <QLocalSocket>

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->m_getHttpData->hide();// hide: down studyimage file by this button
    m_httpclient = NULL;
    QDateTime curDateTime=QDateTime::currentDateTime();
    ui->m_endDate->setDate(curDateTime.date());

    //QString patientIdentity,patientName,patientId,patientSex,patientBirthday,patientTelNumber;
    //QString patientAddr,patientCarID,patientType,patientEmail,studyOrderIdentity,studyId,studyuid;
    //QString scheduledDateTime,ScheduledDate,orderDateTime,studyDescription,studyModality,aETitle;
    //QString studyType,studyCode,studyState,studyCost,studyDate,studyDepart,sStudyModality,costType;
    QStringList strs = {"patientId", "patientName", "patientSex", "studyModality", "patientBirthday",
                        "studyDescription","studyuid"};
    ui->m_tableWidget->setColumnCount(strs.count());
    //ui->m_tableView->setRowCount(200);
    ui->m_tableWidget->setHorizontalHeaderLabels(strs);
    ui->m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    //ui->m_tableView->verticalHeader()//setResizeMode(QHeaderView::Strtch);
    ui->m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//select rows
    ui->m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);///single rows
    ui->m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);// no edit
    ui->m_tableWidget->show();
    connect(ui->m_tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(getItem(int,int)));

    m_localserver =  new QLocalServer(this);
    connect(m_localserver, SIGNAL(newConnection()), this, SLOT(newclientConnection()));

    QLocalServer::removeServer(ImageAppName);
    if(m_localserver->listen(ImageAppName))
    {
        qDebug() << "Listen succeed";
    }
    else
    {
        qDebug() << "Listen failed";
    }

    ui->m_getMsg->show();
    ui->m_getMsg->setText("Get Server Text!");

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_m_getHttpData_clicked()
{
    m_httpurl = ui->m_URL->text();///"http://127.0.0.1:8080/login/login.html"
    QString studyuid = ui->m_studyuid->text();
    QString seruid = ui->m_seruid->text();
    QString imageuid = ui->m_imageuid->text();
    if (!m_httpclient)
    {
        m_httpclient = new HttpClient(NULL,"F:\\log\\down");
    }
    m_httpclient->getStudyImageFile(m_httpurl,studyuid,seruid,imageuid);


    return;
}


void MainWindow::getItem(int row, int col)
{
    //QString studyuid = item
    QString studyuid = ui->m_tableWidget->item(row,ui->m_tableWidget->columnCount()-1)->text();
    ui->m_studyuid->setText(studyuid);
    ui->m_seruid->setText("");
    ui->m_imageuid->setText("");
    on_m_getHttpData_clicked();

}
void MainWindow::updateStudyImageTable()
{
    disconnect(m_httpclient,&HttpClient::parseDataFinished,this,&MainWindow::updateStudyImageTable);
    if (m_httpclient)
    {
        ///------------?????? memory??---------------------------------------
        int oldrow = ui->m_tableWidget->rowCount();
        int oldcol = ui->m_tableWidget->columnCount();
        for (int i = 0;i < oldrow;i++)
        {
            for (int j = 0;j < oldcol;j++)
            {
                QTableWidgetItem *item = ui->m_tableWidget->item(i,j);
                if (item)
                {
                    delete item;
                }
            }
        }
        ui->m_tableWidget->setRowCount(0);
        ///------------?????? memory??------------------------------------------
        PatientStudyDB *StudyDB = m_httpclient->getPatientStudyDB();
        int rowcount = StudyDB->count;
        int colcount = ui->m_tableWidget->columnCount();
        ui->m_tableWidget->setRowCount(rowcount);
        for (int row=0; row<rowcount; row++)
        {
            ///for (int col=0; col<colcount; col++)
            ///{
            ///ui->m_tableView->item(row,col)->setData(col,StudyDB->rowinfo[row][col]);
            ///}m_tableWidget
            /// {"index", "patientId", "patientName", "patientSex",
            ///  "studyModality", "patientBirthday", "studyDescription","studyuid"};
            ui->m_tableWidget->setItem(row,0,new QTableWidgetItem(StudyDB->rowinfo[row].patientId));
            ui->m_tableWidget->setItem(row,1,new QTableWidgetItem(StudyDB->rowinfo[row].patientName));
            ui->m_tableWidget->setItem(row,2,new QTableWidgetItem(StudyDB->rowinfo[row].patientSex));
            ui->m_tableWidget->setItem(row,3,new QTableWidgetItem(StudyDB->rowinfo[row].studyModality));
            ui->m_tableWidget->setItem(row,4,new QTableWidgetItem(StudyDB->rowinfo[row].patientBirthday));
            ui->m_tableWidget->setItem(row,5,new QTableWidgetItem(StudyDB->rowinfo[row].studyDescription));
            ui->m_tableWidget->setItem(row,6,new QTableWidgetItem(StudyDB->rowinfo[row].studyuid));
        }
        ui->m_tableWidget->setColumnHidden(ui->m_tableWidget->columnCount()-1,true);
    }
}

void MainWindow::on_m_getStudyImages_clicked()
{

    ///http://127.0.0.1:8080/healthsystem/ris/stduyimage/?start=19700101&end=20191230&page=1&limit=10
    QString startDate = ui->m_startDate->text();
    QString endDate = ui->m_endDate->text();
    QString mod = ui->m_StudyModality->currentText();
    QUrl url = ui->m_URL->text();
    if (!m_httpclient)
    {
        m_httpclient = new HttpClient(this,"F:\\log\\down");
        m_httpclient->setHost(ui->m_URL->text());
    }
    connect(m_httpclient,&HttpClient::parseDataFinished,this,&MainWindow::updateStudyImageTable);
    m_httpclient->getStudyDBinfo(url,startDate,endDate,"1","100");

}

void MainWindow::on_m_tableWidget_cellClicked(int row, int column)
{
    QString studyuid = ui->m_tableWidget->item(row,ui->m_tableWidget->columnCount()-1)->text();
    ui->m_studyuid->setText(studyuid);
    ui->m_seruid->setText("");
    ui->m_imageuid->setText("");
}

void  MainWindow::newclientConnection()
{
    m_clientSocket = m_localserver->nextPendingConnection();

    qDebug() << m_localserver;
    qDebug() << "sl_newConnection" << m_clientSocket->state();

    connect(m_clientSocket, SIGNAL(readyRead()), this, SLOT(getClientData()));
    connect(m_clientSocket, SIGNAL(connected()), this, SLOT(socketConnect()));
    connect(m_clientSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnect()));
    connect(m_clientSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(socketError(QLocalSocket::LocalSocketError)));
    connect(m_clientSocket, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), this, SLOT(socketStateChanged(QLocalSocket::LocalSocketState)));

}

void MainWindow::getClientData()
{
    qDebug() << m_clientSocket;
    if(!m_clientSocket)
    {
        return;
    }
    QString msg;
    msg = m_clientSocket->readAll();
    qDebug() << msg;
    ui->m_getMsg->setText(msg);

    QString receive = "receive";

    m_clientSocket->write(receive.toUtf8());
    m_clientSocket->flush();
}

void MainWindow::socketConnect()
{
    qDebug() << "A new connection";
}

void MainWindow::socketDisconnect()
{
    qDebug() << "Disconnected";
}

void MainWindow::socketError(QLocalSocket::LocalSocketError socketError)
{
    qDebug() << socketError;
}

void MainWindow::socketStateChanged(QLocalSocket::LocalSocketState socketState)
{
    qDebug() << socketState;
}


void MainWindow::readFromServer(int fd)
{
    QFile filein;
    filein.open(stdin, QIODevice::ReadOnly);
    QSocketNotifier* sn = new QSocketNotifier(filein.handle(), QSocketNotifier::Read, this);
    connect(sn, SIGNAL(activated(int)), this, SLOT(readFromServer(int)));
    //    QFile filein;
    //    if(fd != filein.handle() )
    //    return;

    //    char buffer[256];
    //    //int count = read(filein.handle(), buffer, 256);
    //    ui->m_imageuid->setText("FROM SERVER:" + QString(buffer).left(count));
    //http://blog.chinaunix.net/uid-13830775-id-97753.html
}

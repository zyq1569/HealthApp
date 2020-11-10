#include "studyimage.h"
#include "ui_studyimage.h"

#include "httpclient.h"

#include <QLocalSocket>

StudyImage::StudyImage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StudyImage)
{
    ui->setupUi(this);


    m_httpclient = NULL;

    m_localSocket = new QLocalSocket(this);
    //connect(m_clientSocket, SIGNAL(readyRead()), this, SLOT(getClientData()));
    connect(this,SIGNAL(sendClientMsg(QString)),this,SLOT(sendToImageAppMsg(QString)));

    QStringList strs = {"patientId", "patientName", "patientSex", "studyModality", "patientBirthday",
                        "studyDescription","studyuid"};
    ui->m_tableWidget->setColumnCount(strs.count());

    ui->m_tableWidget->setHorizontalHeaderLabels(strs);
    ui->m_tableWidget->horizontalHeader()->setStretchLastSection(true);

    ui->m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//select rows
    ui->m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);///single rows
    ui->m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);// no edit
    ui->m_tableWidget->resizeColumnsToContents(); //根据内容调整列宽
    ui->m_tableWidget->show();



    connect(ui->m_tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(getItem(int,int)));


    ///temp value
    m_url = "http://127.0.0.1:8080";
}

StudyImage::~StudyImage()
{
    delete ui;
}


void  StudyImage::connectImageApp()
{
    m_localSocket->connectToServer(ImageAppName);
    if(m_localSocket->waitForConnected(1000))
    {
        qDebug() << "Connect to " + QString(ImageAppName) + " succeed.";
    }
    else
    {
        qDebug() << "Connect to " + QString(ImageAppName) + " failed";
    }
}

void  StudyImage::sendToImageAppMsg(QString data)
{
    if (QLocalSocket::UnconnectedState == m_localSocket->state() ||
            QLocalSocket::ClosingState == m_localSocket->state())
    {
        connectImageApp();
    }
    if (QLocalSocket::ConnectedState == m_localSocket->state())
    {
        if(m_localSocket->isValid())
        {
            QString msg = data;
            if(m_localSocket->write(msg.toUtf8()) == -1)
            {
                qDebug() << "An error occurred.Send message failed.";
            }
            else
            {
                qDebug() << "Send message succeed " << msg;
            }
            m_localSocket->flush();
        }
        else
        {
            qDebug() << "Does not connect to any Servers";
        }
    }
    else
    {
        qDebug() << "m_localSocket->state()---> QLocalSocket::ConnectingState ";
    }

}

void  StudyImage::disconnectImageApp()
{

}

void  StudyImage::ReadImageApp()
{
    qDebug() << "localSocket : " << m_localSocket->readAll();
}

void  StudyImage::connectImageAppCrash()
{

}


void StudyImage::on_m_getStudyDbImages_clicked()
{
    QString startDate = ui->m_startDate->text();
    QString endDate = ui->m_endDate->text();
    QString mod = ui->m_StudyModality->currentText();
    QString url = m_url;
    if (!m_httpclient)
    {
        m_httpclient = new HttpClient(this,"F:\\log\\down");
        m_httpclient->setHost(url);
    }
    connect(m_httpclient,&HttpClient::parseDataFinished,this,&StudyImage::updateStudyImageTable);
    m_httpclient->getStudyDBinfo(url,startDate,endDate,"1","100");
}

void StudyImage::updateStudyImageTable()
{
    disconnect(m_httpclient,&HttpClient::parseDataFinished,this,&StudyImage::updateStudyImageTable);
    if (m_httpclient)
    {
        ///------------ to do :: memory??---------------------------------------
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

        PatientStudyDB *StudyDB = m_httpclient->getPatientStudyDB();
        int rowcount = StudyDB->count;
        int colcount = ui->m_tableWidget->columnCount();
        ui->m_tableWidget->setRowCount(rowcount);
        for (int row=0; row<rowcount; row++)
        {
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


void StudyImage::on_m_tableWidget_cellDoubleClicked(int row, int column)
{
    QString studyuid = ui->m_tableWidget->item(row,ui->m_tableWidget->columnCount()-1)->text();
    emit sendClientMsg(studyuid);
}

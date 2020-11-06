#include "mainapp.h"
#include "ui_mainapp.h"

#include "httpclient.h"

#include <QProcess>
#include <QLocalSocket>


MainApp::MainApp(QWidget *parent): QMainWindow(parent), ui(new Ui::MainApp)
{
    ui->setupUi(this);
    m_QProcess = new QProcess(parent);

    m_localSocket = new QLocalSocket(this);
    //connect(m_clientSocket, SIGNAL(readyRead()), this, SLOT(getClientData()));
    connect(this,SIGNAL(sendClientMsg(QString)),this,SLOT(sendToImageAppMsg(QString)));
}

MainApp::~MainApp()
{
    delete ui;
}


void MainApp::on_m_RunApp_clicked()
{
    QString appPath = ui->m_AppDir->text();
    m_QProcess->start(appPath);
}

void MainApp::on_m_SendData_clicked()
{
    //    if (m_QProcess)
    //    {
    //        QString message;
    //        message = "MainApp";
    //        m_QProcess->write(message.toLocal8Bit());
    //    }
    QString data = ui->m_SendAppMsg->text();

    emit sendClientMsg(data);
}

void  MainApp::connectImageApp()
{
    m_localSocket->connectToServer(ImageAppName);
    if(m_localSocket->waitForConnected(1000))
    {
        //setOutputText("Connected");
        qDebug() << "Connect to " + QString(ImageAppName) + " succeed.";
    }
    else
    {
        //setOutputText("Connect failed");
        qDebug() << "Connect to " + QString(ImageAppName) + " failed";
    }
}

void  MainApp::sendToImageAppMsg(QString data)
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

void  MainApp::disconnectImageApp()
{

}

void  MainApp::ReadImageApp()
{
    qDebug() << "localSocket1 : " << m_localSocket->readAll();
}

void  MainApp::connectImageAppCrash()
{

}


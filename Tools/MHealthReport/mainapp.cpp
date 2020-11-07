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


void MainApp::on_m_getStudyDbImages_clicked()
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
    connect(m_httpclient,&HttpClient::parseDataFinished,this,&MainApp::updateStudyImageTable);
    m_httpclient->getStudyDBinfo(url,startDate,endDate,"1","100");
}

void MainApp::updateStudyImageTable()
{
    disconnect(m_httpclient,&HttpClient::parseDataFinished,this,&MainApp::updateStudyImageTable);
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

void MainApp::ParseDwonData()
{
    m_patientstudydb.count = 0;
    m_patientstudydb.rowinfo.clear();
    if (m_currentfiletype == DownFileType::dbinfo && m_currentDownData.size() > 1)
    {
        QJsonParseError jsonError;
        QJsonDocument paserDoc = QJsonDocument::fromJson(m_currentDownData, &jsonError);
        if (jsonError.error == QJsonParseError::NoError)
        {
            QJsonObject paserObj = paserDoc.object();
            if (paserObj.contains("code"))
            {
                QJsonValue codeValue = paserObj.take("code");
                if (codeValue.isDouble())
                {
                    m_patientstudydb.code = codeValue.toInt();
                }
            }
            if (paserObj.contains("msg"))
            {
                QJsonValue msgValue = paserObj["msg"];
                if (msgValue.isString())
                    m_patientstudydb.msg = msgValue.toString();
            }
            if (paserObj.contains("count"))
            {
                QJsonValue countValue = paserObj["count"];
                if (countValue.isDouble())
                    m_patientstudydb.count = countValue.toInt();
            }
            if (paserObj.contains("data"))
            {
                QJsonValue dataValue = paserObj.take("data");
                if (dataValue.isArray())
                {
                    QJsonArray array = dataValue.toArray();
                    for(int i = 0; i < array.size(); ++i)
                    {
                        StudyRowInfo rowinfo;
                        QJsonValue tmp = array.at(i);
                        setPatientDBinfo(tmp,rowinfo);
                        m_patientstudydb.rowinfo.push_back(rowinfo);
                    }
                }
            }
        }
        emit parseDataFinished();
    }
}

#include "studyimage.h"
#include "ui_studyimage.h"

#include "httpclient.h"
#include "patientdata.h"
#include <QLocalSocket>
#include <QMessageBox>
StudyImage::StudyImage(QWidget *parent) : QMainWindow(parent), ui(new Ui::StudyImage),m_sharedInfo(qApp->applicationPid()/*SHAREDHEALTH*/)
{
    ui->setupUi(this);
    ///---------------
    m_httpclient = NULL;
    m_localSocket = new QLocalSocket(this);
    //connect(m_clientSocket, SIGNAL(readyRead()), this, SLOT(getClientData()));
    connect(this,SIGNAL(sendClientMsg(QString)),this,SLOT(sendToImageAppMsg(QString)));

    QStringList strs = {"patientId", "patientName", "patientSex",  "patientBirthday","studyState","studyModality",
                        "studyDescription","studyuid","PatientIdentity","PatientID","StudyOrderIdentity"
                       };
    ui->m_tableWidget->setColumnCount(strs.count());

    ui->m_tableWidget->setHorizontalHeaderLabels(strs);
    ui->m_tableWidget->horizontalHeader()->setStretchLastSection(true);

    ui->m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//select rows
    ui->m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);///single rows
    ui->m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);// no edit
    //ui->m_tableWidget->resizeColumnsToContents(); //根据内容调整列宽
    ui->m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列bai
    ui->m_tableWidget->verticalHeader()->setFixedWidth(30);
    ui->m_tableWidget->horizontalHeader()->setHighlightSections(false);
    ui->m_tableWidget->show();
    int column = ui->m_tableWidget->columnCount();
    ui->m_tableWidget->setColumnHidden(column-1,true);
    ui->m_tableWidget->setColumnHidden(column-2,true);
    ui->m_tableWidget->setColumnHidden(column-3,true);
    ui->m_tableWidget->setColumnHidden(column-4,true);

    //connect(ui->m_tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(getItem(int,int)));=on_m_tableWidget_cellDoubleClicked
    ///------------CustomContextMenu---------------------------------
    ui->m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_menu = new QMenu(ui->m_tableWidget);

    QAction *action = new QAction("患者报告",this);
    //connect(action,SIGNAL(triggered()),this,SLOT(editReport()));
    connect(action,SIGNAL(triggered()),this,SLOT(viewReport()));
    m_menu->addAction(action);

    //temp del
    //action = new QAction("编辑信息",this);
    //connect(action,SIGNAL(triggered()),this,SLOT(editPatientInfo()));
    //m_menu->addAction(action);

    action = new QAction("检查图像",this);
    connect(action,SIGNAL(triggered()),this,SLOT(viewImage()));
    m_menu->addAction(action);
    ///------------CustomContextMenu---------------------------------
    ///
    ///
    m_urlImage  = false;
    m_urlReport = false;

    if (!m_httpclient)
    {
        m_httpclient = new HttpClient(this,getDownDir());
    }
    if (!m_hreadThread)
    {
        m_hreadThread = new HreadThread(&m_sharedInfo, false);
        m_hreadThread->start();
    }
    //savereport
    connect(m_hreadThread,SIGNAL(savereport(QString)),this,SLOT(editorSaveReport(QString)));
    ///启动共享内存
    m_sharedInfo.open();
}

void StudyImage::viewImage()
{
    //QMessageBox::information(NULL, tr("检查"),tr("查看图像!"));
    QString studyuid = ui->m_tableWidget->item(m_currentRow,ui->m_tableWidget->columnCount()-4)->text();
    QString studystate =  ui->m_tableWidget->item(m_currentRow,ui->m_tableWidget->columnCount()-7)->text();
    if (studystate == "已检查" || studystate == "诊断" || studystate == "报告审核")
    {
        //emit sendClientMsg(studyuid);
        if (m_urlImage)
        {
            emit lookImage(studyuid);
        }
        else
        {
            //emit sendClientMsg(studyuid);
            QString info= getServerHttpUrl()+"&"+getDownDir()+"&"+studyuid;
            emit sendClientMsg(info/*studyuid*/);
        }
    }
    else
    {
        QMessageBox::information(NULL, tr("未检查"),tr("目前无图像!"));
        return;
    }
}

void StudyImage::editorSaveReport(QString filename)
{
//editorSaveReport
    qDebug() << "editorSaveReport " << filename;
}

void StudyImage::sendEditorReport()
{
    m_sharedInfo.write(m_reportFile);
    disconnect(m_httpclient,SIGNAL(parseReportFinished()),this,SLOT(sendEditorReport));
}

void StudyImage::viewReport()
{
    //QMessageBox::information(NULL, tr("检查"),tr("查看报告!"));
    QString StudyOrderIdentity = ui->m_tableWidget->item(m_currentRow,ui->m_tableWidget->columnCount()-1)->text();
    QString studyuid           = ui->m_tableWidget->item(m_currentRow,ui->m_tableWidget->columnCount()-4)->text();
    if (m_urlReport)
    {
        emit lookReport(StudyOrderIdentity);
    }
    else
    {
        m_httpclient->setDwonloadDir(getDownDir());
        m_httpclient->setHost(getServerHttpUrl());
        m_httpclient->getStudyReportFile(getServerHttpUrl(),StudyOrderIdentity, studyuid);
        ///"http://" + serverHost + "/WADO?StudyOrderIdentity=" + StudyOrderIdentity + "&type=odt&
        /// QString info=  getServerHttpUrl()+"&DownDir="+getDownDir()+"&studyuid="+ studyuid+".odt";
        QString info = "file:" + getDownDir()+"/"+ studyuid+"/"+StudyOrderIdentity+".odt";
        //emit sendClientMsg(info);
        //启用共享内存通知报告打开
        //1.openword      Hsharedmemory m_sharedInfo;
        //parseReportFinished
        connect(m_httpclient,SIGNAL(parseReportFinished()),this,SLOT(sendEditorReport));
        m_reportFile = info;
    }
}

void StudyImage::editPatientInfo()
{
    QString Info = "PatientIdentity:"+ ui->m_tableWidget->item(m_currentRow,ui->m_tableWidget->columnCount()-3)->text();
    Info         = Info + "-PatientID:"+ ui->m_tableWidget->item(m_currentRow,ui->m_tableWidget->columnCount()-2)->text();
    Info         = Info + "-StudyOrderIdentity:"+ ui->m_tableWidget->item(m_currentRow,ui->m_tableWidget->columnCount()-1)->text();
    QMessageBox::information(NULL, tr("检查"),("编辑患者信息!"+Info));
}

StudyImage::~StudyImage()
{
    m_sharedInfo.close();

    delete ui;
}

void StudyImage::setUrlImage(bool flag)
{
    m_urlImage = flag;
}

void StudyImage::setUrlReport(bool flag)
{
    m_urlReport = flag;
}

void StudyImage::connectImageApp()
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

void StudyImage::sendToImageAppMsg(QString data)
{
    if (QLocalSocket::UnconnectedState == m_localSocket->state() || QLocalSocket::ClosingState == m_localSocket->state())
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

    if (!m_localSocket->bytesAvailable())
    {
        m_localSocket->waitForReadyRead();
    }

    QTextStream stream(m_localSocket);
    QString respond = stream.readAll();
    //QMessageBox::information(NULL, tr("readImageApp"),respond);
    qDebug() <<"readImageApp"<<respond;
}

void StudyImage::disconnectImageApp()
{

}

void  StudyImage::readImageApp()
{
    qDebug() << "localSocket : " << m_localSocket->readAll();
}

void  StudyImage::connectImageAppCrash()
{

}

///整个检查列表
void StudyImage::on_m_getStudyDbImages_clicked()
{
    QString startDate = ui->m_startDate->dateTime().toString("yyyyMMdd");
    QString endDate   = ui->m_endDate->dateTime().toString("yyyyMMdd");
    QString mod = ui->m_StudyModality->currentText();

//    if (!m_httpclient)
//    {
//        m_httpclient = new HttpClient(this,getDownDir());
//        m_httpclient->setHost(getServerHttpUrl());
//    }
    m_httpclient->setDwonloadDir(getDownDir());
    m_httpclient->setHost(getServerHttpUrl());
    connect(m_httpclient,&HttpClient::parseDataFinished,this,&StudyImage::updateStudyImageTable);
    m_httpclient->getStudyDBinfo(getServerHttpUrl(),startDate,endDate,"1","100");
}

void StudyImage::updateStudyImageTable()
{
    disconnect(m_httpclient,&HttpClient::parseDataFinished,this,&StudyImage::updateStudyImageTable);
    if (m_httpclient)
    {
        ///------------ to do :: memory??---------------------------------------
        int oldrow = ui->m_tableWidget->rowCount();
        int oldcol = ui->m_tableWidget->columnCount();
        for (int i = 0; i < oldrow; i++)
        {
            for (int j = 0; j < oldcol; j++)
            {
                QTableWidgetItem *item = ui->m_tableWidget->item(i,j);
                if (item)
                {
                    delete item;
                }
            }
        }
        ui->m_tableWidget->setRowCount(0);

        //QStringList strs = {"patientId", "patientName", "patientSex",  "patientBirthday","studyState","studyModality",
        // "studyDescription","studyuid","PatientIdentity","PatientID","StudyOrderIdentity"};
        PatientStudyOder *StudyOder = m_httpclient->getPatientStudyOder();
        int rowcount = StudyOder->count;
        ui->m_tableWidget->setRowCount(rowcount);
        for (int row=0; row<rowcount; row++)
        {
            int column=0;
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientID].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientName].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientSex].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientBirthday].Value));
            QString state = StudyOder->orderdata[row].studyorder[StudyState].Value;
            if (state == "1")
            {
                state = "已预约";
            }
            else if (state == "2")
            {
                state = "待检查";
            }
            else if (state == "3")
            {
                state = "已检查";
            }
            else if (state == "4")
            {
                state = "诊断";
            }
            else if (state == "5")
            {
                state = "报告审核";
            }
            else
            {
                state = "未知";
            }
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(state));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyModality].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyDescription].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyUID].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientIdentity].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientID].Value));
            ui->m_tableWidget->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyOrderIdentity].Value));
        }
        //ui->m_tableWidget->resizeColumnsToContents();
    }
}


void StudyImage::on_m_tableWidget_cellDoubleClicked(int row, int column)
{
    QString studyuid = ui->m_tableWidget->item(row,ui->m_tableWidget->columnCount()-4)->text();
    QString studystate =  ui->m_tableWidget->item(row,ui->m_tableWidget->columnCount()-7)->text();
    if (studystate == "已检查" || studystate == "诊断" || studystate == "报告审核")
    {
        //emit sendClientMsg(studyuid);
        if (m_urlImage)
        {
            emit lookImage(studyuid);
        }
        else
        {
            QString info= getServerHttpUrl()+"&"+getDownDir()+"&"+studyuid;
            emit sendClientMsg(info/*studyuid*/);
        }
    }
    else
    {
        QMessageBox::information(NULL, tr("未检查"),tr("目前无图像!"));
        return;
    }
}

void StudyImage::on_m_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    m_currentRow = ui->m_tableWidget->currentIndex().row();
    if (m_currentRow >= 0)
    {
        m_menu->exec(QCursor::pos());
    }
}

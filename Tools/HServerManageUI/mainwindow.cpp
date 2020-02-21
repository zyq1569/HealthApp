#include <QtWidgets>
#include <QSqlDatabase>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include"units.h"

HMainWindow::HMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HMainWindow)
{
    ui->setupUi(this);
    //----------
    for (int i=0; i<QPROCESSSIZE; i++)
    {
        m_pQProcess[i] = nullptr;
        m_bstorescp[i] = false;
    }
    QString Dir = QDir::currentPath();
    m_ExeDir = Dir;
    m_ImageDir = m_ExeDir;
    m_ExeDir = m_ExeDir.remove("/debug");
    m_ExeDir = m_ExeDir.remove("/ServerUI");
    m_ExeDir = m_ExeDir+"/win32/";
    ui->Dir_Store->setText(Dir+"/DCM_SAVE");
    ui->Dir_Pagefile->setText(Dir+"/PageWeb");
    m_Log4j2Config = m_ExeDir + "log";
    m_StoreScpName = "StoreDcmSCP.exe";
    m_QuerRScpName = "dcmqrScp.exe";
    m_WLMScpName = "WorklistScp.exe";
    m_Dcm2DBName = "SaveDcmInfoDb.exe";
    m_WebServerName = "health.jar";
    m_WebServerGoName = "GoWeb.exe";

    QString iniDir = Dir+"/config";
    if (!isDirExist( iniDir))
    {
        CreatDir(iniDir);
    }
    QString configfilename = iniDir+"/serveruiconfig.ini";
    if (isFileExist(configfilename))
    {
        QSettings configini(configfilename,QSettings::IniFormat);
        //dicom
        ui->port_wlm->setText(configini.value("/dicom/worklist_port").toString());
        ui->port_store->setText(configini.value("/dicom/stroe_port").toString());
        ui->port_qr->setText(configini.value("/dicom/query_port").toString());
        ui->Dir_Store->setText(configini.value("/dicom/image_dir").toString());

        //client
        ui->AEtitle->setText(configini.value("/client/aetitle1").toString());
        ui->clientPortValue->setText(configini.value("/client/port1").toString());
        ui->IpAddressValue->setText(configini.value("/client/ip1").toString());

        //web
        ui->Dir_Pagefile->setText(configini.value("/web/pagefile_dir").toString());
        ui->port_webserver->setText(configini.value("/web/port").toString());
        ui->comLevel->setCurrentText(configini.value("/web/log").toString());

        //mariadb
        ui->mysqlServerValue->setText(configini.value("/mariadb/server").toString());
        ui->mysqldbNameValue->setText(configini.value("/mariadb/sqlname").toString());
        ui->mysqlUserNameValue->setText(configini.value("/mariadb/username").toString());
        ui->mysqlPWDValue->setText(configini.value("/mariadb/sqlpwd").toString());
    }

    //----
    QSqlDatabase d = QSqlDatabase::addDatabase("QMYSQL");
    d.setHostName("127.0.0.1");
    d.setDatabaseName("HIT");
    d.setPort(3306);
    d.setUserName("root");
    d.setPassword("root");
    bool flag = d.open();
    if(flag)
    {
        flag=false;
    }
    else
    {
//        QMessageBox::critical(0,QObject::tr("连接数据库失败!!!"), d.lastError().text());
    }
}

HMainWindow::~HMainWindow()
{
    delete ui;
    //----------------
    for (int i=0; i<QPROCESSSIZE; i++)
    {
        if( m_pQProcess[i]!=nullptr)
        {
            m_pQProcess[i]->close();
            delete m_pQProcess[i];
            m_pQProcess[i] = nullptr;
        }
    }
    //---------------------------------------------------------------------
    QString Dir = QDir::currentPath();
    QString iniDir = Dir+"/config";
    if (!isDirExist( iniDir))
    {
        CreatDir(iniDir);
    }
    QString configfilename = iniDir+"/serveruiconfig.ini";
    if (!isFileExist(configfilename))
    {

    }
    QSettings configini(configfilename,QSettings::IniFormat);
    //dicom
    configini.setValue("/dicom/worklist_port",ui->port_wlm->text());
    configini.setValue("/dicom/stroe_port",ui->port_store->text());
    configini.setValue("/dicom/query_port",ui->port_qr->text());
    configini.setValue("/dicom/image_dir",ui->Dir_Store->text());

    //client
    configini.setValue("/client/aetitle1",ui->AEtitle->text());
    configini.setValue("/client/port1",ui->clientPortValue->text());
    configini.setValue("/client/ip1",ui->IpAddressValue->text());

    //web
    configini.setValue("/web/pagefile_dir",ui->Dir_Pagefile->text());
    configini.setValue("/web/port",ui->port_webserver->text());
    configini.setValue("/web/log",ui->comLevel->currentText());

    //mariadb
    configini.setValue("/mariadb/server",ui->mysqlServerValue->text());
    configini.setValue("/mariadb/sqlname",ui->mysqldbNameValue->text());
    configini.setValue("/mariadb/username",ui->mysqlUserNameValue->text());
    configini.setValue("/mariadb/sqlpwd",ui->mysqlPWDValue->text());

    //QMessageBox::information(this, tr("All program!"), tr("All exit ok!"));
}
void HMainWindow::on_StoreSCP_clicked()
{
    QString program = m_ExeDir + m_StoreScpName;
    QStringList arg;
    QString port = ui->port_store->text();
    arg.append(port);
    arg.append(ui->Dir_Store->text());
    arg.append("AppStart");// start sigle string
    if (!m_bstorescp[STORESCPQ] && m_pQProcess[STORESCPQ]==nullptr)
    {
        m_pQProcess[STORESCPQ] =  new QProcess(this);
        m_pQProcess[STORESCPQ]->start(program,arg);
        m_bstorescp[STORESCPQ] = true;
        ui->StoreSCP->setText("运行中!");
        m_bstorescp[STORESCPQ] = true;
        //QMessageBox::information(this, tr("DicomStore Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[STORESCPQ]!=nullptr)
    {
        m_pQProcess[STORESCPQ]->close();
        delete m_pQProcess[STORESCPQ];
        m_pQProcess[STORESCPQ] = nullptr;
        m_bstorescp[STORESCPQ] = false;
        ui->StoreSCP->setText("启动");
        //QMessageBox::information(this, tr("DicomStore Stop!"), tr("close app ok!"));
    }
}

void HMainWindow::on_WLMSCP_clicked()
{
    QString program = m_ExeDir + m_WLMScpName;
    m_MysqlServer = ui->mysqlServerValue->text();
    m_MysqlDbName = ui->mysqldbNameValue->text();
    m_MysqlUserName = ui->mysqlUserNameValue->text();
    m_MysqlPWD = ui->mysqlPWDValue->text();

    QStringList arg;
    arg.append(ui->port_wlm->text());
    arg.append(m_MysqlServer);
    arg.append(m_MysqlDbName);
    arg.append(m_MysqlUserName);
    arg.append(m_MysqlPWD);
    arg.append("AppStart");// start sigle string
    if (!m_bstorescp[WLMSCPQ] && m_pQProcess[WLMSCPQ]==nullptr)
    {
        m_pQProcess[WLMSCPQ] =  new QProcess(this);
        m_pQProcess[WLMSCPQ]->start(program,arg);
        m_bstorescp[WLMSCPQ] = true;
        ui->WLMSCP->setText("运行中!");
    }
    else if( m_pQProcess[WLMSCPQ]!=nullptr)
    {
        m_pQProcess[WLMSCPQ]->close();
        delete m_pQProcess[WLMSCPQ];
        m_pQProcess[WLMSCPQ] = nullptr;
        m_bstorescp[WLMSCPQ] = false;
        ui->WLMSCP->setText("启动");
    }
}

void HMainWindow::on_QRSCP_clicked()
{
    QString program = m_ExeDir + m_QuerRScpName;
    m_MysqlServer = ui->mysqlServerValue->text();
    m_MysqlDbName = ui->mysqldbNameValue->text();
    m_MysqlUserName = ui->mysqlUserNameValue->text();
    m_MysqlPWD = ui->mysqlPWDValue->text();

    QStringList arg;
    arg.append(ui->port_qr->text());
    arg.append(ui->Dir_Store->text());
    arg.append(ui->AEtitle->text());
    arg.append(ui->clientPortValue->text());
    arg.append(ui->IpAddressValue->text());
    arg.append(m_MysqlServer);
    arg.append(m_MysqlDbName);
    arg.append(m_MysqlUserName);
    arg.append(m_MysqlPWD);
    if (!m_bstorescp[QUERSCPQ] && m_pQProcess[QUERSCPQ]==nullptr)
    {
        m_pQProcess[QUERSCPQ] =  new QProcess(this);
        m_pQProcess[QUERSCPQ]->start(program,arg);
        m_bstorescp[QUERSCPQ] = true;
        ui->QRSCP->setText("运行中!");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[QUERSCPQ]!=nullptr)
    {
        m_pQProcess[QUERSCPQ]->close();
        delete m_pQProcess[QUERSCPQ];
        m_pQProcess[QUERSCPQ] = nullptr;
        m_bstorescp[QUERSCPQ] = false;
        ui->QRSCP->setText("启动");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
    }
}

void HMainWindow::on_Dcm2DB_clicked()
{
    QString program = m_ExeDir + m_Dcm2DBName;

    m_MysqlServer = ui->mysqlServerValue->text();
    m_MysqlDbName = ui->mysqldbNameValue->text();
    m_MysqlUserName = ui->mysqlUserNameValue->text();
    m_MysqlPWD = ui->mysqlPWDValue->text();

    QStringList arg;
    arg.append(ui->Dir_Store->text());
    arg.append(m_MysqlServer);
    arg.append(m_MysqlDbName);
    arg.append(m_MysqlUserName);
    arg.append(m_MysqlPWD);
    if (!m_bstorescp[DCM2DBQ] && m_pQProcess[DCM2DBQ]==nullptr)
    {
        m_pQProcess[DCM2DBQ] =  new QProcess(this);
        m_pQProcess[DCM2DBQ]->start(program,arg);
        m_bstorescp[DCM2DBQ] = true;
        ui->Dcm2DB->setText("运行中!");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[DCM2DBQ]!=nullptr)
    {
        m_pQProcess[DCM2DBQ]->close();
        delete m_pQProcess[DCM2DBQ];
        m_pQProcess[DCM2DBQ] = nullptr;
        m_bstorescp[DCM2DBQ] = false;
        ui->Dcm2DB->setText("启动");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
    }
}

void HMainWindow::on_WebServer_clicked()
{
    QString goWebServer = QDir::currentPath()+"/"+m_WebServerGoName;
    if (!isFileExist(goWebServer))
    {
        goWebServer =  m_ExeDir + m_WebServerGoName;
    }
    if (isFileExist(goWebServer)){
        //1 mysql: 1 ip 2 name 3 user  4pwd
        //5 page web / 6 port
        //7 studyimage dir
        //8 level
        m_MysqlServer = ui->mysqlServerValue->text();
        m_MysqlDbName = ui->mysqldbNameValue->text();
        m_MysqlUserName = ui->mysqlUserNameValue->text();
        m_MysqlPWD = ui->mysqlPWDValue->text();

        m_WebSerPort = ui->port_webserver->text();
        QString program = goWebServer;
        QStringList arg;
        //mysql
        arg.append(m_MysqlServer);
        arg.append(m_MysqlDbName);
        arg.append(m_MysqlUserName);
        arg.append(m_MysqlPWD);
        //web
        arg.append(ui->Dir_Pagefile->text());
        arg.append(m_WebSerPort);
        //image dir
        arg.append(ui->Dir_Store->text()+"/Images");
        //Level
        arg.append(ui->comLevel->currentText());

        if (!m_bstorescp[WEBSER] && m_pQProcess[WEBSER]==nullptr)
        {
            m_pQProcess[WEBSER] =  new QProcess(this);
            m_pQProcess[WEBSER]->start(program,arg);
            m_bstorescp[WEBSER] = true;
            ui->WebServer->setText("运行中!");
            //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
        }
        else if( m_pQProcess[WEBSER]!=nullptr)
        {
            m_pQProcess[WEBSER]->close();
            delete m_pQProcess[WEBSER];
            m_pQProcess[WEBSER] = nullptr;
            m_bstorescp[WEBSER] = false;
            ui->WebServer->setText("启动");
            //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
        }
    }else {
        QString JavaWeb = m_ExeDir + m_WebServerName;
        if (!isFileExist(goWebServer)){
            QMessageBox::information(this, tr("No web program Exist!"), tr("No web program Exist!"));
            return;
        }
        m_MysqlServer = ui->mysqlServerValue->text();
        m_MysqlDbName = ui->mysqldbNameValue->text();
        m_MysqlUserName = ui->mysqlUserNameValue->text();
        m_MysqlPWD = ui->mysqlPWDValue->text();
        m_WebSerPort = ui->port_webserver->text();
        QString program = "java";
        QStringList arg;

        arg.append("-jar");
        arg.append(m_ExeDir + m_WebServerName);

        arg.append("com.mysql.cj.jdbc.Driver");
        QString DriverUrl = "jdbc:mysql://"+m_MysqlServer+":3306/"+m_MysqlDbName+"?useUnicode=true&characterEncoding=UTF-8&serverTimezone=UTC";
        arg.append(DriverUrl);
        arg.append(m_MysqlUserName);
        arg.append(m_MysqlPWD);
        arg.append(m_WebSerPort);
        arg.append(ui->Dir_Store->text()+"/Images");
        arg.append(ui->Dir_Pagefile->text());
        arg.append(m_Log4j2Config);
        arg.append(ui->comLevel->currentText());
        if (!m_bstorescp[WEBSER] && m_pQProcess[WEBSER]==nullptr)
        {
            m_pQProcess[WEBSER] =  new QProcess(this);
            m_pQProcess[WEBSER]->start(program,arg);
            m_bstorescp[WEBSER] = true;
            ui->WebServer->setText("运行中!");
            //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
        }
        else if( m_pQProcess[WEBSER]!=nullptr)
        {
            m_pQProcess[WEBSER]->close();
            delete m_pQProcess[WEBSER];
            m_pQProcess[WEBSER] = nullptr;
            m_bstorescp[WEBSER] = false;
            ui->WebServer->setText("启动");
            //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
        }
    }
}

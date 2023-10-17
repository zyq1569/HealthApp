#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "units.h"
#include "logging.h"

#ifndef QT_NO_SYSTEMTRAYICON
#include <windows.h>
#endif

#include <QtWidgets>
#include <QtGlobal>
#include <QModelIndex>
#include "MysqlDb.h"
/// to do: 调整界面的布局 参考http://c.biancheng.net/view/9422.html
HMainWindow::HMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::HMainWindow)
{
    ui->setupUi(this);
    QString Dir     = QDir::currentPath();
    ///----------------------
    ///--Systray-icon---windows
#ifndef QT_NO_SYSTEMTRAYICON
    createTrayIcon();
    connect(m_TrayIcon, &QSystemTrayIcon::activated, this, &HMainWindow::iconActivated);
#endif
    ///------------------------------------------------------

    ///------------------------------------------------------
    QString logDir = Dir+"/win32/log";
#if defined(Q_OS_LINUX)
    logDir = iniDir+"/linux/log";
#endif
    if (!isDirExist(logDir))
    {
        CreatDir(logDir);
    }

    INFO_LOG("-----------HServerUI  start!-----------------------");
    //-------------------------------------------------------
    for (int i=0; i<QPROCESSSIZE; i++)
    {
        m_pQProcess[i] = nullptr;
        m_bstorescp[i] = false;
    }
    //QString Dir   = QDir::currentPath();

    m_ExeDir          = Dir;
    m_ImageDir        = m_ExeDir;
    m_ExeDir          = m_ExeDir.remove("/debug");
    m_ExeDir          = m_ExeDir.remove("/ServerUI");
    ui->Dir_Store->setText(Dir+"/DCM_SAVE");
    ui->Dir_Pagefile->setText(Dir+"/PageWeb");
    m_Log4j2Config    = m_ExeDir + "log";
    m_WebServerName   = "health.jar";
    ui->cBSqlite->setChecked(true);
#if defined(Q_OS_LINUX)
    m_ExeDir          = m_ExeDir+"/linux/";
    m_StoreScpName    = "StoreDcmSCP";
    m_QuerRScpName    = "dcmqrScp";
    m_WLMScpName      = "WorklistScp";
    m_Dcm2DBName      = "SaveDcmInfoDb";
    m_WebServerGoName = "GoWeb";
#elif defined(Q_OS_WIN32)
    m_ExeDir          = m_ExeDir+"/win32/";
    m_StoreScpName    = "StoreDcmSCP.exe";
    m_QuerRScpName    = "dcmqrScp.exe";
    m_WLMScpName      = "WorklistScp.exe";
    m_Dcm2DBName      = "SaveDcmInfoDb.exe";
    m_WebServerGoName = "GoWeb.exe";
#elif defined(Q_OS_WIN64)
    m_ExeDir          = m_ExeDir+"/win64/";
    m_StoreScpName    = "StoreDcmSCP.exe";
    m_QuerRScpName    = "dcmqrScp.exe";
    m_WLMScpName      = "WorklistScp.exe";
    m_Dcm2DBName      = "SaveDcmInfoDb.exe";
    m_WebServerGoName = "GoWeb.exe";
#endif

    QString iniDir = Dir+"/config";
    if (!isDirExist( iniDir))
    {
        CreatDir(iniDir);
    }
    QString configfilename = iniDir+"/serveruiconfig.ini";
#if defined(Q_OS_LINUX)
    configfilename = iniDir+"/serveruiconfig_linux.ini";
#endif
    //--------------- client query info
    m_model = nullptr;
    m_model =  new QStandardItemModel();
    m_model->setColumnCount(4);
    m_model->setHeaderData(0,Qt::Horizontal,QString::fromLocal8Bit("AEtitle"));
    m_model->setHeaderData(1,Qt::Horizontal,QString::fromLocal8Bit("Port"));
    m_model->setHeaderData(2,Qt::Horizontal,QString::fromLocal8Bit("IpAddress"));
    m_model->setHeaderData(3,Qt::Horizontal,QString::fromLocal8Bit("Comment"));
    //---------------------------------------------------------------------------
    QSettings configini(configfilename,QSettings::IniFormat);
    if (isFileExist(configfilename))
    {
        //dicom
        ui->port_wlm->setText(configini.value("/dicom/worklist_port").toString());
        ui->port_store->setText(configini.value("/dicom/stroe_port").toString());
        ui->port_qr->setText(configini.value("/dicom/query_port").toString());
        ui->Dir_Store->setText(configini.value("/dicom/image_dir").toString());
        ui->comLevel_Store->setCurrentText(configini.value("/dicom/log_Store").toString());
        ui->comLevel_worklist->setCurrentText(configini.value("/dicom/log_worklist").toString());
        ui->comLevel_Query->setCurrentText(configini.value("/dicom/log_Query").toString());
        ui->comLevel_SaveInfo->setCurrentText(configini.value("/dicom/log_SaveInfo").toString());

        ui->cBSqlite->setChecked(configini.value("/sqlite/localdb").toBool());
        on_cBSqlite_clicked();
        //client //configini.setValue("/client/count",count);
        int count = configini.value("/client/count").toInt();
        //m_model->setColumnCount(3);
        m_model->setRowCount(count);
        //m_model->setHeaderData(0,Qt::Horizontal,QString::fromLocal8Bit("AEtitle"));
        //m_model->setHeaderData(1,Qt::Horizontal,QString::fromLocal8Bit("Port"));
        //m_model->setHeaderData(2,Qt::Horizontal,QString::fromLocal8Bit("IpAddress"));
        if (0 < count)
        {
            ui->AEtitle->setText(configini.value("/client/aetitle0").toString());
            ui->clientPortValue->setText(configini.value("/client/port0").toString());
            ui->IpAddressValue->setText(configini.value("/client/ip0").toString());
            //m_model->setHeaderData(3,Qt::Horizontal,QString::fromLocal8Bit("Comment"));
            QString pre = "";
            for (int i=0; i<count; i++)
            {
                pre = QString::number(i);
                m_model->setItem(i, 0, new QStandardItem(configini.value("/client/aetitle"+pre).toString()));
                m_model->setItem(i, 1, new QStandardItem(configini.value("/client/port"+pre).toString()));
                m_model->setItem(i, 2, new QStandardItem(configini.value("/client/ip"+pre).toString()));
            }
        }
        ui->query_clientinfo->setModel(m_model);
        ui->query_clientinfo->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        ui->query_clientinfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->query_clientinfo->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
        //--------------------------------------------------------------------------

        //web
        ui->Dir_Pagefile->setText(configini.value("/web/pagefile_dir").toString());
        ui->port_webserver->setText(configini.value("/web/port").toString());
        ui->comLevel_web->setCurrentText(configini.value("/web/log_web").toString());

        //mariadb
        ui->mysqlServerValue->setText(configini.value("/mariadb/server").toString());
        ui->mysqldbNameValue->setText(configini.value("/mariadb/sqlname").toString());
        ui->mysqlUserNameValue->setText(configini.value("/mariadb/username").toString());
        ui->mysqlPWDValue->setText(configini.value("/mariadb/sqlpwd").toString());
    }
    INFO_LOG("-----------HServerUI  start!-----------------------");

}

HMainWindow::~HMainWindow()
{
    //LOG(INFO) << "HMainWindow::~HMainWindow";
    QString Dir = QDir::currentPath();
    QString iniDir = Dir+"/config";
    if (!isDirExist( iniDir))
    {
        CreatDir(iniDir);
    }
    QString configfilename = iniDir+"/serveruiconfig.ini";
#if defined(Q_OS_LINUX)
    configfilename = iniDir+"/serveruiconfig_linux.ini";
#endif

    QSettings configini(configfilename,QSettings::IniFormat);
    //if (isFileExist(configfilename))
    {
        //QSettings configini(configfilename,QSettings::IniFormat);
        //dicom
        configini.setValue("/dicom/worklist_port",ui->port_wlm->text());
        configini.setValue("/dicom/stroe_port",ui->port_store->text());
        configini.setValue("/dicom/query_port",ui->port_qr->text());
        configini.setValue("/dicom/image_dir",ui->Dir_Store->text());

        configini.setValue("/dicom/log_Store",ui->comLevel_Store->currentText());
        configini.setValue("/dicom/log_worklist",ui->comLevel_worklist->currentText());
        configini.setValue("/dicom/log_Query",ui->comLevel_Query->currentText());
        configini.setValue("/dicom/log_SaveInfo",ui->comLevel_SaveInfo->currentText());

        configini.setValue("/sqlite/localdb",ui->cBSqlite->isChecked());

        //client
        //configini.setValue("/client/aetitle1",ui->AEtitle->text());
        //configini.setValue("/client/port1",ui->clientPortValue->text());
        //configini.setValue("/client/ip1",ui->IpAddressValue->text());
        int count = m_model->rowCount();
        configini.setValue("/client/count",count);
        QString pre = "";
        for (int i=0 ; i<count ; i++)
        {
            pre = QString::number(i);
            configini.setValue("/client/aetitle"+ pre,m_model->data(m_model->index(i,0)).toString() );
            configini.setValue("/client/port"   + pre,m_model->data(m_model->index(i,1)).toString() );
            configini.setValue("/client/ip"     + pre,m_model->data(m_model->index(i,2)).toString() );
        }
        m_model->removeRows(0,m_model->rowCount());
        delete m_model;
        m_model = nullptr;
        //------------------

        //web
        configini.setValue("/web/pagefile_dir",ui->Dir_Pagefile->text());
        configini.setValue("/web/port",ui->port_webserver->text());
        configini.setValue("/web/log_web",ui->comLevel_web->currentText());

        //mariadb
        configini.setValue("/mariadb/server",ui->mysqlServerValue->text());
        configini.setValue("/mariadb/sqlname",ui->mysqldbNameValue->text());
        configini.setValue("/mariadb/username",ui->mysqlUserNameValue->text());
        configini.setValue("/mariadb/sqlpwd",ui->mysqlPWDValue->text());
    }

    //QMessageBox::information(this, tr("All program!"), tr("All exit ok!"));
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
    INFO_LOG("-----------All App  exit!-----------------------");
    //---------------------------------------------------------------------
    delete ui;//last!!
    INFO_LOG("-----------HServerUI  exit!-----------------------");
}
void HMainWindow::on_StoreSCP_clicked()
{
    QString program = m_ExeDir + m_StoreScpName;
    if (!isFileExist(program))
    {
        QMessageBox::information(this, m_StoreScpName, tr("No StoreSCP program Exist!"));
        return;
    }
    QStringList arg;
    QString port = ui->port_store->text();
    arg.append(port);
    arg.append(ui->Dir_Store->text());

    //#ifdef defined(Q_OS_WIN32) || defined(Q_OS_WIN32)
    //    arg.append("AppStart");// start sigle string
    //#endif
    QString log_lev = QString::number(ui->comLevel_Store->currentIndex()*10000);
    arg.append(log_lev);
    if (!m_bstorescp[STORESCPQ] && m_pQProcess[STORESCPQ]==nullptr)
    {
        m_pQProcess[STORESCPQ] =  new QProcess(this);
        m_pQProcess[STORESCPQ]->start(program,arg);
        m_bstorescp[STORESCPQ] = true;
        ui->StoreSCP->setText("运行中!");
        m_bstorescp[STORESCPQ] = true;
        //QMessageBox::information(this, tr("DicomStore Start!"), tr("run ok!"));
        INFO_LOG("-----------DicomStore Start!-----------------------");
    }
    else if( m_pQProcess[STORESCPQ]!=nullptr)
    {
        m_pQProcess[STORESCPQ]->close();
        delete m_pQProcess[STORESCPQ];
        m_pQProcess[STORESCPQ] = nullptr;
        m_bstorescp[STORESCPQ] = false;
        ui->StoreSCP->setText("启动");
        //QMessageBox::information(this, tr("DicomStore Stop!"), tr("close app ok!"));
        INFO_LOG("-----------DicomStore Stop!-----------------------");
    }
}

void HMainWindow::on_WLMSCP_clicked()
{
    QString program = m_ExeDir + m_WLMScpName;
    if (!isFileExist(program))
    {
        QMessageBox::information(this,m_WLMScpName, tr("No WLMSCP program Exist!"));
        return;
    }
    m_MysqlServer   = ui->mysqlServerValue->text();
    m_MysqlDbName   = ui->mysqldbNameValue->text();
    m_MysqlUserName = ui->mysqlUserNameValue->text();
    m_MysqlPWD      = ui->mysqlPWDValue->text();

    QStringList arg;
    arg.append(ui->port_wlm->text());
    if (ui->cBSqlite->isChecked())
    {
        arg.append("0.0.0.0");
    }
    else
    {
    arg.append(m_MysqlServer);
    }
    arg.append(m_MysqlDbName);
    arg.append(m_MysqlUserName);
    arg.append(m_MysqlPWD);
    arg.append("AppStart");// start sigle string
    QString log_lev = QString::number(ui->comLevel_worklist->currentIndex()*10000);
    arg.append(log_lev);
    if (!m_bstorescp[WLMSCPQ] && m_pQProcess[WLMSCPQ]==nullptr)
    {
        m_pQProcess[WLMSCPQ] =  new QProcess(this);
        m_pQProcess[WLMSCPQ]->start(program,arg);
        m_bstorescp[WLMSCPQ] = true;
        ui->WLMSCP->setText("运行中!");
        INFO_LOG("-----------Dicom Worklist Start!-----------------------");
    }
    else if( m_pQProcess[WLMSCPQ]!=nullptr)
    {
        m_pQProcess[WLMSCPQ]->close();
        delete m_pQProcess[WLMSCPQ];
        m_pQProcess[WLMSCPQ] = nullptr;
        m_bstorescp[WLMSCPQ] = false;
        ui->WLMSCP->setText("启动");
        INFO_LOG("-----------Dicom Worklist Stop!-----------------------");
    }
}

void HMainWindow::on_QRSCP_clicked()
{
    QString program = m_ExeDir + m_QuerRScpName;
    if (!isFileExist(program))
    {
        QMessageBox::information(this, m_QuerRScpName, tr("No QRSCP program Exist!"));
        return;
    }
    m_MysqlServer   = ui->mysqlServerValue->text();
    m_MysqlDbName   = ui->mysqldbNameValue->text();
    m_MysqlUserName = ui->mysqlUserNameValue->text();
    m_MysqlPWD      = ui->mysqlPWDValue->text();

    QStringList arg;
    arg.append(ui->port_qr->text());
    arg.append(ui->Dir_Store->text());
    arg.append(ui->AEtitle->text());
    arg.append(ui->clientPortValue->text());
    arg.append(ui->IpAddressValue->text());
    if (ui->cBSqlite->isChecked())
    {
        arg.append("0.0.0.0");
    }
    else
    {
        arg.append(m_MysqlServer);
    }
    arg.append(m_MysqlDbName);
    arg.append(m_MysqlUserName);
    arg.append(m_MysqlPWD);
    QString log_lev = QString::number(ui->comLevel_Query->currentIndex()*10000);
    arg.append(log_lev);
    int count = m_model->rowCount();
    if (count > 1)
    {
        arg.append(QString::number(count-1));
        for (int i=1 ; i<count ; i++)
        {
            QString AEtile     = m_model->data(m_model->index(i,0)).toString();
            QString ClientPort = m_model->data(m_model->index(i,1)).toString();
            QString IpAddress  = m_model->data(m_model->index(i,2)).toString();
            arg.append(AEtile);
            arg.append(ClientPort);
            arg.append(IpAddress);
        }
    }
    if (!m_bstorescp[QUERSCPQ] && m_pQProcess[QUERSCPQ]==nullptr)
    {
        m_pQProcess[QUERSCPQ] =  new QProcess(this);
        m_pQProcess[QUERSCPQ]->start(program,arg);
        m_bstorescp[QUERSCPQ] = true;
        ui->QRSCP->setText("运行中!");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
        INFO_LOG("-----------DcmQR SCPApp Start!-----------------------");
    }
    else if( m_pQProcess[QUERSCPQ]!=nullptr)
    {
        m_pQProcess[QUERSCPQ]->close();
        delete m_pQProcess[QUERSCPQ];
        m_pQProcess[QUERSCPQ] = nullptr;
        m_bstorescp[QUERSCPQ] = false;
        ui->QRSCP->setText("启动");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
        INFO_LOG("-----------DcmQR SCPApp Stop!-----------------------");
    }
}

void HMainWindow::on_Dcm2DB_clicked()
{
    QString program = m_ExeDir + m_Dcm2DBName;
    if (!isFileExist(program))
    {
        QMessageBox::information(this, m_Dcm2DBName, tr("No Dcm2DBapp program Exist!"));
        return;
    }
    m_MysqlServer   = ui->mysqlServerValue->text();
    m_MysqlDbName   = ui->mysqldbNameValue->text();
    m_MysqlUserName = ui->mysqlUserNameValue->text();
    m_MysqlPWD      = ui->mysqlPWDValue->text();

    QStringList arg;
    arg.append(ui->Dir_Store->text());
    if (ui->cBSqlite->isChecked())
    {
        arg.append("0.0.0.0");
    }
    else
    {
        arg.append(m_MysqlServer);
    }
    arg.append(m_MysqlDbName);
    arg.append(m_MysqlUserName);
    arg.append(m_MysqlPWD);
    QString log_lev = QString::number(ui->comLevel_SaveInfo->currentIndex()*10000);
    arg.append(log_lev);
    if (!m_bstorescp[DCM2DBQ] && m_pQProcess[DCM2DBQ]==nullptr)
    {
        m_pQProcess[DCM2DBQ] =  new QProcess(this);
        m_pQProcess[DCM2DBQ]->start(program,arg);
        m_bstorescp[DCM2DBQ] = true;
        ui->Dcm2DB->setText("运行中!");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
        INFO_LOG("-----------DcmInfo2DbApp Start!-----------------------");
    }
    else if( m_pQProcess[DCM2DBQ]!=nullptr)
    {
        m_pQProcess[DCM2DBQ]->close();
        delete m_pQProcess[DCM2DBQ];
        m_pQProcess[DCM2DBQ] = nullptr;
        m_bstorescp[DCM2DBQ] = false;
        ui->Dcm2DB->setText("启动");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
        INFO_LOG("-----------DcmInfo2DbApp Stop!-----------------------");
    }
}

void HMainWindow::on_WebServer_clicked()
{
    QString goWebServer = QDir::currentPath()+"/"+m_WebServerGoName;
    //QString goLogconfig  =  QDir::currentPath()+"/config/goWebConfig.json";
    if (!isFileExist(goWebServer))
    {
        goWebServer =  m_ExeDir + m_WebServerGoName;
    }
    if (isFileExist(goWebServer))
    {
        //if (!isFileExist(goLogconfig)){
        //QMessageBox::information(this, tr("No Web logconfig!"), goLogconfig);
        //return;
        //}
        //1 mysql: 1 ip 2 name 3 user  4pwd
        //5 page web / 6 port
        //7 studyimage dir
        //8 level
        m_MysqlServer   = ui->mysqlServerValue->text();
        m_MysqlDbName   = ui->mysqldbNameValue->text();
        m_MysqlUserName = ui->mysqlUserNameValue->text();
        m_MysqlPWD      = ui->mysqlPWDValue->text();

        m_WebSerPort = ui->port_webserver->text();
        QString program = goWebServer;
        QStringList arg;
        //mysql
        if (ui->cBSqlite->isChecked())
        {
            arg.append("0.0.0.0");
        }
        else
        {
            arg.append(m_MysqlServer);
        }
        arg.append(m_MysqlDbName);
        arg.append(m_MysqlUserName);
        arg.append(m_MysqlPWD);
        //web
        arg.append(ui->Dir_Pagefile->text());
        arg.append(m_WebSerPort);
        //image dir
        arg.append(ui->Dir_Store->text()+"/Images");
        //Level
        arg.append(QString("%1").arg(ui->comLevel_web->currentIndex()));

        if (!m_bstorescp[WEBSER] && m_pQProcess[WEBSER]==nullptr)
        {
            m_pQProcess[WEBSER] =  new QProcess(this);
            m_pQProcess[WEBSER]->start(program,arg);
            m_bstorescp[WEBSER] = true;
            ui->WebServer->setText("运行中!");
            //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
            INFO_LOG("-----------WebServer Start!-----------------------");
        }
        else if( m_pQProcess[WEBSER]!=nullptr)
        {
            m_pQProcess[WEBSER]->close();
            delete m_pQProcess[WEBSER];
            m_pQProcess[WEBSER] = nullptr;
            m_bstorescp[WEBSER] = false;
            ui->WebServer->setText("启动");
            //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
            INFO_LOG("-----------WebServer Stop!-----------------------");
        }
    }
    //else ///Java App
    //{
    //    QString JavaWeb = m_ExeDir + m_WebServerName;
    //    if (!isFileExist(goWebServer))
    //    {
    //        QMessageBox::information(this, tr("No web program Exist!"), tr("No web program Exist!"));
    //        return;
    //    }
    //    m_MysqlServer   = ui->mysqlServerValue->text();
    //    m_MysqlDbName   = ui->mysqldbNameValue->text();
    //    m_MysqlUserName = ui->mysqlUserNameValue->text();
    //    m_MysqlPWD      = ui->mysqlPWDValue->text();
    //    m_WebSerPort    = ui->port_webserver->text();
    //    QString program = "java";
    //    QStringList arg;

    //    //启动java 应用的参数-jar  filename[app.exe] other
    //    arg.append("-jar");
    //    arg.append(m_ExeDir + m_WebServerName);
    //    arg.append("com.mysql.cj.jdbc.Driver");
    //    QString DriverUrl = "jdbc:mysql://"+m_MysqlServer+":3306/"+m_MysqlDbName+"?useUnicode=true&characterEncoding=UTF-8&serverTimezone=UTC";
    //    arg.append(DriverUrl);
    //    arg.append(m_MysqlUserName);
    //    arg.append(m_MysqlPWD);
    //    arg.append(m_WebSerPort);
    //    arg.append(ui->Dir_Store->text()+"/Images");
    //    arg.append(ui->Dir_Pagefile->text());
    //    arg.append(m_Log4j2Config);
    //    arg.append(ui->comLevel_web->currentText());
    //    if (!m_bstorescp[WEBSER] && m_pQProcess[WEBSER]==nullptr)
    //    {
    //        m_pQProcess[WEBSER] =  new QProcess(this);
    //        m_pQProcess[WEBSER]->start(program,arg);
    //        m_bstorescp[WEBSER] = true;
    //        ui->WebServer->setText("运行中!");
    //        //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
    //    }
    //    else if( m_pQProcess[WEBSER]!=nullptr)
    //    {
    //        m_pQProcess[WEBSER]->close();
    //        delete m_pQProcess[WEBSER];
    //        m_pQProcess[WEBSER] = nullptr;
    //        m_bstorescp[WEBSER] = false;
    //        ui->WebServer->setText("启动");
    //        //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
    //    }
    //}
}

void HMainWindow::on_query_clientinfo_doubleClicked(const QModelIndex &index)
{
    // QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("on_query_clientinfo_doubleClicked!"));
}

void HMainWindow::on_query_clientinfo_clicked(const QModelIndex &index)
{
    //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("on_query_clientinfo_clicked!"));
}

void HMainWindow::on_query_add_clicked()
{
    int count = m_model->rowCount();
    QString pre =  QString::number(count);
    QString AEtile = ui->AEtitle->text();
    QString ClientPort = ui->clientPortValue->text();
    QString IpAddress = ui->IpAddressValue->text();
    if (count >= 30)
    {
        QMessageBox::information(this, tr("30 client is Max number!"), tr("no insert!"));
        return;
    }
    for (int i=0; i<count ; i++)
    {
        QString aetitle = m_model->data(m_model->index(i,0)).toString();
        QString port = m_model->data(m_model->index(i,1)).toString();
        QString ip = m_model->data(m_model->index(i,2)).toString();
        if (AEtile == aetitle)
        {
            QMessageBox::information(this, tr("Exit!"), tr("AEtile exit ok, no insert!"));
            return;
        }
        if (ClientPort == port && IpAddress == ip)
        {
            QMessageBox::information(this, tr("port&IP exit!"), tr("same port&ip, no insert!"));
            return;
        }
    }
    m_model->setItem(count, 0, new QStandardItem(ui->AEtitle->text()));
    m_model->setItem(count, 1, new QStandardItem(ui->clientPortValue->text()));
    m_model->setItem(count, 2, new QStandardItem(ui->IpAddressValue->text()));
    ui->query_clientinfo->setModel(m_model);
}

void HMainWindow::on_query_delete_clicked()
{
    int curRow=ui->query_clientinfo->currentIndex().row();//选中行
    m_model->removeRow(curRow);
}


void HMainWindow::on_query_modify_clicked()
{
    //int curRow=ui->query_clientinfo->currentIndex().row();//选中行
}

void HMainWindow::closeEvent(QCloseEvent *event)
{
    QCoreApplication::quit();
}
void HMainWindow::changeEvent(QEvent *event)
{
    if(event->type()!=QEvent::WindowStateChange)
        return;
    if(this->windowState()==Qt::WindowMinimized)
    {
#ifndef QT_NO_SYSTEMTRAYICON
        if (m_TrayIcon->isVisible())
        {
            //QMessageBox::information(this, tr("Systray"),
            //                         tr("The program will keep running in the "
            //                            "system tray. To terminate the program, "
            //                            "choose <b>close</b> in the context menu "
            //                            "of the system tray entry."));
            hide();
            event->ignore();
        }
#endif
    }
}

void HMainWindow::createTrayIcon()
{
    m_RestoreAction = new QAction(tr("&Restore"), this);
    connect(m_RestoreAction, &QAction::triggered, this, &QWidget::showNormal);

    m_QuitAction = new QAction(tr("&Quit"), this);
    connect(m_QuitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    m_TrayIconMenu = new QMenu(this);
    m_TrayIconMenu->addAction(m_RestoreAction);
    m_TrayIconMenu->addSeparator();
    m_TrayIconMenu->addAction(m_QuitAction);

    m_Icon = QIcon(":/images/server.png"), tr("Server");
    m_TrayIcon = new QSystemTrayIcon(this);
    m_TrayIcon->setIcon(m_Icon);
    setWindowIcon(m_Icon);
    m_TrayIcon->setToolTip("Healthsystem ServerManage UI");
    m_TrayIcon->setContextMenu(m_TrayIconMenu);
    m_TrayIcon->show();
}

#ifndef QT_NO_SYSTEMTRAYICON
void HMainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {

    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        /// 1. set Window TOPMOST 2. set Window NOTOPMOST
        this->showNormal();
        ::SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        ::SetWindowPos(HWND(this->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        this->show();
        break;
    case QSystemTrayIcon::MiddleClick:
    {
        QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(1/*MessageIcon::Information*/);
        m_TrayIcon->showMessage("HServerManageUI", "This is a HServerManageUI(PACS&RIS) App!", msgIcon,500);
        break;
    }

    default:
        ;
    }
}
#endif

void HMainWindow::on_Sql_Echo_clicked()
{
    m_MysqlServer   = ui->mysqlServerValue->text();
    m_MysqlDbName   = ui->mysqldbNameValue->text();
    m_MysqlUserName = ui->mysqlUserNameValue->text();
    m_MysqlPWD      = ui->mysqlPWDValue->text();

    MysqlDb mysql;
    std::string error;
    bool ok = mysql.open(m_MysqlServer.toStdString().c_str(), m_MysqlUserName.toStdString().c_str(), m_MysqlPWD.toStdString().c_str(), m_MysqlDbName.toStdString().c_str(), error);
    if (ok)
    {
        QMessageBox::information(this,"infor", "success");
    }
    else
    {
        QMessageBox::information(this,"infor", "fail!");
    }
}

void HMainWindow::on_cBSqlite_clicked()
{
    QString mess = "some app running, sqlite --> maridb|mysql db ? stop app.";
    bool flag = true;
    if (ui->cBSqlite->isChecked())
    {
        flag = false;
        mess = "some app running, maridb|mysql --> sqlite db ? stop app.";
    }
    if(m_bstorescp[WLMSCPQ] || m_bstorescp[STORESCPQ] || m_bstorescp[QUERSCPQ] || m_bstorescp[DCM2DBQ] ||m_bstorescp[WEBSER] )
    {
        if (QMessageBox::Yes == QMessageBox::question(NULL, "question", mess+  " stop some app running?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
        {
            if(m_pQProcess[DCM2DBQ])
            {
                m_pQProcess[DCM2DBQ]->close();
                delete m_pQProcess[DCM2DBQ];
                m_pQProcess[DCM2DBQ] = nullptr;
                m_bstorescp[DCM2DBQ] = false;
                ui->Dcm2DB->setText("启动");
                INFO_LOG("-----------DcmInfo2DbApp Stop!-----------------------");
            }
            if(m_pQProcess[QUERSCPQ])
            {
                m_pQProcess[QUERSCPQ]->close();
                delete m_pQProcess[QUERSCPQ];
                m_pQProcess[QUERSCPQ] = nullptr;
                m_bstorescp[QUERSCPQ] = false;
                ui->QRSCP->setText("启动");
                INFO_LOG("-----------DcmQR SCPApp Stop!-----------------------");
            }
            if(m_pQProcess[WLMSCPQ])
            {
                m_pQProcess[WLMSCPQ]->close();
                delete m_pQProcess[WLMSCPQ];
                m_pQProcess[WLMSCPQ] = nullptr;
                m_bstorescp[WLMSCPQ] = false;
                ui->WLMSCP->setText("启动");
                INFO_LOG("-----------Dicom Worklist Stop!-----------------------");
            }
            if(m_pQProcess[STORESCPQ])
            {
                m_pQProcess[STORESCPQ]->close();
                delete m_pQProcess[STORESCPQ];
                m_pQProcess[STORESCPQ] = nullptr;
                m_bstorescp[STORESCPQ] = false;
                ui->StoreSCP->setText("启动");
                INFO_LOG("-----------DicomStore Stop!-----------------------");
            }
        }
        else
        {
            ui->cBSqlite->setChecked(flag);
            flag = !flag;
            return;
        }
    }
    if (flag)
    {
        ui->groupBox_3->setDisabled(false);
        ui->mysqlServerValue->setDisabled(false);
        ui->mysqldbNameValue->setDisabled(false);
        ui->mysqlUserNameValue->setDisabled(false);
        ui->mysqlPWDValue->setDisabled(false);
    }
    else
    {
        ui->groupBox_3->setDisabled(true);
        ui->mysqlServerValue->setDisabled(true);
        ui->mysqldbNameValue->setDisabled(true);
        ui->mysqlUserNameValue->setDisabled(true);
        ui->mysqlPWDValue->setDisabled(true);
    }
}


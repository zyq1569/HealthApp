#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"

HMainWindow::HMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HMainWindow)
{
    ui->setupUi(this);
    //----------
    for (int i=0; i<QPROCESSSIZE; i++)
    {
        m_pQProcess[i] = NULL;
        m_bstorescp[i] = false;
    }
    m_ExeDir = QDir::currentPath();
    m_ImageDir = m_ExeDir;
    m_ExeDir = m_ExeDir.remove("/debug");
    m_ExeDir = m_ExeDir.remove("/ServerUI");
    m_ExeDir = m_ExeDir+"/win32/";
    m_StoreScpName = "StoreDcmSCP.exe";
    m_QuerRScpName = "dcmqrScp.exe";
    m_WLMScpName = "WorklistScp.exe";
    m_Dcm2DBName = "SaveDcmInfoDb.exe";
}

HMainWindow::~HMainWindow()
{
    delete ui;
    //----------------
    for (int i=0; i<QPROCESSSIZE; i++)
    {
        if( m_pQProcess[i]!=NULL)
        {
            m_pQProcess[i]->close();
            delete m_pQProcess[i];
            m_pQProcess[i] = NULL;
        }
    }
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
    if (!m_bstorescp[STORESCPQ] && m_pQProcess[STORESCPQ]==NULL)
    {
        m_pQProcess[STORESCPQ] =  new QProcess(this);
        m_pQProcess[STORESCPQ]->start(program,arg);
        m_bstorescp[STORESCPQ] = true;
        ui->StoreSCP->setText("运行中!");
        m_bstorescp[STORESCPQ] = true;
        //QMessageBox::information(this, tr("DicomStore Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[STORESCPQ]!=NULL)
    {
        m_pQProcess[STORESCPQ]->close();
        delete m_pQProcess[STORESCPQ];
        m_pQProcess[STORESCPQ] = NULL;
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
    if (!m_bstorescp[WLMSCPQ] && m_pQProcess[WLMSCPQ]==NULL)
    {
        m_pQProcess[WLMSCPQ] =  new QProcess(this);
        m_pQProcess[WLMSCPQ]->start(program,arg);
        m_bstorescp[WLMSCPQ] = true;
        ui->WLMSCP->setText("运行中!");
    }
    else if( m_pQProcess[WLMSCPQ]!=NULL)
    {
        m_pQProcess[WLMSCPQ]->close();
        delete m_pQProcess[WLMSCPQ];
        m_pQProcess[WLMSCPQ] = NULL;
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
    if (!m_bstorescp[QUERSCPQ] && m_pQProcess[QUERSCPQ]==NULL)
    {
        m_pQProcess[QUERSCPQ] =  new QProcess(this);
        m_pQProcess[QUERSCPQ]->start(program,arg);
        m_bstorescp[QUERSCPQ] = true;
        ui->QRSCP->setText("运行中!");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[QUERSCPQ]!=NULL)
    {
        m_pQProcess[QUERSCPQ]->close();
        delete m_pQProcess[QUERSCPQ];
        m_pQProcess[QUERSCPQ] = NULL;
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
    if (!m_bstorescp[DCM2DBQ] && m_pQProcess[DCM2DBQ]==NULL)
    {
        m_pQProcess[DCM2DBQ] =  new QProcess(this);
        m_pQProcess[DCM2DBQ]->start(program,arg);
        m_bstorescp[DCM2DBQ] = true;
        ui->Dcm2DB->setText("运行中!");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[DCM2DBQ]!=NULL)
    {
        m_pQProcess[DCM2DBQ]->close();
        delete m_pQProcess[DCM2DBQ];
        m_pQProcess[DCM2DBQ] = NULL;
        m_bstorescp[DCM2DBQ] = false;
        ui->Dcm2DB->setText("启动");
        //QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
    }
}

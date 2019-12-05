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
}
void HMainWindow::on_StoreSCP_clicked()
{
    QString program = m_ExeDir + m_StoreScpName;
    QStringList arg;
    QString port = ui->port_store->text();
    arg.append(port);
    arg.append(ui->Dir_Store->text());
    if (!m_bstorescp[STORESCPQ] && m_pQProcess[STORESCPQ]==NULL)
    {
        m_pQProcess[STORESCPQ] =  new QProcess(this);
        m_pQProcess[STORESCPQ]->start(program,arg);
        m_bstorescp[STORESCPQ] = true;
        ui->StoreSCP->setText("运行中!");
        m_bstorescp[STORESCPQ] = true;
        QMessageBox::information(this, tr("DicomStore Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[STORESCPQ]!=NULL)
    {
        m_pQProcess[STORESCPQ]->close();
        delete m_pQProcess[STORESCPQ];
        m_pQProcess[STORESCPQ] = NULL;
        m_bstorescp[STORESCPQ] = false;
        ui->StoreSCP->setText("启动");
        QMessageBox::information(this, tr("DicomStore Stop!"), tr("close app ok!"));
    }
}

void HMainWindow::on_WLMSCP_clicked()
{

}

void HMainWindow::on_QRSCP_clicked()
{

}

void HMainWindow::on_Dcm2DB_clicked()
{
    QString program = m_ExeDir + m_Dcm2DBName;
    QStringList arg;
    arg.append(ui->Dir_Store->text());
    if (!m_bstorescp[DCM2DBQ] && m_pQProcess[DCM2DBQ]==NULL)
    {
        m_pQProcess[DCM2DBQ] =  new QProcess(this);
        m_pQProcess[DCM2DBQ]->start(program,arg);
        m_bstorescp[DCM2DBQ] = true;
        ui->Dcm2DB->setText("运行中!");
        QMessageBox::information(this, tr("Dcm2DBNameApp Start!"), tr("run ok!"));
    }
    else if( m_pQProcess[DCM2DBQ]!=NULL)
    {
        m_pQProcess[DCM2DBQ]->close();
        delete m_pQProcess[DCM2DBQ];
        m_pQProcess[DCM2DBQ] = NULL;
        m_bstorescp[DCM2DBQ] = false;
        ui->Dcm2DB->setText("启动");
        QMessageBox::information(this, tr("Dcm2DBNameApp Stop!"), tr("close app ok!"));
    }
}

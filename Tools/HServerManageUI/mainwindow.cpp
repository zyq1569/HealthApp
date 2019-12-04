#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"

HMainWindow::HMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HMainWindow)
{
    ui->setupUi(this);
    ExeDir = QDir::currentPath();
    ExeDir = ExeDir.remove("/debug");
    ExeDir = ExeDir.remove("/ServerUI");
    ExeDir = ExeDir+"/win32/";
    StoreScpName = "StoreDcmSCP.exe";
    QuerRScpName = "dcmqrScp.exe";
    WLMScpName = "WorklistScp.exe";
}

HMainWindow::~HMainWindow()
{
    delete ui;
}
void HMainWindow::on_StoreSCP_clicked()
{
    QString program = "D:\\code\\C++\\HealthApp\\bin\\win32\\StoreDcmSCP.exe";
    QStringList arg;
    QString port = ui->port_store->text();
    arg.append(port);
    arg.append("D:\\code\\C++\\HealthApp\\Tools\\HServerManageUI\\bin");
    if (!h_bstorescp && h_qProcess==NULL)
    {
        h_qProcess =  new QProcess(this);
        h_qProcess->start(program,arg);
        h_bstorescp = true;
        ui->StoreSCP->setText("运行中！");
        QMessageBox::information(this, tr("Current Dir."), tr("run ok!"));
    }
    else
    {
        h_qProcess->close();
        delete h_qProcess;
        h_qProcess = NULL;
        h_bstorescp = false;
        QMessageBox::information(this, tr("Current Dir."), tr("close app ok!"));
        ui->StoreSCP->setText("启动");
    }
}

void HMainWindow::on_WLMSCP_clicked()
{

}

void HMainWindow::on_QRSCP_clicked()
{

}

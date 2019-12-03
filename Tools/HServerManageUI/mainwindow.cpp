#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"

HMainWindow::HMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HMainWindow)
{
    ui->setupUi(this);
    h_bstorescp = false;
    h_qProcess = NULL;
}

HMainWindow::~HMainWindow()
{
    delete ui;
}

void HMainWindow::on_runStoreSCP_clicked()
{
    QString Qdir = QDir::currentPath();
    QString program = "D:\\code\\C++\\HealthApp\\bin\\win32\\StoreDcmSCP.exe";
    QStringList arg;
    arg.append("1024");
    arg.append("D:\\code\\C++\\HealthApp\\Tools\\HServerManageUI\\bin");
    if (!h_bstorescp && h_qProcess==NULL)
    {
        h_qProcess =  new QProcess(this);
        h_qProcess->start(program,arg);
        h_bstorescp = true;
        //ui->textEdit(on_runStoreSCP_clicked())

        //setText("运行中！")；

        QMessageBox::information(this, tr("Current Dir."), tr("run ok!"));
    }
    else
    {
        h_qProcess->close();
        delete h_qProcess;
        h_qProcess = NULL;
        h_bstorescp = false;
        QMessageBox::information(this, tr("Current Dir."), tr("close app ok!"));
    }


}

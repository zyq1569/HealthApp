#include "config.h"
#include "ui_config.h"

#include <QDir>
Config::Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Config)
{
    ui->setupUi(this);

    connect(ui->m_saveBt,SIGNAL(clicked()),this,SLOT(saveServerconfig()));

    ui->m_imageViewer->setTristate(false);
    QString dir = QDir::currentPath();
    int  i = dir.lastIndexOf("/");
    QString viewerdir =  dir.left(i-1);
    dir = viewerdir + "/Starviewer/starviewer.exe";
    ui->m_viewerDir->setText(dir);
}

void Config::setConfig(QString serverip, QString serverport, int viewer)
{
    ui->m_serverIP->setText(serverip);
    ui->m_serverPort->setText(serverport);
    ui->m_imageViewer->setCheckState((Qt::CheckState)viewer);
    if (viewer > 1)
    {
        ui->m_viewerDir->setEnabled(false);
    }
    else
    {
        ui->m_viewerDir->setEnabled(true);
    }
}


void Config::saveServerconfig()
{
    int state = ui->m_imageViewer->checkState();
    if (ui->m_serverIP->text().length() > 7 && ui->m_serverPort->text() > 1)
    {
        emit saveConfig(ui->m_serverIP->text(),ui->m_serverPort->text(),state);
    }
    if (state > 1)
    {
        ui->m_viewerDir->setEnabled(false);
    }
    else
    {
        ui->m_viewerDir->setEnabled(true);
    }
}

Config::~Config()
{
    delete ui;
}

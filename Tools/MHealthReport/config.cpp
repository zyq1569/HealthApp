#include "config.h"
#include "ui_config.h"

#include <QDir>
Config::Config(QWidget *parent) :QWidget(parent),ui(new Ui::Config)
{
    ui->setupUi(this);

    connect(ui->m_saveBt,SIGNAL(clicked()),this,SLOT(saveServerconfig()));

    ui->m_imageViewer->setTristate(false);
    QString dir = QDir::currentPath();
    int  i = dir.lastIndexOf("/");
    QString otherdir =  dir.left(i);
    dir = otherdir + "/Starviewer/starviewer.exe";
    ui->m_viewerDir->setText(dir);
    ui->m_viewerDir->setPlaceholderText(dir);

    ui->m_reportViewer->setTristate(false);
    dir = otherdir + "/OpenWord/word.exe";
    ui->m_reportrDir->setText(dir);

    ui->m_reportrDir->setPlaceholderText(dir);
}

void Config::setConfig(QString serverip, QString serverport, int viewer, int report)
{
    ui->m_serverIP->setText(serverip);
    ui->m_serverPort->setText(serverport);
    ui->m_imageViewer->setCheckState((Qt::CheckState)viewer);
    if (viewer > 1)
    {
        ui->m_viewerDir->setEnabled(true);
    }
    else
    {
        ui->m_viewerDir->setEnabled(false);
    }

    ui->m_reportViewer->setCheckState((Qt::CheckState)report);
    if (report > 1)
    {
        ui->m_reportrDir->setEnabled(true);
    }
    else
    {
        ui->m_reportrDir->setEnabled(false);
    }
}


void Config::saveServerconfig()
{
    int state = ui->m_imageViewer->checkState();
    if (ui->m_serverIP->text().length() > 7 && ui->m_serverPort->text() > 1)
    {
        emit saveConfig(ui->m_serverIP->text(), ui->m_serverPort->text(), state, ui->m_reportViewer->checkState());
    }
    if (state > 1)
    {
        ui->m_viewerDir->setEnabled(true);
    }
    else
    {
        ui->m_viewerDir->setEnabled(false);
    }

    state = ui->m_reportViewer->checkState();
    if (state > 1)
    {
        ui->m_reportrDir->setEnabled(true);
    }
    else
    {
        ui->m_reportrDir->setEnabled(false);
    }
}

Config::~Config()
{
    delete ui;
}

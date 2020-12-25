#include "config.h"
#include "ui_config.h"

Config::Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Config)
{
    ui->setupUi(this);

    connect(ui->m_saveBt,SIGNAL(clicked()),this,SLOT(saveServerconfig()));

    ui->m_imageViewer->setTristate(false);
}

void Config::setConfig(QString serverip, QString serverport, int viewer)
{
    ui->m_serverIP->setText(serverip);
    ui->m_serverPort->setText(serverport);
    ui->m_imageViewer->setCheckState((Qt::CheckState)viewer);
}


void Config::saveServerconfig()
{
    if (ui->m_serverIP->text().length() > 7 && ui->m_serverPort->text() > 1)
    {
       emit saveConfig(ui->m_serverIP->text(),ui->m_serverPort->text(),ui->m_imageViewer->checkState());
    }
}

Config::~Config()
{
    delete ui;
}

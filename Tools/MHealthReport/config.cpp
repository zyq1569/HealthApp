#include "config.h"
#include "ui_config.h"

Config::Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Config)
{
    ui->setupUi(this);

    connect(ui->m_saveBt,SIGNAL(clicked()),this,SLOT(saveServerconfig()));
}

void Config::setConfig(QString serverip, QString serverport)
{
    ui->m_serverIP->setText(serverip);
    ui->m_serverPort->setText(serverport);
}

//void Config::getConfig(QString &serverip, QString &serverport)
//{
//    serverip = ui->m_serverIP->text();
//    serverport = ui->m_serverPort->text();
//}

void Config::saveServerconfig()
{
    if (ui->m_serverIP->text().length() > 7 && ui->m_serverPort->text() > 1)
    {
        emit saveConfig(ui->m_serverIP->text(),ui->m_serverPort->text());
    }
}

Config::~Config()
{
    delete ui;
}

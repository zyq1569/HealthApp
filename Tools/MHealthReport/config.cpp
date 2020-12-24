#include "config.h"
#include "ui_config.h"

Config::Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Config)
{
    ui->setupUi(this);
}

void Config::setConfig(QString serverip, QString serverport)
{
    ui->m_serverIP->setText(serverip);
    ui->m_serverPort->setText(serverport);
}

void Config::getConfig(QString &serverip, QString &serverport)
{
    serverip = ui->m_serverIP->text();
    serverport = ui->m_serverPort->text();
}

Config::~Config()
{
    delete ui;
}

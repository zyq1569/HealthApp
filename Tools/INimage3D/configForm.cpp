#include "ConfigForm.h"
#include "ui_ConfigForm.h"

#include "mainwindow.h"


#include <QSettings>

ConfigForm::ConfigForm(QWidget *parent) :QWidget(parent), ui(new Ui::ConfigForm)
{
	ui->setupUi(this);
	setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	setWindowFlags(Qt::Dialog);
	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_ShowModal);



	QSettings settings("config.ini", QSettings::IniFormat);

	ui->m_ck3D->setChecked(settings.value("start/start3D").toBool());
	ui->m_ck4Plane->setChecked(settings.value("start/start4P").toBool());
	ui->m_ckColor->setChecked(settings.value("start/3Dcolor").toBool());

	ui->m_ckWL->setChecked(settings.value("wl/check").toBool());
	ui->m_window->setPlainText(settings.value("wl/win").toString());
	ui->m_level->setPlainText(settings.value("wl/lel").toString());

	m_mainwindow = (MainWindow*)parent;


	connect(ui->m_pbSave, &QPushButton::clicked, [this]
	{
		InitConfig();
	});

	ui->m_ck3D->hide();
	ui->m_ck4Plane->hide();
	ui->m_lable->hide();
	
}

ConfigForm::~ConfigForm()
{
	QSettings settings("config.ini", QSettings::IniFormat);
	settings.setValue("start/start3D", ui->m_ck3D->isChecked());
	settings.setValue("start/start4P", ui->m_ck4Plane->isChecked());
	settings.setValue("start/3Dcolor", ui->m_ckColor->isChecked());

	settings.setValue("wl/check", ui->m_ckWL->isChecked());
	settings.setValue("wl/win", ui->m_window->toPlainText().toInt());
	settings.setValue("wl/lel", ui->m_level->toPlainText().toInt());


	settings.sync();

    delete ui;
}

void ConfigForm::InitConfig()
{
	//m_checkStart3D  =  m_checkStart4Plane = m_check3Dcolor = m_checkDefaultWL = false;
	m_mainwindow->m_checkStart3D     = ui->m_ck3D->isChecked();
	m_mainwindow->m_checkStart4Plane = ui->m_ck4Plane->isChecked();
	m_mainwindow->m_check3Dcolor     = ui->m_ckColor->isChecked();
	m_mainwindow->m_checkDefaultWL   = ui->m_ckWL->isChecked();
	m_mainwindow->m_DefaultWindow    = ui->m_window->toPlainText().toInt();
	m_mainwindow->m_DefaultLevel     = ui->m_level->toPlainText().toInt();
}

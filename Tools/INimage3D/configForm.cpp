#include "ConfigForm.h"
#include "ui_ConfigForm.h"

#include "mainwindow.h"


#include <QSettings>
#include <QColorDialog>
#pragma execution_character_set("utf-8")
ConfigForm::ConfigForm(QWidget *parent) :QWidget(parent), ui(new Ui::ConfigForm)
{
	ui->setupUi(this);
	setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	setWindowFlags(Qt::Dialog);
	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_ShowModal);



	QSettings settings("config.ini", QSettings::IniFormat);

	ui->m_ckColor->setChecked(settings.value("start/3Dcolor", false).toBool());
	ui->m_ckOpacity->setChecked(settings.value("start/3Dopacity",false).toBool());
	ui->m_cbInterType->setCurrentIndex(settings.value("start/3DInterType", 0).toInt());

	ui->m_ckWL->setChecked(settings.value("wl/check").toBool());
	ui->m_window->setPlainText(settings.value("wl/win").toString());
	ui->m_SampleDistanceGroup->setChecked(settings.value("start/3DsampleDistanceCheck", false).toBool());
    ui->m_SampleDistance->setValue(settings.value("start/3DsampleDistance").toDouble());

	m_mainwindow = (MainWindow*)parent;


	connect(ui->m_pbSave, &QPushButton::clicked, [this]
	{
		InitConfig();
	});	

    connect(ui->m_pbSaveExit, &QPushButton::clicked, [this]
    {
        InitConfig();
        hide();
    });
}

ConfigForm::~ConfigForm()
{
	QSettings settings("config.ini", QSettings::IniFormat);
	settings.setValue("start/3Dcolor", ui->m_ckColor->isChecked());
	settings.setValue("start/3Dopacity", ui->m_ckOpacity->isChecked()); 
	settings.setValue("start/3DInterType", ui->m_cbInterType->currentIndex());
    settings.setValue("start/3DsampleDistanceCheck", ui->m_SampleDistanceGroup->isChecked());
    settings.setValue("start/3DsampleDistance", ui->m_SampleDistance->value());

	settings.setValue("wl/check", ui->m_ckWL->isChecked());
	settings.setValue("wl/win", ui->m_window->toPlainText().toInt());
	settings.setValue("wl/lel", ui->m_level->toPlainText().toInt());


	settings.sync();

    delete ui;
}

void ConfigForm::InitConfig()
{
	//m_checkStart3D  =  m_checkStart4Plane = m_check3Dcolor = m_checkDefaultWL = false;
	m_mainwindow->m_check3Dcolor        = ui->m_ckColor->isChecked();
	m_mainwindow->m_checkDefaultWL      = ui->m_ckWL->isChecked();
	m_mainwindow->m_DefaultWindow       = ui->m_window->toPlainText().toInt();
	m_mainwindow->m_DefaultLevel        = ui->m_level->toPlainText().toInt();
	m_mainwindow->m_checkOpacity        = ui->m_ckOpacity->isChecked();
	m_mainwindow->m_cbInterType         = ui->m_cbInterType->currentIndex();
    m_mainwindow->m_colorGradient       = ui->m_colorGradientCK->isChecked();
    m_mainwindow->m_sampleDistanceCheck = ui->m_SampleDistanceGroup->isChecked();
    m_mainwindow->m_sampleDistance      = ui->m_SampleDistance->value();
	//QColor color;
	//bool ok;
	//color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &ok, this));

}

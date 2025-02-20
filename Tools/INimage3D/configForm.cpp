#include "ConfigForm.h"
#include "ui_ConfigForm.h"

#include "mainwindow.h"


ConfigForm::ConfigForm(QWidget *parent) :
	QWidget(parent),
    ui(new Ui::ConfigForm)
{
	ui->setupUi(this);
	//setWindowFlags(Qt::WindowCloseButtonHint| Qt::FramelessWindowHint | Qt::Dialog);
	setWindowFlags(Qt::Dialog);
	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_ShowModal);
}

ConfigForm::~ConfigForm()
{
    delete ui;
}

void ConfigForm::InitConfig(MainWindow* mainwindow)
{
	//m_checkStart3D  =  m_checkStart4Plane = m_check3Dcolor = m_checkDefaultWL = false;
	mainwindow->m_checkStart3D     = ui->m_ck3D->isChecked();
	mainwindow->m_checkStart4Plane = ui->m_ck4Plane->isChecked();
	mainwindow->m_check3Dcolor     = ui->m_ckColor->isChecked();
	mainwindow->m_checkDefaultWL   = ui->m_ckWL->isChecked();
	mainwindow->m_DefaultWindow = ui->m_window->toPlainText().toInt();
	mainwindow->m_DefaultLevel  = ui->m_level->toPlainText().toInt();
}

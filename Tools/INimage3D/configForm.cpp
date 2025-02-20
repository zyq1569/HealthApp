#include "ConfigForm.h"
#include "ui_ConfigForm.h"

ConfigForm::ConfigForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigForm)
{
    ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	setWindowModality(Qt::WindowModal);
}

ConfigForm::~ConfigForm()
{
    delete ui;
}

#include "patientsform.h"
#include "ui_patientsform.h"

PatientsForm::PatientsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientsForm)
{
    ui->setupUi(this);
}

PatientsForm::~PatientsForm()
{
    delete ui;
}

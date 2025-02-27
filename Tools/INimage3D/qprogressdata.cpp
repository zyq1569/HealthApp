#include "qprogressdata.h"
#include "ui_qprogressdata.h"

QProgressData::QProgressData(QWidget *parent) :   QWidget(parent),   ui(new Ui::QProgressData)
{
    ui->setupUi(this);


	setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	setWindowFlags(Qt::Dialog);
	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_ShowModal);
}

QProgressData::~QProgressData()
{
    delete ui;
}

#include "qfourpaneviewer.h"
#include "ui_qfourpaneviewer.h"

QFourpaneviewer::QFourpaneviewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QFourpaneviewer)
{
    ui->setupUi(this);
}

QFourpaneviewer::~QFourpaneviewer()
{
    delete ui;
}

#include "q3dviewer.h"
#include "ui_q3dviewer.h"

Q3dviewer::Q3dviewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Q3dviewer)
{
    ui->setupUi(this);
}

Q3dviewer::~Q3dviewer()
{
    delete ui;
}

#include "volumeparams.h"
#include "ui_volumeparams.h"

VolumeParams::VolumeParams(QWidget *parent) :  QWidget(parent), ui(new Ui::VolumeParams)
{
    ui->setupUi(this);
    QString str1 = "體數據1";
    QString str2 = "體數據2";
    QString str3 = "體數據3";
    QString str4 = "體數據4";
    QString str5 = "體數據5";
    QString str6 = "體數據6";
    ui->listWidget->addItems(QStringList() << str1 << str2 << str3 << str4 << str5 << str6);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &VolumeParams::SlotSelectRow);
}

VolumeParams::~VolumeParams()
{
    delete ui;
}


void VolumeParams::SlotSelectRow()
{

}
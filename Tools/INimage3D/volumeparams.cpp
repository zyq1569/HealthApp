#include "volumeparams.h"
#include "ui_volumeparams.h"

#include "showbounds3d.h"

VolumeParams::VolumeParams(QWidget *parent) :  QWidget(parent), ui(new Ui::VolumeParams)
{
    ui->setupUi(this);
    //QString str1 = "Volume1";
    //QString str2 = "Volume2";
    //QString str3 = "Volume3";
    //QString str4 = "Volume4";
    //QString str5 = "Volume5";
    //QString str6 = "Volume6";
    //ui->listWidget->addItems(QStringList() << str1 << str2 << str3 << str4 << str5 << str6);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &VolumeParams::SlotSelectRow);
}

VolumeParams::~VolumeParams()
{
    delete ui;
}


void VolumeParams::SlotSelectRow()
{

}

void VolumeParams::UpdateUI(ShowBounds3D *bounds3D, const std::vector<Bounds3D>&result)
{
    if (bounds3D)
    {
        m_Bound3D = bounds3D;
        size_t size = result.size();
        QStringList list;
        for (int i = 0; i < size; i++)
        {
            QString str = "Volume" + QString::number(i);
            list << str;

            const auto& b = result[i];
            //if (b[0] < 0 || b[1] >= dims[0] || b[2] < 0 || b[3] >= dims[1] || b[4] < 0 || b[5] >= dims[2] || b[0] > b[1] || b[2] > b[3] || b[4] > b[5])
        }

        ui->listWidget->addItems(list);

        //connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &VolumeParams::SlotSelectRow);
    }
}
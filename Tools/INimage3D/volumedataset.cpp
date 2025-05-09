#include "volumedataset.h"
#include "ui_volumedataset.h"

VolumeDataSet::VolumeDataSet(QWidget *parent) : QWidget(parent), ui(new Ui::VolumeDataSet)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowCloseButtonHint);//
    setAttribute(Qt::WA_QuitOnClose, false);

    connect(ui->m_pbSave, &QPushButton::pressed, this, &VolumeDataSet::SaveSplitParm);
    connect(ui->m_pbHide, &QPushButton::pressed, this, [this]
    {
        hide();
    });
    //
    connect(ui->m_pbSaveRect, &QPushButton::released, this, &VolumeDataSet::SaveRectParm);
    connect(ui->m_pbObliquerSaveRect, &QPushButton::released, this, &VolumeDataSet::SaveObliquerRectParm);


    ui->m_topS->setMinimum(0);
    ui->m_topE->setMinimum(0);
    ui->m_centerS->setMinimum(0);
    ui->m_centerE->setMinimum(0);
    ui->m_bottomS->setMinimum(0);
    ui->m_bottomE->setMinimum(0);
}

VolumeDataSet::~VolumeDataSet()
{
    delete ui;
}

void VolumeDataSet::SetSlicesNumber(int *dim, int *extent1, int *extent2)
{
    QString str = "slices:";
    str += QString::number(dim[0]) + "|"+ QString::number(dim[1]) + "|" + QString::number(dim[2]);
    str += "(" + QString::number(extent1[1]+1) + "*" + QString::number(extent1[3]+1) + ")";
    ui->m_AllNumber->setText(str);
    //ui->m_AllNumber->setText(QString::number(m_slicesNumber));

    m_slicesNumber = dim[2] - 1;
    ui->m_topS->setMaximum(m_slicesNumber);
    ui->m_topE->setMaximum(m_slicesNumber);
    ui->m_centerS->setMaximum(m_slicesNumber);
    ui->m_centerE->setMaximum(m_slicesNumber);
    ui->m_bottomS->setMaximum(m_slicesNumber);
    ui->m_bottomE->setMaximum(m_slicesNumber);

    int delta = m_slicesNumber / 3;

    m_topStart    = 0;
    m_topEnd      = delta;
    m_centerStart = delta+1;
    m_centerEnd   = 2 * delta;
    m_bottomStart = 2 * delta + 1;
    m_bottomEnd   = m_slicesNumber;

    ui->m_topS->setValue(m_topStart);
    ui->m_topE->setValue(m_topEnd);
    ui->m_centerS->setValue(m_centerStart);
    ui->m_centerE->setValue(m_centerEnd);
    ui->m_bottomS->setValue(m_bottomStart);
    ui->m_bottomE->setValue(m_bottomEnd);
    m_dims = dim;
}

void VolumeDataSet::SaveRectParm()
{
    int orientation = ui->m_cbSelectSlice->currentIndex();
    int deltaX = ui->m_deltaX->value(), deltaY = ui->m_deltaY->value();
    int w = ui->m_rectW->value(), h = ui->m_rectH->value();

    emit RectData(orientation, deltaX, deltaY, w, h);
}
void VolumeDataSet::SaveObliquerRectParm()
{
    int orientation = ui->m_cbSelectObliquerSlice->currentIndex();
    int deltaX = ui->m_deltaObliquerX->value(), deltaY = ui->m_deltaObliquerY->value();
    int w = ui->m_rectObliquerW->value(), h = ui->m_rectObliquerH->value();
    int angle = ui->m_angleOblique->value();
    emit ObliquerRectParm(orientation, deltaX, deltaY, w, h);
}

void VolumeDataSet::SaveSplitParm()
{
    int start, end, index = ui->m_cbSelect->currentIndex();

    m_topStart    = ui->m_topS->value();
    m_topEnd      = ui->m_topE->value();
    m_centerStart = ui->m_centerS->value();
    m_centerEnd   = ui->m_centerE->value();
    m_bottomStart = ui->m_bottomS->value();
    m_bottomEnd   = ui->m_bottomE->value();

    if (index == 0)
    {
        start = 0;
        end   = m_slicesNumber;
    }
    else if (index == 1)
    {
        start = m_topStart;
        end   = m_topEnd;
    }
    else if (index == 2)
    {
        start = m_centerStart;
        end   = m_centerEnd;
    }
    else if (index == 3)
    {
        start = m_bottomStart;
        end   = m_bottomEnd;
    }
    emit SplitImageData(m_dims, start, end);
}


#include "volumedataset.h"
#include "ui_volumedataset.h"

VolumeDataSet::VolumeDataSet(QWidget *parent) : QWidget(parent), ui(new Ui::VolumeDataSet)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);//& ~Qt::WindowCloseButtonHint

    connect(ui->m_pbSave, &QPushButton::pressed, this, &VolumeDataSet::SaveSplitParm);
}

VolumeDataSet::~VolumeDataSet()
{
    delete ui;
}

void VolumeDataSet::SetSlicesNumber(int *dim)
{
    m_slicesNumber = dim[2];
    ui->m_AllNumber->setText(QString::number(m_slicesNumber));
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
}

void VolumeDataSet::SaveSplitParm()
{
    int start, end, index = ui->m_cbSelect->currentIndex();
    if (index == 0)
    {

    }
    else if (index == 1)
    {

    }
    else if (index == 2)
    {

    }
    emit SplitImageData(m_dims, start, end);
}


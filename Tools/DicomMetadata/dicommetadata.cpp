#include "dicommetadata.h"
#include "ui_dicommetadata.h"
#include "sqtree.h"
DicomMetadata::DicomMetadata(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DicomMetadata)
{
    ui->setupUi(this);

    m_dicomInfo = new SQtree(true);
    if (m_dicomInfo)
    {
        m_vboxlayout = new QVBoxLayout(ui->metadata_frame);
        centralWidget()->setLayout(m_vboxlayout);
        if (m_vboxlayout)
        {
            m_vboxlayout->setContentsMargins(0, 0, 0, 0);
            m_vboxlayout->setSpacing(0);
            m_vboxlayout->addWidget(m_dicomInfo);
        }
    }
}

DicomMetadata::~DicomMetadata()
{
    if (m_dicomInfo)
    {
        delete m_dicomInfo;
        m_dicomInfo = nullptr;
    }
    if (m_vboxlayout)
    {
        delete m_vboxlayout;
        m_vboxlayout = nullptr;
    }

    delete ui;


}


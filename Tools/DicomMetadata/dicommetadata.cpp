#include "dicommetadata.h"
#include "ui_dicommetadata.h"

DicomMetadata::DicomMetadata(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DicomMetadata)
{
    ui->setupUi(this);
}

DicomMetadata::~DicomMetadata()
{
    delete ui;
}


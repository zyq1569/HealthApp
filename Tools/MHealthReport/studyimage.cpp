#include "studyimage.h"
#include "ui_studyimage.h"

StudyImage::StudyImage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StudyImage)
{
    ui->setupUi(this);
}

StudyImage::~StudyImage()
{
    delete ui;
}

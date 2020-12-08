#include "mainapp.h"
#include "ui_mainapp.h"

#include "studyimage.h"
#include "patientstudyregister.h"
#include <QProcess>

#define SAFEDELETE(pointer) \
{                           \
    if(pointer)             \
{                       \
    delete pointer;     \
    }                       \
    pointer = NULL;         \
    }


/// UI https://www.cnblogs.com/swarmbees/p/11160996.html#--%E9%81%AE%E7%BD%A9%E6%8E%A7%E4%BB%B6
MainApp::MainApp(QWidget *parent): QMainWindow(parent), ui(new Ui::MainApp)
{
    ui->setupUi(this);

    m_QProcess = new QProcess(parent);

    //start exe
    QString appPath = "F:/temp/HealthApp/Tools/Test/TestHttpClient/release/TestHttpClient.exe";//ui->m_AppDir->text();
    //m_QProcess->start(appPath);
    this->setCentralWidget(ui->m_tabWidgetTotal);

    m_PatientStudyRegister     = new PatientStudyRegister(this);
    ui->m_tabWidgetTotal->addTab(m_PatientStudyRegister,    "登记检查");
    ui->m_tabWidgetTotal->setCurrentIndex(1);

    m_StudyImage     = new StudyImage(this);
    ui->m_tabWidgetTotal->addTab(m_StudyImage,    "检查列表");
    ui->m_tabWidgetTotal->setCurrentIndex(2);


    //connect(m_StudyImage, SIGNAL(sendNumber(QString, QString)), this, SLOT(receiveNumber(QString, QString)));
}

MainApp::~MainApp()
{
    //    if (m_StudyImage)
    //    {
    //        delete  m_StudyImage;
    //        m_StudyImage = NULL;
    //    }
    SAFEDELETE(m_StudyImage)

            delete ui;
}



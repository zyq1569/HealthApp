#include "mainapp.h"
#include "ui_mainapp.h"

#include "studyimage.h"
#include "patientstudyregister.h"
#include <QProcess>
#include <QWebEngineView>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>

#define SAFEDELETE(pointer) \
{                           \
    if(pointer)             \
{                       \
    delete pointer;     \
    }                       \
    pointer = NULL;         \
    }

/// Qt + vc 中文编译问题  https://blog.csdn.net/libaineu2004/article/details/19245205
/// UI https://www.cnblogs.com/swarmbees/p/11160996.html#--%E9%81%AE%E7%BD%A9%E6%8E%A7%E4%BB%B6
MainApp::MainApp(QWidget *parent): QMainWindow(parent), ui(new Ui::MainApp)
{
    ui->setupUi(this);

    m_QProcess = new QProcess(parent);

    //start test  exe
    //QString appPath = "F:/temp/HealthApp/Tools/Test/TestHttpClient/release/TestHttpClient.exe";//ui->m_AppDir->text();
    //m_QProcess->start(appPath);
    this->setCentralWidget(ui->m_tabWidgetTotal);

    m_PatientStudyRegister     = new PatientStudyRegister(this);
    ui->m_tabWidgetTotal->addTab(m_PatientStudyRegister,    "登记检查");
    //ui->m_tabWidgetTotal->setCurrentIndex(0);

    m_StudyImage     = new StudyImage(this);
    ui->m_tabWidgetTotal->addTab(m_StudyImage,    "检查列表");
    ui->m_tabWidgetTotal->setCurrentIndex(1);

    m_view = new QWebEngineView(this);
    m_view->setUrl(QUrl("http://127.0.0.1:8080/login/TestReport.html"));
    ui->m_tabWidgetTotal->addTab(m_view, "Report");
    //m_view->show();
    //ui->m_tabWidgetTotal->setCurrentIndex(2);
    connect(ui->m_tabWidgetTotal,SIGNAL(tabBarClicked(int)),this,SLOT(TabBarClicked(int)));
    //connect(m_StudyImage, SIGNAL(sendNumber(QString, QString)), this, SLOT(receiveNumber(QString, QString)));
    connect(m_view->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded,this,&MainApp::slog_cookieAdded);
}

void MainApp::slog_cookieAdded(const QNetworkCookie &cookie)
{
    qDebug()<<"Cookie Added-->"<< cookie.domain()<<cookie.name()<<cookie.value()<< endl;
}

void MainApp::TabBarClicked(int index)
{
    if (index == 2)
    {
        //m_view->setUrl(QUrl("http://127.0.0.1:8080/login/TestReport.html"));
        m_view->show();
    }
    else
    {
        m_view->hide();
    }
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



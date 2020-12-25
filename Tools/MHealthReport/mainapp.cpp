#include "mainapp.h"
#include "ui_mainapp.h"

#include "studyimage.h"
#include "patientstudyregister.h"

#include "config.h"

#include <QProcess>
#include <QWebEngineView>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QNetworkProxyFactory>
#include <QMessageBox>
#include <QSettings>

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

    m_serverIP = "127.0.0.1";
    m_serverPort = "8080";

    //start test  exe
    //QString appPath = "F:/temp/HealthApp/Tools/Test/TestHttpClient/release/TestHttpClient.exe";//ui->m_AppDir->text();
    //m_QProcess->start(appPath);
    this->setCentralWidget(ui->m_tabWidgetTotal);

    ///PatientStudyRegister
    m_PatientStudyRegister     = new PatientStudyRegister(this);
    ui->m_tabWidgetTotal->addTab(m_PatientStudyRegister,    "登记检查");
    //ui->m_tabWidgetTotal->setCurrentIndex(0);

    ///StudyImage
    m_StudyImage     = new StudyImage(this);
    ui->m_tabWidgetTotal->addTab(m_StudyImage,    "检查列表");
    ui->m_tabWidgetTotal->setCurrentIndex(1);
    connect(m_StudyImage,SIGNAL(lookReport(QString)),this,SLOT(lookStudyReport(QString)));

    ///QWebEngineView
    m_url = "http://"+m_serverIP+":"+m_serverPort;
    m_view = new QWebEngineView(this);
    QNetworkProxyFactory::setUseSystemConfiguration(false);//off SystemConfiguration
    m_view->setUrl(QUrl(m_url+"/login/test/testReport.html#Temple"));
    ui->m_tabWidgetTotal->addTab(m_view, "检查报告");
    //m_view->show();
    //ui->m_tabWidgetTotal->setCurrentIndex(2);

    ///根据配置来设置
    m_imageView = new QWebEngineView(this);
    QNetworkProxyFactory::setUseSystemConfiguration(false);//off SystemConfiguration
    //m_imageView->setUrl(QUrl(m_url+"/view/view.html?1.2.826.0.1.3680043.9.7606.48.1214245415.1267414711.906286"));
    ui->m_tabWidgetTotal->addTab(m_imageView, "图像浏览");
    ui->m_tabWidgetTotal->removeTab( ui->m_tabWidgetTotal->indexOf(m_imageView));

    ///Config
    m_config          = new Config(this);
    ui->m_tabWidgetTotal->addTab(m_config,"维护配置");
    //ui->m_tabWidgetTotal->setCurrentIndex(2);
    connect(m_config,SIGNAL(saveConfig(QString,QString)),this,SLOT(saveServerConfig(QString,QString)));

    ///----------------------------------------------------------------------------------------------------------------------

    connect(ui->m_tabWidgetTotal,SIGNAL(tabBarClicked(int)),this,SLOT(TabBarClicked(int)));
    //connect(m_StudyImage, SIGNAL(sendNumber(QString, QString)), this, SLOT(receiveNumber(QString, QString)));
    connect(m_view->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded,this,&MainApp::slog_cookieAdded);
    ///clearHttpCache
    QWebEngineProfile * engineProfile = m_view->page()->profile();
    engineProfile->clearHttpCache();
    engineProfile->clearAllVisitedLinks();
    //    QString cachePath = engineProfile->cachePath();
    //    QDir cachePathDir(cachePath);
    //    if (cachePathDir.exists())
    //    {
    //        bool rlt = cachePathDir.rmdir(cachePath); //删除缓存目录
    //        if (!rlt)
    //        {
    //            qDebug() << "cachePathDir fail!";
    //        }
    //    }

    ///---------------------------------------------------------------------------------
    ///
    QString Dir     = QDir::currentPath();
    QString iniDir = Dir+"/config";
    QDir dir(iniDir);
    if(!dir.exists())
    {
        QDir dir(iniDir); // 注意
        dir.setPath("");
        if (!dir.mkpath(iniDir))
        {
            // error!
        }
    }
    QString configfilename = iniDir+"/MHealthReport.ini";
#if defined(Q_OS_LINUX)
    configfilename = iniDir+"/MHealthReport_linux.ini";
#endif
    QSettings configini(configfilename,QSettings::IniFormat);
    QFileInfo fileInfo(configfilename);
    if(fileInfo.exists())
    {
        m_serverIP   = configini.value("/webserver/server_IP","127.0.0.1").toString();
        m_serverPort = configini.value("/webserver/server_Port","8080").toString();
        m_imageViewer = configini.value("/imageviewer/viewer_state",0).toInt();
        m_config->setConfig(m_serverIP,m_serverPort,m_imageViewer);
    }
}

void MainApp::saveServerConfig(QString serverIP, QString serverPort)
{
    m_serverIP = serverIP;
    m_serverPort = serverPort;
}


void MainApp::lookStudyReport(QString StudyOrderIdentity)
{
    m_url = "http://"+m_serverIP+":"+m_serverPort;
    static bool flag = true;
    if (flag)
    {
        m_view->setUrl(QUrl(m_url+"/login/test/studyReport.html#"+StudyOrderIdentity));
    }
    else
    {
        m_view->setUrl(QUrl(m_url+"/login/test/oderReport.html#"+StudyOrderIdentity));
    }
    m_view->show();
    flag = !flag;
    //QMessageBox::information(NULL, tr("studyReport"),StudyOrderIdentity);
    ui->m_tabWidgetTotal->setCurrentIndex(2);
}

void MainApp::slog_cookieAdded(const QNetworkCookie &cookie)
{
    //qDebug()<<"Cookie Added-->"<< cookie.domain()<<cookie.name()<<cookie.value()<< endl;
}

void MainApp::TabBarClicked(int index)
{
    if (index == 2)
    {
        m_view->show();
    }
    else
    {
        m_view->hide();
    }
}

MainApp::~MainApp()
{
    QString Dir     = QDir::currentPath();
    QString iniDir = Dir+"/config";
    QDir dir(iniDir);
    if(!dir.exists())
    {
        QDir dir(iniDir); // 注意
        dir.setPath("");
        if (!dir.mkpath(iniDir))
        {
            // error!
        }
    }
    QString configfilename = iniDir+"/MHealthReport.ini";
#if defined(Q_OS_LINUX)
    configfilename = iniDir+"/MHealthReport_linux.ini";
#endif
    QSettings configini(configfilename,QSettings::IniFormat);
    //if (isFileExist(configfilename))
    {
        configini.setValue("/webserver/server_IP",m_serverIP);
        configini.setValue("/webserver/server_Port",m_serverPort);
        configini.setValue("/imageviewer/viewer_state",m_imageViewer);
    }
    //    if (m_StudyImage)
    //    {
    //        delete  m_StudyImage;
    //        m_StudyImage = NULL;
    //    }
    SAFEDELETE(m_StudyImage)
            SAFEDELETE(m_PatientStudyRegister)
            SAFEDELETE(m_StudyImage)
            SAFEDELETE(m_view)

            delete ui;
}



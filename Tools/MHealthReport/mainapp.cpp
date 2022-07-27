#include "mainapp.h"
#include "ui_mainapp.h"

#include "studyimage.h"
#include "patientstudyregister.h"
#include "patientdata.h"
#include "config.h"

#include <QProcess>
#include <QWebEngineView>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QNetworkProxyFactory>
#include <QMessageBox>
#include <QSettings>

#define SHAREDHEALTH "SharedHealth"

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

    m_serverIP    =  "127.0.0.1";
    m_serverPort  =  "8080";

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
    connect(m_StudyImage,SIGNAL(lookImage(QString)),this,SLOT(lookStudyImage(QString)));

    ///配置是否使用专业word编辑报告
    ///QWebEngineView
    //m_url = "http://"+m_serverIP+":"+m_serverPort;
    m_reportview = new QWebEngineView(this);
    QNetworkProxyFactory::setUseSystemConfiguration(false);//off SystemConfiguration
    //m_reportview->setUrl(QUrl(m_url+"/login/test/testReport.html#Temple"));
    ui->m_tabWidgetTotal->addTab(m_reportview, "检查报告");
    //m_reportview->show();
    //ui->m_tabWidgetTotal->setCurrentIndex(2);

    ///根据配置来设置
    m_imageView       = new QWebEngineView(this);
    QNetworkProxyFactory::setUseSystemConfiguration(false);//off SystemConfiguration
    //m_imageView->setUrl(QUrl(m_url+"/view/view.html?1.2.826.0.1.3680043.9.7606.48.1214245415.1267414711.906286"));
    ui->m_tabWidgetTotal->addTab(m_imageView, "图像浏览");

    ///Config
    m_config          = new Config(this);
    ui->m_tabWidgetTotal->addTab(m_config,"维护配置");
    //ui->m_tabWidgetTotal->setCurrentIndex(2);
    connect(m_config,SIGNAL(saveConfig(QString,QString,int,int)),this,SLOT(saveServerConfig(QString,QString,int,int)));

    ///----------------------------------------------------------------------------------------------------------------------

    connect(ui->m_tabWidgetTotal,SIGNAL(tabBarClicked(int)),this,SLOT(TabBarClicked(int)));
    //connect(m_StudyImage, SIGNAL(sendNumber(QString, QString)), this, SLOT(receiveNumber(QString, QString)));
    connect(m_reportview->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded,this,&MainApp::slog_cookieAdded);

    ///clearHttpCache
    QWebEngineProfile * engineProfile = m_reportview->page()->profile();
    engineProfile->clearHttpCache();
    engineProfile->clearAllVisitedLinks();
    //QString cachePath = engineProfile->cachePath();
    //QDir cachePathDir(cachePath);
    //if (cachePathDir.exists())
    //{
    //bool rlt = cachePathDir.rmdir(cachePath); //删除缓存目录
    //if (!rlt)
    //{
    //    qDebug() << "cachePathDir fail!";
    //}
    //}
    ///---------------------------------------------------------------------------------
    QString Dir     = QDir::currentPath();
    QString iniDir = Dir+"/config";
    QDir dir(iniDir);
    if(!dir.exists())
    {
        QDir createDir(iniDir); // 注意
        createDir.setPath("");
        if (!createDir.mkpath(iniDir))
        {
            // error!
        }
    }
    QString cacheDir = Dir+"/cache";
    QDir downdir(cacheDir);
    if(!downdir.exists())
    {
        QDir createDir(cacheDir); // 注意
        createDir.setPath("");
        if (!createDir.mkpath(cacheDir))
        {
            // error!
        }
    }
    m_imageViewerEnable  = 0;
    m_reportViewerEnable = 0;
    QString configfilename = iniDir+"/MHealthReport.ini";
#if defined(Q_OS_LINUX)
    configfilename = iniDir+"/MHealthReport_linux.ini";
#endif
    QFile file(configfilename);
    file.open(QIODevice::ReadWrite|QIODevice::Text);
    file.close();
    QSettings configini(configfilename,QSettings::IniFormat);
    QFileInfo fileInfo(configfilename);
    if(fileInfo.exists())
    {
        m_serverIP             = configini.value("/webserver/server_IP","127.0.0.1").toString();
        m_serverPort           = configini.value("/webserver/server_Port","8080").toString();
        m_imageViewerEnable    = configini.value("/imageviewer/viewer_state",0).toInt();
        m_reportViewerEnable   = configini.value("/reportviewer/report_state",0).toInt();
    }
    else
    {
        QMessageBox::information(NULL, configfilename+" fail!","creat file");
    }
    if(fileInfo.exists())
    {
        m_config->setConfig(m_serverIP,m_serverPort,m_imageViewerEnable,m_reportViewerEnable);
    }
    m_url = "http://"+m_serverIP+":"+m_serverPort;
    m_reportview->setUrl(QUrl(m_url+"/login/test/testReport.html#studyTemp"));
    if (m_imageViewerEnable > 1)
    {
        ui->m_tabWidgetTotal->removeTab( ui->m_tabWidgetTotal->indexOf(m_imageView) );
        ///---begin starviewer.exe
        QString currentdir = QDir::currentPath();
        int  pos = currentdir.lastIndexOf("/");
        QString viewerdir =  currentdir.left(pos);
        viewerdir = viewerdir + "/starviewer/starviewer.exe";
        //QMessageBox::information(NULL, tr("viewerdir"),viewerdir);
        QFileInfo fileExe(viewerdir);
        if(fileExe.exists())
        {
            m_viewerProcess = new QProcess(parent);
            m_viewerProcess->start(viewerdir);
        }
        ///----end starviewer.exe
    }
    else
    {
        m_StudyImage->setUrlImage(true);
    }

    if (m_reportViewerEnable > 1)
    {
        ui->m_tabWidgetTotal->removeTab( ui->m_tabWidgetTotal->indexOf(m_reportview) );
        ///---begin HEditor.exe----------------
        QString currentdir = QDir::currentPath();
        int  pos = currentdir.lastIndexOf("/");
        QString wordDir =  currentdir.left(pos);
        wordDir = wordDir + "/openword/HEditor.exe";
        QFileInfo fileExe(wordDir);
        if(fileExe.exists())
        {
            m_wordProcess = new QProcess(parent);
            m_wordProcess->start(wordDir);
        }
        ///----end word.exe--------------------
    }
    else
    {
        m_StudyImage->setUrlReport(true);
    }

    QString HttpUrl = "http://" + m_serverIP + ":" + m_serverPort;
    setServerHttpUrl(HttpUrl);
    setDownDir(cacheDir);
    ///------------------------------------------------------------------------------------
    ///
    ///
    ///D-BUS: register
    /// ------------------------------------------------------------------------------------
    ///
//    int timerID = startTimer(1000 / 33);///ms



    ///启动共享内存
    //m_sharedInfo.open();
}

void MainApp::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
//    switch (event->timerId()-1)
//    {
//    case timer1 :
//      qDebug() << "timer1" << endl;
//      break;
//            break;
//        default:
//            qDebug() << "no  !!"<<endl;
//            break;
//    }
//    int timerID = 0;
//    killTimer(timerID);

}


void MainApp::saveServerConfig(QString serverIP, QString serverPort, int viewer, int report)
{
    m_serverIP           = serverIP;
    m_serverPort         = serverPort;
    m_imageViewerEnable  = viewer;
    m_reportViewerEnable = report;
    QString HttpUrl      = "http://" + m_serverIP + ":" + m_serverPort;
    setServerHttpUrl(HttpUrl);
}

void MainApp::lookStudyImage(QString studyuid)
{
    m_url = "http://"+m_serverIP+":"+m_serverPort;
    static bool flag = true;
    if (flag)
    {
        m_imageView->setUrl(QUrl(m_url+"/view/appview.html?"+studyuid));
    }
    else
    {
        m_imageView->setUrl(QUrl(m_url+"/view/view.html?"+studyuid));
    }
    m_imageView->show();
    flag = !flag;
    //QMessageBox::information(NULL, tr("studyReport"),StudyOrderIdentity);
    ui->m_tabWidgetTotal->setCurrentIndex(3);
}

void MainApp::lookStudyReport(QString StudyOrderIdentity)
{
    if (m_reportViewerEnable > 1)
    {
        //1. 开始通知下载报告文件
        //2.下载完后，通知HEditor.exe 打开对应报告
        ///emit showwordreport;
        /// 启动报告编辑器


        return;
    }

    m_url = "http://"+m_serverIP+":"+m_serverPort;
    static bool flag = true;
    if (flag)
    {
        m_reportview->setUrl(QUrl(m_url+"/login/test/studyReport.html#"+StudyOrderIdentity));
    }
    else
    {
        m_reportview->setUrl(QUrl(m_url+"/login/test/oderReport.html#"+StudyOrderIdentity));
    }
    m_reportview->show();
    flag = !flag;
    //QMessageBox::information(NULL, tr("studyReport"),StudyOrderIdentity);
    /// use openword!
    if (m_reportViewerEnable > 1)
    {
        ///emit showwordreport;
    }
    else
    {
        ui->m_tabWidgetTotal->setCurrentIndex(2);
    }
}

void MainApp::slog_cookieAdded(const QNetworkCookie &cookie)
{
    //qDebug()<<"Cookie Added-->"<< cookie.domain()<<cookie.name()<<cookie.value()<< endl;
}

void MainApp::TabBarClicked(int index)
{
    if (index == 2)
    {
        m_reportview->show();
    }
    else
    {
        m_reportview->hide();
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
            //error!
        }
    }
    QString configfilename = iniDir+"/MHealthReport.ini";
#if defined(Q_OS_LINUX)
    configfilename = iniDir+"/MHealthReport_linux.ini";
#endif
    QSettings configini(configfilename,QSettings::IniFormat);
    QFileInfo fileInfo(configfilename);
    if (fileInfo.exists())
    {
        configini.setValue("/webserver/server_IP",      m_serverIP);
        configini.setValue("/webserver/server_Port",    m_serverPort);
        configini.setValue("/imageviewer/viewer_state", m_imageViewerEnable);
        configini.setValue("/reportviewer/report_state",m_reportViewerEnable);
    }
    //if (m_StudyImage)
    //{
    //    delete  m_StudyImage;
    //    m_StudyImage = NULL;
    //}
    SAFEDELETE(m_StudyImage)
    SAFEDELETE(m_PatientStudyRegister)
    SAFEDELETE(m_StudyImage)
    SAFEDELETE(m_reportview)
    ///需要确认是否是否使用close()？
    //if (m_wordProcess)
    //{
    //    m_wordProcess->close();
    //}
    //m_viewerProcess
    if (m_viewerProcess)
    {
        m_viewerProcess->close();
    }

    //m_sharedInfo.close();
    delete ui;
}



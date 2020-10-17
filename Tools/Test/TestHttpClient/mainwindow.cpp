#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpclient.h"

#include "QNetworkAccessManager"
#include "QNetworkReply"
#include "QTextCodec"
#include "QFile"
#include "QFileInfo"


MainWindow::MainWindow(QWidget *parent):QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->m_progressBar->hide();
    m_httpclient = NULL;

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_m_getHttpData_clicked()
{
    m_httpurl = ui->m_URL->text();///"http://127.0.0.1:8080/login/login.html"
    QString studyuid = ui->m_studyuid->text();
    QString seruid = ui->m_seruid->text();
    QString imageuid = ui->m_imageuid->text();
    if (!m_httpclient)
    {
        m_httpclient = new HttpClient(NULL,"F:\\log\\down");
    }
    m_httpclient->getStudyImageFile(m_httpurl,studyuid,seruid,imageuid);


    return;
}


void MainWindow::onDownloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    ui->m_progressBar->setMaximum(totalBytes); //最大值
    ui->m_progressBar->setValue(bytesRead);  //当前值
}


void MainWindow::on_m_getStudyImages_clicked()
{
    ///http://127.0.0.1:8080/healthsystem/ris/stduyimage/?start=19700101&end=20191230&page=1&limit=10
    QString startDate = ui->m_startDate->text();
    QString endDate = ui->m_endDate->text();
    QString mod = ui->m_StudyModality->currentText();
    QUrl url = ui->m_URL->text();

}


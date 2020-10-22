#include "hthreadobject.h"

#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QWidget>

ProgressDialog::ProgressDialog(const QUrl &url, QWidget *parent):QProgressDialog(parent)
{
    inIt(url);
}

void ProgressDialog::inIt(QUrl url)
{
    setWindowTitle(tr("Downloading http://%1.").arg(url.path()));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setLabelText(tr("Downloading http://%1.").arg(url.path()));
    setMinimum(0);
    setValue(0);
    setMinimumDuration(0);
    setMinimumSize(QSize(400, 75));
}

ProgressDialog::~ProgressDialog()
{

}

void ProgressDialog::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    setMaximum(totalBytes);
    setValue(bytesRead);
    setLabelText(tr("Downloading %1/%2.").arg(bytesRead).arg(totalBytes));
}


///--------------------------------------------------------------
HThreadObject::HThreadObject(QObject *parent) : QObject(parent)
{
    m_file = nullptr;
    m_networkmanager = nullptr;
}


HThreadObject::~HThreadObject()
{

}

void HThreadObject::ReadyRead()
{
    if (m_file && m_networkreply)
    {
        m_file->write(m_networkreply->readAll());
    }
}

#ifndef QT_NO_SSL
void HThreadObject::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors)
    {
        if (!errorString.isEmpty())
        {
            errorString += '\n';
        }
        errorString += error.errorString();
    }
    qDebug() <<errorString;
    m_networkreply->ignoreSslErrors();
}
#endif

void HThreadObject::Finished()
{
    if (m_file)
    {
        if (m_file->isOpen())
        {
            QFileInfo fileinfo;
            fileinfo.setFile(m_file->fileName());
            m_file->close();
            delete  m_file;
            m_file = NULL;
        }
    }
    emit notifyResult(SaveFile_Ok);
    m_networkreply->deleteLater();
    m_networkreply = nullptr;
    startNework();
}

void HThreadObject::setInput(QList<HttpInfo> httpInfo)
{
    m_httpInfo.clear();
    m_httpInfo.append(httpInfo);
}

void HThreadObject::startNework()
{
    int size = m_httpInfo.size();
    if (m_index >= size)
    {
        return;
    }

    QString filename = m_httpInfo[m_index].fullpathfilename;
    m_file = new QFile(filename);
    //qDebug() <<filename;
    if (!m_file->open(QFile::WriteOnly | QIODevice::Truncate))
    {
        m_file->close();
        delete m_file;
        m_file = nullptr;
        notifyResult(OpenFile_Fail);
        return ;
    }
    m_networkreply = m_networkmanager->get(QNetworkRequest(m_httpInfo[m_index].url));
    connect(m_networkreply, &QIODevice::readyRead, this, &HThreadObject::ReadyRead);
    connect(m_networkreply, &QNetworkReply::finished, this, &HThreadObject::Finished);
#ifndef QT_NO_SSL
    connect(m_networkmanager, &QNetworkAccessManager::sslErrors, this, &HThreadObject::sslErrors);
#endif

    m_index++;
}

void HThreadObject::work()
{
    m_index = 0;
    if (!m_networkmanager)
    {
        m_networkmanager = new QNetworkAccessManager(this);
    }
    if (m_networkmanager)
    {
        startNework();
    }
}

///------------------------------------------------
HManageThread::HManageThread()
{
    for (int i=0; i<Max_thread_size; i++)
    {
        worker[i] = new HThreadObject;
        worker[i]->moveToThread(&workerThread[i]);
        connect(&workerThread[i], &QThread::finished, worker[i], &QObject::deleteLater);
        connect(this, &HManageThread::operate, worker[i], &HThreadObject::work);
        connect(worker[i], &HThreadObject::notifyResult, this, &HManageThread::handleResults);
        workerThread[i].start();
    }
}

HManageThread::~HManageThread()
{
    for (int i=0; i<Max_thread_size; i++)
    {
        workerThread[i].quit();
        workerThread[i].wait();
    }
}

void HManageThread::start(QList<HttpInfo> httpInfo)
{
    int size = httpInfo.size();
    m_total = size;
    m_remainder = size;
    m_fileinfo = "Total "+ QString("%1").arg(m_total) + " files save ok!";
    int averg = size/Max_thread_size;
    int mod = size%Max_thread_size;
    int index = 0;
    for (int i=0; i<Max_thread_size; i++)
    {
        number[i] = averg;
        if(mod>0)
        {
            number[i]++;
            mod --;
        }
        QList<HttpInfo> temp;
        int max = averg*(i+1);
        if (max > size)
        {
            max = size;
        }
        for (; number[i]>0&&index<size; number[i]--)
        {
            temp.push_back(httpInfo[index++]);
        }
        worker[i]->setInput(temp);
    }
    QUrl url = httpInfo[0].url;
    QString host = url.host();
    QString  port = ":"+QString("%1").arg(url.port());
    static QProgressDialog progressDialog(NULL);
    static bool setok = false;
    if (!setok)
    {
        setok = true;
        progressDialog.setModal(true);
        progressDialog.setRange(1, 100);
        progressDialog.setValue(100);
        progressDialog.setMinimumDuration(0);
        progressDialog.setWindowTitle("Down Files...");
        //progressDialog.setWindowFlags();
        progressDialog.setMinimumSize(QSize(300, 75));
        //progressDialog.setLabelText("Downloading "+host+port);
        progressDialog.setCancelButton(0);
    }
    progressDialog.setLabelText("Start Downloading...");

    connect(this, &HManageThread::ProgressInfo, &progressDialog, &QProgressDialog::setLabelText);
    //connect(this, &HManageThread::readfiles, &progressDialog, &QProgressDialog::setValue);
    connect(this, &HManageThread::finished, &progressDialog, &ProgressDialog::hide);
    progressDialog.show();
    emit operate();
}


void HManageThread::cancelDownload()
{

}

void HManageThread::handleResults(const int &msg)
{
    m_remainder--;
    if (m_remainder <1)
    {
        emit finished();
    }
    //else
    //{
    //emit readfiles(m_total-m_remainder);
    emit ProgressInfo(tr("Downloading %1 / %2   ...").arg(m_total-m_remainder).arg(m_total));
    //}
}


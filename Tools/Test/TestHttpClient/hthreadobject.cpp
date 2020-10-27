#include "hthreadobject.h"

#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QWidget>

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
    startNetwork();
}

void HThreadObject::setInput(QList<HttpInfo> httpInfo)
{
    m_httpInfo.clear();
    m_httpInfo.append(httpInfo);
}

void HThreadObject::startNetwork()
{
    int size = m_httpInfo.size();
    if (m_taskIndex >= size)
    {
        return;
    }

    QString filename = m_httpInfo[m_taskIndex].fullpathfilename;
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
    m_networkreply = m_networkmanager->get(QNetworkRequest(m_httpInfo[m_taskIndex].url));
    connect(m_networkreply, &QIODevice::readyRead, this, &HThreadObject::ReadyRead);
    connect(m_networkreply, &QNetworkReply::finished, this, &HThreadObject::Finished);
#ifndef QT_NO_SSL
    connect(m_networkmanager, &QNetworkAccessManager::sslErrors, this, &HThreadObject::sslErrors);
#endif

    m_taskIndex++;
}

void HThreadObject::work()
{
    m_taskIndex = 0;
    if (!m_networkmanager)
    {
        m_networkmanager = new QNetworkAccessManager(this);
    }
    if (m_networkmanager)
    {
        startNetwork();
    }
}


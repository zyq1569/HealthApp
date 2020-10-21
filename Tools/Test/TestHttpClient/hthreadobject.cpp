#include "hthreadobject.h"

#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>

#define  WAIT_HTTP 0
#define  DOWN_HTTP 1


HThreadObject::HThreadObject(QObject *parent) : QObject(parent)
{
    m_file = nullptr;
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

void HThreadObject::setState(int state)
{

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
    QUrl url = m_httpInfo[m_index].url;
    m_networkreply = m_networkmanager->get(QNetworkRequest(url));
    connect(m_networkreply, &QIODevice::readyRead, this, &HThreadObject::ReadyRead);
    connect(m_networkreply, &QNetworkReply::finished, this, &HThreadObject::Finished);

    m_index++;
}

void HThreadObject::work()
{
    m_index = 0;
    m_networkmanager = new QNetworkAccessManager(this);
    if (m_networkmanager)
    {
        startNework();
    }
}

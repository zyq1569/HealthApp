#include "hthreadobject.h"

#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>

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
    emit notifyResult(1);
    m_networkreply->deleteLater();
    m_networkreply = nullptr;
}

void HThreadObject::work(const QUrl url, QString fullpathfilename)
{
    m_file = new QFile(fullpathfilename);
    if (!m_file->open(QFile::WriteOnly | QIODevice::Truncate))
    {
        m_file->close();
        delete m_file;
        m_file = nullptr;
        notifyResult(-1);
        return ;
    }
    m_networkreply = m_networkmanager.get(QNetworkRequest(url));
    connect(m_networkreply, &QIODevice::readyRead, this, &HThreadObject::ReadyRead);
    connect(m_networkreply, &QNetworkReply::finished, this, &HThreadObject::Finished);
}

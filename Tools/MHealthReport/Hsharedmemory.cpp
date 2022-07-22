#include "Hsharedmemory.h"

Hsharedmemory::Hsharedmemory(qint64 id) :
    m_SharedMemory(nullptr), m_Pid(id)
{

}

Hsharedmemory::~Hsharedmemory()
{

}

void Hsharedmemory::open()
{
    if(!m_SharedMemory)
    {
        m_SharedMemory = new QSharedMemory(GLOBAL_SHARE_MEMORY_KEY);
        m_SharedMemory->create(sizeof(PROCESS_CHANNEL));
    }
    m_SharedMemory->attach();
}

void Hsharedmemory::close()
{
    if(m_SharedMemory)
    {
        m_SharedMemory->detach();
        delete m_SharedMemory;
        m_SharedMemory = NULL;
    }
}

void Hsharedmemory::write(const QString str)
{
    m_SharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)m_SharedMemory->data();
    pc->flag = FLAG_ON;
    pc->command = CMD_TEXT;
    pc->pid = m_Pid;
    std::string stdStr = str.toStdString();
    strcpy_s(pc->data, stdStr.c_str());
    m_SharedMemory->unlock();
}

/**
 * @brief Write data into shared memory
 */
void Hsharedmemory::write2Sender(const QString str)
{
    m_SharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)m_SharedMemory->data();
    pc->flag = FLAG_ON;
    pc->command = CMD_TEXT;
    pc->pid = m_Pid;
    std::string stdStr = str.toStdString();
    strcpy_s(pc->rec, stdStr.c_str());
    m_SharedMemory->unlock();
}

void Hsharedmemory::write(char *str)
{
    m_SharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)m_SharedMemory->data();
    pc->flag = FLAG_ON;
    pc->command = CMD_TEXT;
    pc->pid = m_Pid;
    strcpy_s(pc->data, str);
    m_SharedMemory->unlock();
}

QString Hsharedmemory::read() const
{
    m_SharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)m_SharedMemory->data();
    if(pc->flag==FLAG_OFF || pc->command!=CMD_TEXT || pc->pid==m_Pid)
    {
        m_SharedMemory->unlock();
        return QString();
    }

    pc->flag = FLAG_OFF;
    pc->command = CMD_NULL;
    QString s = QString::fromLatin1(pc->data);
    m_SharedMemory->unlock();

    return s;
}

//    QString readFromReceiver() const;
QString Hsharedmemory::readFromReceiver() const
{
    m_SharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)m_SharedMemory->data();
    if(pc->flag==FLAG_OFF || pc->command!=CMD_TEXT || pc->pid==m_Pid)
    {
        m_SharedMemory->unlock();
        return QString();
    }

    pc->flag = FLAG_OFF;
    pc->command = CMD_NULL;
    QString s = QString::fromLatin1(pc->rec);
    m_SharedMemory->unlock();

    return s;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief HreadThread::HreadThread
/// \param sharedMemory
/// \param parent
///
HreadThread::HreadThread(Hsharedmemory *sharedMemory , bool sender, QObject *parent) : QThread(parent), m_SharedMemory(sharedMemory), m_sender(sender)
{
    clear();
}

void HreadThread::run()
{
    if (m_sender)
    {
        while(true)
        {
            QString s = m_SharedMemory->readFromReceiver();
            if (!s.isEmpty())
            {
                m_info = s;
                emit savereport(m_info);
            }
            QThread::msleep(300);
        }
    }
    else
    {
        while(true)
        {
            QString s = m_SharedMemory->read();///file:
            int n = s.length();
            if(!s.isEmpty() && s.length() > 5)
            {
                if (s.toUpper().contains("FILE:"))
                {
                    m_info = s.right(n - 5);
                    emit reportInfo(m_info);
                }
            }
            QThread::msleep(300);
        }
    }

}

void HreadThread::clear()
{
    m_info = "";
}

void HreadThread::send2Sender()
{
    m_SharedMemory->write2Sender(m_info);
}

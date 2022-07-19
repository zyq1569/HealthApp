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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief HreadThread::HreadThread
/// \param sharedMemory
/// \param parent
///
HreadThread::HreadThread(Hsharedmemory *sharedMemory, QObject *parent) : QThread(parent), m_SharedMemory(sharedMemory)
{
    clear();
}

void HreadThread::run()
{
    while(true)
    {
        QString s = m_SharedMemory->read();///file:
        if(!s.isEmpty() && s.length() > 5)
        {
            //printf("RECV: %s\n", s.toStdString().c_str());
            if (s.toUpper().contains("FILE:"))
            {
                m_info = s.right(6);
                emit reportInfo(m_info);
            }
        }
        QThread::msleep(300);
    }
}

void HreadThread::clear()
{
    m_info = "";
}

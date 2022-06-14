#include "Hsharedmemory.h"

Hsharedmemory::Hsharedmemory(int id) :
    m_SharedMemory(nullptr), m_Id(id)
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
    pc->pid = m_Id;
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
    pc->pid = m_Id;
    strcpy_s(pc->data, str);
    m_SharedMemory->unlock();
}

QString Hsharedmemory::read() const
{
    m_SharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)m_SharedMemory->data();
    if(pc->flag==FLAG_OFF || pc->command!=CMD_TEXT || pc->pid==m_Id)
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

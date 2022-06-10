#include "Hsharedmemory.h"

Hsharedmemory::Hsharedmemory(int id) :
    m_SharedMemory(nullptr)
    , mId(id)
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
    mSharedMemory->attach();
}

void Hsharedmemory::close()
{
    if(mSharedMemory)
    {
        mSharedMemory->detach();
        delete mSharedMemory;
        mSharedMemory = NULL;
    }
}

void Hsharedmemory::write(const QString str)
{
    mSharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)mSharedMemory->data();
    pc->flag = FLAG_ON;
    pc->command = CMD_TEXT;
    pc->pid = mId;
    std::string stdStr = str.toStdString();
    strcpy(pc->data, stdStr.c_str());
    mSharedMemory->unlock();
}

void Hsharedmemory::write(char *str)
{
    mSharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)mSharedMemory->data();
    pc->flag = FLAG_ON;
    pc->command = CMD_TEXT;
    pc->pid = mId;
    strcpy(pc->data, str);
    mSharedMemory->unlock();
}

QString Hsharedmemory::read() const
{
    mSharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)mSharedMemory->data();
    if(pc->flag==FLAG_OFF || pc->command!=CMD_TEXT || pc->pid==mId)
    {
        mSharedMemory->unlock();
        return QString();
    }

    pc->flag = FLAG_OFF;
    pc->command = CMD_NULL;
    QString s = QString::fromLatin1(pc->data);
    mSharedMemory->unlock();

    return s;
}

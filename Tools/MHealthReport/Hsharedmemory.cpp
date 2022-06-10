#include "sharedmemory.h"

TSharedMemory::TSharedMemory(int id) :
    mSharedMemory(nullptr)
  , mId(id)
{

}

TSharedMemory::~TSharedMemory()
{

}

void TSharedMemory::open()
{
    if(!mSharedMemory) {
        mSharedMemory = new QSharedMemory(GLOBAL_SHARE_MEMORY_KEY);
        mSharedMemory->create(sizeof(PROCESS_CHANNEL));
    }
    mSharedMemory->attach();
}

void TSharedMemory::close()
{
    if(mSharedMemory)
    {
        mSharedMemory->detach();
        delete mSharedMemory;
        mSharedMemory = NULL;
    }
}

void TSharedMemory::write(const QString str)
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

void TSharedMemory::write(char *str)
{
    mSharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)mSharedMemory->data();
    pc->flag = FLAG_ON;
    pc->command = CMD_TEXT;
    pc->pid = mId;
    strcpy(pc->data, str);
    mSharedMemory->unlock();
}

QString TSharedMemory::read() const
{
    mSharedMemory->lock();
    PROCESS_CHANNEL *pc = (PROCESS_CHANNEL*)mSharedMemory->data();
    if(pc->flag==FLAG_OFF || pc->command!=CMD_TEXT || pc->pid==mId) {
        mSharedMemory->unlock();
        return QString();
    }

    pc->flag = FLAG_OFF;
    pc->command = CMD_NULL;
    QString s = QString::fromLatin1(pc->data);
    mSharedMemory->unlock();

    return s;
}

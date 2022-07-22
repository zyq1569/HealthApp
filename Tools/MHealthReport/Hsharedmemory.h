#ifndef TSHAREDMEMORY_H
#define TSHAREDMEMORY_H

#include <QSharedMemory>
#include <QThread>

#define FLAG_OFF    0x0
#define FLAG_ON     0x1

#define CMD_NULL    0x0
#define CMD_RAISE   0x1
#define CMD_TEXT    0x2
#define CMD_EXIT    0x3

#ifdef QT_DEBUG
#define GLOBAL_SHARE_MEMORY_KEY     "MHealthReport_D"
#else
#define GLOBAL_SHARE_MEMORY_KEY     "MHealthReport"
#endif

#define SHARE_LEN 2048


struct PROCESS_CHANNEL
{
    char flag;
    char command;
    qint64 pid;
    char data[SHARE_LEN];
    char rec[SHARE_LEN];
};

// 考虑轻量级线程https://github.com/Qthreads/qthreads
class Hsharedmemory
{
public:
    Hsharedmemory(qint64 id);
    ~Hsharedmemory();

    /**
     * @brief Open shared memory
     */
    void open();

    /**
     * @brief Close shared memory
     */
    void close();

    /**
     * @brief Write data into shared memory
     */
    void write(const QString str);
    void write(void *data, int size);
    void write(char *str);
    void write(const QVariant &data);

    /**
     * @brief Write data into shared memory
     */
    void write2Sender(const QString str);

    /**
     * @brief Read string from shared memory
     * @return QString
     */
    QString read() const;

    /**
     * @brief Read string from shared memory
     * @return QString
     */
    QString readFromReceiver() const;

private:
    QSharedMemory *m_SharedMemory;
    qint64 m_Pid;
};


class HreadThread : public QThread
{
    Q_OBJECT
public:
    HreadThread(Hsharedmemory *sharedMemory, QObject *parent = Q_NULLPTR);

    void clear();

    void send2Sender();
private:
    Hsharedmemory *m_SharedMemory;

    // QThread interface
protected:
    void run() Q_DECL_OVERRIDE;

protected:
    QString m_info;

signals:
    void reportInfo(QString);

};

#endif // TSHAREDMEMORY_H

#ifndef HTHREADOBJECT_H
#define HTHREADOBJECT_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QList>
class QFile;
class QNetworkReply;

#define  OpenFile_Fail -1
#define  SaveFile_Ok 1
#define  Finished_All 2

struct HttpInfo
{
    QUrl url;
    QString fullpathfilename;
};
class HThreadObject : public QObject
{
    Q_OBJECT
public:
    explicit HThreadObject(QObject *parent = nullptr);
    ~HThreadObject();
    void setState(int state);
    void setInput(QList<HttpInfo> httpInfo);
public slots:
    void work();
    void ReadyRead();
    void Finished();
signals:
    void notifyResult(const int &state);
private:
    void startNework();
private:
    QNetworkReply *m_networkreply;
    QNetworkAccessManager *m_networkmanager;
    QFile *m_file;
    int m_index;
    QList<HttpInfo> m_httpInfo;
};

///--------------------------------------------------------------------------------------------
#define  Max_thread_size 3
class HManageThread : public QObject
{
    Q_OBJECT
    QThread workerThread[Max_thread_size];
    HThreadObject *worker[Max_thread_size];
    int number[Max_thread_size];
public:
    HManageThread()
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
    ~HManageThread()
    {
        for (int i=0; i<Max_thread_size; i++)
        {
            workerThread[i].quit();
            workerThread[i].wait();
        }

    }
    void start(QList<HttpInfo> httpInfo)
    {
        int size = httpInfo.size();
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
        emit operate();
    }
public slots:
    void handleResults(const int &)
    {

    }
signals:
    void operate();
};

#endif // HTHREADOBJECT_H

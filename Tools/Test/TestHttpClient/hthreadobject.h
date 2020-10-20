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

class HThreadObject : public QObject
{
    Q_OBJECT
public:
    explicit HThreadObject(QObject *parent = nullptr);
    ~HThreadObject();

public slots:
    void work(const QUrl url, QString fullpathfilename);
    void ReadyRead();
    void Finished();
signals:
    void notifyResult(const int &state);
private:
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QFile *m_file;
};

///--------------------------------------------------------------------------------------------
class HManageThread : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    HManageThread()
    {
        HThreadObject *worker = new HThreadObject;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &HManageThread::operate, worker, &HThreadObject::work);
        connect(worker, &HThreadObject::notifyResult, this, &HManageThread::handleResults);
        workerThread.start();
    }
    ~HManageThread()
    {
        workerThread.quit();
        workerThread.wait();
    }
public slots:
    void handleResults(const int &)
    {

    }
signals:
    void operate(const QUrl url, QString fullpathfilename);
};

#endif // HTHREADOBJECT_H

#ifndef HTHREADOBJECT_H
#define HTHREADOBJECT_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QList>

class QFile;

class HThreadObject : public QObject
{
    Q_OBJECT
public:
    explicit HThreadObject(QObject *parent = nullptr);
    ~HThreadObject();
public slots:
    void work(const QString &parameter)
    {
        QString result;
        /* here is the expensive or blocking operation */
        emit notifyResult(result);
    }
signals:
    void notifyResult(const QString &result);
private:
    QUrl m_url;
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QFile *m_file;
};

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
    void handleResults(const QString &)
    {

    }
signals:
    void operate(const QString &);
};

#endif // HTHREADOBJECT_H

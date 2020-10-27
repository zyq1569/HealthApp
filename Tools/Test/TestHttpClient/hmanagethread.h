#ifndef HMANAGETHREAD_H
#define HMANAGETHREAD_H

#include "hthreadobject.h"

#include <QObject>
#include <QProgressDialog>

///--------------------------------------------------------------------------------------------

#define  Max_thread_size 3
class HManageThread : public QObject
{
    Q_OBJECT
    QThread workerThread[Max_thread_size];
    HThreadObject *worker[Max_thread_size];
    int number[Max_thread_size];
public:
    HManageThread();
    ~HManageThread();
    void start(QList<HttpInfo> httpInfo);
public slots:
    void handleResults(const int &msg);
    void cancelDownload();
signals:
    void operate();
    void downloadProgress(qint64 bytesRead, qint64 totalBytes);
    void ProgressInfo(QString text);
    void readfiles(qint64 bytesRead);
    void finished();
private:
    int m_total,m_remainder;
    QString m_fileinfo;
    QObject *m_parent;
};
#endif // HMANAGETHREAD_H

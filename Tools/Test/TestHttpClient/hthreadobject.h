#ifndef HTHREADOBJECT_H
#define HTHREADOBJECT_H

#include <QProgressDialog>
#include <QObject>
#include <QMutex>
#include <QThread>
#include <QUrl>
#include <QList>
#include <QMessageBox>
#include <QSslError>

class QFile;
class QNetworkReply;
class QNetworkAccessManager;

#define  OpenFile_Fail -1
#define  SaveFile_Ok 1
#define  Finished_All 2

struct HttpInfo
{
    QUrl url;
    QString fullpathfilename;
};

///-----------------
class ProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(const QUrl &url, QWidget *parent = nullptr);
    ~ProgressDialog();
    void inIt(QUrl ur);

public slots:
    void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);
};

///--------------------------
class HThreadObject : public QObject
{
    Q_OBJECT
public:
    explicit HThreadObject(QObject *parent = nullptr);
    ~HThreadObject();
    void setInput(QList<HttpInfo> httpInfo);

public slots:
    void work();
    void ReadyRead();
    void Finished();

#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

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

#endif // HTHREADOBJECT_H

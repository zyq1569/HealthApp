#ifndef HTHREADOBJECT_H
#define HTHREADOBJECT_H

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



///---------------down files from "HttpInfo"---thread---
struct HttpInfo
{
    QUrl url;
    QString fullpathfilename;
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
    void startNetwork();

private:
    QNetworkReply *m_networkreply;
    QNetworkAccessManager *m_networkmanager;
    QFile *m_file;
    int m_taskIndex;
    QList<HttpInfo> m_httpInfo;
};



#endif // HTHREADOBJECT_H

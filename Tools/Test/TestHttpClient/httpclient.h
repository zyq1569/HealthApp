#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QProgressDialog>
#include <QUrl>
#include <QSslError>
#include <QNetworkAccessManager>


class QNetworkReply;
class QAuthenticator;
class QFile;

//class QNetworkAccessManager;

class ProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    explicit ProgressDialog(const QUrl &url, QWidget *parent = nullptr);
    ~ProgressDialog();

public slots:
    void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);
};

enum DownFileType {
    dcm,
    json,
    other
};

class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr, QString dir = "");


public:
    void setDwonloadDir(QString dir);
    void getStudyDBinfo(QUrl url,QString start,QString end,QString page,QString limit);//start=19700101&end=20191230&page=1&limit=10
    void getStudyImageFile(QUrl url,QString studyuid="",QString seruid = "", QString imguid = "");

signals:

public slots:

private slots:
    void cancelDownload();
    void httpFinished();
    void httpReadyRead();
    void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

private:
    bool openFileForWrite(const QString &fileName);
    void downFileFromWeb(QUrl httpUrl,QString savefilename,QString downDir);
    void startRequest(const QUrl &requestedUrl);
private:
    QUrl m_url;
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QFile *m_file;
    bool m_httpRequestAborted;
    QString m_downDir;
    DownFileType m_currentfiletype;
    QByteArray m_currentDownData;

};

#endif // HTTPCLIENT_H

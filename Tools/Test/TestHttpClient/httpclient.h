#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QProgressDialog>
#include <QUrl>
#include <QSslError>
#include <QNetworkAccessManager>
#include <QList>

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

///-----------------------------------------------------------------------------------------------------------------------
enum DownFileType
{
    dcm,
    dbinfo,
    studyini,
    other
};

typedef  QString  OFString;
struct DicomFileInfo
{
    OFString patientName;
    OFString patientSex;
    OFString patientAge;
    OFString patientBirthDate;
    OFString patientBirthTime;
    OFString patientId;

    OFString modality;
    OFString manufacturer;
    OFString institutionName;

    OFString studyId;
    OFString studyUID;
    OFString studyDescription;
    OFString studyDate;
    OFString studyTime;

    OFString seriesUID;
    OFString seriesDescription;
    OFString seriesNumber;

    OFString imageSOPInstanceUID;
    OFString instanceNumber;
};

struct ImageInfo
{
    //(0008,0018) UI 1.2.840.113619.2.55.3.604688119.699.1256270047.398.2   # 1, 52 SOP Instance UID
    OFString imageSOPInstanceUID;
    // (0020, 0013) IS 2                                                 # 1, 2 Instance Number;
    int instanceNumber;
};

struct SeriesInfo
{
    QList<ImageInfo> imagesInfoList;
    OFString seriesUID;
    OFString seriesDescription;
    //Series Number
    int seriesNumber;
};

struct StudyInfo
{
    OFString patientName;
    OFString patientId;
    OFString modality;
    OFString studyId;
    OFString studyUID;
    OFString studyDate;
    OFString studyDescription;
    QList<SeriesInfo> seriesInfoList;
};

struct HSeries
{
    QString SeriesUID;
    QList <QString> ImageSOPUI;
};
struct HStudy
{
    QString StudyUID;
    int imageCount;
    QList <HSeries> Serieslist;
};

///----------------------------------------------------------------------------------------
struct StudyRowInfo
{
    QString patientIdentity,patientName,patientId,patientSex,patientBirthday,patientTelNumber;
    QString patientAddr,patientCarID,patientType,patientEmail,studyOrderIdentity,studyId,studyuid;
    QString scheduledDateTime,ScheduledDate,orderDateTime,studyDescription,studyModality,aETitle;
    QString studyType,studyCode,studyState,studyCost,studyDate,studyDepart,sStudyModality,costType;
};

struct PatientStudyDB
{
    int code;
    QString msg,ver;
    int count;
    QList<StudyRowInfo> rowinfo;
};

///----------------------------------------------------------------------------------------
class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr, QString dir = "");


public:
    void setPatientDBinfo(QJsonValue &JsonValue,StudyRowInfo &Rowinfo);
    void setDwonloadDir(QString dir);
    void getStudyDBinfo(QUrl url,QString start,QString end,QString page,QString limit);//start=19700101&end=20191230&page=1&limit=10
    void getStudyImageFile(QUrl url,QString studyuid="",QString seruid = "", QString imguid = "");
    PatientStudyDB* getPatientStudyDB();
signals:
void parseDataFinished();
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
    void ParseDwonData();
private:
    QUrl m_url;
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QFile *m_file;
    bool m_httpRequestAborted;
    QString m_downDir;
    DownFileType m_currentfiletype;
    QByteArray m_currentDownData;
    PatientStudyDB m_patientstudydb;

};

#endif // HTTPCLIENT_H

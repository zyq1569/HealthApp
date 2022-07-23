#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QUrl>
//#include <QSslError>
#include <QNetworkAccessManager>
#include <QList>
#include <QDir>

class QNetworkReply;
class QAuthenticator;
class QFile;
class HManageThread;


///-----------------------------------------------------------------------------------------------------------------------
enum DownFileType
{
    dcm,
    dbinfo,
    studyini,
    report,
    other
};

typedef  QString  OFString;
struct DicomFileInfo
{
    OFString patientName,patientSex, patientAge,patientBirthDate, patientBirthTime,  patientId;
    OFString modality, manufacturer, institutionName;
    OFString studyId, studyUID,studyDescription,studyDate, studyTime;
    OFString seriesUID, seriesDescription, seriesNumber;
    OFString imageSOPInstanceUID, instanceNumber;
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
    OFString patientName, patientId, modality, studyId, studyUID, studyDate, studyDescription;
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

///------2020-11-17-------------new study order data struct
///
const int g_OrderCount = 40;
enum eStudyOrderIndex
{
    PatientIdentity, PatientID ,  PatientName,  PatientNameEnglish ,  PatientSex,   PatientBirthday,
    PatientAddr, PatientEmail, PatientCarID, PatientTelNumber, PatientType ,
    PatientState,  StudyOrderIdentity, StudyID, ClinicID,StudyUID, StudyModality ,
    StudyAge , ScheduledDateTime , AETitle , OrderDateTime , StudyDescription ,
    StudyDepart , StudyCode ,  StudyCost ,  CostType  , StudyType ,  StudyState,
    StudyDateTime , InstitutionName , ProcedureStepStartDate  , StudyModalityIdentity ,
    StudyManufacturer , RegisterID, PatientJob, PatientNation, PatientMarriage, PatientHometown,
    PatientHisID, PatientHistoryTell
};

const QString g_StudyOrder[] =
{
    "PatientIdentity","PatientID",  "PatientName" ,  "PatientNameEnglish" ,  "PatientSex",   "PatientBirthday",
    "PatientAddr", "PatientEmail", "PatientCarID", "PatientTelNumber", "PatientType" ,
    "PatientState",  "StudyOrderIdentity", "StudyID", "ClinicID","StudyUID", "StudyModality" ,
    "StudyAge" , "ScheduledDateTime" , "AETitle" , "OrderDateTime" , "StudyDescription" ,
    "StudyDepart" , "StudyCode" ,  "StudyCost" ,  "CostType"  , "StudyType" ,  "StudyState",
    "StudyDateTime" , "InstitutionName" , "ProcedureStepStartDate"  , "StudyModalityIdentity" ,
    "StudyManufacturer" , "RegisterID", "PatientJob", "PatientNation", "PatientMarriage", "PatientHometown",
    "PatientHisID", "PatientHistoryTell"
};

struct DateNameValue
{
    QString Name;
    QString Value;
};

class StudyOrderData
{
public:
    StudyOrderData()
    {
        for (int i=0; i<g_OrderCount; i++)
        {
            studyorder[i].Name = g_StudyOrder[i];
            studyorder[i].Value = "";
        }
    }
public:
    DateNameValue studyorder[g_OrderCount];
};

struct PatientStudyOder
{
    int code;
    QString msg,ver;
    int count;
    QList<StudyOrderData> orderdata;
};

///------2020-11-17-------------new study order data struct
///
///
///----------------------------------------------------------------------------------------
class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr, QString dir = "");

public:
    void setStudyOrder(QJsonValue &JsonValue,StudyOrderData &OrderData);
    PatientStudyOder* getPatientStudyOder();
    void setDwonloadDir(QString dir);
    //start=19700101&end=20191230&page=1&limit=10
    void getStudyDBinfo(QUrl url,QString start,QString end,QString page,QString limit);
    void getStudyImageFile(QUrl url,QString studyuid="",QString seruid = "", QString imguid = "");
    void getStudyReportFile(QUrl url, QString StudyOrderIdentity, QString studyuid="",QString seruid = "", QString imguid = "");

    // updateStudyOrder
    void updateStudyOrder(QByteArray Json);

    bool CreatDir(QString fullPath)
    {
        QDir dir(fullPath); // 注意
        if(dir.exists())
        {
            return true;
        }
        else
        {
            dir.setPath("");
            bool ok = dir.mkpath(fullPath);
            return ok;
        }
    }
    void setHost(QString host)
    {
        m_host = host;
    }

signals:
    void parseDataFinished();
    void parseReportFinished();

private slots:
    void cancelDownload();
    void httpFinished();
    void httpReadyRead();
    //void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
    void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif

private:
    bool openFileForWrite(const QString &fileName);
    void downFileFromWeb(QUrl httpUrl,QString savefilename,QString downDir);
    void startRequest(const QUrl &requestedUrl);
    void ParseDwonData();
    void ParseStudyOderData();

private:
    QUrl m_url;
    QString m_host;
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QFile *m_file;
    bool m_httpRequestAborted;
    QString m_downDir;
    DownFileType m_currentfiletype;
    QByteArray m_currentDownData;
    PatientStudyOder m_patientstudyorder;
    HManageThread *m_managethread;
    QObject *m_parent;

};

#endif // HTTPCLIENT_H

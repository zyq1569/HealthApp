#ifndef DICOMSENDER_H
#define DICOMSENDER_H

#include<QStringList>
#include<QRunnable>
#include<QObject>
#include<map>
#include<vector>
#include<set>
#include "util.h"


class DestinationEntry
{
public:
    DestinationEntry() {};

    DestinationEntry(std::string name, std::string destinationhost, int destinationport, std::string destinationaetitle, std::string ouraetitle)
    {
        this->name = name;
        this->destinationHost = destinationhost;
        this->destinationPort = destinationport;
        this->destinationAETitle = destinationaetitle;
        this->ourAETitle = ouraetitle;
    }

    std::string name;
    std::string destinationHost;
    int destinationPort;
    std::string destinationAETitle;
    std::string ourAETitle;
};

class Study
{
public:
    Study(const Study &study)
    {
        this->studyuid = study.studyuid; this->studydate = study.studydate; this->studydesc = study.studydesc;
        this->dir = study.dir ; this->sopclassuid = study.sopclassuid; this->transfersyntax = study.transfersyntax;

        foreach(std::string it, study.filespath)
        {
            this->filespath.push_back(it);
        }
    };

    Study()
    {

    };

public:
    std::string studyuid,studydate,studydesc,dir,sopclassuid,transfersyntax,originalXfer;
    std::vector<std::string> filespath;

};

class Patient
{
public:
    Patient()
    {

    };
    std::string patientid,patientname;
    std::vector<Study> studydatas;
};

class PatientData
{
    PatientData()
    {

    };
    std::vector<Patient> patientdatas;
};

class DcmDataset;
class DcmItem;
class DcmTagKey;
class OFString;
class Taskthread: public QObject ,public QRunnable
{
    Q_OBJECT

public:
    explicit Taskthread(QObject *parent = nullptr);
    ~Taskthread();
    void run();
    void dicomDataJob();
    void dicomSendJob();
    int sendStudy(Study &studys);
    bool isCanceled();
    void setJob(int type);
    static bool updateStringAttributeValue(DcmItem *dataset, const DcmTagKey &key, OFString &value);
    void registerCodecs();
    void registercleanup();

public slots:
    void scanDir(QString dir, std::vector<Patient> listpat);

    void sendDcm(DestinationEntry dest, std::vector<Patient> listpat);

signals:
    void finishScanDir(std::vector<Patient> listpatient);// to parent thread

    void finishSendDcm(int );// to parent thread

public:
    QString m_scanDir;
    uint m_sendFiles;
    DestinationEntry m_dest;
    std::vector<Patient> m_listpatient;

public:
    static bool  g_static_check;

private:
    int m_type;
};


class DicomSender: public QObject
{
    Q_OBJECT

public:
    DicomSender();

    bool IsCanceled();

    void ScanPatient(QString dir);

    bool Echo();

    bool SendPatiens(std::vector<Patient> listpat);

    void SetUpateDcmFileAnonymous(bool key = false);

public slots:
    void finishlistpatient(std::vector<Patient> listpat);

    void finishSendDcm(int sendFiles);


signals:
    void scandicomfile(QString dir, std::vector<Patient> listpat);// to thread
    void finishscandicomfile();// to forms

    void senddicomfile(DestinationEntry dest, std::vector<Patient> listpat);// to thread
    void finishsenddicomfile(int sendFiles);// to forms

public:
    DestinationEntry m_destination;
    std::vector<Patient> m_listpatient;

    Taskthread m_taskScanDicom,m_taskSendDicom;

};

#endif // DICOMSENDER_H

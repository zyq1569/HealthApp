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

class Taskthread: public QObject ,public QRunnable
{
    Q_OBJECT

public:
    explicit Taskthread(QObject *parent = nullptr);
    ~Taskthread();
    void run();
    void dicomDataJob();
    void dicomSendJob();
    void setJob(int type);

public slots:
    void scandir(QString dir, std::vector<Patient> listpat);

signals:
    void finish(std::vector<Patient> listpatient);// to parent thread

public:
    QString scanDir;
    std::vector<Patient> listpatient;

private:
    int m_type;
};


class DicomSender: public QObject
{
    Q_OBJECT

public:
    DicomSender();

    int SendStudy(Study &studys);

    int SendDcmFiles(Study &studys);

    bool IsCanceled();

    void ScanPatient(QString dir);

    void AddStudy(QString dir);

    void UpdatePatientdatas(DcmDataset *data);

public slots:
    void finishlistpatient(std::vector<Patient> listpat);

signals:
    void scandicomfile(QString dir, std::vector<Patient> listpat);// to thread
    void finishscandicomfile();// to forms

public:
    DestinationEntry m_destination;
    std::vector<Patient> m_listpatient;

    Taskthread m_taskScanDicom,m_taskSendDicom;

};

#endif // DICOMSENDER_H

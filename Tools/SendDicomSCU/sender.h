#ifndef DICOMSENDER_H
#define DICOMSENDER_H

#include<QStringList>
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
    std::string studyuid,studydate,studydesc,dir,sopclassuid,transfersyntax;
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
class DicomSender
{

public:
    DicomSender();

    int SendStudy(Study &studys);

    int SendDcmFiles(Study &studys);

    bool IsCanceled();

    void ScanPatient(QString dir);

    void AddStudy(QString dir);

    void UpdatePatientdatas(DcmDataset *data);
public:
    DestinationEntry destination;
    std::vector<Patient> listpatient;


};

#endif // DICOMSENDER_H

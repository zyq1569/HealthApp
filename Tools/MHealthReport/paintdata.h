#ifndef PAINTDATA_H
#define PAINTDATA_H
#include<QString>


#define PAINTDATA  PaintData::getInstance()

typedef struct PatientStudyData
{
    /// patient table
    QString       PatientIdentity;
    QString       PatientID;
    QString       PatientName;
    QString       PatientNameEnglish;
    QString       PatientSex;
    QString       PatientBirthday;
    QString       PatientAddr;
    QString       PatientEmail;
    QString       PatientCarID;
    QString       PatientTelNumber;
    QString       PatientType;
    QString       PatientState;
    /// order table
    QString       StudyOrderIdentity;
    ///PatientIdentity
    QString       StudyID;
    QString       StudyUID;
    QString       StudyModality;
    QString       StudyAge;
    QString       ScheduledDateTime;
    QString       AETitle;
    QString       OrderDateTime;
    QString       StudyDescription;
    QString       StudyDepart;
    QString       StudyCode;
    QString       StudyCost;
    QString       CostType;
    QString       StudyType;
    QString       StudyState;
    QString       StudyDateTime;
    QString       InstitutionName;
    QString       ProcedureStepStartDate;
    QString       StudyModalityIdentity;
    QString       StudyManufacturer;
    QString       RegisterID;
}PatientStudyData;

class PaintData
{
public:


    static PaintData * getInstance();


    PaintData();


    bool dataCnn();


    bool insertData(PatientStudyData data);


    int selectData(QString regId, QString patientID);


    bool updateData(PatientStudyData data);


    bool deleteData(QString regId, QString patientID);


    PatientStudyData getDataPaint() const;

private:

    static PaintData * s_instance;
    PatientStudyData pStudyData;
};

#endif // DATABASE_H

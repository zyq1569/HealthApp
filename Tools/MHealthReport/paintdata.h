#ifndef PAINTDATA_H
#define PAINTDATA_H
#include<QString>


#define PAINTDATA  PaintData::getInstance()

typedef struct DataPaint
{
    QString regID;
    QString patientID;
    QString hisId;
    QString name;
    QString sex;
    QString age;
    QString nationality;
    QString tel;
    QString address;
    QString marriage;
    QString birthplace;
    QString job;
    QString feeType;
    QString idCard;
    QString familyHistory;
    QString infectiousHistory;
    QString pastHistory;
    QString menarche;
    QString lastMenstruation;
    QString reproductiveHistory;
    QString outPatientNo;
    QString hospitalizedNo;
    QString inpatientArea;
    QString bedNo;
    QString fromOffices;
    QString fromDoctor;
    QString sendTime;
    QString receiveTime;
    QString receiver;
    QString site;
    QString specimenQt;
    QString fixedMode;
    QString chiefComplaint;
    QString currentHistory;
    QString clinical;
}DataPaint;

class PaintData
{
public:


    static PaintData * getInstance();


    PaintData();


    bool dataCnn();


    bool insertData(DataPaint data);


    int selectData(QString regId, QString patientID);


    bool updateData(DataPaint data);


    bool deleteData(QString regId, QString patientID);


    DataPaint getDataPaint() const;

private:

    static PaintData * instance;
    DataPaint dataPaint;
};

#endif // DATABASE_H

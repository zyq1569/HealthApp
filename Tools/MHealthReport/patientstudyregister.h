#ifndef PATIENTSTUDYREGISTER_H
#define PATIENTSTUDYREGISTER_H

#include <QMainWindow>

namespace Ui {
class PatientStudyRegister;
}

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

class PatientStudyRegister : public QMainWindow
{
    Q_OBJECT

public:
    explicit PatientStudyRegister(QWidget *parent = nullptr);
    ~PatientStudyRegister();

public:
    void initControl();
    void clearInfo();

public slots:
//    void flageChange();                           //内容改变
    void on_actionSavePatientInfo_triggered();    //保存患者信息
    void on_actionClearPatientInfo_triggered();   //清除患者信息

private:
    Ui::PatientStudyRegister *ui;
};

#endif // PATIENTSTUDYREGISTER_H

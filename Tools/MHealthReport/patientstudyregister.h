#ifndef PATIENTSTUDYREGISTER_H
#define PATIENTSTUDYREGISTER_H

#include <QMainWindow>

namespace Ui {
class PatientStudyRegister;
}

typedef struct StudyData  {
    QString PatientIdentity   ,
    PatientName    ,
    PatientID      ,
    PatientSex      ,
    PatientBirthday  ,
    PatientTelNumber  ,
    PatientAddr      ,
    PatientCarID    ,
    PatientType     ,
    PatientEmail     ,
    StudyOrderIdentity,
    StudyID           ,
    StudyUID           ,
    ScheduledDateTime  ,
    ScheduledDate      ,
    OrderDateTime     ,
    StudyDescription  ,
    StudyModality      ,
    AETitle           ,
    StudyType         ,
    StudyCode         ,
    StudyState        ,
    StudyCost          ,
    StudyDateTime     ,
    SstudyID           ,
    StudyDepart    ,
    SstudyModality ,
    SstudyUID,
    CostType;
}StudyData;


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

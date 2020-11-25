#ifndef PATIENTSTUDYREGISTER_H
#define PATIENTSTUDYREGISTER_H

#include <QMainWindow>
#include <QNetworkAccessManager>


namespace Ui {
class PatientStudyRegister;
}

class HttpClient;
///-------------------------------------------------------------------
//`PatientIdentity` BIGINT(66) UNSIGNED NOT NULL,
//`PatientID` CHAR(50) NOT NULL,
//`PatientName` CHAR(50) NULL DEFAULT 'unknow',
//`PatientNameEnglish` CHAR(50) NULL DEFAULT '""' COMMENT '姓名拼音(提供给设备-设备几乎都不支持汉字)',
//`PatientSex` CHAR(10) NULL DEFAULT 'O' COMMENT 'O :为待补充',
//`PatientBirthday` CHAR(22) NULL DEFAULT '19000101',
//`PatientAddr` VARCHAR(120) NULL DEFAULT '""' COMMENT '地址',
//`PatientEmail` VARCHAR(50) NULL DEFAULT '""' COMMENT '电子邮件地址',
//`PatientCarID` VARCHAR(50) NULL DEFAULT '""' COMMENT '身份ID:身份证',
//`PatientTelNumber` VARCHAR(50) NULL DEFAULT '""' COMMENT '电话号码',
//`PatientType` INT(11) NULL DEFAULT 0 COMMENT '费用类型：0.自费 2.公费 3.半公费4.其它',
//`PatientState` INT(11) NULL DEFAULT 0 COMMENT '标记状态 小于0为标记删除',
//`PatientJob` VARCHAR(100) NULL DEFAULT 'JOB' COMMENT '职业',
//`PatientNation` CHAR(100) NULL DEFAULT '汉族' COMMENT '民族：汉族、',
//`PatientMarriage` CHAR(50) NULL DEFAULT '未知' COMMENT '婚姻情况:未婚、已婚、未知',
//`PatientHometown` CHAR(50) NULL DEFAULT '广东' COMMENT '籍贯:广州、深圳',
//`PatientHisID` CHAR(50) NULL DEFAULT '-1' COMMENT 'HIS号',
//`PatientHistoryTell` VARCHAR(200) NULL DEFAULT '无' COMMENT '家族病史/传染病史/既往病史/备注/主诉/现病史',
///--------------------------------------------------------------------
typedef struct StudyData  {
    QString
    PatientIdentity       ,
    PatientID             ,
    PatientName           ,
    PatientNameEnglish    ,
    PatientSex            ,
    PatientBirthday       ,
    PatientAddr           ,
    PatientEmail          ,
    PatientCarID          ,
    PatientTelNumber      ,
    PatientType           ,
    PatientState          ,
    PatientJob            ,
    PatientNation         ,
    PatientMarriage       ,
    PatientHometown       ,
    PatientHisID          ,
    PatientHistoryTell    ,

    StudyOrderIdentity    ,
    StudyID               ,
    StudyUID              ,
    StudyModality         ,
    StudyAge              ,
    ScheduledDateTime     ,
    AETitle               ,
    OrderDateTime         ,
    StudyDescription      ,
    StudyDepart           ,
    StudyCode             ,
    StudyCost             ,
    CostType              ,
    StudyType             ,
    StudyState            ,
    StudyDateTime         ,
    InstitutionName       ,
    ProcedureStepStartDate,
    StudyModalityIdentity ,
    StudyManufacturer     ,
    RegisterID
;
}StudyData;

enum QuestType{
    nothing,
    queryStudyOder,
    updateStudyOder
};

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
    void on_clearStudyInfo();

private:
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QString m_url;
    QuestType m_questType;
    bool m_httpSuccess;


private slots:
    void httpFinished();
    void httpReadyRead();

private:
    Ui::PatientStudyRegister *ui;
};

#endif // PATIENTSTUDYREGISTER_H

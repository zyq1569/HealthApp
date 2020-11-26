#ifndef PatientData_H
#define PatientData_H
#include<QString>


#define PATIENTDATA  PatientData::getInstance()

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

const QString getServerHttpUrl();
const QString getDownDir();

void setServerHttpUrl(QString url);
void setDownDir(QString dir);

enum QuestType{
    nothing,
    queryStudyOder,
    updateStudyOder
};

class PatientData
{
public:


    static PatientData * getInstance();


    PatientData();


    bool dataCnn();


    bool insertData(PatientStudyData data);


    int selectData(QString regId, QString patientID);


    bool updateData(PatientStudyData data);


    bool deleteData(QString regId, QString patientID);


    PatientStudyData getDataPatient() const;

private:

    static PatientData * s_instance;
    PatientStudyData pStudyData;
};

#endif // DATABASE_H

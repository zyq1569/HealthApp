#include "patientstudyregister.h"
#include "ui_patientstudyregister.h"


#include "httpclient.h"
///-------Json-------------
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>


PatientStudyRegister::PatientStudyRegister(QWidget *parent) :
    QMainWindow(parent),  ui(new Ui::PatientStudyRegister)
{
    ui->setupUi(this);

    initControl();

    m_questType = nothing;

    m_url ="http://127.0.0.1:8080";

}

PatientStudyRegister::~PatientStudyRegister()
{
    delete ui;

}

void PatientStudyRegister::initControl()
{
    ui->m_comNation->addItem(tr("汉族"));
    ui->m_comNation->addItem(tr("回族"));
    ui->m_comNation->addItem(tr("蒙古族"));
    ui->m_comNation->addItem(tr("藏族"));
    ui->m_comNation->addItem(tr("维吾尔族"));
    ui->m_comNation->addItem(tr("苗族"));
    ui->m_comNation->addItem(tr("彝族"));
    ui->m_comNation->addItem(tr("壮族"));
    ui->m_comNation->addItem(tr("布依族"));
    ui->m_comNation->addItem(tr("朝鲜族"));
    ui->m_comNation->addItem(tr("满族"));
    ui->m_comNation->addItem(tr("侗族"));
    ui->m_comNation->addItem(tr("瑶族"));
    ui->m_comNation->addItem(tr("白族"));
    ui->m_comNation->addItem(tr("土家族"));
    ui->m_comNation->addItem(tr("哈尼族"));
    ui->m_comNation->addItem(tr("哈萨克族"));
    ui->m_comNation->addItem(tr("傣族"));
    ui->m_comNation->addItem(tr("黎族"));
    ui->m_comNation->addItem(tr("僳僳族"));
    ui->m_comNation->addItem(tr("佤族"));
    ui->m_comNation->addItem(tr("畲族"));
    ui->m_comNation->addItem(tr("高山族"));
    ui->m_comNation->addItem(tr("拉祜族"));
    ui->m_comNation->addItem(tr("水族"));
    ui->m_comNation->addItem(tr("东乡族"));
    ui->m_comNation->addItem(tr("纳西族"));
    ui->m_comNation->addItem(tr("景颇族"));
    ui->m_comNation->addItem(tr("柯尔克孜族"));
    ui->m_comNation->addItem(tr("土族"));
    ui->m_comNation->addItem(tr("达斡尔族"));
    ui->m_comNation->addItem(tr("仫佬族"));
    ui->m_comNation->addItem(tr("羌族"));
    ui->m_comNation->addItem(tr("布朗族"));
    ui->m_comNation->addItem(tr("撒拉族"));
    ui->m_comNation->addItem(tr("毛南族"));
    ui->m_comNation->addItem(tr("仡佬族"));
    ui->m_comNation->addItem(tr("锡伯族"));
    ui->m_comNation->addItem(tr("阿昌族"));
    ui->m_comNation->addItem(tr("普米族"));
    ui->m_comNation->addItem(tr("塔吉克族"));
    ui->m_comNation->addItem(tr("怒族"));
    ui->m_comNation->addItem(tr("乌孜别克族"));
    ui->m_comNation->addItem(tr("俄罗斯族"));
    ui->m_comNation->addItem(tr("鄂温克族"));
    ui->m_comNation->addItem(tr("德昂族"));
    ui->m_comNation->addItem(tr("保安族"));
    ui->m_comNation->addItem(tr("裕固族"));
    ui->m_comNation->addItem(tr("京族"));
    ui->m_comNation->addItem(tr("塔塔尔族"));
    ui->m_comNation->addItem(tr("独龙族"));
    ui->m_comNation->addItem(tr("鄂伦春族"));
    ui->m_comNation->addItem(tr("赫哲族"));
    ui->m_comNation->addItem(tr("门巴族"));
    ui->m_comNation->addItem(tr("珞巴族"));
    ui->m_comNation->addItem(tr("基诺族"));

    ui->m_StudyOrderDate->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->m_StudyOrderDate->setDateTime(QDateTime::currentDateTime());

    ui->m_RegDate->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->m_RegDate->setDateTime(QDateTime::currentDateTime());
}

/********************       录入数据           ***********************/
void PatientStudyRegister::on_actionSavePatientInfo_triggered()
{
    StudyData orderdata;
    orderdata.PatientID = ui->m_PatientID->text();
    orderdata.PatientHisID = ui->m_HisID->text();
    orderdata.PatientName = ui->m_PatientName->text();
    orderdata.StudyAge = ui->m_Age->text();
    orderdata.PatientSex = ui->m_BirthDay->text();
    orderdata.PatientTelNumber = ui->m_Address->text();
    orderdata.PatientNation = ui->m_comNation->currentText();
    orderdata.PatientHometown = ui->m_HomeTown->text();
    orderdata.PatientJob = ui->m_Job->text();
    orderdata.PatientEmail = ui->m_Email->text();

    orderdata.StudyID = ui->m_StudyID->text();
    orderdata.StudyDepart = ui->m_comStudyDepart->currentText();

    orderdata.StudyModality = ui->m_comModality->currentText();
    orderdata.CostType = ui->m_comCostType->currentText();
    orderdata.StudyCost = ui->m_StudyCost->text();
    orderdata.OrderDateTime = ui->m_StudyOrderDate->text();
    orderdata.ScheduledDateTime = ui->m_RegDate->text();
    orderdata.StudyManufacturer = ui->m_StudyManufacturer->text();
    orderdata.RegisterID = ui->m_RegUser->text();

    QString studyescription = ui->m_comStudyContent->currentText();
    studyescription = studyescription + ui->m_StudyDescription->text();
    studyescription = studyescription + "/Important tel:"+ui->m_ImportantTel->text();
    orderdata.StudyDescription = studyescription;

    QJsonObject json;
    json.insert("PatientID",orderdata.PatientID);
    json.insert("PatientHisID",orderdata.PatientHisID);
    json.insert("PatientName",orderdata.PatientName);
    json.insert("StudyAge",orderdata.StudyAge);
    json.insert("PatientSex",orderdata.PatientSex);
    json.insert("PatientTelNumber",orderdata.PatientTelNumber);
    json.insert("PatientNation",orderdata.PatientNation);
    json.insert("PatientHometown",orderdata.PatientHometown);
    json.insert("PatientJob",orderdata.PatientJob);
    json.insert("PatientEmail",orderdata.PatientEmail);
    json.insert("StudyID",orderdata.StudyID);
    json.insert("StudyDepart",orderdata.StudyDepart);
    json.insert("StudyModality",orderdata.StudyModality);

    json.insert("StudyCode","001");///?
    json.insert("StudyType","0");///?
    json.insert("StudyState","1");///?

    json.insert("PatientHistoryTell","HistoryTell");


    json.insert("CostType",orderdata.CostType);
    json.insert("StudyCost",orderdata.StudyCost);
    json.insert("OrderDateTime",orderdata.OrderDateTime);

    json.insert("StudyDateTime",orderdata.OrderDateTime);///?
    json.insert("ProcedureStepStartDate",orderdata.OrderDateTime);///?

    json.insert("ScheduledDateTime",orderdata.ScheduledDateTime);
    json.insert("StudyManufacturer",orderdata.StudyManufacturer);

    json.insert("AETitle",orderdata.StudyManufacturer);///?
    json.insert("InstitutionName",orderdata.StudyManufacturer);///?
    json.insert("StudyModalityIdentity","007");///?

    json.insert("RegisterID",orderdata.RegisterID);
    json.insert("StudyDescription",orderdata.StudyDescription);

    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);

    m_httpSuccess = false;
    m_questType = updateStudyOder;
    m_networkreply = m_networkmanager.post(QNetworkRequest(m_url+"/healthsystem/ris/SaveStudyOrde/"),byteArray); //m_networkmanager.get(QNetworkRequest(m_url));
    connect(m_networkreply, &QIODevice::readyRead, this, &PatientStudyRegister::httpReadyRead);
    connect(m_networkreply, &QNetworkReply::finished, this, &PatientStudyRegister::httpFinished);

}

void PatientStudyRegister::httpFinished()
{
    if (m_networkreply->error())
    {
        m_networkreply->deleteLater();
        m_networkreply = nullptr;
        m_httpSuccess = false;
        return;
    }

    m_networkreply->deleteLater();
    m_networkreply = nullptr;
}

void PatientStudyRegister::httpReadyRead()
{
    QByteArray byteArray = m_networkreply->readAll();
    switch (m_questType)
    {
    case queryStudyOder:
        break;
    case updateStudyOder:
        QString state = byteArray;
        if (state.toUpper() == "OK")
        {
            m_httpSuccess = true;
        }
        break;

    }
    QString state = byteArray;
}

/********************       清除患者信息        ***********************/
void PatientStudyRegister::on_actionClearPatientInfo_triggered()
{
    clearInfo();
}

/********************       清除信息            ***********************/
void PatientStudyRegister::clearInfo()
{
    ui->m_PatientID->clear();
    ui->m_HisID->clear();
    ui->m_Age->clear();
    ui->m_Address->clear();
    ui->m_comNation->setCurrentIndex(0);
    ui->m_HomeTown->clear();
    ui->m_Job->clear();
    ui->m_Email->clear();
    ui->m_StudyID->clear();
    ui->m_comStudyDepart->setCurrentIndex(0);
    ui->m_comModality->setCurrentIndex(0);

    ui->m_comCostType->setCurrentIndex(0);
    ui->m_StudyCost->clear();
    ui->m_StudyOrderDate->clear();
    ui->m_RegDate->clear();
    ui->m_StudyManufacturer->clear();
    ui->m_RegUser->clear();

    ui->m_comStudyContent->setCurrentIndex(0);
    ui->m_StudyDescription->clear();
    ui->m_ImportantTel->clear();



}


//bool PatientStudyRegister::getIsAutoSaveFlage() const
//{
//    return isAutoSaveFlage;
//}

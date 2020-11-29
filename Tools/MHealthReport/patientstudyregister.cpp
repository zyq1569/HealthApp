#include "patientstudyregister.h"
#include "ui_patientstudyregister.h"


#include "httpclient.h"

///-------Json-------------
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <QNetworkReply>
#include <QMessageBox>


PatientStudyRegister::PatientStudyRegister(QWidget *parent) :
    QMainWindow(parent),  ui(new Ui::PatientStudyRegister)
{
    ui->setupUi(this);

    initControl();

    m_questType = nothing;


    connect(ui->addNewStudy,SIGNAL(clicked()),this,SLOT(on_clearStudyInfo()));
    connect(ui->lookupPatients,SIGNAL(clicked()),this,SLOT(on_showStudyInfo()));

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

void PatientStudyRegister::editPatientStudyInfo(PatientStudyOder data, QString studyOrderIdentity)
{
    disconnect(&m_PatientsForm,SIGNAL(editPatientStudyData(PatientStudyOder , QString )),
               this,SLOT(editPatientStudyInfo(PatientStudyOder , QString )));

    const QString id = studyOrderIdentity;
    int rows = data.count;
    for (int i=0; i<rows; i++)
    {
        if (id == data.orderdata[i].studyorder[StudyOrderIdentity].Value)
        {
            ///-------------Patient--------------------------------------------------
            m_PatientIdentity =  data.orderdata[i].studyorder[PatientIdentity].Value;
            ui->m_PatientID->setText(data.orderdata[i].studyorder[PatientID].Value);
            ui->m_HisID->setText(data.orderdata[i].studyorder[PatientHisID].Value);
            ui->m_PatientName->setText(data.orderdata[i].studyorder[PatientName].Value);
            ui->m_Age->setText(data.orderdata[i].studyorder[StudyAge].Value);
            ui->m_comSex->setCurrentText(data.orderdata[i].studyorder[PatientSex].Value);
            ui->m_BirthDay->setText(data.orderdata[i].studyorder[PatientBirthday].Value);
            ui->m_TelNumber->setText(data.orderdata[i].studyorder[PatientTelNumber].Value);
            ui->m_Address->setText(data.orderdata[i].studyorder[PatientAddr].Value);
            ui->m_comNation->setCurrentText(data.orderdata[i].studyorder[PatientNation].Value);
            ui->m_HomeTown->setText(data.orderdata[i].studyorder[PatientHometown].Value);
            ui->m_comMarry->setCurrentText(data.orderdata[i].studyorder[PatientMarriage].Value);
            ui->m_IDCard->setText(data.orderdata[i].studyorder[PatientCarID].Value);
            ui->m_Job->setText(data.orderdata[i].studyorder[PatientJob].Value);
            ui->m_Email->setText(data.orderdata[i].studyorder[PatientEmail].Value);

            ///-------------Study--------------------------------------------------
            ui->m_StudyID->setText(data.orderdata[i].studyorder[StudyID].Value);
            ui->m_ClinicID->setText(data.orderdata[i].studyorder[ClinicID].Value);
            ui->m_comStudyDepart->setCurrentText(data.orderdata[i].studyorder[StudyDepart].Value);
            //ui->m_ClinicID->setText(
            ui->m_comStudyContent->setCurrentText(data.orderdata[i].studyorder[StudyCode].Value);
            ui->m_comModality->setCurrentText(data.orderdata[i].studyorder[StudyModality].Value);
            ui->m_comCostType->setCurrentText(data.orderdata[i].studyorder[CostType].Value);
            ui->m_StudyCost->setText(data.orderdata[i].studyorder[StudyCost].Value);
            ui->m_StudyOrderDate->setDateTime(QDateTime::fromString(
                                                  data.orderdata[i].studyorder[OrderDateTime].Value, "yyyy-MM-dd hh:mm:ss"));
            ui->m_RegDate->setDateTime(QDateTime::fromString(
                                           data.orderdata[i].studyorder[ScheduledDateTime].Value, "yyyy-MM-dd hh:mm:ss"));
            ui->m_StudyManufacturer->setText(data.orderdata[i].studyorder[StudyManufacturer].Value);
            //ui->m_ImportantTel
            ui->m_RegUser->setText(data.orderdata[i].studyorder[RegisterID].Value);

            ui->m_StudyDescription->setText(data.orderdata[i].studyorder[StudyDescription].Value);

            break;
        }
    }
}
void PatientStudyRegister::resetStudy()
{
    m_StudyOrderIdentity = "";
    ui->m_StudyID->clear();
    ui->m_ClinicID->clear();

    ui->m_HospitalID->clear();
    ui->m_StudyCost->clear();

    ui->m_StudyManufacturer->clear();
    ui->m_ImportantTel->clear();
    ui->m_RegUser->clear();
    ui->m_StudyDescription->clear();

    ui->m_comStudyDepart->setEditText("");
    ui->m_comStudyContent->setEditText("");
    ui->m_comModality->setEditText("");
    ui->m_comCostType->setEditText("");
}

void PatientStudyRegister::on_showStudyInfo()
{
    m_PatientsForm.setAttribute(Qt::WA_ShowModal, true);
    m_PatientsForm.setWindowFlags(m_PatientsForm.windowFlags()&  ~Qt::WindowMinimizeButtonHint);
    connect(&m_PatientsForm,SIGNAL(editPatientStudyData(PatientStudyOder , QString )),
            this,SLOT(editPatientStudyInfo(PatientStudyOder , QString )));
    m_PatientsForm.show();
}

void PatientStudyRegister::on_clearStudyInfo()
{
    resetStudy();
}

/********************       录入数据           ***********************/
void PatientStudyRegister::on_actionSavePatientInfo_triggered()
{
    StudyData orderdata;
    orderdata.PatientIdentity = m_PatientIdentity;
    orderdata.StudyOrderIdentity = m_StudyOrderIdentity;
    orderdata.PatientID = ui->m_PatientID->text();
    orderdata.PatientHisID = ui->m_HisID->text();
    orderdata.PatientName = ui->m_PatientName->text();
    orderdata.StudyAge = ui->m_Age->text();
    orderdata.PatientBirthday = ui->m_BirthDay->text();
    orderdata.PatientSex = ui->m_comSex->currentText();
    orderdata.PatientMarriage = ui->m_comMarry->currentText();
    orderdata.PatientCarID = ui->m_IDCard->text();
    orderdata.PatientTelNumber = ui->m_Address->text();
    orderdata.PatientNation = ui->m_comNation->currentText();
    orderdata.PatientHometown = ui->m_HomeTown->text();
    orderdata.PatientAddr = ui->m_Address->text();
    orderdata.PatientJob = ui->m_Job->text();
    orderdata.PatientEmail = ui->m_Email->text();

    orderdata.StudyID = ui->m_StudyID->text();
    orderdata.StudyDepart = ui->m_comStudyDepart->currentText();

    orderdata.StudyModality = ui->m_comModality->currentText();
    orderdata.CostType = ui->m_comCostType->currentText();
    orderdata.StudyCost = ui->m_StudyCost->text();
    orderdata.StudyCode = ui->m_StudyContent->text();
    orderdata.OrderDateTime = ui->m_StudyOrderDate->text();
    orderdata.ScheduledDateTime = ui->m_RegDate->text();
    orderdata.StudyManufacturer = ui->m_StudyManufacturer->text();
    orderdata.RegisterID = ui->m_RegUser->text();

    //QString studyescription = studyescription + "/Important tel:"+ui->m_ImportantTel->text();
    orderdata.StudyDescription = ui->m_comStudyContent->currentText();

    QJsonObject json;
    json.insert("PatientID",orderdata.PatientID);
    json.insert("PatientHisID",orderdata.PatientHisID);
    json.insert("PatientName",orderdata.PatientName);
    json.insert("StudyAge",orderdata.StudyAge);
    json.insert("PatientBirthday",orderdata.PatientBirthday);
    json.insert("PatientSex",orderdata.PatientSex);
    json.insert("PatientMarriage",orderdata.PatientMarriage);
    json.insert("PatientCarID",orderdata.PatientCarID);


    json.insert("PatientTelNumber",orderdata.PatientTelNumber);
    json.insert("PatientNation",orderdata.PatientNation);
    json.insert("PatientHometown",orderdata.PatientHometown);
    json.insert("PatientAddr",orderdata.PatientAddr);

    json.insert("PatientJob",orderdata.PatientJob);
    json.insert("PatientEmail",orderdata.PatientEmail);
    json.insert("StudyID",orderdata.StudyID);
    json.insert("StudyDepart",orderdata.StudyDepart);
    json.insert("StudyModality",orderdata.StudyModality);

    json.insert("StudyCode",orderdata.StudyCode);///?用于对应设备的检查部位编码使用
    json.insert("StudyType","0");///?0影像设备检查
    json.insert("StudyState","1");///?检查状态：-1.标记删除 1.预约 2.等待检查 3.已检查 4.诊断 5.报告审核

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
    m_networkreply = m_networkmanager.post(QNetworkRequest(getServerHttpUrl()+"/healthsystem/ris/SaveStudyOrde/"),byteArray);

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
        if (QMessageBox::question(NULL,
                                  "Save New Patient", "Save New Patient To Server Fail!. try again?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        {
            on_actionSavePatientInfo_triggered();
        }
        return;
    }

    m_networkreply->deleteLater();
    m_networkreply = nullptr;

    if (m_httpSuccess)
    {
        QMessageBox::information(NULL, tr("Save New Patient"),tr("Save OK!"));
    }
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
    clearAllInfo();
}

/********************       清除信息            ***********************/
void PatientStudyRegister::clearAllInfo()
{
    ui->m_PatientName->clear();
    ui->m_PatientID->clear();
    ui->m_BirthDay->clear();
    ui->m_TelNumber->clear();
    ui->m_IDCard->clear();

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
    m_PatientIdentity = "";

    resetStudy();
}



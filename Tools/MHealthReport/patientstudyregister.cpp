#include "patientstudyregister.h"
#include "ui_patientstudyregister.h"

PatientStudyRegister::PatientStudyRegister(QWidget *parent) :
    QMainWindow(parent),  ui(new Ui::PatientStudyRegister)
{
    ui->setupUi(this);

    initControl();
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

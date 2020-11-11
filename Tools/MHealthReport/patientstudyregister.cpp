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
    ui->comboBoxNation->addItem(tr("汉族"));
    ui->comboBoxNation->addItem(tr("回族"));
    ui->comboBoxNation->addItem(tr("蒙古族"));
    ui->comboBoxNation->addItem(tr("藏族"));
    ui->comboBoxNation->addItem(tr("维吾尔族"));
    ui->comboBoxNation->addItem(tr("苗族"));
    ui->comboBoxNation->addItem(tr("彝族"));
    ui->comboBoxNation->addItem(tr("壮族"));
    ui->comboBoxNation->addItem(tr("布依族"));
    ui->comboBoxNation->addItem(tr("朝鲜族"));
    ui->comboBoxNation->addItem(tr("满族"));
    ui->comboBoxNation->addItem(tr("侗族"));
    ui->comboBoxNation->addItem(tr("瑶族"));
    ui->comboBoxNation->addItem(tr("白族"));
    ui->comboBoxNation->addItem(tr("土家族"));
    ui->comboBoxNation->addItem(tr("哈尼族"));
    ui->comboBoxNation->addItem(tr("哈萨克族"));
    ui->comboBoxNation->addItem(tr("傣族"));
    ui->comboBoxNation->addItem(tr("黎族"));
    ui->comboBoxNation->addItem(tr("僳僳族"));
    ui->comboBoxNation->addItem(tr("佤族"));
    ui->comboBoxNation->addItem(tr("畲族"));
    ui->comboBoxNation->addItem(tr("高山族"));
    ui->comboBoxNation->addItem(tr("拉祜族"));
    ui->comboBoxNation->addItem(tr("水族"));
    ui->comboBoxNation->addItem(tr("东乡族"));
    ui->comboBoxNation->addItem(tr("纳西族"));
    ui->comboBoxNation->addItem(tr("景颇族"));
    ui->comboBoxNation->addItem(tr("柯尔克孜族"));
    ui->comboBoxNation->addItem(tr("土族"));
    ui->comboBoxNation->addItem(tr("达斡尔族"));
    ui->comboBoxNation->addItem(tr("仫佬族"));
    ui->comboBoxNation->addItem(tr("羌族"));
    ui->comboBoxNation->addItem(tr("布朗族"));
    ui->comboBoxNation->addItem(tr("撒拉族"));
    ui->comboBoxNation->addItem(tr("毛南族"));
    ui->comboBoxNation->addItem(tr("仡佬族"));
    ui->comboBoxNation->addItem(tr("锡伯族"));
    ui->comboBoxNation->addItem(tr("阿昌族"));
    ui->comboBoxNation->addItem(tr("普米族"));
    ui->comboBoxNation->addItem(tr("塔吉克族"));
    ui->comboBoxNation->addItem(tr("怒族"));
    ui->comboBoxNation->addItem(tr("乌孜别克族"));
    ui->comboBoxNation->addItem(tr("俄罗斯族"));
    ui->comboBoxNation->addItem(tr("鄂温克族"));
    ui->comboBoxNation->addItem(tr("德昂族"));
    ui->comboBoxNation->addItem(tr("保安族"));
    ui->comboBoxNation->addItem(tr("裕固族"));
    ui->comboBoxNation->addItem(tr("京族"));
    ui->comboBoxNation->addItem(tr("塔塔尔族"));
    ui->comboBoxNation->addItem(tr("独龙族"));
    ui->comboBoxNation->addItem(tr("鄂伦春族"));
    ui->comboBoxNation->addItem(tr("赫哲族"));
    ui->comboBoxNation->addItem(tr("门巴族"));
    ui->comboBoxNation->addItem(tr("珞巴族"));
    ui->comboBoxNation->addItem(tr("基诺族"));

    ui->dateTimeEditJieShou->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dateTimeEditJieShou->setDateTime(QDateTime::currentDateTime());

    ui->dateTimeEditSongJian->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->dateTimeEditSongJian->setDateTime(QDateTime::currentDateTime());
}

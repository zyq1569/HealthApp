#include "patientsform.h"
#include "ui_patientsform.h"


PatientsForm::PatientsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientsForm)
{
    ui->setupUi(this);
    m_httpclient = nullptr;

    QStringList strs = {"patientId", "PatientHisID","patientName","StudyAge", "patientSex",
                        "Birthday","PatientCarID","StudyState","StudyCode","OrderTime",
                        "StudyID","ClinicID","RegisterID","Description",
                        "PatientTelNumber","PatientAddr","PatientNation","PatientHometown",
                        "PatientMarriage","PatientEmail",
                        "StudyDepart","StudyModality","CostType",
                        "StudyCost","ProcedureStepStartDate",
                        "StudyManufacturer","StudyOrderIdentity"};

    ui->m_PatientsTable->setColumnCount(strs.count());
    ui->m_PatientsTable->setHorizontalHeaderLabels(strs);
    ui->m_PatientsTable->horizontalHeader()->setStretchLastSection(true);
    ui->m_PatientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);//select rows
    ui->m_PatientsTable->setSelectionMode(QAbstractItemView::SingleSelection);///single rows
    ui->m_PatientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);// no edit
    ui->m_PatientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列
    ui->m_PatientsTable->verticalHeader()->setFixedWidth(27);
    ui->m_PatientsTable->horizontalHeader()->setHighlightSections(false);
    for (int i = 14, c=ui->m_PatientsTable->columnCount(); i<c ; i++)
    {
        ui->m_PatientsTable->setColumnHidden(i,true);
    }
    ui->m_PatientsTable->show();
    connect(ui->m_PatientsTable,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(editPatient(int,int)));
    connect(ui->lookupPatient,SIGNAL(clicked()),this,SLOT(getPatients()));

}

void PatientsForm::getPatients()
{
    QString startDate = ui->dateTimeStart->dateTime().toString("yyyyMMdd");
    QString endDate   = ui->dateTimeEnd->dateTime().toString("yyyyMMdd");
    QString mod = ui->comModality->currentText();

    if (!m_httpclient)
    {
        m_httpclient = new HttpClient(this,getDownDir());
        m_httpclient->setHost(getServerHttpUrl());
    }
    connect(m_httpclient,&HttpClient::parseDataFinished,this,&PatientsForm::showPatients);
    m_httpclient->getStudyDBinfo(getServerHttpUrl(),startDate,endDate,"1","100");

}
//QStringList strs = {"patientId", "PatientHisID","patientName","StudyAge", "patientSex",
//"Birthday","PatientCarID","StudyState","StudyCode","OrderTime",
//"StudyID","ClinicID","RegisterID","Description",
//"PatientTelNumber","PatientAddr","PatientNation","PatientHometown",
//"PatientMarriage","PatientEmail",
//"StudyDepart","StudyModality","CostType",
//"StudyCost","ProcedureStepStartDate",
//"StudyManufacturer"};
void  PatientsForm::showPatients()
{
    disconnect(m_httpclient,&HttpClient::parseDataFinished,this,&PatientsForm::showPatients);
    if (m_httpclient)
    {
        int oldrow = ui->m_PatientsTable->rowCount();
        int oldcol = ui->m_PatientsTable->columnCount();
        for (int i = 0;i < oldrow;i++)
        {
            for (int j = 0;j < oldcol;j++)
            {
                QTableWidgetItem *item = ui->m_PatientsTable->item(i,j);
                if (item)
                {
                    delete item;
                }
            }
        }
        ui->m_PatientsTable->setRowCount(0);
        m_currentData.orderdata.clear();
        PatientStudyOder *StudyOder = m_httpclient->getPatientStudyOder();
        m_currentData.msg = StudyOder->msg;
        m_currentData.ver = StudyOder->ver;
        m_currentData.code = StudyOder->code;

        int rowcount = StudyOder->count;
        m_currentData.count = rowcount;
        ui->m_PatientsTable->setRowCount(rowcount);
        for (int row=0; row<rowcount; row++)
        {
            m_currentData.orderdata.push_back(StudyOder->orderdata[row]);
            int column=0;
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientID].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientHisID].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientName].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyAge].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientSex].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientBirthday].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientCarID].Value));
            QString state = StudyOder->orderdata[row].studyorder[StudyState].Value;
            if (state == "1")      {                state = "已预约";  }
            else if (state == "2") {                state = "待检查";  }
            else if (state == "3") {                state = "已检查";  }
            else if (state == "4") {                state = "诊断";    }
            else if (state == "5") {                state = "报告审核";}
            else                   {                state = "未知";    }
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(state));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyCode].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[OrderDateTime].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyID].Value));

            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[ClinicID].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[RegisterID].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyDescription].Value));
            //"PatientTelNumber","PatientAddr","PatientNation","PatientHometown",
            //"PatientMarriage","PatientEmail",
            //"StudyDepart","StudyModality","CostType",
            //"StudyCost","ProcedureStepStartDate",
            //"StudyManufacturer","StudyOrderIdentity"};
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientTelNumber].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientAddr].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientNation].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientHometown].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientMarriage].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientEmail].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyDepart].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyModality].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[CostType].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyCost].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[ProcedureStepStartDate].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyManufacturer].Value));
            ui->m_PatientsTable->setItem(row,column++,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyOrderIdentity].Value));
        }
    }
}

void PatientsForm::editPatient(int row, int column)
{
    QString StudyOrderIdentity = ui->m_PatientsTable->item(row,ui->m_PatientsTable->columnCount()-1)->text();
    emit editPatientStudyData(m_currentData,StudyOrderIdentity);
    this->hide();
}


PatientsForm::~PatientsForm()
{
    delete ui;
}

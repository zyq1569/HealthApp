#include "patientsform.h"
#include "ui_patientsform.h"


#include "httpclient.h"

PatientsForm::PatientsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientsForm)
{
    ui->setupUi(this);


    QStringList strs = {"patientId", "PatientHisID","patientName","StudyAge", "patientSex",
                        "Birthday","PatientCarID","StudyState","StudyCode","OrderTime",
                        "StudyID","ClinicID","RegisterID","Description",
                        "PatientTelNumber","PatientAddr","PatientNation","PatientHometown",
                        "PatientMarriage","PatientEmail",
                        "StudyDepart","StudyModality","CostType",
                        "StudyCost","ProcedureStepStartDate",
                        "StudyManufacturer"};

    ui->m_PatientsTable->setColumnCount(strs.count());

    ui->m_PatientsTable->setHorizontalHeaderLabels(strs);
    ui->m_PatientsTable->horizontalHeader()->setStretchLastSection(true);

    ui->m_PatientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);//select rows
    ui->m_PatientsTable->setSelectionMode(QAbstractItemView::SingleSelection);///single rows
    ui->m_PatientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);// no edit
    ui->m_PatientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列
    ui->m_PatientsTable->verticalHeader()->setFixedWidth(20);
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

//        QStringList strs = {"patientId", "PatientHisID","patientName","StudyAge", "patientSex",
//                            "Birthday","PatientCarID","StudyState","StudyCode","OrderTime",
//                            "StudyID","ClinicID","RegisterID","Description",
//
//                            "PatientTelNumber","PatientAddr","PatientNation","PatientHometown",
//                            "PatientMarriage","PatientEmail",
//                            "StudyDepart","StudyModality","CostType",
//                            "StudyCost","ProcedureStepStartDate",
//                            "StudyManufacturer"};
        PatientStudyOder *StudyOder = m_httpclient->getPatientStudyOder();
        int rowcount = StudyOder->count;
        ui->m_PatientsTable->setRowCount(rowcount);
        for (int row=0; row<rowcount; row++)
        {
            ui->m_PatientsTable->setItem(row,0,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientID].Value));
            ui->m_PatientsTable->setItem(row,1,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientName].Value));
            ui->m_PatientsTable->setItem(row,2,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientSex].Value));
            ui->m_PatientsTable->setItem(row,3,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientBirthday].Value));
            QString state = StudyOder->orderdata[row].studyorder[StudyState].Value;
            if (state == "1")
            {
                state = "已预约";
            }
            else if (state == "2")
            {
                state = "待检查";
            }
            else if (state == "3")
            {
                state = "已检查";
            }
            else if (state == "4")
            {
                state = "诊断";
            }
            else if (state == "5")
            {
                state = "报告审核";
            }
            else
            {
                state = "未知";
            }
            ui->m_PatientsTable->setItem(row,4,new QTableWidgetItem(state));
            ui->m_PatientsTable->setItem(row,5,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyModality].Value));
            ui->m_PatientsTable->setItem(row,6,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyDescription].Value));
            ui->m_PatientsTable->setItem(row,7,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyUID].Value));

            ui->m_PatientsTable->setItem(row,8,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientIdentity].Value));
            ui->m_PatientsTable->setItem(row,9,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[PatientID].Value));
            ui->m_PatientsTable->setItem(row,10,new QTableWidgetItem(StudyOder->orderdata[row].studyorder[StudyOrderIdentity].Value));

        }
        ui->m_PatientsTable->setColumnHidden(ui->m_PatientsTable->columnCount()-1,true);
        ui->m_PatientsTable->setColumnHidden(ui->m_PatientsTable->columnCount()-2,true);
        ui->m_PatientsTable->setColumnHidden(ui->m_PatientsTable->columnCount()-3,true);
        ui->m_PatientsTable->setColumnHidden(ui->m_PatientsTable->columnCount()-4,true);
    }
}

void PatientsForm::editPatient(int row, int column)
{

}


PatientsForm::~PatientsForm()
{
    delete ui;
}

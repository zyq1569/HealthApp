#include "patientsform.h"
#include "ui_patientsform.h"

PatientsForm::PatientsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientsForm)
{
    ui->setupUi(this);


    QStringList strs = {"patientId", "PatientHisID","patientName","StudyAge", "patientSex",
                        "Birthday","PatientCarID","StudyCode","OrderTime",
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
    for (int i = 13, c=ui->m_PatientsTable->columnCount(); i<c ; i++)
    {
        ui->m_PatientsTable->setColumnHidden(i,true);
    }
    //ui->m_PatientsTable->setColumnHidden(0,false);11 12 13 14 15 19 22

    ui->m_PatientsTable->show();
}

PatientsForm::~PatientsForm()
{
    delete ui;
}

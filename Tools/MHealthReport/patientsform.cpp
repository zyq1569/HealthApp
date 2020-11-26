#include "patientsform.h"
#include "ui_patientsform.h"

PatientsForm::PatientsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientsForm)
{
    ui->setupUi(this);


    QStringList strs = {"patientId", "PatientHisID","patientName","StudyAge", "patientSex", "patientBirthday",
                        "PatientTelNumber","PatientAddr","PatientNation","PatientHometown","PatientMarriage",
                        "PatientCarID","PatientEmail","StudyID","ClinicID",
                        "StudyDepart","StudyCode","StudyModality","CostType",
                        "StudyCost","OrderDateTime","ProcedureStepStartDate","StudyManufacturer",
                        "RegisterID","StudyDescription"};

    ui->m_PatientsTable->setColumnCount(strs.count());

    ui->m_PatientsTable->setHorizontalHeaderLabels(strs);
    ui->m_PatientsTable->horizontalHeader()->setStretchLastSection(true);

    ui->m_PatientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);//select rows
    ui->m_PatientsTable->setSelectionMode(QAbstractItemView::SingleSelection);///single rows
    ui->m_PatientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);// no edit
    ui->m_PatientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均分列
    ui->m_PatientsTable->verticalHeader()->setFixedWidth(20);
    ui->m_PatientsTable->horizontalHeader()->setHighlightSections(false);
    ui->m_PatientsTable->show();
}

PatientsForm::~PatientsForm()
{
    delete ui;
}

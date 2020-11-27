#ifndef PATIENTSFORM_H
#define PATIENTSFORM_H

#include "patientdata.h"
#include "httpclient.h"

#include <QWidget>


class HttpClient;


namespace Ui {
class PatientsForm;
}

class PatientsForm : public QWidget
{
    Q_OBJECT

public:
    explicit PatientsForm(QWidget *parent = nullptr);
    ~PatientsForm();

signals:
    void editPatientStudyData(PatientStudyOder data, QString id);

private:
    HttpClient *m_httpclient;
    PatientStudyOder m_currentData;


public slots:
    void editPatient(int row, int column);
    void getPatients();
    void showPatients();

private:
    Ui::PatientsForm *ui;
};

#endif // PATIENTSFORM_H

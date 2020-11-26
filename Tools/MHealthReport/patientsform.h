#ifndef PATIENTSFORM_H
#define PATIENTSFORM_H

#include "patientdata.h"


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

private:
    Ui::PatientsForm *ui;
};

#endif // PATIENTSFORM_H

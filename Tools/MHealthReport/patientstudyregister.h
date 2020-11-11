#ifndef PATIENTSTUDYREGISTER_H
#define PATIENTSTUDYREGISTER_H

#include <QMainWindow>

namespace Ui {
class PatientStudyRegister;
}

class PatientStudyRegister : public QMainWindow
{
    Q_OBJECT

public:
    explicit PatientStudyRegister(QWidget *parent = nullptr);
    ~PatientStudyRegister();

public:
    void initControl();

private:
    Ui::PatientStudyRegister *ui;
};

#endif // PATIENTSTUDYREGISTER_H

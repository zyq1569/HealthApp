#ifndef PATIENTSTUDYREGISTER_H
#define PATIENTSTUDYREGISTER_H

#include "patientsform.h"
#include "patientdata.h"

#include <QMainWindow>
#include <QNetworkAccessManager>


namespace Ui {
class PatientStudyRegister;
}

class HttpClient;

class PatientStudyRegister : public QMainWindow
{
    Q_OBJECT

public:
    explicit PatientStudyRegister(QWidget *parent = nullptr);
    ~PatientStudyRegister();

public:
    void initControl();
    void clearInfo();

public slots:
    //    void flageChange();                           //内容改变
    void on_actionSavePatientInfo_triggered();    //保存患者信息
    void on_actionClearPatientInfo_triggered();   //清除患者信息
    void on_clearStudyInfo();

private:
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QString m_url;
    QuestType m_questType;
    bool m_httpSuccess;
    PatientsForm m_PatientsForm;


private slots:
    void httpFinished();
    void httpReadyRead();

private:
    Ui::PatientStudyRegister *ui;
};

#endif // PATIENTSTUDYREGISTER_H

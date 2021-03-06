#ifndef PATIENTSTUDYREGISTER_H
#define PATIENTSTUDYREGISTER_H

#include "patientsform.h"
#include "patientdata.h"
#include "httpclient.h"

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
    void clearAllInfo();
    void resetStudy();
    void showMessage(QString str);

public slots:
    void on_actionSavePatientInfo_triggered();    //保存患者信息
    void on_actionClearPatientInfo_triggered();   //清除患者信息
    void on_showStudyInfo();
    void on_clearStudyInfo();
    void editPatientStudyInfo(PatientStudyOder data, QString studyOrderIdentity);

private:
    QNetworkAccessManager m_networkmanager;
    QNetworkReply *m_networkreply;
    QuestType m_questType;
    bool m_httpSuccess;
    PatientsForm m_PatientsForm;
    QString m_PatientIdentity,m_StudyOrderIdentity;

private slots:
    void httpFinished();
    void httpReadyRead();

private:
    Ui::PatientStudyRegister *ui;
};

#endif // PATIENTSTUDYREGISTER_H

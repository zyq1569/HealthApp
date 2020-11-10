#ifndef MAINAPP_H
#define MAINAPP_H

#include <QMainWindow>


class QProcess;
class StudyImage;
class PatientStudyRegister;

QT_BEGIN_NAMESPACE
namespace Ui { class MainApp; }
QT_END_NAMESPACE

class MainApp : public QMainWindow
{
    Q_OBJECT

public:
    MainApp(QWidget *parent = nullptr);
    ~MainApp();


private:
    QProcess *m_QProcess;
    StudyImage *m_StudyImage;
    PatientStudyRegister *m_PatientStudyRegister;

private:
    Ui::MainApp *ui;
};
#endif // MAINAPP_H

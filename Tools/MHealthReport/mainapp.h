#ifndef MAINAPP_H
#define MAINAPP_H

#include <QMainWindow>


class QProcess;
class StudyImage;
class PatientStudyRegister;
class QWebEngineView;

QT_BEGIN_NAMESPACE
namespace Ui { class MainApp; }
QT_END_NAMESPACE

class MainApp : public QMainWindow
{
    Q_OBJECT

public:
    MainApp(QWidget *parent = nullptr);
    ~MainApp();

public slots:
    void TabBarClicked(int index);

private:
    QProcess *m_QProcess;
    StudyImage *m_StudyImage;
    QWebEngineView *m_view;
    PatientStudyRegister *m_PatientStudyRegister;

private:
    Ui::MainApp *ui;
};
#endif // MAINAPP_H

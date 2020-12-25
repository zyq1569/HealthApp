#ifndef MAINAPP_H
#define MAINAPP_H

#include <QMainWindow>
#include <QNetworkCookie>

class QProcess;
class StudyImage;
class PatientStudyRegister;
class QWebEngineView;
class Config;

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
    void lookStudyReport(QString StudyOrderIdentity);
    void slog_cookieAdded(const QNetworkCookie &cookie);
    void saveServerConfig(QString serverIP, QString serverPort);

private:
    QProcess *m_QProcess;
    StudyImage *m_StudyImage;
    QWebEngineView *m_view,*m_imageView;
    Config *m_config;
    PatientStudyRegister *m_PatientStudyRegister;
    QString m_serverIP;
    QString m_serverPort;
    QString m_url;
    int m_imageViewer;

private:
    Ui::MainApp *ui;
};
#endif // MAINAPP_H

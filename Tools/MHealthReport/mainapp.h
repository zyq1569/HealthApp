#ifndef MAINAPP_H
#define MAINAPP_H

#include <QMainWindow>


#define ImageAppName  "StarViewer"


class QProcess;
class QLocalServer;
class QLocalSocket;
class HttpClient;

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
    /// socket IPC
    void connectImageApp();
    void sendToImageAppMsg(QString data);
    void disconnectImageApp();
    void ReadImageApp();
    void connectImageAppCrash();


private slots:
    /// StudyDB info
    void on_m_getStudyDbImages_clicked();

    void updateStudyImageTable();

    void on_m_tableWidget_cellDoubleClicked(int row, int column);

signals:
    void sendClientMsg(QString data);

private:
    QProcess *m_QProcess;
    QLocalSocket *m_localSocket;
    HttpClient *m_httpclient;
    QString m_url;

private:
    Ui::MainApp *ui;
};
#endif // MAINAPP_H

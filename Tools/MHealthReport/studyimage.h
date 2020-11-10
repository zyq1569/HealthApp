#ifndef STUDYIMAGE_H
#define STUDYIMAGE_H

#include <QMainWindow>

#define ImageAppName  "StarViewer"

class QLocalServer;
class QLocalSocket;
class HttpClient;


namespace Ui {
class StudyImage;
}

class StudyImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit StudyImage(QWidget *parent = nullptr);
    ~StudyImage();

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
    QLocalSocket *m_localSocket;
    HttpClient *m_httpclient;
    QString m_url;

private:
    Ui::StudyImage *ui;
};

#endif // STUDYIMAGE_H

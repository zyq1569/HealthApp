#ifndef STUDYIMAGE_H
#define STUDYIMAGE_H

#include <QMainWindow>

#define ImageAppName  "StarViewer"

class QLocalServer;
class QLocalSocket;
class HttpClient;
class QMenu;

namespace Ui
{
class StudyImage;
}

class StudyImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit StudyImage(QWidget *parent = nullptr);
    ~StudyImage();
public:
    void setUrlImage(bool flag);
    void setUrlReport(bool flag);

public slots:
    /// socket IPC
    void connectImageApp();
    void sendToImageAppMsg(QString data);
    void disconnectImageApp();
    void ReadImageApp();
    void connectImageAppCrash();

    void EditReport();
    void EditPatientInfo();
    void ViewImage();

private slots:
    /// StudyDB info
    void on_m_getStudyDbImages_clicked();

    void updateStudyImageTable();

    void on_m_tableWidget_cellDoubleClicked(int row, int column);

    void on_m_tableWidget_customContextMenuRequested(const QPoint &pos);

signals:
    void sendClientMsg(QString data);
    void lookReport(QString data);
    void lookImage(QString data);

signals:
    void sendNumber(QString, QString);

private:
    QLocalSocket *m_localSocket;
    HttpClient *m_httpclient;

    QMenu *m_menu;
    int m_currentRow;
    bool m_urlImage, m_urlReport;

private:
    Ui::StudyImage *ui;
};

#endif // STUDYIMAGE_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define QPROCESSSIZE 4

#define WLMSCPQ 0
#define STORESCPQ 1
#define QUERSCPQ 2
#define DCM2DBQ 3

#include <QMainWindow>

namespace Ui {
class HMainWindow;
}
class QProcess;
class HMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HMainWindow(QWidget *parent = nullptr);
    ~HMainWindow();
public:
    QString m_ExeDir,m_ImageDir;
    QString m_StoreScpName;
    QString m_QuerRScpName;
    QString m_WLMScpName;
    QString m_Dcm2DBName;
    QString m_MysqlServer,m_MysqlDbName,m_MysqlUserName,m_MysqlPWD;
private slots:
    void on_StoreSCP_clicked();

    void on_WLMSCP_clicked();

    void on_QRSCP_clicked();

    void on_Dcm2DB_clicked();

protected:
    QProcess *m_pQProcess[QPROCESSSIZE];//,*m_pStoreQProcess,*m_pQuerQProcess,*m_pWLMQProcess,*m_pDcmQProcess;
    bool m_bstorescp[QPROCESSSIZE];

private:
    Ui::HMainWindow *ui;
};

#endif // MAINWINDOW_H

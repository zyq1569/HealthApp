#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define QPROCESSSIZE 5

#define WLMSCPQ 0
#define STORESCPQ 1
#define QUERSCPQ 2
#define DCM2DBQ 3
#define WEBSER 4

#include <QMainWindow>
#include<QStandardItemModel>
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
    QString m_MysqlServer,m_WebServerName,m_WebServerGoName,m_MysqlDbName,m_MysqlUserName,m_MysqlPWD;
    QString m_WebSerPort;
    QString m_Log4j2Config;
private slots:
    void on_StoreSCP_clicked();

    void on_WLMSCP_clicked();

    void on_QRSCP_clicked();

    void on_Dcm2DB_clicked();

    void on_WebServer_clicked();

    void on_query_clientinfo_doubleClicked(const QModelIndex &index);

    void on_query_clientinfo_clicked(const QModelIndex &index);

    void on_query_add_clicked();

    void on_query_delete_clicked();

    void on_query_modify_clicked();

protected:
    QProcess *m_pQProcess[QPROCESSSIZE];//,*m_pStoreQProcess,*m_pQuerQProcess,*m_pWLMQProcess,*m_pDcmQProcess;
    QStandardItemModel *m_model;
    bool m_bstorescp[QPROCESSSIZE];

private:
    Ui::HMainWindow *ui;
};

#endif // MAINWINDOW_H

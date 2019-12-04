#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    QString ExeDir;
    QString StoreScpName;
    QString QuerRScpName;
    QString WLMScpName;
private slots:
    void on_StoreSCP_clicked();

    void on_WLMSCP_clicked();

    void on_QRSCP_clicked();

protected:
    QProcess *h_qProcess;
    bool h_bstorescp;

private:
    Ui::HMainWindow *ui;
};

#endif // MAINWINDOW_H

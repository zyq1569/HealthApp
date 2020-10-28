#ifndef MAINAPP_H
#define MAINAPP_H

#include <QMainWindow>


class QProcess;

QT_BEGIN_NAMESPACE
namespace Ui { class MainApp; }
QT_END_NAMESPACE

class MainApp : public QMainWindow
{
    Q_OBJECT

public:
    MainApp(QWidget *parent = nullptr);
    ~MainApp();

private slots:
    void on_m_RunApp_clicked();

    void on_m_SendData_clicked();

private:
    QProcess *m_QProcess;
private:
    Ui::MainApp *ui;
};
#endif // MAINAPP_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sqlite3.h"
#include "sender.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QStandardItemModel;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    DicomSender sender;
    QStandardItemModel *m_pMOdel;
public:
    void stopSend();
    void stopScan();
    void startSend(int destination, int threads);

    void loadDestinationList();
    void loadGlobalDestinationList();
    void saveDestinationList();
    void startScan(QString &path);

private slots:
    void on_pbUpdate_clicked();

    void on_pBSend_clicked();

    void on_pBDir_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

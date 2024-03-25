#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    int m_TransferSyntax;
private slots:

    void on_pBdcmpath_clicked();

    void on_decoder_clicked();

    void on_pBshowimage_clicked();

    void on_cbmTransferSyntax_currentIndexChanged(int index);

    void on_dcm2Image_clicked();
    void registerCodecs();
    void registerCleanup();

private:
    Ui::MainWindow *ui;

private:
    QString m_dcmpath,m_pngpath;
};

#endif // MAINWINDOW_H

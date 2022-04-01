#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedMemory>
#include <kxmlguiwindow.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QPushButton;
class QTextEdit;
class QTextList;
class MainWindow : public QMainWindow/*, public KXmlGuiWindow*/
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    QPushButton *pb1,*pb2;
    QTextEdit *pt;
    QTextList *px1,*px2;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

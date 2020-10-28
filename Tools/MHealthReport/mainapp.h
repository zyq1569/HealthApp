#ifndef MAINAPP_H
#define MAINAPP_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainApp; }
QT_END_NAMESPACE

class MainApp : public QMainWindow
{
    Q_OBJECT

public:
    MainApp(QWidget *parent = nullptr);
    ~MainApp();

private:
    Ui::MainApp *ui;
};
#endif // MAINAPP_H

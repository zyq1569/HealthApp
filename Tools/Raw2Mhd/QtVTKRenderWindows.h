#ifndef QTVTKRENDERWINDOWS_H
#define QTVTKRENDERWINDOWS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class QtVTKRenderWindows; }
QT_END_NAMESPACE

class QtVTKRenderWindows : public QMainWindow
{
    Q_OBJECT

public:
    QtVTKRenderWindows(QWidget *parent = nullptr);
    ~QtVTKRenderWindows();

private slots:
  void raw2mhd();

private:
    Ui::QtVTKRenderWindows *ui;
};
#endif // QTVTKRENDERWINDOWS_H

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

public:
	QString m_fileDir, m_filesName, m_extraName, m_numberStyle,m_saveFileName,m_argvstr;
	int m_width, m_hight, m_start, m_end, m_bzip;

public:
	void raw2vtkmhd();

private slots:
  void raw2mhd();

private:
    Ui::QtVTKRenderWindows *ui;
};
#endif // QTVTKRENDERWINDOWS_H

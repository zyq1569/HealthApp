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

public slots:
	void showImage3D();

private:
    Ui::MainWindow *ui;

public:
	QTabWidget *m_workspace;
	QWidget *m_image3D;
	//QAction *m_openAction;
	//QMenu *m_fileMenu;
	//QToolBar *m_mainToolbar;
};
#endif // MAINWINDOW_H

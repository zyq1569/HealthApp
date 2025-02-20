#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class vtkMetaImageReader;
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
	void showImage4Plane();
	void setEnabledQAction();
	void initMetaFile();

private:
    Ui::MainWindow *ui;

public:
	QTabWidget *m_workspace;
	QWidget *m_image3D, *m_image4Plane;

	QAction *m_openAction, *m_show3D, *m_show4Plane;
	QMenu *m_fileMenu;
	vtkMetaImageReader *m_MetaReader;
	QToolBar *m_mainToolbar;
	int m_index3D, m_index4P;
	bool m_closeMetaFile;
};
#endif // MAINWINDOW_H

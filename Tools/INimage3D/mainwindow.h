#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "configForm.h"

class vtkMetaImageReader;
class QProgressData;

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

public:
	void starViewer();

private:
    Ui::MainWindow *ui;

public:
	QTabWidget *m_workspace;
	QWidget *m_image3D, *m_image4Plane;

	QAction *m_openAction, *m_show3D, *m_show4Plane, *m_cfigQA;
	QMenu *m_fileMenu;
	vtkMetaImageReader *m_MetaReader;
	QProgressData *m_qProgressBar;
	QToolBar *m_mainToolbar;
	int m_index3D, m_index4P;
	bool m_closeMetaFile;


	ConfigForm *m_configForm;
	bool m_checkStart3D, m_checkStart4Plane, m_check3Dcolor, m_checkDefaultWL;
	int m_DefaultLevel, m_DefaultWindow;// = 862 = 1528
};
#endif // MAINWINDOW_H

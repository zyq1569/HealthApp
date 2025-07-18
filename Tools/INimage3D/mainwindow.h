#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "configForm.h"

class vtkMetaImageReader;
class QProgressData;
class vtkImageData;
class vtkXMLImageDataReader;
class vtkAlgorithmOutput;

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
	void showImage4Plane();
	void setEnabledQAction();
	void initMetaFile();

public:
	void starViewer();
    //++++
    void autoRun();
    void miniTrayIcon();
    QAction *m_QuitAction;
    QMenu *m_TrayIconMenu;
    //++

private:
    Ui::MainWindow *ui;

public:
	QTabWidget *m_workspace;
	QWidget  *m_image4Plane;

	QAction *m_openAction, *m_show3D, *m_show4Plane, *m_cfigQA, *m_editor, *m_saveImage,*m_splitImageData;
	QMenu *m_fileMenu;
	vtkMetaImageReader *m_MetaReader;
	vtkXMLImageDataReader *m_XMLImageDataReader;
	vtkImageData *m_vtkImageData;
	vtkAlgorithmOutput *m_vtkAlgorithmOutput;
	QProgressData *m_qProgressBar;
	QToolBar *m_mainToolbar;
	QString m_commdFilePath,m_commdParmIniFilePath;
	int m_index3D, m_index4P;
	bool m_closeMetaFile;


	ConfigForm *m_configForm;
	bool m_checkStart3D, m_checkStart4Plane, m_check3Dcolor, m_checkDefaultWL, m_checkOpacity,m_colorGradient,m_sampleDistanceCheck;
	int m_DefaultLevel, m_DefaultWindow, m_cbInterType;
    double m_sampleDistance;

    //add 体数据XYZ 信息
    int m_numberX, m_numberY, m_numberZ;

//protected:
//    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H

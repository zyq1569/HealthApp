#ifndef QFOURPANEVIEWER_H
#define QFOURPANEVIEWER_H

#include "qeditorbyvalues.h"
#include "vtkDistanceWidget.h"
#include "vtkImagePlaneWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkSmartPointer.h"

class vtkGenericOpenGLRenderWindow;
class vtkResliceCursorCallback;
class vtkCellPicker;
class MainWindow;

class vtkImageMarchingCubes;
class vtkSmartVolumeMapper;
class vtkColorTransferFunction;
class vtkPolyDataMapper;
class vtkVolumeProperty;
class vtkVolume;
class vtkPiecewiseFunction;
class MainWindow;
class VtkColorGradient;

//class vtkGPUVolumeRayCastMapper;
#include <QWidget>

namespace Ui {
class QFourpaneviewer;
}

class QFourpaneviewer : public QWidget
{
    Q_OBJECT

public:
    explicit QFourpaneviewer(QWidget *parent = nullptr);

    ~QFourpaneviewer();

private:
    Ui::QFourpaneviewer *ui;

public slots:
	void ResetViewer();
	void ResetColor3D(VtkColorStyle colorValue);
    void UpdateColorGradient3D(VtkColorStyle colorValue);

signals:
    void LoadConfigFiles();

public:
	QString m_fileNameMhd;
	vtkResliceImageViewer*        m_resliceImageViewer[3];
	vtkImagePlaneWidget*          m_planeWidget[3];
	vtkGenericOpenGLRenderWindow* m_renderWindow[3], *m_2DViewRenderWindow;
	vtkResliceCursorCallback*     m_resliceCallback;
	vtkCellPicker*                m_cellPicker;
	vtkProperty*                  m_ipwProp;
    vtkRenderer*                  m_ren;
	QAction *m_actionReset;
	double m_defaultLevel;
	double m_defaultWindow;

	void INimage3D();

	void Show3DPlane();

	void ShowEditorsWidget();

    void SavePaneImage();


	//show 3D
public:
	MainWindow *m_MainWindow;
	/// The main mapper for volume rendering.
	vtkSmartVolumeMapper *m_volumeMapper;
	/// Properties of volume rendering.
	vtkVolumeProperty *m_volumeProperty;
	/// The volume actor.
	vtkVolume *m_vtkVolume;

	/// The filter to compute isosurfaces.
	vtkImageMarchingCubes *m_isosurfaceFilter;
	/// The actor for isosurfaces.
	vtkActor *m_isosurfaceActor;
	vtkRenderer *m_renderer;
	vtkPolyDataMapper* m_isosurfaceMapper;
	vtkPiecewiseFunction *m_pieceF, *m_pieceGradF;
	vtkColorTransferFunction *m_colorTranF;
	VtkColorGradient *m_vtkColorGradient;

	bool m_showEditors;

public:
    //绘制灰度直方图的读取参数
    int m_imageGrayHis[4096] = { 0 };
    int m_imageGrayHisMaxValue, m_imageGrayHisMaxindex;
    //灰值小于0的数量,  大于4096数量,    灰度值最大,       灰度最小值       灰度值中灰度值数量最大数及灰度值
    int m_lValue = -1,m_hValue = -1, m_maxGray = -1, m_minGray = -1, m_grayMaxOfNumbers = 0, m_grayMaxOfNumbersIndex = 0;

    //像素总点数
    int m_numberPixels;

public:
	void INshowVolume3D();

	Ui::QFourpaneviewer * GetUI() { return ui; };
};

#endif // QFOURPANEVIEWER_H

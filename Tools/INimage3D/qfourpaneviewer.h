#ifndef QFOURPANEVIEWER_H
#define QFOURPANEVIEWER_H


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

	MainWindow* m_mainwindow;
	int m_defaultLevel;
	int m_defaultWindow;

	void INimage3D();

	void Show3DPlane();

	MainWindow *m_MainWindow;

	//show 3D
public:
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

	void INshowVolume3D();

	Ui::QFourpaneviewer * GetUI() { return ui; };
};

#endif // QFOURPANEVIEWER_H

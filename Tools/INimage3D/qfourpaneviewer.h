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

class vtkGPUVolumeRayCastMapper;
class vtkMarchingCubes;
class vtkPolyDataMappers;
class vtkCornerAnnotation;

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
	void Update3DColorByPointEditor(VtkColorStyle colorValue);
    void Update3DColorByCoordinate(VtkColorStyle colorValue);

signals:
    void LoadConfigFiles();

public:
	QString m_fileNameMhd;

public:
	vtkResliceImageViewer*        m_resliceImageViewer[3];
	vtkImagePlaneWidget*          m_planeWidget[3];
	vtkGenericOpenGLRenderWindow* m_renderWindow[3], *m_2DViewRenderWindow;
	vtkResliceCursorCallback*     m_resliceCallback;
	vtkCellPicker*                m_cellPicker;
	vtkProperty*                  m_ipwProp;
    vtkRenderer*                  m_ren;
    vtkCornerAnnotation*          m_cornerAts[3];
	QAction *m_actionReset;
	double m_defaultLevel;
	double m_defaultWindow;

    void ShowImagePlaneAnd3D();

	void ShowImagePlane();

    void ShowImage3D();

	void ShowEditorsWidget();

    void SaveImagePaneBMP();

	//show 3D
public:
	MainWindow *m_MainWindow;
	/// The main mapper for volume rendering.
    //vtkGPUVolumeRayCastMapper* m_volumeMapper;//
    vtkSmartVolumeMapper *m_volumeMapper;
	/// Properties of volume rendering.
	vtkVolumeProperty *m_volumeProperty;
	/// The volume actor.
	vtkVolume *m_vtkVolume;

	/// The filter to compute isosurfaces.
	vtkImageMarchingCubes *m_isosurfaceFilter;
	/// The actor for isosurfaces.
	vtkActor *m_isosurfaceActor;
    bool m_bremoveActor;
	vtkRenderer *m_renderer;
	vtkPolyDataMapper* m_isosurfaceMapper;
	vtkPiecewiseFunction *m_pieceF, *m_pieceGradF;
	vtkColorTransferFunction *m_colorTranF;
	VtkColorGradient *m_vtkColorGradient;
	bool m_showEditors;

public:
    //���ƻҶ�ֱ��ͼ�Ķ�ȡ����
    int m_imageGrayHis[4096] = { 0 };
    int m_imageGrayHisMaxValue, m_imageGrayHisMaxindex;
    //��ֵС��0������,  ����4096����,    �Ҷ�ֵ���,       �Ҷ���Сֵ       �Ҷ�ֵ�лҶ�ֵ������������Ҷ�ֵ
    int m_lValue = -1,m_hValue = -1, m_maxGray = -1, m_minGray = -1, m_grayMaxOfNumbers = 0, m_grayMaxOfNumbersIndex = 0;

    //�����ܵ���
    int m_numberPixels;

public:

	Ui::QFourpaneviewer * GetUI() { return ui; };
};

#endif // QFOURPANEVIEWER_H

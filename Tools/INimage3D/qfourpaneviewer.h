#ifndef QFOURPANEVIEWER_H
#define QFOURPANEVIEWER_H

#include "qeditorbyvalues.h"
#include "vtkDistanceWidget.h"
#include "vtkImagePlaneWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"

#include "volumedataset.h"

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
class vtkExtractVOI;

class classvtkCellArray;
class classvtkPolyData;
class classvtkPolyDataMapper2D;
class classvtkActor2D;
class classvtkPoints;

#include <QWidget>

namespace Ui {
    class QFourpaneviewer;
}

class vtkResliceImageViewerP :public vtkResliceImageViewer
{
public:
    ///@{
    /**
     * Standard VTK methods.
     */
    static vtkResliceImageViewerP* New();
    vtkTypeMacro(vtkResliceImageViewerP, vtkResliceImageViewer);
    //void PrintSelf(ostream& os, vtkIndent indent) override;
    ///@}
      /**
   * Increment/Decrement slice by 'inc' slices
   */
    void InOrDecrementSlice(int inc);

    int GetIncrementIndex()
    {
        return m_IncrementIndex;
    }

protected:
    vtkResliceImageViewerP()
    {
        m_IncrementIndex = 0;
    }
    ~vtkResliceImageViewerP()
    {

    }
public:
    int m_IncrementIndex;
};

///++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

    void SplitImageData(int *dims, int start, int end);
    void SaveRectangleImageParm(int orientation, int w, int h, int dx, int dy, int angle = 0);
    void SaveObliquerRectangleImageParm(int orientation, int w, int h, int dx, int dy, int angle = 0);

signals:
    void LoadConfigFiles();

public:
    QString m_fileNameMhd;

public:
    vtkResliceImageViewerP*       m_resliceImageViewer[3];
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
    //显示拆分体数据及设置矩形框等参数的界面
    void ShowEditorSplitImageData();

    //show 3D

    //draw
    void DrawRectangleAxisAlignedPlane(vtkImagePlaneWidget* planeWidget, vtkImageData* imageData, vtkRenderer* renderer, int w, int h, double deltaX = 0, double deltaY = 0, int inc = 0, int angle = 0);
    void DrawRectangleObliquerPlane(vtkImagePlaneWidget* planeWidget, vtkImageData* imageData, vtkRenderer* renderer, int w, int h, double deltaX = 0, double deltaY = 0, int inc = 0, int angle = 0);
    void SaveRectangleImageTIFF(vtkResliceImageViewer* vtkResliceViewer, double *p1, double *p2);
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

    //0430
    vtkImageData *m_showImageData, *m_topImageData, *m_centerImageData, *m_bottomImageData;
    vtkExtractVOI *m_extractVOI;
    vtkAlgorithmOutput  *m_vtkAlgorithm;
    VolumeDataSet *m_volumeDataSet;

    //vtkCellArray        *m_lines;
    //vtkPolyData         *m_polyData;
    //vtkPolyDataMapper2D *m_mapper;
    //vtkActor2D          *m_actor;
    //vtkPoints           *m_points;
public:
    //绘制灰度直方图的读取参数
    int m_imageGrayHis[4096] = { 0 };
    int m_imageGrayHisMaxValue, m_imageGrayHisMaxindex;
    //灰值小于0的数量,  大于4096数量,    灰度值最大,       灰度最小值       灰度值中灰度值数量最大数及灰度值
    int m_lValue = -1, m_hValue = -1, m_maxGray = -1, m_minGray = -1, m_grayMaxOfNumbers = 0, m_grayMaxOfNumbersIndex = 0;

    //像素总点数
    int m_numberPixels;

public:

    Ui::QFourpaneviewer * GetUI() { return ui; };
};

#endif // QFOURPANEVIEWER_H

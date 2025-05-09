#include "qfourpaneviewer.h"
#include "ui_qfourpaneviewer.h"

#include "mainwindow.h"
///---------------------------
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDICOMImageReader.h"
#include "vtkDistanceRepresentation.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkDistanceWidget.h"
#include "vtkHandleRepresentation.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageSlabReslice.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLookupTable.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkResliceCursor.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkResliceCursorWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkMetaImageReader.h>
#include <vtkCamera.h>

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkMarchingCubes.h>
#include <vtkCornerAnnotation.h>

#include <vtkExtractVOI.h>
//-------------showVolume3D
#include <vtkSmartVolumeMapper.h>
#include <vtkRayCastImageDisplayHelper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageMarchingCubes.h>
#include <vtkVolumeProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkBMPWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkcolorgradient.h>
#include <vtkWindowToImageFilter.h> //save image
#include <vtkImageShiftScale.h>
#include <vtkMatrix4x4.h>
//show rect
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>

#include <vtkAutoInit.h>
#include <vtkImageCast.h>
#include <vtkImageResliceMapper.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

///////////////////////////////////
#include <ThreadWeaver/ThreadWeaver>
//--------------
#include <QAction>
#include <QDateTime>
#include <QFileDialog>

using namespace ThreadWeaver;

template<typename T> const T& clamp(const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

template<typename T> T clampValue(T val, T minVal, T maxVal)
{
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

class Volume3DJob : public Job
{
public:
    Volume3DJob(QFourpaneviewer *fourpaneviewer = nullptr) : m_fourPaneviewer(fourpaneviewer)
    {}
protected:
    void run(JobPointer, Thread*) Q_DECL_OVERRIDE
    {
        if (!m_fourPaneviewer)
            return;

        m_fourPaneviewer->ShowImage3D();
    }
private:
    QFourpaneviewer *m_fourPaneviewer;
};

class VolumeGrayHistogramJob :public Job
{
public:
    VolumeGrayHistogramJob(QFourpaneviewer *parent = nullptr, vtkImageData *imageData = nullptr) :m_fourPaneviewer(parent), m_imageData(imageData)
    {}

protected:
    void run(JobPointer, Thread*) Q_DECL_OVERRIDE
    {
        if (!m_fourPaneviewer || !m_imageData)
            return;

        //加载灰值数据--[-1,4096]
        vtkImageData *image = m_imageData;

        int vl, L = 0, H = 0, max = 0, min = 0;
        double pixelvalue;
        int *dims = image->GetDimensions();
        m_fourPaneviewer->m_numberPixels = dims[0] * dims[1] * dims[2];;
        for (int z = 0; z < dims[2]; z++)
        {
            for (int y = 0; y < dims[1]; y++)
            {
                for (int x = 0; x < dims[0]; x++)
                {
                    pixelvalue = image->GetScalarComponentAsDouble(x, y, z, 0);
                    vl = static_cast<int>(pixelvalue);

                    if (vl < min)
                    {
                        min = vl;
                    }

                    if (vl > max)
                    {
                        max = vl;
                    }

                    if (vl < 0)
                    {
                        L++;
                    }
                    else if (vl > 4096)
                    {
                        H++;
                    }
                    else
                    {
                        m_fourPaneviewer->m_imageGrayHis[vl]++;
                    }
                }
            }
        }
        int star = min, end = max + 1, grayNumbers = 0;
        if (min < 1)
        {
            star = 1;
        }
        for (int i = star; i < end; i++)
        {
            if (m_fourPaneviewer->m_imageGrayHis[i] > grayNumbers)
            {
                m_fourPaneviewer->m_grayMaxOfNumbersIndex = i;
                grayNumbers = m_fourPaneviewer->m_imageGrayHis[i];
            }
        }
        m_fourPaneviewer->m_grayMaxOfNumbers = grayNumbers;
        if (m_fourPaneviewer->m_imageGrayHis[0] > grayNumbers)
        {
            m_fourPaneviewer->m_grayMaxOfNumbers = m_fourPaneviewer->m_imageGrayHis[0] + grayNumbers / 10;
        }
        m_fourPaneviewer->m_lValue = L;
        m_fourPaneviewer->m_hValue = H;
        m_fourPaneviewer->m_maxGray = max;
        m_fourPaneviewer->m_minGray = min;
    };
private:
    QFourpaneviewer *m_fourPaneviewer;
    vtkImageData *m_imageData;
};

//class VolumePlaneJob : public Job
//{
//public:
//	VolumePlaneJob(QFourpaneviewer *Fourpaneviewer = nullptr) : m_Fourpaneviewer(Fourpaneviewer)
//	{}
//protected:
//	void run(JobPointer, Thread*) Q_DECL_OVERRIDE
//	{
//		if (!m_Fourpaneviewer)
//			return;
//
//		m_Fourpaneviewer->INimage3D();
//	}
//private:
//	QFourpaneviewer *m_Fourpaneviewer;
//};
//
#define vtkREP vtkResliceCursorLineRepresentation 
#define DRAW_RECT
class vtkResliceCursorCallback : public vtkCommand
{
public:
    static vtkResliceCursorCallback* New()
    {
        return new vtkResliceCursorCallback;
    }
    void DisplayToSlice(vtkResliceImageViewer* Viewer, int x, int y, double worldOut[3])
    {
        vtkSmartPointer<vtkCoordinate> coord = vtkSmartPointer<vtkCoordinate>::New();
        coord->SetCoordinateSystemToDisplay();
        coord->SetViewport(Viewer->GetRenderer());
        coord->SetValue(x, y);
        double* world = coord->GetComputedWorldValue(Viewer->GetRenderer());

        double origin[3], normal[3];
        Viewer->GetResliceCursor()->GetPlane(Viewer->GetSliceOrientation())->GetOrigin(origin);
        Viewer->GetResliceCursor()->GetPlane(Viewer->GetSliceOrientation())->GetNormal(normal);

        double v[3] = { world[0] - origin[0],   world[1] - origin[1], world[2] - origin[2] };
        double d = vtkMath::Dot(v, normal);
        worldOut[0] = world[0] - d * normal[0];
        worldOut[1] = world[1] - d * normal[1];
        worldOut[2] = world[2] - d * normal[2];
    }

    void DrawRectangle(vtkResliceImageViewer* vtkResliceViewer)
    {
        double p1[3], p2[3];
        DisplayToSlice(vtkResliceViewer, m_startPos[0], m_startPos[1], p1);
        DisplayToSlice(vtkResliceViewer, m_endPos[0], m_endPos[1], p2);

        // 生成矩形的四个顶点（闭合）
        vtkNew<vtkPoints> points;
        points->InsertNextPoint(p1[0], p1[1], p1[2]);
        points->InsertNextPoint(p2[0], p1[1], p1[2]);
        points->InsertNextPoint(p2[0], p2[1], p2[2]);
        points->InsertNextPoint(p1[0], p2[1], p2[2]);
        points->InsertNextPoint(p1[0], p1[1], p1[2]);

        vtkNew<vtkCellArray> lines;
        vtkNew<vtkIdList> ids;
        ids->SetNumberOfIds(5);
        for (int i = 0; i < 5; ++i)
        {
            ids->SetId(i, i);
        }

        lines->InsertNextCell(ids);

        vtkNew<vtkPolyData> polyData;
        polyData->SetPoints(points);
        polyData->SetLines(lines);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(polyData);

        m_Actor = vtkSmartPointer<vtkActor>::New();
        m_Actor->SetMapper(mapper);
        m_Actor->GetProperty()->SetColor(1, 0, 0);
        m_Actor->GetProperty()->SetLineWidth(2);

        vtkResliceViewer->GetRenderer()->AddActor(m_Actor);
        vtkResliceViewer->Render();

        int TargetSlice = vtkResliceViewer->GetSlice();
    }

    void SaveObliquerRectangleImageTIFF(vtkResliceImageViewer* vtkResliceViewer, double p1[3], double p2[3])
    {
        vtkResliceImageViewer* viewer = vtkResliceViewer;
        auto image = viewer->GetInput();
        if (!image)
        {
            return;
        }
        vtkImageData* input = image;

        vtkResliceCursor* cursor = viewer->GetResliceCursor();
        double center[3];
        cursor->GetCenter(center);

        // 获取当前切面方向
        double xAxis[3] = { 0,0,0 }, yAxis[3] = { 0,0,0 }, zAxis[3] = { 0,0,0 };
        double *pxAxis = cursor->GetAxis(0);
        double *pyAxis = cursor->GetAxis(1);
        xAxis[0] = pxAxis[0]; xAxis[1] = pxAxis[1]; xAxis[2] = pxAxis[2];
        yAxis[0] = pyAxis[0];  yAxis[1] = pyAxis[1];  yAxis[2] = pyAxis[2];
        vtkMath::Cross(xAxis, yAxis, zAxis);

        // 👉 翻转 Y 轴，避免图像镜像
        double yAxisFlip[3] = { -yAxis[0], -yAxis[1], -yAxis[2] };

        // 原图 spacing
        double spacingIn[3];
        input->GetSpacing(spacingIn);

        // 计算 oblique spacing
        double spacingU = std::sqrt(
            std::pow(xAxis[0] * spacingIn[0], 2) +
            std::pow(xAxis[1] * spacingIn[1], 2) +
            std::pow(xAxis[2] * spacingIn[2], 2));

        double spacingV = std::sqrt(
            std::pow(yAxisFlip[0] * spacingIn[0], 2) +
            std::pow(yAxisFlip[1] * spacingIn[1], 2) +
            std::pow(yAxisFlip[2] * spacingIn[2], 2));

        // 将 world 点投影到切片平面上
        auto Project = [&](double pt[3], double& u, double& v)
        {
            double vec[3] = { pt[0] - center[0], pt[1] - center[1], pt[2] - center[2] };
            u = vtkMath::Dot(vec, xAxis);
            v = vtkMath::Dot(vec, yAxisFlip);
        };

        double u1, v1, u2, v2;
        Project(p1, u1, v1);
        Project(p2, u2, v2);

        double uMin = std::min(u1, u2);
        double uMax = std::max(u1, u2);
        double vMin = std::min(v1, v2);
        double vMax = std::max(v1, v2);

        // 计算输出图像大小
        int dims[3] =
        {
            static_cast<int>((uMax - uMin) / spacingU + 0.5),
            static_cast<int>((vMax - vMin) / spacingV + 0.5),
            1
        };
        if (dims[0] <= 0 || dims[1] <= 0) return;

        // 构造 ResliceAxes（从切片坐标转到 world 坐标）
        vtkNew<vtkMatrix4x4> resliceAxes;
        for (int i = 0; i < 3; ++i)
        {
            resliceAxes->SetElement(i, 0, xAxis[i]);
            resliceAxes->SetElement(i, 1, yAxisFlip[i]);
            resliceAxes->SetElement(i, 2, zAxis[i]);
            resliceAxes->SetElement(i, 3, center[i] + uMin * xAxis[i] + vMin * yAxisFlip[i]);
        }

        // 创建 Reslicer
        vtkNew<vtkImageReslice> reslicer;
        reslicer->SetInputData(input);
        reslicer->SetResliceAxes(resliceAxes);
        reslicer->SetOutputSpacing(spacingU, spacingV, 1.0);
        reslicer->SetOutputOrigin(0, 0, 0);
        reslicer->SetOutputExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, 0);
        reslicer->SetInterpolationModeToLinear();
        reslicer->Update();

        // 归一化为 8-bit 图像
        vtkImageData* slice = reslicer->GetOutput();
        double range[2];
        slice->GetScalarRange(range);

        vtkNew<vtkImageShiftScale> shiftScale;
        shiftScale->SetInputData(slice);
        shiftScale->SetOutputScalarTypeToUnsignedChar();
        shiftScale->SetShift(-range[0]);
        shiftScale->SetScale(255.0 / (range[1] - range[0]));
        shiftScale->ClampOverflowOn();
        shiftScale->Update();
        vtkNew<vtkPNGWriter> writer;
        writer->SetFileName("RectangleImage_output.png");
        writer->SetInputConnection(shiftScale->GetOutputPort());
        writer->Write();

        vtkNew<vtkTIFFWriter> TIFFwriter;
        TIFFwriter->SetFileName("RectangleImage_output.tiff");
        TIFFwriter->SetInputData(slice);
        TIFFwriter->Write();
    }

    void SaveAxisAlignedRectangleImageTIFF(vtkResliceImageViewer* vtkResliceViewer, int StartPosition[2], int EndPosition[2])
    {
        vtkImageData *Imagedata = vtkResliceViewer->GetInput();
        vtkRenderer *Renderer = vtkResliceViewer->GetRenderer();
        if (!Imagedata || !Renderer)
            return;

        //1: 转换屏幕坐标为世界坐标
        vtkSmartPointer<vtkCoordinate> coord = vtkSmartPointer<vtkCoordinate>::New();
        coord->SetCoordinateSystemToDisplay();

        // 获取矩形对角的两个点
        int x0 = std::min(StartPosition[0], EndPosition[0]);
        int y0 = std::min(StartPosition[1], EndPosition[1]);
        int x1 = std::max(StartPosition[0], EndPosition[0]);
        int y1 = std::max(StartPosition[1], EndPosition[1]);

        //将显示坐标转为世界坐标，再转图像索引坐标
        coord->SetValue(x0, y0);
        double world0[2], world1[2];
        double *wd = coord->GetComputedWorldValue(Renderer);
        world0[0] = wd[0];
        world0[1] = wd[1];
        coord->SetValue(x1, y1);
        wd = coord->GetComputedWorldValue(Renderer);
        world1[0] = wd[0];
        world1[1] = wd[1];

        //2: 世界坐标转为图像索引
        int extent[6];
        Imagedata->GetExtent(extent);

        double spacing[3];
        Imagedata->GetSpacing(spacing);
        double origin[3];
        Imagedata->GetOrigin(origin);

        int imgX0 = static_cast<int>((world0[0] - origin[0]) / spacing[0]);
        int imgY0 = static_cast<int>((world0[1] - origin[1]) / spacing[1]);
        int imgX1 = static_cast<int>((world1[0] - origin[0]) / spacing[0]);
        int imgY1 = static_cast<int>((world1[1] - origin[1]) / spacing[1]);

        imgX0 = clamp(imgX0, extent[0], extent[1]);
        imgX1 = clamp(imgX1, extent[0], extent[1]);
        imgY0 = clamp(imgY0, extent[2], extent[3]);
        imgY1 = clamp(imgY1, extent[2], extent[3]);

        //3: 提取 VOI 区域
        vtkSmartPointer<vtkExtractVOI> extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
        extractVOI->SetInputData(Imagedata);
        extractVOI->SetVOI(
            std::min(imgX0, imgX1), std::max(imgX0, imgX1),
            std::min(imgY0, imgY1), std::max(imgY0, imgY1),
            0, 0// 如果同时提取所有切面VOI区域的数据修改为,应该没什么意义 extent[4], extent[5]
        );
        extractVOI->Update();

        // 4: 保存为 TIFF
        vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
        writer->SetFileName("SelectedRegion.tiff");//不保存为PNG等,tiff直接原始数据.方便后期处理
        writer->SetInputConnection(extractVOI->GetOutputPort());
        writer->Write();

    }
    void Execute(vtkObject* caller, unsigned long ev, void* callData) override
    {
        static bool flag_RESLICE_AXIS_ALIGNED = false;
        if (ev == vtkCommand::LeftButtonDoubleClickEvent)
        {
#ifdef RESLICE_AXIS_ALIGNED
            for (int i = 0; i < 3; i++)
            {
                m_resliceImageViewer[i]->SetResliceMode(flag_RESLICE_AXIS_ALIGNED ? 1 : 0);
                m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
                m_resliceImageViewer[i]->Reset();
                m_resliceImageViewer[i]->Render();
            }
            flag_RESLICE_AXIS_ALIGNED = !flag_RESLICE_AXIS_ALIGNED;
#endif
        }
        else if (ev == vtkCommand::RightButtonPressEvent && m_bSaveRectImage)
        {
            auto interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
            vtkRenderWindow* renderWindow = interactor->GetRenderWindow();
            vtkResliceImageViewer* currentViewer = nullptr;
            //找到对应的 Viewer
            for (int i = 0; i < 3; ++i)
            {
                if (renderWindow == m_resliceImageViewer[i]->GetRenderWindow())
                {
                    currentViewer = m_resliceImageViewer[i];
                    break;
                }
            }
            if (m_ptNum == 0)
            {
                m_startPos[0] = currentViewer->GetInteractor()->GetEventPosition()[0];
                m_startPos[1] = currentViewer->GetInteractor()->GetEventPosition()[1];
                m_ptNum++;
            }
            else
            {
                m_endPos[0] = currentViewer->GetInteractor()->GetEventPosition()[0];
                m_endPos[1] = currentViewer->GetInteractor()->GetEventPosition()[1];
                m_ptNum = 0;

                //方式1：绘制矩形 
#ifdef DRAW_RECT
                vtkSmartPointer<vtkCoordinate> coord = vtkSmartPointer<vtkCoordinate>::New();
                coord->SetCoordinateSystemToDisplay();
                coord->SetViewport(currentViewer->GetRenderer());

                coord->SetValue(m_startPos[0], m_startPos[1]);
                double worldStart[3];
                std::memcpy(worldStart, coord->GetComputedWorldValue(currentViewer->GetRenderer()), sizeof(double) * 3);

                coord->SetValue(m_endPos[0], m_endPos[1]);
                double worldEnd[3];
                std::memcpy(worldEnd, coord->GetComputedWorldValue(currentViewer->GetRenderer()), sizeof(double) * 3);

                vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
                points->InsertNextPoint(worldStart[0], worldStart[1], worldStart[2]);
                points->InsertNextPoint(worldEnd[0], worldStart[1], worldStart[2]);
                points->InsertNextPoint(worldEnd[0], worldEnd[1], worldStart[2]);
                points->InsertNextPoint(worldStart[0], worldEnd[1], worldStart[2]);
                points->InsertNextPoint(worldStart[0], worldStart[1], worldStart[2]); // close loop

                vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
                vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
                ids->SetNumberOfIds(5);
                for (int i = 0; i < 5; ++i)
                {
                    ids->SetId(i, i);
                }

                lines->InsertNextCell(ids);

                vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
                polyData->SetPoints(points);
                polyData->SetLines(lines);

                vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
                mapper->SetInputData(polyData);

                vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
                actor->SetMapper(mapper);
                actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
                actor->GetProperty()->SetLineWidth(2.0);

                currentViewer->GetRenderer()->AddActor(actor);
                currentViewer->GetRenderWindow()->Render();
                int mode = currentViewer->GetResliceMode();
                if (mode == vtkResliceImageViewer::RESLICE_AXIS_ALIGNED)
                {
                    SaveAxisAlignedRectangleImageTIFF(currentViewer, m_startPos, m_endPos);
                }
                else
                {
                    SaveObliquerRectangleImageTIFF(currentViewer, worldStart, worldEnd);
                }

#else
              //方式2：绘制矩形
                DrawRectangle(currentViewer);
                currentViewer->Render();
#endif //   DRAW_RECT        

            }
        }
        else if (ev == vtkCommand::MiddleButtonPressEvent)
        {
            m_bSaveRectImage = !m_bSaveRectImage;
            if (m_bSaveRectImage)
            {
                m_widget[0]->setCursor(Qt::CrossCursor);
                m_widget[1]->setCursor(Qt::CrossCursor);
                m_widget[2]->setCursor(Qt::CrossCursor);
            }

        }
        QString sliceInfo = "";
        if (flag_RESLICE_AXIS_ALIGNED)
        {
            for (int i = 0; i < 3; i++)
            {
                int max = m_resliceImageViewer[i]->GetSliceMax();
                int index = m_resliceImageViewer[i]->GetSlice();
                sliceInfo = QObject::tr("ims: %1/%2 ").arg(index).arg(max);
                m_cornerAts[i]->SetText(2, sliceInfo.toLatin1().constData());
                m_resliceImageViewer[i]->Render();
            }
        }
        else
        {
            for (int i = 0; i < 3; i++)
            {
                sliceInfo = "";
                m_cornerAts[i]->SetText(2, sliceInfo.toLatin1().constData());
                m_resliceImageViewer[i]->Render();
            }
        }
        // Render everything
        for (int i = 0; i < 3; i++)
        {
            RCW[i]->Render();
        }
    }

    vtkResliceCursorCallback()
    {
        m_ptNum = 0;
        m_bSaveRectImage = false;
    }
public:
    vtkResliceCursorWidget*                   RCW[3];
    vtkResliceImageViewer*   m_resliceImageViewer[3];
    vtkCornerAnnotation*     m_cornerAts[3];
    QVTKRenderWidget*        m_widget[3];

    vtkSmartPointer<vtkActor> m_Actor;
private:
    int  m_startPos[2];
    int  m_endPos[2];
    int  m_ptNum;
    bool m_bSaveRectImage;
};

//--------------

QFourpaneviewer::QFourpaneviewer(QWidget *parent) : QWidget(parent), ui(new Ui::QFourpaneviewer)
{
    ui->setupUi(this);
    m_MainWindow = (MainWindow*)parent;

    for (int i = 0; i < 3; i++)
    {
        m_planeWidget[i] = nullptr;
        m_resliceImageViewer[i] = nullptr;
        m_renderWindow[i] = nullptr;
    }
    m_2DViewRenderWindow = nullptr;
    m_resliceCallback = nullptr;
    m_cellPicker = nullptr;
    //
    m_ipwProp = nullptr;
    m_ren = nullptr;
    m_actionReset = nullptr;
    m_vtkColorGradient = nullptr;
    m_volumeDataSet = nullptr;

    m_numberPixels = 0;

    if (m_MainWindow->m_checkDefaultWL)
    {
        m_defaultLevel = m_MainWindow->m_DefaultLevel;
        m_defaultWindow = m_MainWindow->m_DefaultWindow;
    }
    else
    {
        m_defaultLevel = 862;
        m_defaultWindow = 1528;
    }
    //////////////////////////////////////////////////////////////////
    //color 窗体颜色编辑值信号
    connect(ui->m_editorByValues, &QEditorByValues::signalsColorValue, this, &QFourpaneviewer::Update3DColorByPointEditor);
    connect(this, &QFourpaneviewer::LoadConfigFiles, ui->m_editorByValues, &QEditorByValues::loadFileValues);

    /////////////////////////////////////////////////////////////////////

    m_actionReset = new QAction(this);
    addAction(m_actionReset);
    m_actionReset->setShortcut(tr("ctrl+s"));

    connect(m_actionReset, SIGNAL(triggered()), SLOT(ResetViewer()));//Open an existing DICOM folder
    //---------------------showVolume3D----------------------------------
    m_pieceF = nullptr;
    m_pieceGradF = nullptr;
    m_colorTranF = nullptr;

    //m_volumeMapper = vtkGPUVolumeRayCastMapper::New();
    m_volumeMapper = vtkSmartVolumeMapper::New();

    m_isosurfaceFilter = vtkImageMarchingCubes::New();

    m_volumeProperty = vtkVolumeProperty::New();

    m_isosurfaceActor = vtkActor::New();
    m_isosurfaceActor->GetProperty()->SetColor(1.0, 1.0, 1.0);  // 设置（RGB）

    m_renderer = vtkRenderer::New();

    m_vtkVolume = vtkVolume::New();
    m_vtkVolume->SetProperty(m_volumeProperty);
    m_vtkVolume->SetMapper(m_volumeMapper);

    m_isosurfaceMapper = vtkPolyDataMapper::New();// isosurfaceMapper;
    m_isosurfaceMapper->SetInputConnection(m_isosurfaceFilter->GetOutputPort());
    m_isosurfaceActor->SetMapper(m_isosurfaceMapper);

    m_renderer->AddViewProp(m_vtkVolume);

    m_pieceF = vtkPiecewiseFunction::New();
    m_pieceGradF = vtkPiecewiseFunction::New();
    m_colorTranF = vtkColorTransferFunction::New();

    //分层
    m_extractVOI = vtkExtractVOI::New();

    //m_lines = vtkCellArray::New();    
    //m_polyData = vtkPolyData::New();    
    //m_mapper = vtkPolyDataMapper2D::New(); 
    //m_actor = vtkActor2D::New();
    //m_points = vtkPoints::New();
    //add ----ISO
    m_renderer->AddActor(m_vtkVolume);  // 添加体绘制到渲染器
    //m_renderer->AddActor(m_isosurfaceActor);  // 添加等值面到渲染器
    m_bremoveActor = false;
    //-----------------------------------------------------------
    ui->m_SplitterLR->widget(1)->setMaximumWidth(650);
    ui->m_SplitterLR->widget(1)->setMinimumWidth(320);
    //QSplitterHandle* hand = ui->m_SplitterLR->handle(1);
    //if (hand)
    //{
    //	hand->setDisabled(true);
    //}

    ui->m_editorsWidget->hide();
    m_showEditors = false;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ui->m_editorsWidget->setCurrentIndex(1);//设置坐标系方式选择点,另外一个是具体点坐标输入方式设置颜色及透明度
    ui->m_editorsWidget->setStyleSheet("background-color:rgb(240,240,240)}");
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

#define VTKRCP    vtkResliceCursorRepresentation
#define VTKFILTER vtkWindowToImageFilter
void QFourpaneviewer::SaveImagePaneBMP()
{
    QString dir = QCoreApplication::applicationDirPath() + "\\";
    vtkSmartPointer<vtkPNGWriter>  writer = vtkSmartPointer<vtkPNGWriter>::New();
    vtkSmartPointer<vtkJPEGWriter> writerJ = vtkSmartPointer<vtkJPEGWriter>::New();
    vtkSmartPointer<vtkBMPWriter>  writerB = vtkSmartPointer<vtkBMPWriter>::New();
    vtkSmartPointer<vtkTIFFWriter> writerT = vtkSmartPointer<vtkTIFFWriter>::New();

    vtkSmartPointer<vtkImageWriter> imagewriter;

    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetTableRange(0.0, 2048.0);  // 适用于 8 位灰度图像 (0-255)
    lookupTable->SetValueRange(0.0, 1.0);   // 0 = 黑色, 1 = 白色// 设置颜色范围 (黑 -> 白)
    lookupTable->SetSaturationRange(0.0, 0.0); // 0 = 无彩色 (纯灰度)
    lookupTable->Build();
    vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images(*.png );;Images(*.bmp);;images(*.jpg);;images(*.tiff)"));
    QString upperstr = fileName.toUpper();
    bool btiff = false;
    int pos = 4;
    if (upperstr.contains("PNG"))
    {
        imagewriter = writer;
    }
    else if (upperstr.contains("BMP"))
    {
        imagewriter = writerB;
    }
    else if (upperstr.contains("JPG"))
    {
        imagewriter = writerJ;
    }
    else if (upperstr.contains("TIFF"))
    {
        imagewriter = writerT;
        btiff = true;
        pos = 5;
    }
    else
    {
        fileName = dir + QString::number(QDateTime::currentDateTime().toTime_t()) + ".png";
    }
    pos = fileName.length() - pos;
    for (int i = 0; i < 3; i++)
    {
        if (VTKRCP* rep = VTKRCP::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation()))
        {
            if (vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(rep->GetReslice()))
            {
                // default background color is the min value of the image scalar range
                vtkImageData* data = reslice->GetOutput();
                QString imagefileName = fileName;
                QString insertStr = "_" + QString::number(i) + ".";
                imagefileName = imagefileName.replace(pos, 1, insertStr);
                std::string str = qPrintable(imagefileName);
                writer->SetFileName(str.c_str());
                if (btiff)
                {
                    writer->SetInputData(data);
                }
                else
                {
                    mapToColors->SetInputData(data);
                    mapToColors->SetLookupTable(lookupTable); // 需要提前设置适当的查找表
                    mapToColors->Update();
                    writer->SetInputData(mapToColors->GetOutput());
                }
                writer->Update();
                writer->Write();
            }
        }
    }//非斜切面 参考SaveAxisAlignedRectangleImageTIFF 保存数据
    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(ui->m_image3DView->renderWindow());
    windowToImageFilter->Update();
    windowToImageFilter->Modified();
    QString imagefileName = fileName;
    QString insertStr = "_3.";
    imagefileName = imagefileName.replace(pos, 1, insertStr);
    std::string str = qPrintable(imagefileName);
    writer->SetFileName(str.c_str());
    writer->SetInputData(windowToImageFilter->GetOutput());
    writer->Update();
    writer->Write();
}
void QFourpaneviewer::ShowEditorsWidget()
{
    /*

    if (m_MainWindow->m_colorGradient)
    {    */
    //只保留折线调图
    if (!m_vtkColorGradient)
    {
        m_vtkColorGradient = new VtkColorGradient(this);
        m_vtkColorGradient->setGeometry(this->width() / 2, 5, m_vtkColorGradient->width(), m_vtkColorGradient->height());
    }
    if (m_vtkColorGradient)
    {
        (m_vtkColorGradient->isHidden()) ? (m_vtkColorGradient->show()) : (m_vtkColorGradient->hide());
    }
    //ui->m_editorsWidget->hide();
    /*}
    else
    {
        m_showEditors ? (ui->m_editorsWidget->hide()) : (ui->m_editorsWidget->show());
        m_showEditors = !m_showEditors;

        //+++++++++++++++++++++++++++++++++++
        //只有第一次加载文件,需要时再修改
        static bool bfirst = true;
        if (bfirst)
        {
            if (m_showEditors)
            {
                ui->m_editorByValues->setMaximum(m_maxGray + 1);
                ui->m_editorByValues->setMinimum(m_minGray - 1);
                emit LoadConfigFiles();
                bfirst = false;
            }
        }
        //++++++++++++++++++++++++++++++++++++++++

        if (m_vtkColorGradient)
        {
            m_vtkColorGradient->hide();
        }
    }*/
}
void QFourpaneviewer::ShowImagePlaneAnd3D()
{
    Queue queue3D;

    //读取灰度值信息
    QSharedPointer<Job> jobgray(new VolumeGrayHistogramJob(this, m_MainWindow->m_vtkImageData));
    queue3D.enqueue(jobgray);// 将任务加入队列

    QSharedPointer<Job> job(new Volume3DJob(this));
    queue3D.enqueue(job);// 将任务加入队列

    ShowImagePlane();
    ResetViewer();//第一个切图窗口
    ui->m_image3DView->renderWindow()->Render();

}
void QFourpaneviewer::ResetViewer()
{
    for (int i = 0; i < 3; i++)
    {
        m_resliceImageViewer[i]->SetResliceMode(1);
        vtkREP::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation())->SetWindowLevel(m_defaultWindow, m_defaultLevel);
    }
    for (int i = 0; i < 3; i++)
    {
        m_resliceImageViewer[i]->Reset();
        m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
        m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
        m_resliceImageViewer[i]->Render();
    }
}
void QFourpaneviewer::Update3DColorByCoordinate(VtkColorStyle colorValue)
{
    if (m_volumeProperty)
    {
        //设置透明度和梯度透明度
        if (colorValue.m_colorOpacity)
        {
            m_volumeProperty->DisableGradientOpacityOff();
            QList<QPointF> points = colorValue.m_gradientPoinst;
            if (m_pieceGradF)
            {
                m_pieceGradF->RemoveAllPoints();
                int size = points.size();
                int slope = colorValue.m_slope;
                for (unsigned short i = 0; i < size; i++)
                {
                    m_pieceGradF->AddPoint(points[i].rx(), (points[i].ry()*slope) / 100.0);
                }
                /**
                 * Set the opacity of a volume to an opacity transfer function based
                 * on gradient magnitude for the given component.
                 */
                m_volumeProperty->SetGradientOpacity(m_pieceGradF);
            }
            if (m_pieceF)
            {
                m_pieceF->RemoveAllPoints();
                int size = points.size();
                for (unsigned short i = 0; i < size; i++)
                {
                    m_pieceF->AddPoint(points[i].rx(), points[i].ry());
                }
                /**
                 * Set the opacity of a volume to an opacity transfer function based
                 * on scalar value for the component indicated by index.
                 */
                m_volumeProperty->SetScalarOpacity(m_pieceF);
            }
        }
        else
        {
            m_pieceGradF->RemoveAllPoints();
            m_pieceF->RemoveAllPoints();
            m_pieceF->AddPoint(-2000, 1.0); m_pieceF->AddPoint(2000, 1.0);  // 低梯度区域完全不透明

            m_volumeProperty->SetGradientOpacity(m_pieceGradF);
            m_volumeProperty->SetScalarOpacity(m_pieceF);

            m_volumeProperty->DisableGradientOpacityOn();//关闭梯度透明度
        }

        QList<VtkColorPoint> points = colorValue.m_colorPoint;
        if (m_colorTranF)
        {
            m_colorTranF->RemoveAllPoints();
            for (unsigned short i = 0; i < points.size(); i++)
            {
                double x = points.at(i).m_X;
                QColor color = points.at(i).m_Color;
                m_colorTranF->AddRGBPoint(x, color.redF(), color.greenF(), color.blueF());
            }
            m_volumeProperty->SetColor(m_colorTranF);
        }

        //lightShade
        if (colorValue.m_lightShade)
        {
            m_volumeProperty->ShadeOn();
            m_volumeProperty->SetAmbient(colorValue.m_Ambient);//环境光系数
            m_volumeProperty->SetDiffuse(colorValue.m_Diffuse);//散射光系数
            m_volumeProperty->SetSpecular(colorValue.m_Specular);//反射光系数
            m_volumeProperty->SetSpecularPower(colorValue.m_SpecularPower);//高光强度
        }
        else
        {
            m_volumeProperty->ShadeOff();
        }

        m_volumeMapper->SetBlendMode(colorValue.m_blendMode);
        //+++++++++++++
        int blendMode = colorValue.m_blendMode;
        if (blendMode == 5)
        {
            m_volumeMapper->SetBlendMode(0);//目前简化使用混合面值方式
            m_renderer->AddActor(m_isosurfaceActor);
            m_bremoveActor = true;
            m_isosurfaceMapper->ScalarVisibilityOff();
            m_isosurfaceFilter->ComputeScalarsOff(); //ScalarsOff 数据（否则 Mapper 会默认启用 Scalar）
            m_isosurfaceActor->GetProperty()->SetAmbient(colorValue.m_Ambient);//环境光系数
            m_isosurfaceActor->GetProperty()->SetDiffuse(colorValue.m_Diffuse);//散射光系数
            m_isosurfaceActor->GetProperty()->SetSpecular(colorValue.m_Specular);//反射光系数
            m_isosurfaceActor->GetProperty()->SetSpecularPower(colorValue.m_SpecularPower);//高光强度
            m_isosurfaceActor->GetProperty()->SetOpacity(colorValue.m_slope);
        }
        else if (m_bremoveActor)
        {
            m_renderer->RemoveActor(m_isosurfaceActor);
            m_bremoveActor = false;
        }
        //+++++++++++++

        ui->m_image3DView->renderWindow()->Render();
    }
}
void QFourpaneviewer::Update3DColorByPointEditor(VtkColorStyle colorValue)
{
    if (m_volumeProperty)
    {
        if (colorValue.m_colorOpacity)
        {
            m_volumeProperty->DisableGradientOpacityOff();
            QList<VtkColorPoint> points = colorValue.m_colorPoint;
            if (m_pieceGradF)
            {
                m_pieceGradF->RemoveAllPoints();
                for (unsigned short i = 0; i < points.size(); i++)
                {
                    double x = points.at(i).m_X;
                    QColor color = points.at(i).m_Color;
                    m_pieceGradF->AddPoint(x, color.alphaF());
                }
                m_volumeProperty->SetGradientOpacity(m_pieceGradF);
            }
            if (m_pieceF)
            {
                m_pieceF->RemoveAllPoints();
                for (unsigned short i = 0; i < points.size(); i++)
                {
                    double x = points.at(i).m_X;
                    QColor color = points.at(i).m_Color;
                    m_pieceF->AddPoint(x, color.alphaF());
                }
                m_volumeProperty->SetScalarOpacity(m_pieceF);
            }
        }
        else
        {
            m_pieceGradF->RemoveAllPoints();
            m_pieceF->RemoveAllPoints();
            m_pieceF->AddPoint(-2000, 1.0); m_pieceF->AddPoint(2000, 1.0);  // 低梯度区域完全不透明

            m_volumeProperty->SetGradientOpacity(m_pieceGradF);
            m_volumeProperty->SetScalarOpacity(m_pieceF);

            m_volumeProperty->DisableGradientOpacityOn();//关闭梯度透明度
        }

        if (colorValue.m_colorAdd)
        {
            QList<VtkColorPoint> points = colorValue.m_colorPoint;
            if (m_colorTranF)
            {
                m_colorTranF->RemoveAllPoints();
                for (unsigned short i = 0; i < points.size(); i++)
                {
                    double x = points.at(i).m_X;
                    QColor color = points.at(i).m_Color;
                    m_colorTranF->AddRGBPoint(x, color.redF(), color.greenF(), color.blueF());
                }
                m_volumeProperty->SetColor(m_colorTranF);
            }
        }
        else
        {
            m_colorTranF->RemoveAllPoints();
            m_volumeProperty->SetColor(m_colorTranF);
        }

        //lightShade
        if (colorValue.m_lightShade)
        {
            m_volumeProperty->ShadeOn();
            m_volumeProperty->SetAmbient(colorValue.m_Ambient);//环境光系数
            m_volumeProperty->SetDiffuse(colorValue.m_Diffuse);//散射光系数
            m_volumeProperty->SetSpecular(colorValue.m_Specular);//反射光系数
            m_volumeProperty->SetSpecularPower(colorValue.m_SpecularPower);//高光强度
        }
        else
        {
            m_volumeProperty->ShadeOff();
        }

        //++++++++++++++++++++++++++++
        if (m_MainWindow->m_sampleDistanceCheck)
        {
            m_volumeMapper->SetRequestedRenderModeToRayCast();
            m_volumeMapper->SetSampleDistance(m_MainWindow->m_sampleDistance);
        }
        else///ToDefault
        {
            m_volumeMapper->SetRequestedRenderModeToDefault();
            m_volumeMapper->SetSampleDistance(-1.0);
        }
        //+++++++++++++++++++++++++++

        m_renderer->RemoveActor(m_isosurfaceActor); // AddActor(m_isosurfaceActor);  // 添加等值面到渲染器
        ui->m_image3DView->renderWindow()->Render();
    }
}
void QFourpaneviewer::ShowImagePlane()
{
    if (!m_MainWindow->m_vtkImageData)
    {
        return;
    }
    m_showImageData = m_MainWindow->m_vtkImageData;
    m_vtkAlgorithm = m_MainWindow->m_vtkAlgorithmOutput;
    int imageDims[3];

    vtkImageData *imageData = m_showImageData;
    imageData->GetDimensions(imageDims);

    for (int i = 0; i < 3; i++)
    {
        m_resliceImageViewer[i] = vtkResliceImageViewer::New();
        m_renderWindow[i] = vtkGenericOpenGLRenderWindow::New(); //vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
        m_resliceImageViewer[i]->SetRenderWindow(m_renderWindow[i]);

        m_cornerAts[i] = vtkCornerAnnotation::New();
        m_resliceImageViewer[i]->GetRenderer()->AddViewProp(m_cornerAts[i]);
    }

    ui->m_sagital2DView->setRenderWindow(m_resliceImageViewer[0]->GetRenderWindow());
    m_resliceImageViewer[0]->SetupInteractor(ui->m_sagital2DView->renderWindow()->GetInteractor());

    ui->m_coronal2DView->setRenderWindow(m_resliceImageViewer[1]->GetRenderWindow());
    m_resliceImageViewer[1]->SetupInteractor(ui->m_coronal2DView->renderWindow()->GetInteractor());

    ui->m_axial2DView->setRenderWindow(m_resliceImageViewer[2]->GetRenderWindow());
    m_resliceImageViewer[2]->SetupInteractor(ui->m_axial2DView->renderWindow()->GetInteractor());

    for (int i = 0; i < 3; i++)
    {
        // make them all share the same reslice cursor object.
        vtkREP *rep = vtkREP::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation());
        m_resliceImageViewer[i]->SetResliceCursor(m_resliceImageViewer[0]->GetResliceCursor());
        rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
        //-----------------------------------------------------------------------------------------------------
        rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
        rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
        rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
        //-------------------------------------------------------------------------------------------------------
        m_resliceImageViewer[i]->SetInputData(imageData);
        m_resliceImageViewer[i]->SetSliceOrientation(i);
        m_resliceImageViewer[i]->SetResliceModeToAxisAligned();
        rep->SetWindowLevel(m_defaultWindow, m_defaultLevel);
    }

    //mpr2DView--->hide- to->showVolume3D
    //m_cellPicker = vtkCellPicker::New();	//m_cellPicker->SetTolerance(0.005);
    //m_ipwProp = vtkProperty::New();	//m_ren     = vtkRenderer::New();
    //m_2DViewRenderWindow = vtkGenericOpenGLRenderWindow::New();	//ui->m_mpr2DView->setRenderWindow(m_2DViewRenderWindow);
    //ui->m_mpr2DView->renderWindow()->AddRenderer(m_ren);	//vtkRenderWindowInteractor* iren = ui->m_mpr2DView->interactor();
    double color[3] = { 0, 0, 0 };
    for (int i = 0; i < 3; i++)
    {
        m_planeWidget[i] = vtkImagePlaneWidget::New();
        m_planeWidget[i]->RestrictPlaneToVolumeOn();
        m_planeWidget[i]->GetPlaneProperty()->SetColor(color);
        m_resliceImageViewer[i]->GetRenderer()->SetBackground(color);
        m_planeWidget[i]->TextureInterpolateOff();
        m_planeWidget[i]->SetResliceInterpolateToCubic();
        m_planeWidget[i]->SetInputConnection(m_vtkAlgorithm);
        m_planeWidget[i]->SetPlaneOrientation(i);
        m_planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
        m_planeWidget[i]->DisplayTextOn();
        m_planeWidget[i]->SetWindowLevel(m_defaultWindow, m_defaultLevel);
    }

    m_resliceCallback = vtkResliceCursorCallback::New();
    m_resliceCallback->m_widget[0] = ui->m_axial2DView;
    m_resliceCallback->m_widget[1] = ui->m_coronal2DView;
    m_resliceCallback->m_widget[2] = ui->m_sagital2DView;


    vtkScalarsToColors *LookupTablecolor = m_resliceImageViewer[0]->GetLookupTable();
    for (int i = 0; i < 3; i++)
    {
        m_resliceCallback->RCW[i] = m_resliceImageViewer[i]->GetResliceCursorWidget();
        m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, m_resliceCallback);
        m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, m_resliceCallback);
        m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, m_resliceCallback);
        m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, m_resliceCallback);
        m_resliceImageViewer[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, m_resliceCallback);
        m_resliceImageViewer[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, m_resliceCallback);

        //add 在增加中间键切换常规切面或者切面
        m_resliceImageViewer[i]->GetInteractor()->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, m_resliceCallback);
        m_resliceImageViewer[i]->GetInteractor()->AddObserver(vtkCommand::RightButtonPressEvent, m_resliceCallback);
        m_resliceImageViewer[i]->GetInteractor()->AddObserver(vtkCommand::MiddleButtonPressEvent, m_resliceCallback);
        m_resliceCallback->m_resliceImageViewer[i] = m_resliceImageViewer[i];
        m_resliceCallback->m_cornerAts[i] = m_cornerAts[i];

        // Make them all share the same color map.
        m_resliceImageViewer[i]->SetLookupTable(LookupTablecolor);
        m_planeWidget[i]->GetColorMap()->SetLookupTable(LookupTablecolor);
        m_planeWidget[i]->SetColorMap(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
    }
    for (int i = 0; i < 3; i++)
    {
        m_resliceImageViewer[i]->Reset();
        m_resliceImageViewer[i]->SetResliceMode(1);
        m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
        m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
        m_resliceImageViewer[i]->Render();
    }
}
void QFourpaneviewer::ShowImage3D()
{
    if (!m_MainWindow->m_vtkImageData)
        return;
    m_showImageData = m_MainWindow->m_vtkImageData;

    vtkImageData *imageData = m_showImageData;
    //
    m_pieceF->AddPoint(0, 1.0); m_pieceF->AddPoint(255, 1.0);  // 低梯度区域完全不透明 // 高梯度区域同样不透明（相当于禁用梯度影响）
    m_MainWindow->m_checkOpacity ? (m_pieceF->AddPoint(167.00000000000000, 0.16862745098039220), m_pieceF->AddPoint(218.00000000000000, 0.41960784313725491)) : (m_pieceF->AddPoint(255, 1.0));

    m_volumeProperty->SetScalarOpacity(m_pieceF);

    if (m_MainWindow->m_check3Dcolor)
    {
        //气孔 半透明/黑色，可见但不影响主体观察。		//金属正常区域 白色，结构清晰可见。		//夹杂物 红色 + 高不透明度，突出异常区域。
        m_colorTranF->AddRGBPoint(-1000.0, 0.0, 0.0, 0.0); // 空气 -> 黑色
        m_colorTranF->AddRGBPoint(200.0, 0.8, 0.8, 0.8);   // 低密度区域 -> 灰色
        m_colorTranF->AddRGBPoint(800.0, 1.0, 1.0, 1.0);   // 金属主体 -> 白色
        m_colorTranF->AddRGBPoint(1500.0, 190, 143, 74);  // 夹杂物 -> 红色（高密度）
        m_volumeProperty->SetColor(m_colorTranF);
    }

    //关闭梯度透明
    m_pieceGradF->AddPoint(1, 0.0);
    m_pieceGradF->AddPoint(70, 0.5);
    m_pieceGradF->AddPoint(130, 1.0);
    m_pieceGradF->AddPoint(300, 0.1);
    m_volumeProperty->SetGradientOpacity(m_pieceGradF);

    m_volumeProperty->ShadeOn();
    //m_volumeProperty->ShadeOff();
    m_volumeProperty->SetInterpolationType(m_MainWindow->m_cbInterType);

    //-----一般 环境光系数+散射光系数+反射光系数=1.0,  提供亮度可以大于1.0
    m_volumeProperty->SetAmbient(0.4);//环境光系数
    m_volumeProperty->SetDiffuse(0.5);//散射光系数
    m_volumeProperty->SetSpecular(0.2);//反射光系数
    m_volumeProperty->SetSpecularPower(1.0);//高光强度
    //------------------------------------------------------------------
    if (m_MainWindow->m_checkOpacity)
    {
        m_volumeProperty->DisableGradientOpacityOff();//关闭梯度透明度
    }
    else
    {
        m_volumeProperty->DisableGradientOpacityOn();//关闭梯度透明度
    }

    m_volumeMapper->SetInputData(imageData);
    m_volumeMapper->SetBlendModeToComposite();
    m_volumeMapper->SetRequestedRenderModeToDefault();

    // force the mapper to compute a sample distance based on data spacing
    m_volumeMapper->SetSampleDistance(-1.0);

    //m_volumeMapper->SetRequestedRenderModeToGPU(); // 强制使用 GPU
    m_isosurfaceFilter->SetInputData(imageData);
    if (m_MainWindow->m_sampleDistanceCheck)
    {
        m_volumeMapper->SetSampleDistance(m_MainWindow->m_sampleDistance);
    }

    m_renderer->SetBackground(0, 0, 0);
    m_renderer->ResetCamera();
    m_renderer->GetActiveCamera()->Zoom(1.5);
    //重设相机的剪切范围；
    m_renderer->ResetCameraClippingRange();

    //透明度参数
    m_renderer->SetUseDepthPeeling(1);
    m_renderer->SetMaximumNumberOfPeels(100);
    m_renderer->SetOcclusionRatio(0.1);
    //
    ui->m_image3DView->renderWindow()->AddRenderer(m_renderer);
}

//------------------SplitImageData
#include <vtkImageActor.h>
void QFourpaneviewer::ShowEditorSplitImageData()
{
    //++++++++++++++++++++
    VTKRCP* rep = VTKRCP::SafeDownCast(m_resliceImageViewer[1]->GetResliceCursorWidget()->GetRepresentation());
    vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(rep->GetReslice());
    int extent[6], extent2[6];
    reslice->GetOutput()->GetExtent(extent);
    rep = VTKRCP::SafeDownCast(m_resliceImageViewer[0]->GetResliceCursorWidget()->GetRepresentation());
    reslice = vtkImageReslice::SafeDownCast(rep->GetReslice());
    reslice->GetOutput()->GetExtent(extent2);
    //++++++++++++++++++++
    int* dims = m_MainWindow->m_vtkImageData->GetDimensions();
    if (!m_volumeDataSet)
    {
        m_volumeDataSet = new VolumeDataSet();
        m_volumeDataSet->SetSlicesNumber(dims, extent, extent2);
        connect(m_volumeDataSet, &VolumeDataSet::SplitImageData, this, &QFourpaneviewer::SplitImageData);
        connect(m_volumeDataSet, &VolumeDataSet::RectData, this, &QFourpaneviewer::SaveRectangleImageParm);
        connect(m_volumeDataSet, &VolumeDataSet::ObliquerRectParm, this, &QFourpaneviewer::SaveObliquerRectangleImageParm);
        
    }
    m_volumeDataSet->show();
}
void QFourpaneviewer::SplitImageData(int *dims, int start, int end)
{
    // 1. 提取 VOI：只取 Z=30~40 层
        //int* dims = m_MainWindow->m_vtkImageData->GetDimensions();   
    m_extractVOI->SetInputData(m_MainWindow->m_vtkImageData);
    m_extractVOI->SetVOI(0, dims[0] - 1, 0, dims[1] - 1, start, end);  // x, y, z 范围
    m_extractVOI->Update();
    m_showImageData = m_extractVOI->GetOutput();
    m_vtkAlgorithm = m_extractVOI->GetOutputPort();

    for (int i = 0; i < 3; i++)
    {
        m_resliceImageViewer[i]->SetInputData(m_showImageData);
        m_resliceImageViewer[i]->Render();

        m_planeWidget[i]->SetInputConnection(m_vtkAlgorithm);
    }
    m_volumeMapper->SetInputData(m_showImageData);
    ui->m_image3DView->renderWindow()->Render();
}
void QFourpaneviewer::SaveRectangleImageParm(int orientation, int w, int h, int dx, int dy, int angle)
{
    vtkRenderer* renderer            = m_resliceImageViewer[2]->GetRenderer();
    vtkImagePlaneWidget* planeWidget = m_planeWidget[2];
    if (orientation == 1)//XZ
    {
        renderer    = m_resliceImageViewer[1]->GetRenderer();
        planeWidget = m_planeWidget[1];
    }
    else if (orientation == 2)//YZ
    {
        renderer    = m_resliceImageViewer[0]->GetRenderer();
        planeWidget = m_planeWidget[0];
    }
    DrawRectangleAxisAlignedPlane(planeWidget, m_showImageData, renderer, w, h);
}
void QFourpaneviewer::SaveObliquerRectangleImageParm(int orientation, int w, int h, int dx, int dy, int angle)
{
    vtkRenderer* renderer            = m_resliceImageViewer[2]->GetRenderer();
    vtkImagePlaneWidget* planeWidget = m_planeWidget[2];
    if (orientation == 1)//XZ
    {
        renderer    = m_resliceImageViewer[1]->GetRenderer();
        planeWidget = m_planeWidget[1];
    }
    else if (orientation == 2)//YZ
    {
        renderer    = m_resliceImageViewer[0]->GetRenderer();
        planeWidget = m_planeWidget[0];
    }
    DrawRectangleObliquerPlane(planeWidget, m_showImageData, renderer, w, h, dx, dy);
}
void QFourpaneviewer::DrawRectangleAxisAlignedPlane(vtkImagePlaneWidget* planeWidget, vtkImageData* imageData, vtkRenderer* renderer, int w, int h, double deltaX, double deltaY, int angle)
{
    if (!planeWidget || !renderer) return;
    // 清除旧 actor
    static std::map<vtkImagePlaneWidget*, vtkSmartPointer<vtkActor>> g_widgetToBoxActorMap;
    if (g_widgetToBoxActorMap.count(planeWidget))
    {
        vtkActor* oldActor = g_widgetToBoxActorMap[planeWidget];
        if (renderer->HasViewProp(oldActor))
        {
            renderer->RemoveActor(oldActor);
        }
        g_widgetToBoxActorMap.erase(planeWidget);
    }
    // 1. 获取图像 spacing
    if (!imageData)
    {
        std::cerr << "Error: planeWidget has no input image data." << std::endl;
        return;
    }

    double spacing[3];
    imageData->GetSpacing(spacing);

    // 2. 获取平面方向（0 = YZ, 1 = XZ, 2 = XY）
    int orientation = planeWidget->GetPlaneOrientation();

    double spacingU = 1.0, spacingV = 1.0;
    if (orientation == 0) {        // YZ plane
        spacingU = spacing[1];     // Y
        spacingV = spacing[2];     // Z
    }
    else if (orientation == 1) { // XZ plane
        spacingU = spacing[0];     // X
        spacingV = spacing[2];     // Z
    }
    else if (orientation == 2) { // XY plane
        spacingU = spacing[0];     // X
        spacingV = spacing[1];     // Y
    }

    // 3. 获取中心点和方向向量
    double origin[3], p1[3], p2[3];
    planeWidget->GetOrigin(origin);
    planeWidget->GetPoint1(p1);
    planeWidget->GetPoint2(p2);
    // 获取平面中心点
    double center[3];
    planeWidget->GetCenter(center); // 中心仍然可用

    // 计算 Right/Up 向量
    double uVec[3], vVec[3];
    for (int i = 0; i < 3; ++i)
    {
        uVec[i] = p1[i] - origin[i]; // U方向：Point1 - Origin
        vVec[i] = p2[i] - origin[i]; // V方向：Point2 - Origin
    }
    vtkMath::Normalize(uVec);
    vtkMath::Normalize(vVec);

    // 4. 尺寸物理长度  计算四个角点（世界坐标）
    double halfWidth = (w * spacingU) / 2.0;
    double halfHeight = (h * spacingV) / 2.0;
    double corners[4][3]; // 左下，右下，右上，左上
    for (int i = 0; i < 3; ++i)
    {
        corners[0][i] = center[i] - halfWidth * uVec[i] - halfHeight * vVec[i]; // 左下
        corners[1][i] = center[i] + halfWidth * uVec[i] - halfHeight * vVec[i]; // 右下
        corners[2][i] = center[i] + halfWidth * uVec[i] + halfHeight * vVec[i]; // 右上
        corners[3][i] = center[i] - halfWidth * uVec[i] + halfHeight * vVec[i]; // 左上
    }
    ///+++++++++++++++++++++++++++
    bool useProjectionMethod = false;
    if (useProjectionMethod)
    {
        //一半切面数据无法正常显示框
        // 方法一：投影回切片平面
        //double normal[3];
        double p1[3], p2[3], origin[3];
        planeWidget->GetPoint1(p1);
        planeWidget->GetPoint2(p2);
        planeWidget->GetOrigin(origin);

        double uVec[3], vVec[3], normal[3];
        vtkMath::Subtract(p1, origin, uVec);
        vtkMath::Subtract(p2, origin, vVec);
        vtkMath::Cross(uVec, vVec, normal);
        vtkMath::Normalize(normal);
        //planeWidget->GetNormal(normal); // 获取当前平面的法向量
        for (int i = 0; i < 4; ++i)
        {
            double vec[3];
            vtkMath::Subtract(corners[i], center, vec);// vec = corner - center
            double dot = vtkMath::Dot(vec, normal);// 投影长度 = 向量在法向量方向上的分量
            double correction[3];
            for (int j = 0; j < 3; ++j)
            {
                correction[j] = dot * normal[j];// 修正向量 = 法向量 × 投影分量
            }
            vtkMath::Subtract(corners[i], correction, corners[i]);// 投影回平面
        }
    }
    else
    {
        // 方法二：世界坐标 -> 显示坐标 -> 非主轴值设为常数 -> 再转回世界坐标
        for (int i = 0; i < 4; ++i)
        {
            double display[3];
            vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
            coordinate->SetCoordinateSystemToWorld();
            coordinate->SetValue(corners[i]);
            int* disp = coordinate->GetComputedDisplayValue(renderer);

            // 修改非主轴方向的屏幕坐标
            int axis = planeWidget->GetPlaneOrientation(); // 0=X, 1=Y, 2=Z
            display[0] = disp[0];
            display[1] = disp[1];
            display[2] = 0.0; // Z值设为0，简化深度投影

            // 转回世界坐标
            renderer->SetDisplayPoint(display);
            renderer->DisplayToWorld();
            double* world = renderer->GetWorldPoint();
            if (world[3] != 0.0)
            {
                for (int j = 0; j < 3; ++j)
                {
                    corners[i][j] = world[j] / world[3];
                }
            }
        }
    }
    ///++++++++++++++++++++++++++++++

    // 5. 构建点和线 绘制矩形框
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < 4; ++i)
    {
        points->InsertNextPoint(corners[i]);
    }
    points->InsertNextPoint(corners[0]); // 闭合矩形

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
    ids->SetNumberOfIds(5);
    for (int i = 0; i < 5; ++i)
    {
        ids->SetId(i, i);
    }
    lines->InsertNextCell(ids);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.0, 1.0, 0.0);
    actor->GetProperty()->SetLineWidth(2.0);
    actor->GetProperty()->SetOpacity(1);

    renderer->AddActor(actor);
    renderer->Render();

    g_widgetToBoxActorMap[planeWidget] = actor;

    //+++++++++++++++++++++++++++++++++++++++
    // ==========================
    //先只考虑非斜切面的情况.斜切面另外函数实现
    // 保存当前矩形图像为 TIFF:
    // todo .需要重新计算分辨率的比例值来裁剪图像
    // ==========================
    {
        QString strOrientation = "XY_Rectangle_reslice.tiff";
        VTKRCP* rep = VTKRCP::SafeDownCast(m_resliceImageViewer[orientation]->GetResliceCursorWidget()->GetRepresentation());
        vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(rep->GetReslice());
        int extent[6];
        reslice->GetOutput()->GetExtent(extent);
        int newWidth = w, newHeigth = h;
        int* dims = m_resliceImageViewer[2]->GetInput()->GetDimensions();
        int xMin, xMax, yMin, yMax;
        // 中心点像素坐标（reslice 输出的中心是 [0,0]，但数据坐标范围是 [0,wPix-1], [0,hPix-1]）
        int cx = (extent[0] + extent[1] + 1) / 2, cy = (extent[2] + extent[3] + 1) / 2;
        int orgW = extent[1], grgH = extent[3];
        if (orientation == 2)//XY
        {
            // VOI 提取范围：确保不越界
            xMin = std::max(cx - w / 2, extent[0]);
            xMax = std::min(cx + w / 2, orgW);
            yMin = std::max(cy - h / 2, extent[2]);
            yMax = std::min(cy + h / 2 , grgH);

        }
        else if (orientation == 1)//XZ
        {
            cy = (extent[0] + extent[1]) / 2;
            cx = (extent[2] + extent[3]) / 2;
            newWidth = qRound((double)w * extent[3] / dims[2]);
            newHeigth = qRound((double)h * extent[1] / dims[0]);
            orgW = extent[3];
            grgH = extent[1];
            // VOI 提取范围：确保不越界
            yMin = std::max(cx - newWidth / 2, extent[0]);
            yMax = std::min(cx + newWidth / 2, orgW);
            xMin = std::max(cy - newHeigth / 2, extent[2]);
            xMax = std::min(cy + newHeigth / 2, grgH);
            strOrientation = "XZ_Rectangle_reslice.tiff";
        }
        else //if(orientation == 0)//YZ
        {
            newWidth = qRound((double)w * extent[1] / dims[0]);
            newHeigth = qRound((double)h * extent[3] / dims[2]);
            // VOI 提取范围：确保不越界
            xMin = std::max(cx - newWidth / 2, extent[0]);
            xMax = std::min(cx + newWidth / 2, orgW);
            yMin = std::max(cy - newHeigth / 2, extent[2]);
            yMax = std::min(cy + newHeigth / 2, grgH);
            strOrientation = "YZ_Rectangle_reslice.tiff";
        }

        vtkSmartPointer<vtkExtractVOI> extract = vtkSmartPointer<vtkExtractVOI>::New();
        extract->SetInputConnection(reslice->GetOutputPort());
        extract->SetVOI(xMin, xMax, yMin, yMax, 0, 0);  // 2D 图像 --> All image //extract->SetVOI(0, extent[1], 0, extent[3], 0, 0); 
        extract->Update();
        //---->写入 TIFF 文件 采样的原始数据.
        vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
        writer->SetInputConnection(extract->GetOutputPort());
        writer->SetFileName(qPrintable(strOrientation));//writer->SetFileName("Rectangle_reslice.tiff");
        writer->Write();
    }
}
void QFourpaneviewer::DrawRectangleObliquerPlane(vtkImagePlaneWidget* planeWidget, vtkImageData* imageData, vtkRenderer* renderer, int w, int h, double deltaX, double deltaY, int angle)
{
    if (!planeWidget || !renderer) return;
    // 清除旧---- actor--------
    static std::map<vtkImagePlaneWidget*, vtkSmartPointer<vtkActor>> g_widgetToBoxActorMap;
    if (g_widgetToBoxActorMap.count(planeWidget))
    {
        vtkActor* oldActor = g_widgetToBoxActorMap[planeWidget];
        if (renderer->HasViewProp(oldActor))
        {
            renderer->RemoveActor(oldActor);
        }
        g_widgetToBoxActorMap.erase(planeWidget);
    }
    //\\\\\\\\\\\\\\\\\\\\\\\\\\
    //*************************************************************
    //         
    //         ↑
    //         |
    //         Y
    //       p1.
    //         |
    //         |  
    //         |
    //  origin .----------- .p2  X -->
    //************************************************************
    ///+++++++++++++++++++++++++++
    // +++++++++++++++++++++++++方法：世界坐标 -> 显示坐标 -> 非主轴值设为常数 -> 再转回世界坐标++++++++++++++++++++++++++  
        // 1. 获取平面方向（0 = YZ, 1 = XZ, 2 = XY）
    int orientation    = planeWidget->GetPlaneOrientation();
    VTKRCP* repOblique = VTKRCP::SafeDownCast(m_resliceImageViewer[orientation]->GetResliceCursorWidget()->GetRepresentation());
    vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(repOblique->GetReslice());
    reslice->SetOutputDimensionality(2);
    vtkImageData* image = reslice->GetOutput();
    if (!image)
        return ;

    vtkMatrix4x4* axesMatrix = reslice->GetResliceAxes();
    if (!axesMatrix)
        return ;

    // 获取世界坐标点
    double origin[3], center[3], corners[4][3],allImageRect[4][3];
    double spacing[2];
    spacing[0] = reslice->GetOutputSpacing()[0];
    spacing[1] = reslice->GetOutputSpacing()[1];
    int extent[6];
    image->GetExtent(extent);//XY:(0,1023,0,1023,0,0)  XZ(0,1023,0,127,0,0) YZ(0,1023,0,127,0,0)
    int width  = extent[1] - extent[0] + 1;
    int height = extent[3] - extent[2] + 1;
    double fullWidth  = spacing[0] * width;
    double fullHeight = spacing[1] * height;

    double xAxis[3], yAxis[3];
    for (int i = 0; i < 3; ++i)
    {
        xAxis[i] = axesMatrix->GetElement(i, 0);
        yAxis[i] = axesMatrix->GetElement(i, 1);
        origin[i] = axesMatrix->GetElement(i, 3);
    }
    // Test 2D图的四个角点按顺时针（左下，右下，右上，左上）
    //for (int i = 0; i < 3; ++i)
    //{
    //    //corners[0][i] = origin[i]; // lower left
    //    //corners[1][i] = origin[i] + halfWidth * xAxis[i]; // lower right
    //    //corners[2][i] = origin[i] + halfWidth * xAxis[i] + halfHeight * yAxis[i]; // upper right
    //    //corners[3][i] = origin[i] + halfHeight * yAxis[i]; // upper left
    //}
    int* dims = imageData->GetDimensions();//XYZ
    double dimX = 1.0, dimY = 1.0;
    if (orientation == 2)// XY
    { 
        dimX = dims[0];
        dimY = dims[1];
    }
    else if (orientation == 1)// YZ
    { 
        dimX = dims[0];
        dimY = dims[2];
    }
    else if (orientation == 0)// XZ
    { 
        dimX = dims[1];
        dimY = dims[2];
    }
    // 计算裁剪矩形宽高
    double cropWidth  = fullWidth * w / dimX;
    double cropHeight = fullHeight * h / dimY;
    double offsetX    = fullWidth * deltaX / dimX;
    double offsetY    = fullHeight * deltaY / dimY;
    // 原点是左下角，因此先计算中心点
    for (int i = 0; i < 3; ++i)
    {
        center[i] = origin[i] + 0.5 * fullWidth  * xAxis[i] + 0.5 * fullHeight * yAxis[i] + 
                    offsetX * xAxis[i] + offsetY * yAxis[i];
    }
    // 半宽/半高
    double hw = 0.5 * cropWidth;
    double hh = 0.5 * cropHeight;
    // 计算角点：左下、右下、右上、左上
    for (int i = 0; i < 3; ++i)
    {
        corners[0][i] = center[i] - hw * xAxis[i] - hh * yAxis[i]; // 左下
        corners[1][i] = center[i] + hw * xAxis[i] - hh * yAxis[i]; // 右下
        corners[2][i] = center[i] + hw * xAxis[i] + hh * yAxis[i]; // 右上
        corners[3][i] = center[i] - hw * xAxis[i] + hh * yAxis[i]; // 左上
    }
    // 5. 构建点和线 绘制矩形框
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < 4; ++i)
    {
        points->InsertNextPoint(corners[i]);
    }
    points->InsertNextPoint(corners[0]); // 闭合矩形
    
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
    ids->SetNumberOfIds(5);
    for (int i = 0; i < 5; ++i)
    {
        ids->SetId(i, i);
    }
    lines->InsertNextCell(ids);
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
    actor->GetProperty()->SetLineWidth(2.0);
    actor->GetProperty()->SetOpacity(1);
    
    renderer->AddActor(actor);
    renderer->Render();
    
    g_widgetToBoxActorMap[planeWidget] = actor;

    //+++++++++++++++++++++++++++++++++++++++
    // ==========================
    // 保存当前矩形图像为 TIFF:
    // ==========================
    QString strOrientation = "XY_ObliquerRectangle_reslice.tiff";
    int newWidth = w, newHeigth = h;
    int xMin, xMax, yMin, yMax;
    // 中心点像素坐标（reslice 输出的中心是 [0,0]，但数据坐标范围是 [0,wPix-1], [0,hPix-1]）
    int cx = (extent[0] + extent[1] + 1) / 2, cy = (extent[2] + extent[3] + 1) / 2;
    int orgW = extent[1], grgH = extent[3];
    if (orientation == 2)//XY
    {
        // VOI 提取范围：确保不越界
        int x = cx - w / 2 + deltaX;
        xMin = std::max(x, extent[0]);
        x = cx + w / 2 + deltaX;
        xMax = std::min(x, orgW);
        int y = cy - h / 2 + deltaY;
        yMin = std::max(y, extent[2]);
        y = cy + h / 2 + deltaY;
        yMax = std::min(y, grgH);
    }
    else if (orientation == 1)//XZ
    {
        newWidth   = qRound((double)w * extent[1] / dims[0]);
        newHeigth  = qRound((double)h * extent[3] / dims[2]);
        int RatioX = qRound((double)deltaX * extent[1] / dims[0]);
        int RatioY = qRound((double)deltaY * extent[3] / dims[2]);
        // VOI 提取范围：确保不越界
        xMin = std::max(cx - newWidth / 2  + RatioX, extent[0]);
        xMax = std::min(cx + newWidth / 2  + RatioX, orgW);
        yMin = std::max(cy - newHeigth / 2 + RatioY, extent[2]);
        yMax = std::min(cy + newHeigth / 2 + RatioY, grgH);
        strOrientation = "XZ_ObliquerRectangle_reslice.tiff";
    }
    else //if(orientation == 0)//YZ
    {
        newWidth = qRound((double)w * extent[1] / dims[0]);
        newHeigth = qRound((double)h * extent[3] / dims[2]);
        int RatioX = qRound((double)deltaX * extent[1] / dims[0]);
        int RatioY = qRound((double)deltaY * extent[3] / dims[2]);
        // VOI 提取范围：确保不越界
        xMin = std::max(cx - newWidth / 2 + RatioX, extent[0]);
        xMax = std::min(cx + newWidth / 2 + RatioX, orgW);
        yMin = std::max(cy - newHeigth / 2 + RatioY, extent[2]);
        yMax = std::min(cy + newHeigth / 2 + RatioY, grgH);
        strOrientation = "YZ_ObliquerRectangle_reslice.tiff";
    }

    vtkSmartPointer<vtkExtractVOI> extract = vtkSmartPointer<vtkExtractVOI>::New();
    extract->SetInputConnection(reslice->GetOutputPort());
    extract->SetVOI(xMin, xMax, yMin, yMax, 0, 0);  // 2D 图像 --> All image //extract->SetVOI(0, extent[1], 0, extent[3], 0, 0); 
    extract->Update();
    //---->写入 TIFF 文件 采样的原始数据.
    vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
    writer->SetInputConnection(extract->GetOutputPort());
    writer->SetFileName(qPrintable(strOrientation));//writer->SetFileName("Rectangle_reslice.tiff");
    writer->Write(); 
  
}
void QFourpaneviewer::SaveRectangleImageTIFF(vtkResliceImageViewer* vtkResliceViewer, double *p1, double *p2)
{
    vtkResliceImageViewer* viewer = vtkResliceViewer;
    if (!viewer)
    {
        return;
    }
    vtkImageData* input = m_showImageData;

    vtkResliceCursor* cursor = viewer->GetResliceCursor();
    double center[3];
    cursor->GetCenter(center);

    // 获取当前切面方向
    double xAxis[3] = { 0,0,0 }, yAxis[3] = { 0,0,0 }, zAxis[3] = { 0,0,0 };
    double *pxAxis = cursor->GetAxis(0);
    double *pyAxis = cursor->GetAxis(1);
    xAxis[0] = pxAxis[0];  xAxis[1] = pxAxis[1];  xAxis[2] = pxAxis[2];
    yAxis[0] = pyAxis[0];  yAxis[1] = pyAxis[1];  yAxis[2] = pyAxis[2];
    vtkMath::Cross(xAxis, yAxis, zAxis);

    // VTK 坐标系和常规图像坐标系 不一样 翻转 Y 轴，避免图像镜像
    double yAxisFlip[3] = { -yAxis[0], -yAxis[1], -yAxis[2] };

    // 原图 spacing
    double spacingIn[3];
    input->GetSpacing(spacingIn);

    // 计算 oblique spacing
    double spacingU = std::sqrt(
        std::pow(xAxis[0] * spacingIn[0], 2) +
        std::pow(xAxis[1] * spacingIn[1], 2) +
        std::pow(xAxis[2] * spacingIn[2], 2));

    double spacingV = std::sqrt(
        std::pow(yAxisFlip[0] * spacingIn[0], 2) +
        std::pow(yAxisFlip[1] * spacingIn[1], 2) +
        std::pow(yAxisFlip[2] * spacingIn[2], 2));

    // 将 world 点投影到切片平面上
    auto Project = [&](double pt[3], double& u, double& v)
    {
        double vec[3] = { pt[0] - center[0], pt[1] - center[1], pt[2] - center[2] };
        u = vtkMath::Dot(vec, xAxis);
        v = vtkMath::Dot(vec, yAxisFlip);
    };

    double u1, v1, u2, v2;
    Project(p1, u1, v1);
    Project(p2, u2, v2);

    double uMin = std::min(u1, u2);
    double uMax = std::max(u1, u2);
    double vMin = std::min(v1, v2);
    double vMax = std::max(v1, v2);

    // 计算输出图像大小
    int dims[3] =
    {
        static_cast<int>((uMax - uMin) / spacingU + 0.5),
        static_cast<int>((vMax - vMin) / spacingV + 0.5),
        1
    };
    if (dims[0] <= 0 || dims[1] <= 0) return;

    // 构造 ResliceAxes（从切片坐标转到 world 坐标）
    vtkNew<vtkMatrix4x4> resliceAxes;
    for (int i = 0; i < 3; ++i)
    {
        resliceAxes->SetElement(i, 0, xAxis[i]);
        resliceAxes->SetElement(i, 1, yAxisFlip[i]);
        resliceAxes->SetElement(i, 2, zAxis[i]);
        resliceAxes->SetElement(i, 3, center[i] + uMin * xAxis[i] + vMin * yAxisFlip[i]);
    }

    // 创建 Reslicer
    vtkNew<vtkImageReslice> reslicer;
    reslicer->SetInputData(input);
    reslicer->SetResliceAxes(resliceAxes);
    reslicer->SetOutputSpacing(spacingU, spacingV, 1.0);
    reslicer->SetOutputOrigin(0, 0, 0);
    reslicer->SetOutputExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, 0);
    reslicer->SetInterpolationModeToLinear();
    reslicer->Update();

    // 归一化为 8-bit 图像
    vtkImageData* slice = reslicer->GetOutput();
    double range[2];
    slice->GetScalarRange(range);

    vtkNew<vtkImageShiftScale> shiftScale;
    shiftScale->SetInputData(slice);
    shiftScale->SetOutputScalarTypeToUnsignedChar();
    shiftScale->SetShift(-range[0]);
    shiftScale->SetScale(255.0 / (range[1] - range[0]));
    shiftScale->ClampOverflowOn();
    shiftScale->Update();
    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName("SaveRectangleImage.png");
    writer->SetInputConnection(shiftScale->GetOutputPort());
    writer->Write();

    vtkNew<vtkTIFFWriter> TIFFwriter;
    TIFFwriter->SetFileName("SaveRectangleImage.tiff");
    TIFFwriter->SetInputData(slice);
    TIFFwriter->Write();
}
QFourpaneviewer::~QFourpaneviewer()
{
    for (int i = 0; i < 3; i++)
    {
        if (m_resliceImageViewer[i])
        {
            m_resliceImageViewer[i]->Delete();
        }

        if (m_planeWidget[i])
        {
            m_planeWidget[i]->Delete();
        }

        if (m_renderWindow[i])
        {
            m_renderWindow[i]->Delete();
        }

        if (m_cornerAts[i])
        {
            m_cornerAts[i]->Delete();
        }
    }

    if (m_resliceCallback)
    {
        m_resliceCallback->Delete();
    }

    if (m_cellPicker)
    {
        m_cellPicker->Delete();
    }

    if (m_ipwProp)
    {
        m_ipwProp->Delete();
    }

    if (m_ren)
    {
        m_ren->Delete();
    }

    if (m_2DViewRenderWindow)
    {
        m_2DViewRenderWindow->Delete();
    }

    if (m_extractVOI)
    {
        m_extractVOI->Delete();
    }

    if (m_volumeDataSet)
    {
        m_volumeDataSet->hide();
        delete m_volumeDataSet;
    }
    ///--------------showVolume3D--------
    m_volumeMapper->Delete();
    m_volumeProperty->Delete();
    m_vtkVolume->Delete();

    m_isosurfaceFilter->Delete();
    m_isosurfaceActor->Delete();
    m_renderer->Delete();

    m_pieceF ? m_pieceF->Delete() : m_pieceF = nullptr;
    m_pieceGradF ? m_pieceGradF->Delete() : m_pieceGradF = nullptr;
    m_colorTranF ? m_colorTranF->Delete() : m_colorTranF = nullptr;
    m_pieceF = nullptr;
    m_pieceGradF = nullptr;
    m_colorTranF = nullptr;
    if (m_isosurfaceMapper)
    {
        m_isosurfaceMapper->Delete();
    }
    //----------------------------------

    if (m_vtkColorGradient)
    {
        m_vtkColorGradient->hide();
        delete m_vtkColorGradient;
    }
    delete ui;
}

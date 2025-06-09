// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "QtVTKRenderWindows.h"
#include "ui_QtVTKRenderWindows.h"

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
#include <vtkImageFlip.h>
#include "vtkAutoInit.h"
#include "vtkMarchingCubes.h"
#include "vtkMetaImageReader.h"
#include "vtkMetaImageWriter.h"
#include <vtkCamera.h>
#include <vtkCornerAnnotation.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkImageActor.h>
#include <vtkImageReslice.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageAppend.h>

#include <vtkProbeFilter.h>
#include <vtkDataSetMapper.h>
#include <vtkWindowLevelLookupTable.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkImageMathematics.h>
//#include <vtkNamedColors.h>
#include <qevent.h>

const double PI = -3.141592653589793238462643383279502884197169399375105820974944;
vtkAlgorithmOutput *g_vtkAlgorithmOutput = NULL;
#define VTKRCP    vtkResliceCursorRepresentation

void FitResliceImageToViewer(vtkResliceImageViewer* viewer);
void FitResliceImageToViewer(vtkResliceImageViewer* viewer)
{
    if (!viewer || !viewer->GetImageActor() || !viewer->GetRenderer())
        return;

    vtkRenderer*    renderer = viewer->GetRenderer();
    vtkCamera*      camera = renderer->GetActiveCamera();
    vtkImageData*   image = viewer->GetInput();

    if (!camera || !image)
    {
        return;
    }

    renderer->ResetCamera();

    //if (0 == viewer->GetSliceOrientation())
    //{
    //    camera->SetViewUp(0, 0, 1); // Z 轴朝上，视角从下看，要反转头朝上
    //}
    // Step 2: Get window aspect ratio
    int* winSize = viewer->GetRenderWindow()->GetSize();
    if (winSize[0] <= 0 || winSize[1] <= 0)
    {
        return;
    }
    double windowAspect = static_cast<double>(winSize[0]) / static_cast<double>(winSize[1]);

    // Step 3: Get image spacing and extent
    int extent[6];
    double spacing[3];
    image->GetExtent(extent);
    image->GetSpacing(spacing);

    // Step 4: Compute width and height depending on orientation
    double width = 1.0, height = 1.0;
    switch (viewer->GetSliceOrientation())
    {
    case vtkResliceImageViewer::SLICE_ORIENTATION_XY:
        width = (extent[1] - extent[0] + 1) * spacing[0]; // X
        height = (extent[3] - extent[2] + 1) * spacing[1]; // Y
        break;
    case vtkResliceImageViewer::SLICE_ORIENTATION_XZ:
        width = (extent[1] - extent[0] + 1) * spacing[0]; // X
        height = (extent[5] - extent[4] + 1) * spacing[2]; // Z
        break;
    case vtkResliceImageViewer::SLICE_ORIENTATION_YZ:
        width = (extent[3] - extent[2] + 1) * spacing[1]; // Y
        height = (extent[5] - extent[4] + 1) * spacing[2]; // Z
        break;
    default:
        return; // unsupported or oblique orientation
    }

    // Step 5: Fit parallel scale
    double imageAspect = width / height;
    if (imageAspect > windowAspect)
    {
        double scale = (width / windowAspect) / 2.0;
        camera->SetParallelScale(scale);
    }
    else
    {
        double scale = height / 2.0;
        camera->SetParallelScale(scale);
    }
    viewer->Render();

}
vtkSmartPointer<vtkPolyData> SweepLineByPoints(const std::vector<std::array<double, 3>>& Points,
    const double direction[3], double distance, unsigned int cols);
vtkSmartPointer<vtkPolyData> SweepLineByPoints(const std::vector<std::array<double, 3>>& Points,
    const double direction[3], double distance, unsigned int cols)
{
    unsigned int rows = static_cast<unsigned int>(Points.size());
    if (rows < 2 || cols == 0) {
        return nullptr;  // 至少需要两点和非零列
    }

    // 单步间距
    double spacing = distance / cols;

    // 对称扫掠从 -cols/2 到 +cols/2，共 cols + 1 个采样点
    cols++;  // 包括中间线
    int halfCols = static_cast<int>(cols / 2);

    vtkNew<vtkPoints> vtkPoints;
    vtkPoints->Allocate(rows * cols);

    vtkNew<vtkCellArray> polys;
    polys->AllocateEstimate((rows - 1) * (cols - 1), 4);

    // 单位化方向向量
    double dir[3] = { direction[0], direction[1], direction[2] };
    double mag = std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
    if (mag == 0) return nullptr;
    dir[0] /= mag; dir[1] /= mag; dir[2] /= mag;

    // 插入对称扫掠点
    unsigned int cnt = 0;
    for (unsigned int row = 0; row < rows; row++)
    {
        const auto& p = Points[row];
        for (int col = -halfCols; col <= halfCols; col++)
        {
            double x[3];
            x[0] = p[0] + dir[0] * col * spacing;
            x[1] = p[1] + dir[1] * col * spacing;
            x[2] = p[2] + dir[2] * col * spacing;
            vtkPoints->InsertPoint(cnt++, x);
        }
    }

    // 构造四边形
    vtkIdType pts[4];
    for (unsigned int row = 0; row < rows - 1; row++)
    {
        for (unsigned int col = 0; col < cols - 1; col++)
        {
            pts[0] = col + row * cols;
            pts[1] = pts[0] + 1;
            pts[2] = pts[0] + cols + 1;
            pts[3] = pts[0] + cols;
            polys->InsertNextCell(4, pts);
        }
    }

    vtkNew<vtkPolyData> surface;
    surface->SetPoints(vtkPoints);
    surface->SetPolys(polys);
    return surface;
}

//https://blog.csdn.net/juluwangriyue/article/details/127272402
vtkSmartPointer<vtkImageData> GenerateCPRImageWithThickness(
    vtkImageData* volume, const std::vector<std::array<double, 3>>& pathPoints, int outputSliceSize = 200,
    int thickness = 1, double spacing = 1.0, const std::string& mode = "NULL" // "mean" or "mip"
)
{
    if (!volume || pathPoints.size() < 2)
        return nullptr;

    auto finalStack = vtkSmartPointer<vtkImageAppend>::New();
    finalStack->SetAppendAxis(0); // Stack along Y
    for (size_t i = 0; i + 1 < pathPoints.size(); ++i)
    {
        std::array<double, 3> p0 = pathPoints[i];
        std::array<double, 3> p1 = pathPoints[i + 1];
        double tangent[3] =
        {
            p1[0] - p0[0],
            p1[1] - p0[1],
            p1[2] - p0[2]
        };
        vtkMath::Normalize(tangent);
        // Build orthogonal coordinate system
        double ref[3] = { 0, 0, 1 };
        if (std::abs(vtkMath::Dot(tangent, ref)) > 0.99)
        {
            ref[0] = 0, ref[1] = 1, ref[2] = 0;
        }
        double normal[3];
        vtkMath::Cross(tangent, ref, normal);
        vtkMath::Normalize(normal);

        double binormal[3];
        vtkMath::Cross(tangent, normal, binormal);
        vtkMath::Normalize(binormal);

        // Midpoint center
        double center[3] =
        {
            0.5 * (p0[0] + p1[0]),
            0.5 * (p0[1] + p1[1]),
            0.5 * (p0[2] + p1[2])
        };
        // 计算点之间距离（以毫米或像素为单位）
        double length = std::sqrt(vtkMath::Distance2BetweenPoints(p0.data(), p1.data()));
        int halfSize = static_cast<int>(length / (2 * spacing));

        // Store thickness slices
        std::vector<vtkSmartPointer<vtkImageData>> sliceLayers;
        for (int t = -thickness; t <= thickness; ++t)
        {
            double offsetCenter[3] =
            {
                center[0] + spacing * t * normal[0],
                center[1] + spacing * t * normal[1],
                center[2] + spacing * t * normal[2]
            };
            auto axes = vtkSmartPointer<vtkMatrix4x4>::New();
            for (int r = 0; r < 3; ++r)
            {
                axes->SetElement(r, 0, tangent[r]);
                axes->SetElement(r, 1, binormal[r]);
                axes->SetElement(r, 2, normal[r]);
                axes->SetElement(r, 3, offsetCenter[r]);
            }
            auto reslice = vtkSmartPointer<vtkImageReslice>::New();
            reslice->SetInputData(volume);
            reslice->SetOutputDimensionality(2);
            reslice->SetResliceAxes(axes);
            reslice->SetInterpolationModeToCubic();
            reslice->SetOutputSpacing(1.0, 1.0, 1.0);
            reslice->SetOutputExtent(-halfSize, halfSize, -outputSliceSize / 2, outputSliceSize / 2, 0, 0);
            reslice->SetBackgroundLevel(0);
            reslice->Update();

            sliceLayers.push_back(reslice->GetOutput());
        }

        vtkSmartPointer<vtkImageData> combined;
        //if (mode == "mip")
        //{
        //    auto mip = vtkSmartPointer<vtkImageMathematics>::New();
        //    mip->SetOperationToMax();
        //    mip->SetInput1Data(sliceLayers[0]);
        //    for (size_t j = 1; j < sliceLayers.size(); ++j) {
        //        mip->SetInput2Data(sliceLayers[j]);
        //        mip->Update();
        //        mip->SetInput1Data(mip->GetOutput());
        //    }
        //    mip->Update();
        //    combined = mip->GetOutput();
        //}
        //else if (mode == "mean")
        //{
        //    auto sum = vtkSmartPointer<vtkImageMathematics>::New();
        //    sum->SetOperationToAdd();
        //    sum->SetInput1Data(sliceLayers[0]);
        //    for (size_t j = 1; j < sliceLayers.size(); ++j) {
        //        sum->SetInput2Data(sliceLayers[j]);
        //        sum->Update();
        //        sum->SetInput1Data(sum->GetOutput());
        //    }
        //    sum->Update();
        //
        //    auto avg = vtkSmartPointer<vtkImageShiftScale>::New();
        //    avg->SetInputData(sum->GetOutput());
        //    avg->SetShift(0);
        //    avg->SetScale(1.0 / sliceLayers.size());
        //    avg->SetOutputScalarTypeToFloat();
        //    avg->Update();
        //
        //    combined = avg->GetOutput();
        //}
        //else
        {
            // 默认返回中间层
            combined = sliceLayers[thickness];
        }

        finalStack->AddInputData(combined);
    }
    finalStack->Update();
    return finalStack->GetOutput();
}

vtkSmartPointer<vtkImageData> GenerateCPRImageFromViewer(    vtkResliceImageViewer* viewer,    
    const std::vector<std::array<double, 3>>& pathPoints,    int thickness = 1,    double spacing = 1.0,    const std::string& mode = "mean"
)
{
    if (!viewer || pathPoints.size() < 2)
        return nullptr;

    vtkImageData* volume = viewer->GetInput();
    if (!volume)
        return nullptr;

    // 自动获取当前切面的法向量
    double sliceNormal[3] = { 0, 0, 0 };
    int orientation = viewer->GetSliceOrientation();
    switch (orientation)
    {
    case 0: sliceNormal[2] = 1; break; // XY
    case 1: sliceNormal[1] = 1; break; // XZ
    case 2: sliceNormal[0] = 1; break; // YZ
    default: return nullptr;
    }

    // Step 1: 拟合样条线并均匀采样
    auto points = vtkSmartPointer<vtkPoints>::New();
    for (const auto& pt : pathPoints)
        points->InsertNextPoint(pt.data());

    auto spline = vtkSmartPointer<vtkParametricSpline>::New();
    spline->SetPoints(points);

    auto functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(10 * pathPoints.size()); // 插值密度
    functionSource->Update();

    auto splineOutput = functionSource->GetOutput();
    auto interpolatedPoints = splineOutput->GetPoints();
    int numSamples = interpolatedPoints->GetNumberOfPoints();

    auto stack = vtkSmartPointer<vtkImageAppend>::New();
    stack->SetAppendAxis(1); // 沿Y轴拼接

    // Step 2: 获取 volume 尺寸，在法向方向上自动设置 outputHeight
    int extent[6];
    double volSpacing[3];
    volume->GetExtent(extent);
    volume->GetSpacing(volSpacing);

    int normalAxis = 2;
    switch (orientation)
    {
    case 0: normalAxis = 2; break; // XY => Z轴
    case 1: normalAxis = 1; break; // XZ => Y轴
    case 2: normalAxis = 0; break; // YZ => X轴
    }

    int outputHeight = extent[2 * normalAxis + 1] - extent[2 * normalAxis] + 1;

    for (int i = 0; i + 1 < numSamples; ++i)
    {
        double p0[3], p1[3];
        interpolatedPoints->GetPoint(i, p0);
        interpolatedPoints->GetPoint(i + 1, p1);

        double tangent[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
        vtkMath::Normalize(tangent);

        double normal[3];
        vtkMath::Cross(tangent, sliceNormal, normal);
        vtkMath::Normalize(normal);

        double binormal[3];
        vtkMath::Cross(tangent, normal, binormal);
        vtkMath::Normalize(binormal);

        double center[3] = {
            0.5 * (p0[0] + p1[0]),
            0.5 * (p0[1] + p1[1]),
            0.5 * (p0[2] + p1[2])
        };

        double length = std::sqrt(vtkMath::Distance2BetweenPoints(p0, p1));
        int outputWidth = static_cast<int>(length / spacing);

        std::vector<vtkSmartPointer<vtkImageData>> slices;

        for (int t = -thickness / 2; t <= thickness / 2; ++t)
        {
            double offsetCenter[3] = {
                center[0] + t * spacing * sliceNormal[0],
                center[1] + t * spacing * sliceNormal[1],
                center[2] + t * spacing * sliceNormal[2]
            };

            auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();
            for (int col = 0; col < 3; ++col)
            {
                matrix->SetElement(0, col, binormal[col]);
                matrix->SetElement(1, col, normal[col]);
                matrix->SetElement(2, col, tangent[col]);
            }
            for (int row = 0; row < 3; ++row)
                matrix->SetElement(row, 3, offsetCenter[row]);
            matrix->SetElement(3, 3, 1.0);

            auto reslicer = vtkSmartPointer<vtkImageReslice>::New();
            reslicer->SetInputData(volume);
            reslicer->SetResliceAxes(matrix);
            reslicer->SetInterpolationModeToLinear();
            reslicer->SetOutputSpacing(spacing, spacing, spacing);
            //reslicer->SetOutputExtent( -outputWidth / 2, outputWidth / 2,-outputHeight / 2, outputHeight / 2, 0, 0);
            reslicer->SetOutputOrigin(0, 0, 0);
            reslicer->Update();
            slices.push_back(reslicer->GetOutput());
        }

        vtkSmartPointer<vtkImageData> mergedSlice;

        if (mode == "mip")
        {
            auto mip = vtkSmartPointer<vtkImageMathematics>::New();
            mip->SetOperationToMax();
            mip->SetInput1Data(slices[0]);
            for (size_t i = 1; i < slices.size(); ++i)
            {
                mip->SetInput2Data(slices[i]);
                mip->Update();
                mip->SetInput1Data(mip->GetOutput());
            }
            mergedSlice = mip->GetOutput();
        }
        else if (mode == "mean")
        {
            auto avg = vtkSmartPointer<vtkImageMathematics>::New();
            avg->SetOperationToAdd();
            avg->SetInput1Data(slices[0]);
            for (size_t i = 1; i < slices.size(); ++i)
            {
                avg->SetInput2Data(slices[i]);
                avg->Update();
                avg->SetInput1Data(avg->GetOutput());
            }
            avg->SetConstantK(1.0 / slices.size());
            avg->SetOperationToMultiplyByK();
            avg->Update();
            mergedSlice = avg->GetOutput();
        }

        if (mergedSlice)
            stack->AddInputData(mergedSlice);
    }

    stack->Update();
    return stack->GetOutput();
}

vtkSmartPointer<vtkImageData> GenerateCPRImage(vtkResliceImageViewer* viewer, const std::vector<std::array<double, 3>>& points,
    unsigned int cols) // 法向量方向的采样数
{
    if (!viewer || points.size() < 2 || cols == 0)
        return nullptr;

    // 获取体数据（volume）
    vtkImageData* volume = viewer->GetInput();
    if (!volume)
        return nullptr;

    // 获取当前切面法向量
    double normal[3] = { 0, 0, 0 };
    int orientation = viewer->GetSliceOrientation();
    if (orientation == 2)       // XY
    {
        normal[2] = 1;
    }
    else if (orientation == 1)  // XZ
    {
        normal[1] = 1;
    }
    else if (orientation == 0)  // YZ
    {
        normal[0] = 1;
    }

    /*
    vtkMatrix4x4* resliceAxes = viewer->GetResliceMatrix();
    double normal[3] = {
    resliceAxes->GetElement(0, 2),
    resliceAxes->GetElement(1, 2),
    resliceAxes->GetElement(2, 2)
    };
    */
    // 单位化法向量
    double mag = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    if (mag == 0) return nullptr;
    for (int i = 0; i < 3; ++i)
    {
        normal[i] /= mag;
    }

    // 获取切面上显示图像的 extent（像素数量）和 spacing（物理间距）
    int extent[6];
    volume->GetExtent(extent);
    double spacing[3];
    volume->GetSpacing(spacing);

    // 切面法向量方向上，计算体数据总高度（物理空间长度）
    double totalLength =
        std::abs(normal[0]) * (extent[1] - extent[0] + 1) * spacing[0] +
        std::abs(normal[1]) * (extent[3] - extent[2] + 1) * spacing[1] +
        std::abs(normal[2]) * (extent[5] - extent[4] + 1) * spacing[2];

    // 使用 distance = totalLength
    double distance = totalLength;

    // 构造 Sweep 面
    unsigned int rows = static_cast<unsigned int>(points.size());
    double spacingStep = distance / cols;
    cols++;  // 包含中心点
    int halfCols = static_cast<int>(cols / 2);

    vtkNew<vtkPoints> vtkPoints;
    vtkPoints->Allocate(rows * cols);
    vtkNew<vtkCellArray> polys;
    polys->AllocateEstimate((rows - 1) * (cols - 1), 4);

    unsigned int cnt = 0;
    for (unsigned int row = 0; row < rows; row++)
    {
        const auto& p = points[row];
        for (int col = -halfCols; col <= halfCols; col++)
        {
            double x[3];
            x[0] = p[0] + normal[0] * col * spacingStep;
            x[1] = p[1] + normal[1] * col * spacingStep;
            x[2] = p[2] + normal[2] * col * spacingStep;
            vtkPoints->InsertPoint(cnt++, x);
        }
    }

    for (unsigned int row = 0; row < rows - 1; row++)
    {
        for (unsigned int col = 0; col < cols - 1; col++)
        {
            vtkIdType pts[4];
            pts[0] = col + row * cols;
            pts[1] = pts[0] + 1;
            pts[2] = pts[0] + cols + 1;
            pts[3] = pts[0] + cols;
            polys->InsertNextCell(4, pts);
        }
    }

    vtkNew<vtkPolyData> sweepSurface;
    sweepSurface->SetPoints(vtkPoints);
    sweepSurface->SetPolys(polys);

    // 从体数据中采样，生成 CPR 图像
    vtkNew<vtkProbeFilter> probe;
    probe->SetInputData(sweepSurface);
    probe->SetInputConnection(g_vtkAlgorithmOutput);
    probe->SetSourceData(volume);
    probe->Update();

    return vtkImageData::SafeDownCast(probe->GetOutput());
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class QeventMouse :public QObject
{
public:
    QeventMouse() {	};
    ~QeventMouse() { };
    bool eventFilter(QObject *object, QEvent *event)
    {
        if (event->type() == QEvent::Wheel)
        {
            QWheelEvent *wheelEvent = (QWheelEvent*)(event);
            QString name = object->objectName();
            int delta = wheelEvent->delta();
            if (delta > 0)
            {
                name = object->objectName();
            }
            else
            {
                name = object->objectName();
            }
            if (name.contains("view"))
            {
                int i = name.replace("view", "").toInt() - 1;
                int now = m_riw[i]->GetSlice() + 1;
                int max = m_riw[i]->GetSliceMax() + 1;
                QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
                m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
            }
        }
        return 0;
    }
public:
    vtkCornerAnnotation *m_cornerAnnotations[3];
    vtkResliceImageViewer *m_riw[3];
private:
};
//鼠标滚动翻业重写
class vtkResliceImageViewerScrollCallback : public vtkCommand
{
public:
    static vtkResliceImageViewerScrollCallback* New()
    {
        return new vtkResliceImageViewerScrollCallback;
    }

    void Execute(vtkObject*, unsigned long ev, void*) override
    {
        if (!this->m_viewer->GetSliceScrollOnMouseWheel())
        {
            return;
        }

        // Do not process if any modifiers are ON
        if (this->m_viewer->GetInteractor()->GetShiftKey() ||
            this->m_viewer->GetInteractor()->GetControlKey() || this->m_viewer->GetInteractor()->GetAltKey())
        {
            return;
        }

        // forwards or backwards
        //int sign = (ev == vtkCommand::MouseWheelForwardEvent) ? 1 : -1;
        int sign = (ev == vtkCommand::MouseWheelForwardEvent) ? m_sliceInc : -m_sliceInc;

        this->m_viewer->IncrementSlice(sign);

        //this->Viewer->SetSlice(this->Viewer->GetSlice() + static_cast<int>(std::round(sign * 1.0)));
        // Abort further event processing for the scroll.
        this->SetAbortFlag(1);
    }

    vtkResliceImageViewerScrollCallback()
        : m_viewer(nullptr), m_sliceInc(1)
    {
    }
    void ChangeSliceScrollOnMouseWheel(bool ChangeSliceScrollOnMouseWheel = false)
    {
        m_sliceInc = (ChangeSliceScrollOnMouseWheel) ? -1 : 1;
    }
public:
    vtkResliceImageViewer* m_viewer;
    int m_sliceInc;
};
class vtkMPRResliceImageViewer :public vtkResliceImageViewer
{
public:
    static vtkMPRResliceImageViewer* New()
    {
        return new vtkMPRResliceImageViewer;
    }
    vtkMPRResliceImageViewer()
    {
        init();
    }
    void init(bool ChangeSliceScrollOnMouseWheel = false)
    {
        this->ScrollCallback = vtkResliceImageViewerScrollCallback::New();
        this->ScrollCallback->m_viewer = this;
        this->ScrollCallback->ChangeSliceScrollOnMouseWheel(ChangeSliceScrollOnMouseWheel);
    }

};
class vtkRCLP :public vtkResliceCursorLineRepresentation
{
public:
    static vtkRCLP* New()
    {
        return new vtkRCLP;
    }

    //void SetResliceParameters(double outputSpacingX, double outputSpacingY, int extentX, int extentY)override
    //{
    //	vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(this->Reslice);
    //
    //	if (reslice)
    //	{
    //		// Set the default color the minimum scalar value
    //		double range[2];
    //		vtkImageData::SafeDownCast(reslice->GetInput())->GetScalarRange(range);
    //		reslice->SetBackgroundLevel(range[0]);
    //		this->ColorMap->SetInputConnection(flip->GetOutputPort());
    //		reslice->TransformInputSamplingOff();
    //		reslice->AutoCropOutputOn();
    //		reslice->SetResliceAxes(this->ResliceAxes);
    //		reslice->SetOutputSpacing(outputSpacingX, outputSpacingY, 1);
    //		reslice->SetOutputOrigin(0.5 * outputSpacingX, 0.5 * outputSpacingY, 0);
    //		reslice->SetOutputExtent(0, extentX - 1, 0, extentY - 1, 0, 0);
    //	}
    //}
    void TransformPlane(vtkPlaneSource* planeToTransform, double targetCenter[3], double targetNormal[3], double targetViewUp[3])
    {
        planeToTransform->SetNormal(targetNormal);
        double currentViewUp[3];
        vtkMath::Subtract(planeToTransform->GetPoint2(), planeToTransform->GetOrigin(), currentViewUp);
        double angle = vtkMath::SignedAngleBetweenVectors(currentViewUp, targetViewUp, targetNormal);
        planeToTransform->Rotate(vtkMath::DegreesFromRadians(angle), targetNormal);
        planeToTransform->SetCenter(targetCenter);
    }
    void RotateASC(int axis, double angle)
    {
        //		vtkResliceCursor* rc = this->GetResliceCursor();
        //		vtkPlane* planeToBeRotated = rc->GetPlane(axis);
        //		double* viewUp = rc->GetViewUp(axis);
        //
        //		const int rcPlaneIdx = this->ResliceCursorActor->GetCursorAlgorithm()->GetReslicePlaneNormal();
        //
        //		vtkPlane* normalPlane = rc->GetPlane(rcPlaneIdx);
        //
        //		double vectorToBeRotated[3], aboutAxis[3], rotatedVector[3];
        //		planeToBeRotated->GetNormal(vectorToBeRotated);
        //		normalPlane->GetNormal(aboutAxis);
        //
        //		this->RotateVectorAboutVector(vectorToBeRotated, aboutAxis, angle, rotatedVector);
        //		this->RotateVectorAboutVector(viewUp, aboutAxis, angle, viewUp);
        //		planeToBeRotated->SetNormal(rotatedVector);
                //RotateAxis
        this->RotateAxis(axis, angle);
    }
};
//------------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand
{
public:
    static vtkResliceCursorCallback* New() { return new vtkResliceCursorCallback; }

    void Execute(vtkObject* caller, unsigned long ev, void* callData) override
    {
        AbortFlagOff();
        vtkRenderWindow* renderWindow = nullptr;;
        vtkResliceImageViewer* currentViewer = nullptr;
        auto interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
        if (interactor)
        {
            renderWindow = interactor->GetRenderWindow();
            for (int i = 0; i < 3; i++)
            {
                if (renderWindow == m_riw[i]->GetRenderWindow())
                {
                    currentViewer = m_riw[i];
                    break;
                }
            }
        }

        if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||
            ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
        {
            // Render everything
            for (int i = 0; i < 3; i++)
            {
                this->RCW[i]->Render();
            }
            this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }
        //add:20250603
        else if (ev == vtkCommand::LeftButtonDoubleClickEvent)
        {
            m_starSpline = false;
            int pointsize = m_points.size();
            if (pointsize > 1)
            {
                auto data = GenerateCPRImageFromViewer(currentViewer, m_points);//GenerateCPRImage(currentViewer, m_points, 60);// GenerateCPRImageWithThickness(currentViewer->GetInput(), m_points);
                auto viewer = vtkSmartPointer<vtkImageViewer2>::New();
                viewer->SetInputData(data);
                auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
                viewer->SetupInteractor(interactor);
                viewer->GetRenderWindow()->SetWindowName("title.c_str()");
                viewer->Render();
                interactor->Start();
            }
            //if (pointsize > 3)
            //{
            //    double normal[3] = { 0, 0, -1 };
            //    auto surface = SweepLineByPoints(m_points, normal, 150, 40);
            //
            //    // Probe the volume with the extruded surface.
            //    vtkNew<vtkProbeFilter> sampleVolume;
            //    sampleVolume->SetInputConnection(1, g_vtkAlgorithmOutput);
            //    sampleVolume->SetInputData(0, surface);
            //
            //    // Compute a simple window/level based on scalar range.
            //    vtkNew<vtkWindowLevelLookupTable> wlLut;
            //    vtkImageData *data = currentViewer->GetInput();
            //    double range = data->GetScalarRange()[1] - data->GetScalarRange()[0];
            //    double level = (data->GetScalarRange()[1] + data->GetScalarRange()[0]) / 2.0;
            //    wlLut->SetWindow(range);
            //    wlLut->SetLevel(level);
            //
            //    // Create a mapper and actor.
            //    vtkNew<vtkDataSetMapper> mapper;
            //    mapper->SetInputConnection(sampleVolume->GetOutputPort());
            //    mapper->SetLookupTable(wlLut);
            //    mapper->SetScalarRange(0, 255);
            //
            //    vtkNew<vtkActor> actor;
            //    actor->SetMapper(mapper);
            //
            //    // Create a renderer, render window, and interactor.
            //    vtkNew<vtkRenderer> renderer;
            //    vtkNew<vtkRenderWindow> renderWindow;
            //    renderWindow->AddRenderer(renderer);
            //    renderWindow->SetWindowName("CurvedReformation");
            //
            //    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
            //    renderWindowInteractor->SetRenderWindow(renderWindow);
            //
            //    //vtkNew<vtkNamedColors> colors;
            //    // Add the actors to the scene.
            //    renderer->AddActor(actor);
            //    //renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());
            //
            //    // Set the camera for viewing medical images.
            //    renderer->GetActiveCamera()->SetViewUp(0, 0, 1);
            //    renderer->GetActiveCamera()->SetPosition(0, 0, 0);
            //    renderer->GetActiveCamera()->SetFocalPoint(0, 1, 0);
            //    renderer->ResetCamera();
            //
            //    // Render and interact
            //    renderWindow->Render();
            //    renderWindowInteractor->Start();
            //}
        }
        else if (ev == vtkCommand::RightButtonPressEvent && m_starSpline)
        {
            int* clickPos = interactor->GetEventPosition();
            vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
            coordinate->SetCoordinateSystemToDisplay();
            coordinate->SetValue(clickPos[0], clickPos[1]);

            double* world = coordinate->GetComputedWorldValue(currentViewer->GetRenderer());
            m_points.push_back({ world[0], world[1], world[2] });
        }
        //end add:20250603
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        else if (ev == vtkResliceImageViewer::SliceChangedEvent)//vtkResliceCursorWidget::ResliceAxesChangedEvent)
        {
            for (int i = 0; i < 3; i++)
            {
                int now = m_riw[i]->GetSlice() + 1;
                int max = m_riw[i]->GetSliceMax() + 1;
                QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
                m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
            }
        }
        else if (ev == vtkCommand::MouseMoveEvent)//vtkResliceCursorWidget::ResliceAxesChangedEvent)
        {
            for (int i = 0; i < 3; i++)
            {
                int now = m_riw[i]->GetSlice() + 1;
                int max = m_riw[i]->GetSliceMax() + 1;
                QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
                m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
            }
            int pointsize = m_points.size();
            if (m_starSpline &&  pointsize > 0)
            {
                vtkSmartPointer<vtkPoints> vtkpoints = vtkSmartPointer<vtkPoints>::New();

                for (int i = 0; i < pointsize; ++i)
                {
                    vtkpoints->InsertNextPoint(m_points[i].at(0), m_points[i].at(1), m_points[i].at(2));
                }
                //+++
                int* clickPos = interactor->GetEventPosition();
                vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
                coordinate->SetCoordinateSystemToDisplay();
                coordinate->SetValue(clickPos[0], clickPos[1]);
                double* world = coordinate->GetComputedWorldValue(currentViewer->GetRenderer());
                vtkpoints->InsertNextPoint(world[0], world[1], world[2]);
                pointsize++;
                //++
                vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
                vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
                ids->SetNumberOfIds(pointsize);
                for (int i = 0; i < pointsize; ++i)
                {
                    ids->SetId(i, i);
                }
                lines->InsertNextCell(ids);

                vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
                polyData->SetPoints(vtkpoints);
                polyData->SetLines(lines);

                vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
                mapper->SetInputData(polyData);

                // 清除旧---- actor--------
                static std::map<vtkResliceImageViewer*, vtkSmartPointer<vtkActor>> g_ResliceImageActorMap;

                vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
                actor->SetMapper(mapper);
                actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
                actor->GetProperty()->SetLineWidth(2.0);
                actor->GetProperty()->SetOpacity(1);

                if (g_ResliceImageActorMap.count(currentViewer))
                {
                    vtkActor* oldActor = g_ResliceImageActorMap[currentViewer];
                    if (currentViewer->GetRenderer()->HasViewProp(oldActor))
                    {
                        currentViewer->GetRenderer()->RemoveActor(oldActor);
                    }
                    g_ResliceImageActorMap.erase(currentViewer);
                }
                currentViewer->GetRenderer()->AddActor(actor);
                currentViewer->GetRenderer()->Render();
                currentViewer->GetRenderer()->GetRenderWindow()->Render();

                g_ResliceImageActorMap[currentViewer] = actor;
            }
        }
        else if (ev == vtkCommand::MouseWheelForwardEvent || ev == vtkCommand::MouseWheelBackwardEvent)
        {
            for (int i = 0; i < 3; i++)
            {
                int now = m_riw[i]->GetSlice() + 1;
                int max = m_riw[i]->GetSliceMax() + 1;
                QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
                m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
            }
        }

        vtkImagePlaneWidget* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
        if (ipw)
        {
            double* wl = static_cast<double*>(callData);

            if (ipw == this->IPW[0])
            {
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[1])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[2])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
            }
        }

        vtkResliceCursorWidget* rcw = dynamic_cast<vtkResliceCursorWidget*>(caller);
        if (rcw)
        {
            vtkResliceCursorLineRepresentation* rep = dynamic_cast<vtkResliceCursorLineRepresentation*>(rcw->GetRepresentation());
            // Although the return value is not used, we keep the get calls
            // in case they had side-effects
            rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
            for (int i = 0; i < 3; i++)
            {
                vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(this->IPW[i]->GetPolyDataAlgorithm());
                ps->SetOrigin(this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetOrigin());
                ps->SetPoint1(this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint1());
                ps->SetPoint2(this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint2());

                // If the reslice plane has modified, update it on the 3D widget
                this->IPW[i]->UpdatePlacement();
            }
        }

        // Render everything
        for (int i = 0; i < 3; i++)
        {
            this->RCW[i]->Render();
        }
        this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
    }
    vtkResliceCursorCallback()
    {
        m_starSpline = false;
    }
public:
    vtkImagePlaneWidget* IPW[3];
    vtkResliceCursorWidget* RCW[3];
    vtkCornerAnnotation *m_cornerAnnotations[3];
    vtkResliceImageViewer *m_riw[3];
    ///////////////////
    bool m_starSpline;
    std::vector<std::array<double, 3>> m_points;
};

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char* argv[])
{
    this->ui = new Ui_QtVTKRenderWindows;
    this->ui->setupUi(this);

    //MprInit();
    //return;
    //vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    //reader->SetDirectoryName(argv[1]);
    //reader->Update();
    //int imageDims[3];
    //reader->GetOutput()->GetDimensions(imageDims);
    //--
    vtkNew<vtkMetaImageReader> reader;
    //std::string dir = qPrintable());//argv[1];	
    QString dir = argv[1];
    dir = "D:\\TEMP\\SZDL\\test-data\\MPR_0408.mhd";
    std::string stddir = qPrintable(dir);
    reader->SetFileName(stddir.c_str());
    reader->Update();
    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims);
    g_vtkAlgorithmOutput = reader->GetOutputPort();
    vtkImageData *imageData = reader->GetOutput();
    //vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
    //ImageFlip->SetInputData(reader->GetOutput());
    //ImageFlip->SetFilteredAxes(0);
    //ImageFlip->Update();
    //imageData = ImageFlip->GetOutput();

    for (int i = 0; i < 3; i++)
    {
        riw[i] = vtkSmartPointer<vtkMPRResliceImageViewer>::New();
        //
        if (i == 1)
        {
            riw[i]->init(true);
        }
        else
        {
            riw[i]->init();
        }
        //
        vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
        riw[i]->SetRenderWindow(renderWindow);
    }
    ////
        //--
    for (int i = 0; i < 3; i++)
    {
        m_cornerAnnotations[i] = vtkCornerAnnotation::New();
        m_cornerAnnotations[i]->GetTextProperty()->SetFontFamilyToArial();
        m_cornerAnnotations[i]->GetTextProperty()->ShadowOn();
        riw[i]->GetRenderer()->AddViewProp(m_cornerAnnotations[i]);
    }

    this->ui->view2->setRenderWindow(riw[0]->GetRenderWindow());
    riw[0]->SetupInteractor(this->ui->view2->renderWindow()->GetInteractor());

    this->ui->view3->setRenderWindow(riw[1]->GetRenderWindow());
    riw[1]->SetupInteractor(this->ui->view3->renderWindow()->GetInteractor());

    this->ui->view1->setRenderWindow(riw[2]->GetRenderWindow());
    riw[2]->SetupInteractor(this->ui->view1->renderWindow()->GetInteractor());
    for (int i = 0; i < 3; i++)
    {
        // make them all share the same reslice cursor object.
        vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());

        riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());
        rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
        //
        rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
        rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
        rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
        //
        riw[i]->SetInputData(imageData/*reader->GetOutput()*/);
        riw[i]->SetSliceOrientation(i);
        riw[i]->SetResliceModeToAxisAligned();
    }
    //----adjust ---
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, PI);
    riw[0]->GetRenderWindow()->Render();
    //---
    ////
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, PI);
    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    this->ui->view4->setRenderWindow(renderWindow);
    this->ui->view4->renderWindow()->AddRenderer(ren);
    vtkRenderWindowInteractor* iren = this->ui->view4->interactor();

    for (int i = 0; i < 3; i++)
    {
        planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        planeWidget[i]->SetInteractor(iren);
        planeWidget[i]->SetPicker(picker);
        planeWidget[i]->RestrictPlaneToVolumeOn();
        double color[3] = { 0, 0, 0 };
        color[i] = 1;
        planeWidget[i]->GetPlaneProperty()->SetColor(color);

        color[0] /= 4.0;
        color[1] /= 4.0;
        color[2] /= 4.0;
        double black[3] = { 0, 0, 0 };
        riw[i]->GetRenderer()->SetBackground(color);// black/*color*/);

        planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        planeWidget[i]->TextureInterpolateOff();
        planeWidget[i]->SetResliceInterpolateToLinear();
        planeWidget[i]->SetInputConnection(reader->GetOutputPort());
        planeWidget[i]->SetPlaneOrientation(i);
        planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
        planeWidget[i]->DisplayTextOn();
        planeWidget[i]->SetDefaultRenderer(ren);
        planeWidget[i]->SetWindowLevel(260, 50);
        planeWidget[i]->On();
        planeWidget[i]->InteractionOn();

        riw[i]->GetRenderer()->ResetCamera();
        riw[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
    }

    resliceMode(1);
    //double bounds[6];
    //imageData->GetCellBounds(imageData->GetNumberOfCells(), bounds);
    //double w = bounds[0] - bounds[0];
    ///
    m_cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

    for (int i = 0; i < 3; i++)
    {
        ///------------------
        m_cbk->m_cornerAnnotations[i] = m_cornerAnnotations[i];
        m_cbk->m_riw[i] = riw[i];
        ///----------------
        m_cbk->IPW[i] = planeWidget[i];
        m_cbk->RCW[i] = riw[i]->GetResliceCursorWidget();

        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, m_cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, m_cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, m_cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, m_cbk);
        riw[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, m_cbk);
        riw[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, m_cbk);

        riw[i]->GetInteractor()->AddObserver(vtkCommand::MouseWheelForwardEvent, m_cbk);
        riw[i]->GetInteractor()->AddObserver(vtkCommand::MouseWheelBackwardEvent, m_cbk);

        riw[i]->GetInteractor()->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, m_cbk);
        riw[i]->GetInteractor()->AddObserver(vtkCommand::RightButtonPressEvent, m_cbk);
        riw[i]->GetInteractor()->AddObserver(vtkCommand::MouseMoveEvent, m_cbk);

        // Make them all share the same color map.
        riw[i]->SetLookupTable(riw[0]->GetLookupTable());
        planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
        // planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
        planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
    }

    /////
        //add
    static QeventMouse filter;
    for (int i = 0; i < 3; i++)
    {
        filter.m_cornerAnnotations[i] = m_cornerAnnotations[i];
        filter.m_riw[i] = riw[i];
    }
    ui->view1->installEventFilter(&filter);
    ui->view2->installEventFilter(&filter);
    ui->view3->installEventFilter(&filter);
    ////
    this->ui->view1->show();
    this->ui->view2->show();
    this->ui->view3->show();

    // Set up action signals and slots
    connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
    connect(this->ui->resliceModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));
    connect(this->ui->thickModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(thickMode(int)));
    this->ui->thickModeCheckBox->setEnabled(0);

    connect(this->ui->radioButton_Max, SIGNAL(pressed()), this, SLOT(SetBlendModeToMaxIP()));
    connect(this->ui->radioButton_Min, SIGNAL(pressed()), this, SLOT(SetBlendModeToMinIP()));
    connect(this->ui->radioButton_Mean, SIGNAL(pressed()), this, SLOT(SetBlendModeToMeanIP()));
    this->ui->blendModeGroupBox->setEnabled(0);

    connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));
    connect(this->ui->AddDistance1Button, SIGNAL(pressed()), this, SLOT(AddDistanceMeasurementToView1()));

    connect(this->ui->m_pbCPR, SIGNAL(pressed()), this, SLOT(StarCPR()));
    //resliceMode(2);
};

void QtVTKRenderWindows::StarCPR()
{
    m_cbk->m_starSpline = true;
    m_cbk->m_points.clear();
}

void QtVTKRenderWindows::slotExit()
{
    qApp->exit();
}

void QtVTKRenderWindows::resliceMode(int mode)
{
    this->ui->thickModeCheckBox->setEnabled(mode ? 1 : 0);
    this->ui->blendModeGroupBox->setEnabled(mode ? 1 : 0);

    for (int i = 0; i < 3; i++)
    {
        riw[i]->SetResliceMode(mode ? 1 : 0);
        riw[i]->GetRenderer()->ResetCamera();
        riw[i]->Render();
    }
}

void QtVTKRenderWindows::thickMode(int mode)
{
    for (int i = 0; i < 3; i++)
    {
        riw[i]->SetThickMode(mode ? 1 : 0);
        riw[i]->Render();
    }
    ui->radioButton_Max->setEnabled(mode);
    ui->radioButton_Min->setEnabled(mode);
    ui->radioButton_Mean->setEnabled(mode);
}

void QtVTKRenderWindows::SetBlendMode(int m)
{
    for (int i = 0; i < 3; i++)
    {
        vtkImageSlabReslice* thickSlabReslice = vtkImageSlabReslice::SafeDownCast(vtkResliceCursorThickLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
        thickSlabReslice->SetBlendMode(m);
        riw[i]->Render();
    }
}

void QtVTKRenderWindows::SetBlendModeToMaxIP()
{
    for (int i = 0; i < 3; i++)
    {
        // make them all share the same reslice cursor object.
        vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
        //
        rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
        rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
        rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
        //
    }
    this->SetBlendMode(VTK_IMAGE_SLAB_MAX);
}

void QtVTKRenderWindows::SetBlendModeToMinIP()
{
    for (int i = 0; i < 3; i++)
    {
        // make them all share the same reslice cursor object.
        vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
        //
        rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
        rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
        rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
        //
    }
    this->SetBlendMode(VTK_IMAGE_SLAB_MIN);
}

void QtVTKRenderWindows::SetBlendModeToMeanIP()
{
    for (int i = 0; i < 3; i++)
    {
        // make them all share the same reslice cursor object.
        vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
        //
        rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
        rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
        rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
        //
    }
    this->SetBlendMode(VTK_IMAGE_SLAB_MEAN);
}

void QtVTKRenderWindows::ResetViews()
{
    // Reset the reslice image views
    for (int i = 0; i < 3; i++)
    {
        riw[i]->Reset();
    }

    // Also sync the Image plane widget on the 3D top right view with any
    // changes to the reslice cursor.
    for (int i = 0; i < 3; i++)
    {
        vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(planeWidget[i]->GetPolyDataAlgorithm());
        ps->SetNormal(riw[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
        ps->SetCenter(riw[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

        // If the reslice plane has modified, update it on the 3D widget
        this->planeWidget[i]->UpdatePlacement();
    }

    // Render in response to changes.
    this->Render();
}

void QtVTKRenderWindows::Render()
{
    for (int i = 0; i < 3; i++)
    {
        //riw[i]->Render();
        FitResliceImageToViewer(riw[i]);
    }
    this->ui->view3->renderWindow()->Render();
}

void QtVTKRenderWindows::AddDistanceMeasurementToView1()
{
    this->AddDistanceMeasurementToView(1);
}

void QtVTKRenderWindows::AddDistanceMeasurementToView(int i)
{
    // remove existing widgets.
    if (this->DistanceWidget[i])
    {
        this->DistanceWidget[i]->SetEnabled(0);
        this->DistanceWidget[i] = nullptr;
    }

    // add new widget
    this->DistanceWidget[i] = vtkSmartPointer<vtkDistanceWidget>::New();
    this->DistanceWidget[i]->SetInteractor(this->riw[i]->GetResliceCursorWidget()->GetInteractor());

    // Set a priority higher than our reslice cursor widget
    this->DistanceWidget[i]->SetPriority(this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

    vtkSmartPointer<vtkPointHandleRepresentation2D> handleRep = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
    vtkSmartPointer<vtkDistanceRepresentation2D> distanceRep = vtkSmartPointer<vtkDistanceRepresentation2D>::New();
    distanceRep->SetHandleRepresentation(handleRep);
    this->DistanceWidget[i]->SetRepresentation(distanceRep);
    distanceRep->InstantiateHandleRepresentation();
    distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
    distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());

    // Add the distance to the list of widgets whose visibility is managed based
    // on the reslice plane by the ResliceImageViewerMeasurements class
    this->riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

    this->DistanceWidget[i]->CreateDefaultRepresentation();
    this->DistanceWidget[i]->EnabledOn();
}

void QtVTKRenderWindows::MprInit()
{
    //return GPTMRP3D();
    vtkNew<vtkMetaImageReader> reader;
    //std::string dir = qPrintable());//argv[1];	
    QString dir = this->ui->m_dcmDIR->toPlainText();
    dir += "\\VTKMetaData.mhd";
    //QDir mhdDir;
    //QFileInfo file(dir);
    //if (!file.isFile())
    //{
    //	saveHDMdata(this->ui->m_dcmDIR->toPlainText());
    //}
    std::string stddir = qPrintable(dir);
    reader->SetFileName(stddir.c_str());
    reader->Update();
    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims);

    for (int i = 0; i < 3; i++)
    {
        riw[i] = vtkSmartPointer< vtkMPRResliceImageViewer >::New();
        vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
        riw[i]->SetRenderWindow(renderWindow);
    }


    this->ui->view1->setRenderWindow(riw[0]->GetRenderWindow());
    riw[0]->SetupInteractor(this->ui->view1->renderWindow()->GetInteractor());

    this->ui->view2->setRenderWindow(riw[1]->GetRenderWindow());
    riw[1]->SetupInteractor(this->ui->view2->renderWindow()->GetInteractor());

    this->ui->view3->setRenderWindow(riw[2]->GetRenderWindow());
    riw[2]->SetupInteractor(this->ui->view3->renderWindow()->GetInteractor());

    ///012  021 120 102  201 210  
    vtkImageData *imageData = reader->GetOutput();

    ///https://gitlab.kitware.com/vtk/vtk/-/issues/18726
    ///https://discourse.vtk.org/t/flip-mpr-image-become-abnormal/9931
    vtkSmartPointer< vtkRenderWindowInteractor >renderWindowInteractor[3];
    vtkSmartPointer< vtkResliceCursorLineRepresentation > vtkrclp[3];
    vtkSmartPointer< vtkResliceCursor > resliceCursor = vtkSmartPointer< vtkResliceCursor >::New();
    ///

    for (int i = 0; i < 3; i++)
    {
        // make them all share the same reslice cursor object.
        vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
        riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());
        rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
        riw[i]->SetInputData(imageData);
        //riw[i]->SetInputData(reslice->GetOutput());
        riw[i]->SetSliceOrientation(i);
        riw[i]->SetResliceModeToAxisAligned();
    }



    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

    vtkSmartPointer< vtkRenderer > ren = vtkSmartPointer< vtkRenderer >::New();

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    this->ui->view4->setRenderWindow(renderWindow);
    this->ui->view4->renderWindow()->AddRenderer(ren);
    vtkRenderWindowInteractor *iren = this->ui->view4->interactor();

    for (int i = 0; i < 3; i++)
    {
        planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        planeWidget[i]->SetInteractor(iren);
        planeWidget[i]->SetPicker(picker);
        planeWidget[i]->RestrictPlaneToVolumeOn();
        double color[3] = { 0, 0, 0 };
        color[i] = 1;
        planeWidget[i]->GetPlaneProperty()->SetColor(color);

        color[0] /= 4.0;
        color[1] /= 4.0;
        color[2] /= 4.0;
        riw[i]->GetRenderer()->SetBackground(color);

        planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        planeWidget[i]->TextureInterpolateOff();
        planeWidget[i]->SetResliceInterpolateToLinear();
        planeWidget[i]->SetInputConnection(reader->GetOutputPort());

        planeWidget[i]->SetPlaneOrientation(i);
        planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
        planeWidget[i]->DisplayTextOn();
        planeWidget[i]->SetDefaultRenderer(ren);
        planeWidget[i]->SetWindowLevel(250, 50);
        planeWidget[i]->On();
        planeWidget[i]->InteractionOn();
    }

    //add
    static QeventMouse filter;
    for (int i = 0; i < 3; i++)
    {
        filter.m_cornerAnnotations[i] = m_cornerAnnotations[i];
        filter.m_riw[i] = riw[i];
    }
    int w, h;
    w = ui->view1->size().width();
    h = ui->view1->size().height();
    ui->view1->installEventFilter(&filter);
    ui->view2->installEventFilter(&filter);
    ui->view3->installEventFilter(&filter);

    for (int i = 0; i < 3; i++)
    {
        riw[i]->SetResliceMode(1);
        riw[i]->GetRenderer()->ResetCamera();
        riw[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
        riw[i]->Render();
    }
    ////////////////////////
    vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();
    for (int i = 0; i < 3; i++)
    {
        cbk->IPW[i] = planeWidget[i];
        cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
        riw[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);

        riw[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, cbk);

        // Make them all share the same color map.
        riw[i]->SetLookupTable(riw[0]->GetLookupTable());
        planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
        //planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
        planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
    }

    this->ui->view1->show();
    this->ui->view2->show();
    this->ui->view3->show();
    ///
    this->ui->thickModeCheckBox->setEnabled(1);
    this->ui->blendModeGroupBox->setEnabled(1);
    ///
    static bool init = true;
    if (init)
    {
        init = false;
        // Set up action signals and slots
        connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
        connect(this->ui->resliceModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resliceMode(int)));
        connect(this->ui->thickModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(thickMode(int)));
        this->ui->thickModeCheckBox->setEnabled(0);

        connect(this->ui->radioButton_Max, SIGNAL(pressed()), this, SLOT(SetBlendModeToMaxIP()));
        connect(this->ui->radioButton_Min, SIGNAL(pressed()), this, SLOT(SetBlendModeToMinIP()));
        connect(this->ui->radioButton_Mean, SIGNAL(pressed()), this, SLOT(SetBlendModeToMeanIP()));
        this->ui->blendModeGroupBox->setEnabled(0);

        connect(this->ui->resetButton, SIGNAL(pressed()), this, SLOT(ResetViews()));
        connect(this->ui->AddDistance1Button, SIGNAL(pressed()), this, SLOT(AddDistanceMeasurementToView1()));

        ///---
        QString sliceInfo;
        for (int i = 0; i < 3; i++)
        {
            m_cornerAnnotations[i] = vtkCornerAnnotation::New();
            filter.m_cornerAnnotations[i] = m_cornerAnnotations[i];
            m_cornerAnnotations[i]->GetTextProperty()->SetFontFamilyToArial();
            m_cornerAnnotations[i]->GetTextProperty()->ShadowOn();
            riw[i]->GetRenderer()->AddViewProp(m_cornerAnnotations[i]);
            //riw[i]->SetSliceScrollOnMouseWheel(true);
            int now = riw[i]->GetSlice() + 1;
            int max = riw[i]->GetSliceMax() + 1;
            sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
            m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
        }
        //---------------------------------------------------------------
    }
}

void QtVTKRenderWindows::on_pushButton_clicked()
{
    double a = -0.12037446071029209*26.1;//-3.1417734245386240 ~~~
    a = -3.1415926535897931;//pi
    const double pi = -3.141592653589793238462643383279502884197169399375105820974944;
    vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, pi);
    riw[0]->GetRenderWindow()->Render();
}


void QtVTKRenderWindows::on_Sagittal_clicked()
{
    /*
    this->ui->view2->setRenderWindow(riw[0]->GetRenderWindow());
    riw[0]->SetupInteractor(this->ui->view2->renderWindow()->GetInteractor());

    this->ui->view3->setRenderWindow(riw[1]->GetRenderWindow());
    riw[1]->SetupInteractor(this->ui->view3->renderWindow()->GetInteractor());

    this->ui->view1->setRenderWindow(riw[2]->GetRenderWindow());
    riw[2]->SetupInteractor(this->ui->view1->renderWindow()->GetInteractor());
    */
    //    //m_Sagittal
    const double pi = -3.141592653589793238462643383279502884197169399375105820974944;

    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, 30 * pi/180);
    //riw[0]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, 30 * pi / 180);
    //riw[0]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(2, 30 * pi / 180);
    //riw[0]->GetRenderWindow()->Render();
    //
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, 30 * pi / 180);
    //riw[2]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, 30 * pi / 180);
    //riw[2]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(2, 30 * pi / 180);
    //riw[2]->GetRenderWindow()->Render();
    //
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[1]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, 30 * pi / 180);
    //riw[1]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[1]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, 30 * pi / 180);
    //riw[1]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[1]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(2, 30 * pi / 180);
    //riw[1]->GetRenderWindow()->Render();
}

//常见人体组织的CT值（HU）
//
//组织          CT值            组织        CT值
//骨组织      ＞400              肝脏      50～70
//钙值       80～300             脾脏      35～60
//血块       64～84             胰腺      30～55
//脑白质     25～34              肾脏      25～50
//脑灰质     28～44              肌肉      40～55
//脑脊液      3～8               胆囊      10～30
//血液       13～32            甲状腺      50～90
//血浆       3～14              脂肪     －20～－100
//渗出液      ＞15                水           0
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
#include <vtkMetaImageWriter.h>
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
#include <vtkImageShiftScale.h>
#include <vtkImagePermute.h>
#include <vtkNamedColors.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3DMapper.h>
#include <vtkContourWidget.h>
#include <vtkKochanekSpline.h>
#include <vtkMatrix3x3.h>
#include <vtkPropPicker.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>

#include "vtkXMLPolyDataReader.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkXMLPolyDataWriter.h"

#include "vtkImageEllipsoidSource.h"
#include "vtkLineSource.h"

#include "vtkSplineFilter.h"
#include "vtkPolyLine.h"
#include <QDateTime>
#include <qevent.h>
#include <QtConcurrent>

const double PI = -3.141592653589793238462643383279502884197169399375105820974944;
vtkAlgorithmOutput *g_vtkAlgorithmOutput = NULL;
#define VTKRCP    vtkResliceCursorRepresentation

///
#include"vtkPoints.h"
#include"vtkPolyDataAlgorithm.h"
#include"vtkPolyData.h"
#include"vtkCellArray.h"

#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"

///+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
///+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "vtkFrenetSerretFrame.h"
#include "vtkSplineDrivenImageSlicer.h"
#include "vtkSplineFilter.h"
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

using namespace std;
static constexpr double MinimumWindowWidth = 0.0001;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
        //https://notes.beyondxin.top/Vtk/%E9%87%8D%E5%BB%BA/CPR.html#41-vtksplinefilter
        if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||
            ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
        {
            // Render everything
            for (int i = 0; i < 3; i++)
            {
                this->RCW[i]->Render();
            }
            //this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }
        //add:20250603
        //https://notes.beyondxin.top/Vtk/%E9%87%8D%E5%BB%BA/CPR.html?h=cpr#4
        //https://www.researchgate.net/publication/357547876_A_Spline-Driven_Image_Slicer
        //https://github.com/djelouze/vtkKinship
        //http://github.com/jeromevelut/vtkKinship
        else if (ev == vtkCommand::LeftButtonDoubleClickEvent)
        {
            m_points[0][2] = m_points[1][2];
            m_starSpline   = false;
            int pointsize  = m_points.size();           
            if (pointsize > 1)
            {
                vtkNew<vtkPoints>  points_line;
                for (const auto&p : m_points)
                {
                    points_line->InsertNextPoint(p[0], p[1], p[2]);
                }
                vtkNew<vtkPolyLine>  polyLine_line;
                polyLine_line->GetPointIds()->SetNumberOfIds(points_line->GetNumberOfPoints());
                for (vtkIdType i = 0; i < points_line->GetNumberOfPoints(); i++)
                {
                    polyLine_line->GetPointIds()->SetId(i, i);
                }
                vtkNew<vtkCellArray>  cells_line;
                cells_line->InsertNextCell(polyLine_line);
                vtkNew<vtkPolyData>  polyData_line;
                polyData_line->SetPoints(points_line);
                polyData_line->SetLines(cells_line);

                vtkNew<vtkSplineFilter> spline_filter_line;
                //+++++++++++++++++++++++++++++++++++++++++
                //绘制样条线               
                spline_filter_line->SetSubdivideToLength();
                spline_filter_line->SetLength(0.2);
                spline_filter_line->SetInputData(polyData_line);
                spline_filter_line->Update();
                vtkNew<vtkPolyDataMapper> splineMapper;
                splineMapper->SetInputConnection(spline_filter_line->GetOutputPort());

                vtkNew<vtkActor> splineActor;
                splineActor->SetMapper(splineMapper);
                splineActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
                splineActor->GetProperty()->SetLineWidth(1.3);
                splineActor->GetProperty()->SetOpacity(1);//currentViewer->GetRenderer()->RemoveActor(m_oldActor);
                currentViewer->GetRenderer()->AddActor(splineActor);
                currentViewer->GetRenderer()->Render();
                currentViewer->GetRenderer()->GetRenderWindow()->Render();                                
                //+++++++++++++++++++++++++++++++++++++++++append,              
                if (m_mainWindows)
                {
                    m_mainWindows->processing(currentViewer, m_cprPoints);
                }
            }
            ///+++++++++++++++         
        }
        else if (ev == vtkCommand::RightButtonPressEvent && m_starSpline)
        {
            double outWorld[3];
            vtkImageData *image = currentViewer->GetInput();
            int* clickPos = interactor->GetEventPosition();
            vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
            coordinate->SetCoordinateSystemToDisplay();
            coordinate->SetValue(clickPos[0], clickPos[1]);
            double* world = coordinate->GetComputedWorldValue(currentViewer->GetRenderer());
            m_points.push_back({ world[0], world[1], world[2] });

            double pickPos[3];
            vtkNew<vtkPropPicker> picker;
            if (picker->Pick(clickPos[0], clickPos[1], 0, currentViewer->GetRenderer()))
            {               
                picker->GetPickPosition(pickPos);
            }
            m_cprPoints.push_back({ pickPos[0], pickPos[1], pickPos[2] });
        }
        //end add:20250603
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        else if (ev == vtkResliceImageViewer::SliceChangedEvent)//vtkResliceCursorWidget::ResliceAxesChangedEvent)
        {
            for (int i = 0; i < 3; i++)
            {
                int now = m_riw[i]->GetSlice() + 1;
                int max = m_riw[i]->GetSliceMax() + 1;
                QString sliceInfo = QObject::tr("ims: %1").arg(max);
                //QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
                m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
            }
        }
        else if (ev == vtkCommand::MouseMoveEvent)//vtkResliceCursorWidget::ResliceAxesChangedEvent)
        {
            for (int i = 0; i < 3; i++)
            {
                int now = m_riw[i]->GetSlice() + 1;
                int max = m_riw[i]->GetSliceMax() + 1;
                QString sliceInfo = QObject::tr("ims: %1").arg(max);
                //QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
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
                vtkSmartPointer<vtkIdList> ids      = vtkSmartPointer<vtkIdList>::New();
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
                actor->GetProperty()->SetColor(0.0, 1.0, 0.0);
                actor->GetProperty()->SetLineWidth(1.0);
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
                m_oldActor = actor;
            }
        }
        else if (ev == vtkCommand::MouseWheelForwardEvent || ev == vtkCommand::MouseWheelBackwardEvent)
        {
            for (int i = 0; i < 3; i++)
            {
                int now = m_riw[i]->GetSlice() + 1;
                int max = m_riw[i]->GetSliceMax() + 1;
                //QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
                QString sliceInfo = QObject::tr("ims: %1").arg(max);
                m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
            }
        }

        vtkImagePlaneWidget* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
        /*
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
        */

        // Render everything
        for (int i = 0; i < 3; i++)
        {
            this->RCW[i]->Render();
        }
        //this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
        if (this->m_cprViewer && m_bRunCPR)
        {
            vtkSliderWidget *sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);           
            if (sliderWidget )
            {
                vtkWidgetRepresentation *rsp = sliderWidget->GetRepresentation();

                vtkSliderRepresentation *sr = static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation());
                double vl = static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
                this->m_cprViewer->SetSlice(vl);
            }           
        }
    }
    vtkResliceCursorCallback()
    {
        m_starSpline  = false;
        m_cprViewer   = nullptr;
        m_bRunCPR     = false;
        m_mainWindows = nullptr;
    }
public:
    //vtkImagePlaneWidget* IPW[3];
    vtkResliceCursorWidget* RCW[3];
    vtkCornerAnnotation *m_cornerAnnotations[3];
    vtkResliceImageViewer *m_riw[3];
    vtkImageViewer2 *m_cprViewer;
    QVTKRenderWidget *m_Widget;
    vtkActor* m_oldActor;
    ///////////////////
    bool m_starSpline, m_bRunCPR;
    std::vector<std::array<double, 3>> m_points,m_cprPoints;
    double m_spacing[3], m_origin[3];
    QtVTKRenderWindows *m_mainWindows;
};

void QtVTKRenderWindows::initDir()
{
    static bool binit = true;
    if (binit)
    {
        binit = false;
        initMPR();
    }
    else
    {

    }
    
}

#include <vtkStripper.h>
#include <vtkOutlineFilter.h>
void QtVTKRenderWindows::showSRVimageSlicer(vtkImageData * itkImageData)
{
    return;
    if (!m_mHDreader)
    {
        m_mHDreader = vtkSmartPointer<vtkMetaImageReader>::New();
    }
    m_mHDreader->SetFileName("F:/temp/HealthApp/Tools/Test/CPRVTK/RelWithDebInfo/0916214701.472116.mhd");
    m_mHDreader->Update();
    itkImageData = m_mHDreader->GetOutput();
    vtkActor *vtkactor = vtkActor::New();
    vtkMarchingCubes *marchingcube = vtkMarchingCubes::New();

    marchingcube->SetInputData(itkImageData);
    marchingcube->SetValue(0, 200);//Setting the threshold;
    marchingcube->ComputeNormalsOn();//计算表面法向量;

    vtkStripper *Stripper = vtkStripper::New();
    Stripper->SetInputConnection(marchingcube->GetOutputPort());//获取三角片
    marchingcube->Delete();
    marchingcube = NULL;

    vtkPolyDataMapper *Mapper = vtkPolyDataMapper::New();//将三角片映射为几何数据；
    Mapper->SetInputConnection(Stripper->GetOutputPort());
    Stripper->Delete();
    Stripper = NULL;
    Mapper->ScalarVisibilityOff();//

    vtkactor->SetMapper(Mapper);//获得皮肤几何数据
    Mapper->Delete();
    Mapper = NULL;
    vtkactor->GetProperty()->SetDiffuseColor(.50, .50, .50);//设置皮肤颜色；
    vtkactor->GetProperty()->SetSpecular(0.3);//反射率；
    vtkactor->GetProperty()->SetOpacity(1.0);//透明度；
    vtkactor->GetProperty()->SetSpecularPower(20);//反射光强度；
    vtkactor->GetProperty()->SetColor(0.23, 0.23, 0.23);//设置角的颜色；
    vtkactor->GetProperty()->SetRepresentationToWireframe();//线框；

    vtkOutlineFilter * outfilterline = vtkOutlineFilter::New();
    outfilterline->SetInputData(itkImageData);


    vtkPolyDataMapper *outmapper = vtkPolyDataMapper::New();
    outmapper->SetInputConnection(outfilterline->GetOutputPort());
    outfilterline->Delete();
    outfilterline = NULL;

    vtkActor *OutlineActor = vtkActor::New();
    OutlineActor->SetMapper(outmapper);
    OutlineActor->GetProperty()->SetColor(0, 0, 0);//线框颜色

    outmapper->Delete();
    outmapper = NULL;
    OutlineActor->Delete();
    OutlineActor = NULL;

    vtkRenderer *vtkrenderer     = vtkRenderer::New();
    vtkRenderWindow* show3DWinow = vtkRenderWindow::New();
    show3DWinow->AddRenderer(vtkrenderer);

    vtkRenderWindowInteractor *show3DInteractor = vtkRenderWindowInteractor::New();
    show3DInteractor->SetRenderWindow(show3DWinow);

    vtkrenderer->AddActor(vtkactor);

    vtkrenderer->SetBackground(1, 1, 1);//设置背景颜色；

    show3DWinow->SetSize(500, 500);

    vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
    show3DInteractor->SetInteractorStyle(style);

    show3DWinow->Render();
    show3DWinow->SetWindowName("Image3D-Viewer");

    show3DInteractor->Initialize();
    show3DInteractor->Start();

    style->Delete();
    style = NULL;
    vtkrenderer->Delete();
    vtkrenderer = NULL;
    vtkactor->Delete();
    vtkactor = NULL;
    show3DWinow->Delete();
    show3DWinow = NULL;
    show3DInteractor->Delete();
    show3DInteractor = NULL;
}

#include <vtkSmartVolumeMapper.h>
#include <vtkImageMarchingCubes.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

//鼠标右键设置窗位

class CustomWLInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static CustomWLInteractorStyle* New();
    vtkTypeMacro(CustomWLInteractorStyle, vtkInteractorStyleTrackballCamera);

    void SetVolume(vtkVolume* volume)
    {
        this->TargetVolume = volume;
    }

    // 鼠标事件重载
    virtual void OnRightButtonDown() override
    {
        this->RightButtonPressed = true;
        this->LastX = this->Interactor->GetEventPosition()[0];
        this->LastY = this->Interactor->GetEventPosition()[1];

        if (TargetVolume)
        {
            auto colorFunc = TargetVolume->GetProperty()->GetRGBTransferFunction();
            double range[2];
            colorFunc->GetRange(range);
            InitialLevel = (range[0] + range[1]) / 2.0;
            InitialWindow = range[1] - range[0];
        }

        this->Superclass::OnRightButtonDown();
    }

    virtual void OnRightButtonUp() override
    {
        this->RightButtonPressed = false;
        this->Superclass::OnRightButtonUp();
    }
    virtual void OnMouseMove() override
    {
        if (this->RightButtonPressed && TargetVolume)
        {
            int x = this->Interactor->GetEventPosition()[0];
            int y = this->Interactor->GetEventPosition()[1];

            int dx = x - LastX;
            int dy = y - LastY;

            double newWindow = InitialWindow + dx * 5.0;  // 调整系数可调
            double newLevel = InitialLevel + dy * 2.0;

            if (newWindow < 1.0)
                newWindow = 1.0;
          
            // 重新设置传递函数
            double minVal = newLevel - newWindow / 2.0;
            double maxVal = newLevel + newWindow / 2.0;

            auto colorFunc = vtkSmartPointer<vtkColorTransferFunction>::New();
            colorFunc->AddRGBPoint(minVal, 0.0, 0.0, 0.0);
            colorFunc->AddRGBPoint(maxVal, 1.0, 1.0, 1.0);

            auto opacityFunc = vtkSmartPointer<vtkPiecewiseFunction>::New();
            opacityFunc->AddPoint(minVal, 0.0);
            opacityFunc->AddPoint(maxVal, 1.0);

            TargetVolume->GetProperty()->SetColor(colorFunc);
            TargetVolume->GetProperty()->SetScalarOpacity(opacityFunc);

            this->Interactor->GetRenderWindow()->Render();
        }
        else
        {
            this->Superclass::OnMouseMove();
        }
    }

    void OnMiddleButtonDown()
    {
        if (TargetVolume)
        {
            static bool inverted = true;
            if (inverted)
            {
                auto volumeProperty = TargetVolume->GetProperty();
                auto colorFunc = volumeProperty->GetRGBTransferFunction();

                double range[2];
                colorFunc->GetRange(range);
                double minVal = range[0];
                double maxVal = range[1];

                // 创建反色映射
                auto invertedColorFunc = vtkSmartPointer<vtkColorTransferFunction>::New();
                if (!oldColorTransferFunction)
                {
                    oldColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
                }
                oldColorTransferFunction = volumeProperty->GetRGBTransferFunction();
                // 简单处理：线性反转灰度：黑 ↔ 白
                invertedColorFunc->AddRGBPoint(minVal, 1.0, 1.0, 1.0);  // 原本是黑的变成白的
                invertedColorFunc->AddRGBPoint(maxVal, 0.0, 0.0, 0.0);  // 原本是白的变成黑的

                // 替换颜色映射
                volumeProperty->SetColor(invertedColorFunc);

                this->Interactor->GetRenderWindow()->Render();
                inverted = false;
            }
            else
            {
                inverted = true;
                // 替换颜色映射
                TargetVolume->GetProperty()->SetColor(oldColorTransferFunction);
            }
        }
        else
        {
            this->Superclass::OnMiddleButtonDown();

        }
    }

protected:
    CustomWLInteractorStyle();
    ~CustomWLInteractorStyle() override = default;

private:
    vtkSmartPointer<vtkVolume> TargetVolume;
    vtkSmartPointer<vtkColorTransferFunction> oldColorTransferFunction;

    bool RightButtonPressed = false;
    int LastX = 0;
    int LastY = 0;

    double InitialWindow = 0;
    double InitialLevel = 0;
};

vtkStandardNewMacro(CustomWLInteractorStyle);
CustomWLInteractorStyle::CustomWLInteractorStyle() = default;

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
//color>
//   <point g = "0.2980392156862745" value = "-1024" r = "1" b = "0.2156862745098039" / >
//   <point g = "0.2980392156862745" value = "236" r = "1"   b = "0.2156862745098039" / >
//   <point g = "0.2980392156862745" value = "287" r = "1"   b = "0.2156862745098039" / >
//   <point g = "0.8196078431372549" value = "535" r = "1"   b = "0.4196078431372549" / >
//   <point g = "1" value = "769" r = "1" b = "1" / >
//   <point g = "1" value = "3071" r = "1" b = "1" / >
//   < / color>
//   <opacity>
//   <point value = "-1024" a = "0" / >
//   <point value = "236" a = "0" / >
//   <point value = "287"  a = "0.1450980392156863" / >
//   <point value = "535"  a = "0.3725490196078431" / >
//   <point value = "769"  a = "0.7254901960784313" / >
//   <point value = "3071" a = "0.5176470588235295" / >
//< / opacity>
void QtVTKRenderWindows::showVolumeImageSlicer(vtkImageData * itkImageData)
{
    //w:3500 l:500
    double window = 2008, level = 404;
    window = 3500;
    level  = 500;
    // 3. 灰度映射（窗宽窗位）
    double minVal = level - window / 2.0;
    double maxVal = level + window / 2.0;

    auto colorFunc = vtkSmartPointer<vtkColorTransferFunction>::New();
    auto opacityFunc = vtkSmartPointer<vtkPiecewiseFunction>::New();
    if (0)
    {
        colorFunc->AddRGBPoint(minVal, 0.0, 0.0, 0.0); // 黑
        colorFunc->AddRGBPoint(maxVal, 1.0, 1.0, 1.0); // 白
        opacityFunc->AddPoint(minVal, 0.0);
        opacityFunc->AddPoint(maxVal, 1.0);
    }
    else
    {
        colorFunc->AddRGBPoint(-1024, 1.0, 0.2980392156862745, 0.2156862745098039);
        colorFunc->AddRGBPoint(236,   1.0, 0.2980392156862745, 0.2156862745098039);
        colorFunc->AddRGBPoint(287,   1.0, 0.2980392156862745, 0.2156862745098039);
        colorFunc->AddRGBPoint(535,   1.0, 0.8196078431372549, 0.4196078431372549);
        colorFunc->AddRGBPoint(769,   1.0, 1.0, 1.0);
        colorFunc->AddRGBPoint(3071,  1.0, 1.0, 1.0); 
        opacityFunc->AddPoint(-1024, 0.0);
        opacityFunc->AddPoint(236,   0.0);
        opacityFunc->AddPoint(287,   0.1450980392156863);
        opacityFunc->AddPoint(535,   0.3725490196078431);
        opacityFunc->AddPoint(769,   0.7254901960784313);
        opacityFunc->AddPoint(3071,  0.5176470588235295);
    }

    vtkNew<vtkVolumeProperty>     m_volumeProperty;
    vtkNew<vtkSmartVolumeMapper>  m_volumeMapper;
    //vtkNew<vtkImageMarchingCubes> m_isosurfaceFilter;

    m_volumeMapper->SetInputData(itkImageData);
    m_volumeMapper->SetBlendModeToComposite();
    m_volumeMapper->SetRequestedRenderModeToDefault();

    // force the mapper to compute a sample distance based on data spacing
    m_volumeMapper->SetSampleDistance(-1.0);

    m_volumeProperty->SetColor(colorFunc);
    m_volumeProperty->SetScalarOpacity(opacityFunc);
    m_volumeProperty->SetColor(colorFunc);
    m_volumeProperty->ShadeOff();

    //m_volumeMapper->SetRequestedRenderModeToGPU(); // 强制使用 GPU
    //m_isosurfaceFilter->SetInputData(itkImageData);
    vtkNew<vtkVolume> m_vtkVolume;
    m_vtkVolume->SetProperty(m_volumeProperty);
    m_vtkVolume->SetMapper(m_volumeMapper);

    vtkNew <vtkRenderer> m_renderer;
    m_renderer->AddViewProp(m_vtkVolume);
    m_renderer->AddVolume(m_vtkVolume);
    m_renderer->SetBackground(0, 0, 0);
    m_renderer->ResetCamera();
    m_renderer->GetActiveCamera()->Zoom(1.5);
    //重设相机的剪切范围；
    m_renderer->ResetCameraClippingRange();

    //透明度参数
    m_renderer->SetUseDepthPeeling(1);
    m_renderer->SetMaximumNumberOfPeels(100);
    m_renderer->SetOcclusionRatio(0.1);

    ui->view4->renderWindow()->AddRenderer(m_renderer);
    //
    vtkNew < vtkRenderWindowInteractor> m_renderWindowInteractor;
    m_renderWindowInteractor->SetRenderWindow(ui->view4->renderWindow());
    //设置相机跟踪模式
    vtkNew < CustomWLInteractorStyle> m_interactorstyle;
    m_interactorstyle->SetVolume(m_vtkVolume);
    m_renderWindowInteractor->SetInteractorStyle(m_interactorstyle);

    ui->view4->renderWindow()->Render();
}

class DataInfo
{
public:
    DataInfo( std::array<int, 3> id = { 0, 0, 0 })
    {
        m_id[0]           = id[0];
        m_id[1]           = id[1];
        m_id[2]           = id[2];
        m_VectorImageData.clear();
    }
    ~DataInfo()
    {

    }
    void SetPoints(std::vector<std::array<double, 3>> points, vtkImageData *imageData)
    {
        m_points = vtkSmartPointer <vtkPoints>::New();
        for (const auto&p : points)
        {
            m_points->InsertNextPoint(p[0], p[1], p[2]);
        }
        m_polyLine = vtkSmartPointer<vtkPolyLine>::New();
        m_polyLine->GetPointIds()->SetNumberOfIds(m_points->GetNumberOfPoints());
        for (vtkIdType i = 0; i < m_points->GetNumberOfPoints(); i++)
        {
            m_polyLine->GetPointIds()->SetId(i, i);
        }
        m_cells = vtkSmartPointer<vtkCellArray>::New();
        m_cells->InsertNextCell(m_polyLine);

        m_polyData = vtkSmartPointer<vtkPolyData>::New();
        m_polyData->SetPoints(m_points);
        m_polyData->SetLines(m_cells);

        m_spline_filter = vtkSmartPointer<vtkSplineFilter>::New();
        m_spline_filter->SetSubdivideToLength();// 按弧长
        m_spline_filter->SetLength(0.2);
        m_spline_filter->SetInputData(m_polyData);
        m_spline_filter->Update();

        m_reslicer = vtkSmartPointer<vtkSplineDrivenImageSlicer>::New();
        m_reslicer->SetInputData(0, imageData);
        m_reslicer->SetPathConnection(m_spline_filter->GetOutputPort());
        m_reslicer->SetSliceExtent(200, 80);
        m_reslicer->SetSliceThickness(1);
    }
public:
    std::vector<vtkSmartPointer<vtkImageData>> m_VectorImageData;
    vtkSmartPointer<vtkPoints> m_points;
    vtkSmartPointer<vtkPolyLine> m_polyLine;
    vtkSmartPointer<vtkCellArray> m_cells;
    vtkSmartPointer<vtkPolyData> m_polyData;
    vtkSmartPointer<vtkSplineFilter> m_spline_filter;
    vtkSmartPointer<vtkSplineDrivenImageSlicer> m_reslicer;
    int m_id[3];
};

std::vector<vtkSmartPointer<vtkImageData>> threadSplineDrivenImageSlicer(DataInfo info)
{
    std::vector<vtkSmartPointer<vtkImageData>> slices;         
    int s = info.m_id[0], l = info.m_id[1] + 1;
    for (int pt_id = s; pt_id < l; pt_id++)
    {
        double range[2];
        info.m_reslicer->SetOffsetPoint(pt_id);
        info.m_reslicer->Update();
        info.m_reslicer->GetOutput()->GetScalarRange(range);
        if (range[0] != range[1])
        {
            vtkImageData *data = info.m_reslicer->GetOutput();//  info.m_slicer->GetOutput();
            vtkSmartPointer<vtkImageData> tempSlice = vtkSmartPointer<vtkImageData>::New();
            tempSlice->DeepCopy(data);
            slices.push_back(tempSlice);
        }
    }          
    return slices;
}

void QtVTKRenderWindows::processing(vtkResliceImageViewer *viewer, std::vector<std::array<double, 3>> m_points, int channel)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer <vtkPoints>::New();
    for (const auto&p : m_points)
    {
        points->InsertNextPoint(p[0], p[1], p[2]);
    }

    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(cells);

    vtkSmartPointer<vtkSplineFilter> spline_filter = vtkSmartPointer<vtkSplineFilter>::New();
    spline_filter->SetSubdivideToLength();// 按弧长
    spline_filter->SetLength(0.2);
    spline_filter->SetInputData(polyData);
    spline_filter->Update();

    vtkSmartPointer<vtkImageAppend>  append3D = vtkSmartPointer<vtkImageAppend>::New();
    append3D->SetAppendAxis(2);    //append->SetAppendAxis(1);
    vtkSmartPointer<vtkSplineDrivenImageSlicer> reslicer = vtkSmartPointer<vtkSplineDrivenImageSlicer>::New();
    vtkImageData * imageData = viewer->GetInput();
    reslicer->SetInputData(0, viewer->GetInput());
    reslicer->SetPathConnection(spline_filter->GetOutputPort());
    reslicer->SetSliceExtent(200, 80);
    reslicer->SetSliceThickness(1);   //reslicer->SetProbeInput(0);  //reslicer->SetSliceSpacing(0.1, 0.1);   //reslicer->SetIncidence(2 * 3.1415926 / 3);
    //double bounds[6];   viewer->GetInput()->GetBounds(bounds);  //std::cout << "Volume bounds: " << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " " << bounds[4] << " " << bounds[5] << std::endl;
    //---->thread
    long long nb_points = spline_filter->GetOutput()->GetNumberOfPoints();
    if (0)
    {
        for (int pt_id = 0; pt_id < nb_points; pt_id++)
        {
            //double p[3]; spline_filter->GetOutput()->GetPoint(pt_id, p); //std::cout << "Centerline point " << pt_id << ": " << p[0] << " " << p[1] << " " << p[2] << std::endl;      //if (p[0] >= bounds[0] && p[0] <= bounds[1]&& p[1] >= bounds[2] && p[1] <= bounds[3]&& p[2] >= bounds[4] && p[2] <= bounds[5])        //{
            reslicer->SetOffsetPoint(pt_id);//double *pt3 = spline_filter->GetOutput()->GetPoint(pt_id);
            reslicer->Update();
            double range[2];
            reslicer->GetOutput()->GetScalarRange(range);
            if (range[0] != range[1])
            {
                vtkNew<vtkImageData> tempSlice;
                tempSlice->DeepCopy(reslicer->GetOutput());           //append->AddInputData(tempSlice);
                append3D->AddInputData(tempSlice);
            }
        }
    }
    else
    {
#define  MAX_THREAD  3 //常规下不要超过  max_n = (CPU核心数的2倍 - 1), 否则可能慢！(服务器自行考虑)
        //vtkSmartPointer<vtkSplineDrivenImageSlicer> reslicerTemp[MAX_THREAD];// ;vtkNew<vtkSplineDrivenImageSlicer> 
        //for (int i = 0; i < MAX_THREAD; i++)
        //{
        //    reslicerTemp[i] = vtkSmartPointer<vtkSplineDrivenImageSlicer>::New();
        //    reslicerTemp[i]->SetInputData(0, viewer->GetInput());
        //    reslicerTemp[i]->SetPathConnection(spline_filter->GetOutputPort());
        //    reslicerTemp[i]->SetSliceExtent(200, 80);
        //    reslicerTemp[i]->SetSliceThickness(1);
        //}
        QList<DataInfo> listDatas;
        for (int i = 0; i < MAX_THREAD; i++)
        {
            int id[4];
            id[0] = i * nb_points / MAX_THREAD;
            id[1] = (i + 1)*nb_points / MAX_THREAD - 1;
            id[2] = i;
            DataInfo info({ id[0], id[1], id[2] });
            info.SetPoints(m_points,viewer->GetInput());
            listDatas.push_back(info);
        }
        auto listFuture = QtConcurrent::mapped(listDatas, threadSplineDrivenImageSlicer);
        listFuture.waitForFinished();
        
        for (const auto& sliceList : listFuture.results())
        {
            for (const auto& slice : sliceList)
            {
                append3D->AddInputData(slice);
            }
        } 
    }  
    ///--->thread
    append3D->Update();
    vtkImageData * itkImageData = append3D->GetOutput();
    showVolumeImageSlicer(itkImageData);
    return;

    QDateTime dateTime = QDateTime::currentDateTime();
    QString str        = dateTime.toString("/MMddhhmmss.mhd");// 将日期时间格式化为字符串
    QString DicomDir   = QCoreApplication::applicationDirPath();   
    std::string Input_Name           = qPrintable(DicomDir);
    std::string path                 = Input_Name + qPrintable(str);
    vtkMetaImageWriter *vtkdatawrite = vtkMetaImageWriter::New();
    vtkdatawrite->SetInputData(itkImageData);    //vtkdatawrite->SetInputData(flip_filter->GetOutput());
    vtkdatawrite->SetFileName(path.c_str());
    vtkdatawrite->Write();
    vtkdatawrite->Delete();

}

void QtVTKRenderWindows::showCPRimageSlicer(vtkImageData * itkImageData)
{
    if (!itkImageData)
    {
        return;
    }
    //viewer->SetInputData(flip_filter->GetOutput());
    m_cprViewer->SetInputData(itkImageData);
    //m_cprViewer->SetInputConnection(append3D->GetOutputPort());
    m_cprViewer->SetColorWindow(2000); // 根据CT值调
    m_cprViewer->SetColorLevel(500);
    m_cprViewer->SetRenderWindow(this->ui->view4->renderWindow());
    m_cprViewer->SetSliceOrientationToXY();  // or ToYZ(), ToXZ()
    m_cprViewer->SetSlice(m_cprViewer->GetSliceMin());  // 初始显示最前面切片
    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> vtkInteractorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_cprViewer->SetupInteractor(interactor);
    m_cprViewer->GetRenderer()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(vtkInteractorStyle);
    //m_cprViewer->GetRenderWindow()->SetWindowName("Test-CPR");
    vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
    sliderRep->SetMinimumValue(m_cprViewer->GetSliceMin());
    sliderRep->SetMaximumValue(m_cprViewer->GetSliceMax());
    sliderRep->SetValue(1.0);
    sliderRep->GetTitleProperty()->SetColor(1, 0, 0);//red
    sliderRep->GetLabelProperty()->SetColor(1, 0, 0);//red
    sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    sliderRep->GetPoint1Coordinate()->SetValue(40, 40);
    sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    sliderRep->GetPoint2Coordinate()->SetValue(500, 40);
    vtkSmartPointer<vtkSliderWidget> sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
    sliderWidget->SetInteractor(interactor);
    sliderWidget->SetRepresentation(sliderRep);
    sliderWidget->SetAnimationModeToAnimate();
    sliderWidget->EnabledOn();
    m_cprViewer->Render();
}

void QtVTKRenderWindows::initMPR()
{
    QString dir = ui->m_dcmDIR->toPlainText(); //dir = "D:/test-data/MPR_0408.mhd";
    std::string stddir = qPrintable(dir);
    m_mHDreader->SetFileName(stddir.c_str());
    m_mHDreader->Update();
    int imageDims[3];
    m_mHDreader->GetOutput()->GetDimensions(imageDims);
    g_vtkAlgorithmOutput    = m_mHDreader->GetOutputPort();
    vtkImageData *imageData = m_mHDreader->GetOutput();

    for (int i = 0; i < 3; i++)
    {
        riw[i] = vtkSmartPointer<vtkMPRResliceImageViewer>::New();
        //
        if (i == 1)  { riw[i]->init(true);  }
        else         { riw[i]->init();      }
        //
        vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
        riw[i]->SetRenderWindow(renderWindow);
    }

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

    riw[0]->GetRenderWindow()->Render();
    //---
    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkRenderWindowInteractor* iren = this->ui->view3->interactor();
    /**/
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
        riw[i]->GetRenderer()->SetBackground(black);// black /color);

        planeWidget[i]->SetTexturePlaneProperty(ipwProp);
        planeWidget[i]->TextureInterpolateOff();
        planeWidget[i]->SetResliceInterpolateToLinear();
        planeWidget[i]->SetInputConnection(m_mHDreader->GetOutputPort());
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
    m_cprViewer = vtkSmartPointer<vtkImageViewer2>::New();
    vtkNew<vtkGenericOpenGLRenderWindow> cprrenderWindow;
    ui->view4->setRenderWindow(cprrenderWindow);
    //m_cprViewer->SetRenderWindow(ui->view4->renderWindow());
    m_cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();
    m_cbk->m_cprViewer = m_cprViewer;
    m_cbk->m_Widget = ui->view4;
    m_cbk->m_mainWindows = this;
    for (int i = 0; i < 3; i++)
    {
        ///------------------
        m_cbk->m_cornerAnnotations[i] = m_cornerAnnotations[i];
        m_cbk->m_riw[i] = riw[i];
        ///----------------
        //m_cbk->IPW[i] = planeWidget[i];
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

    this->ui->view1->show();
    this->ui->view2->show();
    this->ui->view3->show();

    
}

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char* argv[])
{
    this->ui = new Ui_QtVTKRenderWindows;
    this->ui->setupUi(this);
    m_mHDreader = nullptr;
    
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

    connect(this->ui->m_Dir, SIGNAL(pressed()), this, SLOT(initDir()));

    QString dir = QCoreApplication::applicationDirPath() + "/3DMetaData.mhd";
    ui->m_dcmDIR->setPlainText(dir);
    vtkImageData * itkImageData = nullptr;
    showSRVimageSlicer(itkImageData);
    //showVolumeImageSlicer(itkImageData);
    if (!m_mHDreader)
    {
        m_mHDreader = vtkSmartPointer<vtkMetaImageReader>::New();
    }
};

void QtVTKRenderWindows::StarCPR()
{
    m_cbk->m_starSpline = true;
    m_cbk->m_points.clear();
    m_cbk->m_cprPoints.clear();
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

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
            this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }
        //add:20250603
        //https://notes.beyondxin.top/Vtk/%E9%87%8D%E5%BB%BA/CPR.html?h=cpr#4
        //https://www.researchgate.net/publication/357547876_A_Spline-Driven_Image_Slicer
        //https://github.com/djelouze/vtkKinship
        //http://github.com/jeromevelut/vtkKinship
        else if (ev == vtkCommand::LeftButtonDoubleClickEvent)
        {
            m_starSpline   = false;
            int pointsize  = m_points.size();           
            if (pointsize > 1)
            {
                m_curePoints[0][2] = m_curePoints[1][2];
                double origin[3], spacing[3];
                currentViewer->GetInput()->GetOrigin(origin);
                currentViewer->GetInput()->GetSpacing(spacing);
                vtkNew<vtkPoints> points, points_line;
                for (const auto&p : m_points)
                {
                    points->InsertNextPoint(p[0], p[1], p[2]);
                }
                for (const auto&p : m_curePoints)
                {
                    points_line->InsertNextPoint(p[0], p[1], p[2]);
                }
                vtkNew<vtkPolyLine> polyLine, polyLine_line;
                polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
                polyLine_line->GetPointIds()->SetNumberOfIds(points_line->GetNumberOfPoints());
                for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
                {
                    polyLine->GetPointIds()->SetId(i, i);
                    polyLine_line->GetPointIds()->SetId(i, i);
                }
                vtkNew<vtkCellArray> cells, cells_line;
                cells->InsertNextCell(polyLine);
                cells_line->InsertNextCell(polyLine_line);
                vtkNew<vtkPolyData> polyData, polyData_line;
                polyData->SetPoints(points);
                polyData->SetLines(cells);
                polyData_line->SetPoints(points_line);
                polyData_line->SetLines(cells_line);

                vtkNew<vtkSplineFilter> spline_filter, spline_filter_line;
                spline_filter->SetSubdivideToLength();// 按弧长
                spline_filter->SetLength(5);//spline_filter->SetSubdivideToSpecified(); //
                //spline_filter->SetNumberOfSubdivisions(50);
                spline_filter->SetInputData(polyData);//(poly_data);
                spline_filter->Update();

                //+++++++++++++++++++++++++++++++++++++++++
                //绘制样条线
                
                spline_filter_line->SetSubdivideToLength();
                spline_filter_line->SetLength(5);
                spline_filter_line->SetInputData(polyData_line);
                spline_filter_line->Update();
                vtkNew<vtkPolyDataMapper> splineMapper;
                splineMapper->SetInputConnection(spline_filter_line->GetOutputPort());

                vtkNew<vtkActor> splineActor;
                splineActor->SetMapper(splineMapper);
                splineActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
                splineActor->GetProperty()->SetLineWidth(3.0);
                splineActor->GetProperty()->SetOpacity(1);
                //currentViewer->GetRenderer()->RemoveActor(m_oldActor);
                currentViewer->GetRenderer()->AddActor(splineActor);
                currentViewer->GetRenderer()->Render();
                currentViewer->GetRenderer()->GetRenderWindow()->Render();                
                
                //+++++++++++++++++++++++++++++++++++++++++
                vtkNew<vtkImageAppend> append, append3D;
                append3D->SetAppendAxis(2);
                append->SetAppendAxis(1);

                vtkNew<vtkSplineDrivenImageSlicer> reslicer;
                reslicer->SetInputData(0, currentViewer->GetInput());
                //reslicer->SetInputData(1, spline_filter->GetOutput());
                reslicer->SetPathConnection(spline_filter->GetOutputPort());
                reslicer->SetSliceExtent(100,100);
                reslicer->SetSliceThickness(1);
                //reslicer->SetProbeInput(0);
                //reslicer->SetSliceSpacing(0.1, 0.1);
                reslicer->SetIncidence(2 * 3.1415926 / 3);
                double bounds[6];
                currentViewer->GetInput()->GetBounds(bounds);
                std::cout << "Volume bounds: "  << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " "
                          << bounds[4] << " " << bounds[5] << std::endl;
                long long nb_points = spline_filter->GetOutput()->GetNumberOfPoints();
                
                for (int pt_id = 0; pt_id < nb_points; pt_id++)
                {
                    double p[3]; spline_filter->GetOutput()->GetPoint(pt_id, p);
                    //std::cout << "Centerline point " << pt_id << ": " << p[0] << " " << p[1] << " " << p[2] << std::endl;
                    //if (p[0] >= bounds[0] && p[0] <= bounds[1]&& p[1] >= bounds[2] && p[1] <= bounds[3]&& p[2] >= bounds[4] && p[2] <= bounds[5])
                    //{
                    reslicer->SetOffsetPoint(pt_id);//double *pt3 = spline_filter->GetOutput()->GetPoint(pt_id);
                    reslicer->Update();
                    double range[2];
                    reslicer->GetOutput()->GetScalarRange(range);
                    if (range[0] != range[1])
                    {
                        vtkNew<vtkImageData> tempSlice;
                        tempSlice->DeepCopy(reslicer->GetOutput());
                        append->AddInputData(tempSlice);
                        append3D->AddInputData(tempSlice);
                    }
                    //}

                }           
                //
                append->Update();
                append3D->Update();
                double range[2];
                append->GetOutput()->GetScalarRange(range);
                std::cout << "CPR scalar range: " << range[0] << "," << range[1] << std::endl;
                //-----------------------
                // 获取当前日期时间
                QDateTime dateTime = QDateTime::currentDateTime();
                // 将日期时间格式化为字符串
                QString str = dateTime.toString("/MMddhhmmss.mhd");
                QString DicomDir            =  QCoreApplication::applicationDirPath();
                vtkImageData * itkImageData = append3D->GetOutput();
                std::string Input_Name      = qPrintable(DicomDir);
                std::string path            = Input_Name + qPrintable(str);               
                vtkMetaImageWriter *vtkdatawrite = vtkMetaImageWriter::New();
                vtkdatawrite->SetInputData(itkImageData);
                vtkdatawrite->SetFileName(path.c_str());
                vtkdatawrite->Write();
                vtkdatawrite->Delete();
                //-----------------------
                auto viewer = vtkSmartPointer<vtkImageViewer2>::New();
                //viewer->SetInputData(flip_filter->GetOutput());
                viewer->SetInputData(append->GetOutput());
                viewer->SetColorWindow(2000); // 根据CT值调
                viewer->SetColorLevel(500);
                auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
                viewer->SetupInteractor(interactor);
                viewer->GetRenderWindow()->SetWindowName("Test-CPR");
                viewer->Render();
                interactor->Start();
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
            double pickPos[3];
            vtkNew<vtkPropPicker> picker;
            if (picker->Pick(clickPos[0], clickPos[1], 0, currentViewer->GetRenderer()))
            {               
                picker->GetPickPosition(pickPos);
            }
            m_points.push_back({ pickPos[0], pickPos[1], pickPos[2] });
            m_curePoints.push_back({ world[0], world[1], world[2] });          
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
    vtkActor* m_oldActor;
    ///////////////////
    bool m_starSpline;
    std::vector<std::array<double, 3>> m_points,m_curePoints;

    //
    double m_spacing[3], m_origin[3];
};

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char* argv[])
{
    this->ui = new Ui_QtVTKRenderWindows;
    this->ui->setupUi(this);
    
    vtkNew<vtkMetaImageReader> reader;
    //std::string dir = qPrintable());//argv[1];	
    QString dir = argv[1];
    dir = "D:/Test_DICOM/INCISIX-Dental_MHD/3DMetaData.mhd";
    //dir = "D:/TEMP/SZDL/test-data/MPR_0408.mhd";
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
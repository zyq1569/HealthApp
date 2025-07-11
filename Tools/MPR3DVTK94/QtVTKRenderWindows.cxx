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
#include <vtkImageShiftScale.h>
#include <vtkImagePermute.h>
#include <vtkNamedColors.h>

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
/**
 * @brief The CBCTFrenetSerretFrame class
 * 弗莱纳公式（Frenet–Serret formulas）
 */
/*
class CBCTFrenetSerretFrame : public vtkPolyDataAlgorithm
{
public:
    vtkTypeMacro(CBCTFrenetSerretFrame, vtkPolyDataAlgorithm)
    static CBCTFrenetSerretFrame *New();
    vtkSetMacro(view_up_, double)
    vtkGetMacro(view_up_, double)
    static void RotateVector(double *vector, const double *axis, double angle);
protected:
    CBCTFrenetSerretFrame();
    ~CBCTFrenetSerretFrame() override;
    virtual int RequestData( vtkInformation *, vtkInformationVector **, vtkInformationVector *)override;
    virtual int FillInputPortInformation(int port, vtkInformation *info)override;
    // 计算导数
    void ComputeTangentVectors(vtkIdType pointIdNext, vtkIdType pointIdLast, double *tangent);
    // 计算二阶导数
    void ComputeNormalVectors(double *tgNext, double *tgLast, double *normal);
    // 由切线定义的平面上最后法线的投影
    void ComputeConsistentNormalVectors(double *tangent, double *lastNormal, double *normal);
private:
    CBCTFrenetSerretFrame(const CBCTFrenetSerretFrame &);
    void operator=(const CBCTFrenetSerretFrame &);
    double view_up_;
    int consistent_normals_;
};

vtkStandardNewMacro(CBCTFrenetSerretFrame);

CBCTFrenetSerretFrame::CBCTFrenetSerretFrame()
{
    this->consistent_normals_ = 1;
    this->view_up_ = 0;
}

CBCTFrenetSerretFrame::~CBCTFrenetSerretFrame()
{
}

int CBCTFrenetSerretFrame::FillInputPortInformation(int port, vtkInformation *info)
{
    if (port == 0)
    {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    }
    return 1;
}

int CBCTFrenetSerretFrame::RequestData( vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    vtkPolyData *input = vtkPolyData::SafeDownCast( inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast( outInfo->Get(vtkDataObject::DATA_OBJECT()));
    output->DeepCopy(input);
    vtkDoubleArray *tangents = vtkDoubleArray::New();
    tangents->SetNumberOfComponents(3);
    tangents->SetNumberOfTuples(input->GetNumberOfPoints());
    tangents->SetName("FSTangents");
    vtkDoubleArray *normals = vtkDoubleArray::New();
    normals->SetNumberOfComponents(3);
    normals->SetNumberOfTuples(input->GetNumberOfPoints());
    normals->SetName("FSNormals");
    vtkCellArray *lines = output->GetLines();
    lines->InitTraversal();
    vtkIdType nbPoints, *points;
    int cellIdx;
    for (cellIdx = 0; cellIdx < lines->GetNumberOfCells(); cellIdx++)
    {

        lines->GetNextCell(nbPoints, (vtkIdType const*&)points);
        for (int i = 0; i < nbPoints; i++)
        {
            double tangent[3];
            if (i == 0)
            {
                this->ComputeTangentVectors(points[0], points[1], tangent);
            }
            else if (i == nbPoints - 1)
            {
                this->ComputeTangentVectors(points[nbPoints - 2], points[nbPoints - 1], tangent);
            }
            else
            {
                this->ComputeTangentVectors(points[i - 1], points[i + 1], tangent);
            }
            vtkMath::Normalize(tangent);
            tangents->SetTuple(points[i], tangent);
        }
        for (int i = 0; i < nbPoints; i++)
        {
            if (!this->consistent_normals_ || i == 0)
            {
                double tangentLast[3], tangentNext[3], normal[3];
                if (i == 0)
                {
                    tangents->GetTuple(points[i], tangentLast);
                }
                else
                {
                    tangents->GetTuple(points[i - 1], tangentLast);
                }
                if (i == nbPoints - 1)
                {
                    tangents->GetTuple(points[i], tangentNext);
                }
                else
                {
                    tangents->GetTuple(points[i + 1], tangentNext);
                }
                this->ComputeNormalVectors(tangentLast, tangentNext, normal);
                if (this->consistent_normals_)
                {
                    this->RotateVector(normal, tangentLast, this->view_up_);
                }
                vtkMath::Normalize(normal);
                normals->SetTuple(points[i], normal);
            }
            if (this->consistent_normals_ && i != 0)
            {
                double tangent[3], lastNormal[3], normal[3];
                normals->GetTuple(points[i - 1], lastNormal);
                tangents->GetTuple(points[i], tangent);
                this->ComputeConsistentNormalVectors(tangent, lastNormal, normal);
                vtkMath::Normalize(normal);
                normals->SetTuple(points[i], normal);
            }
        }
    }
    output->GetPointData()->AddArray(normals);
    output->GetPointData()->AddArray(tangents);
    normals->Delete();
    tangents->Delete();
    return 1;
}

void CBCTFrenetSerretFrame::ComputeTangentVectors( vtkIdType pointIdNext, vtkIdType pointIdLast, double *tangent)
{
    vtkPolyData *input = static_cast<vtkPolyData *>(this->GetInput(0));
    double ptNext[3];
    double ptLast[3];
    input->GetPoint(pointIdNext, ptNext);
    input->GetPoint(pointIdLast, ptLast);
    int comp;
    for (comp = 0; comp < 3; comp++)
    {
        tangent[comp] = (ptLast[comp] - ptNext[comp]) / 2;
    }
}

void CBCTFrenetSerretFrame::ComputeConsistentNormalVectors(double *tangent,   double *normalLast,   double *normal)
{
    double temp[3];
    vtkMath::Cross(normalLast, tangent, temp);
    vtkMath::Cross(tangent, temp, normal);
}

void CBCTFrenetSerretFrame::ComputeNormalVectors(double *tgNext,  double *tgLast, double *normal) 
{
    int comp;
    for (comp = 0; comp < 3; comp++)
    {
        normal[comp] = (tgNext[comp] - tgLast[comp]);
    }
    if (vtkMath::Norm(normal) == 0)
    {
        double unit[3] = { 1, 0, 0 };
        vtkMath::Cross(tgLast, unit, normal);
    }
}

void CBCTFrenetSerretFrame::RotateVector(double *vector, const double *axis, double angle)
{
    double UdotN = vtkMath::Dot(vector, axis);
    double NvectU[3];
    vtkMath::Cross(axis, vector, NvectU);
    for (int comp = 0; comp < 3; comp++)
    {
        vector[comp] = cos(angle) * vector[comp]
            + (1 - cos(angle)) * UdotN * axis[comp]
            + sin(angle) * NvectU[comp];
    }
}
*/
/**
 * @brief The CBCTSplineDrivenImageSlicer class
 * 沿着样条曲线切割图片
 */
/*


class CBCTSplineDrivenImageSlicer : public vtkImageAlgorithm 
{
public:
    vtkTypeMacro(CBCTSplineDrivenImageSlicer, vtkImageAlgorithm)
        static CBCTSplineDrivenImageSlicer *New();
    void SetPathConnection(int id, vtkAlgorithmOutput *algOutput);
    void SetPathConnection(vtkAlgorithmOutput *algOutput);
    vtkAlgorithmOutput *GetPathConnection();
    vtkSetMacro(offset_point_, vtkIdType)
protected:
    CBCTSplineDrivenImageSlicer();
    ~CBCTSplineDrivenImageSlicer()override;

    virtual int RequestData(vtkInformation *, vtkInformationVector **,
        vtkInformationVector *)override;
    virtual int FillInputPortInformation(int port, vtkInformation *info)override;
    virtual int FillOutputPortInformation(int, vtkInformation *)override;
    virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
        vtkInformationVector *)override;
private:
    CBCTSplineDrivenImageSlicer(const CBCTSplineDrivenImageSlicer &);
    void operator=(const CBCTSplineDrivenImageSlicer &);
    CBCTFrenetSerretFrame *local_frenetFrames_;
    vtkImageReslice *reslicer_;
    int slice_extent_[2]; // 输出image的xy像素数量
    double slice_spacing_[2]; // 输出image的xy间隔
    double slice_thickness_; // 输出image的z轴间隔
    double incidence_;// 初始法向量的旋转
    vtkIdType offset_point_;
    vtkIdType offset_line_;
    vtkIdType probe_input_;
};

#include"vtkPoints.h"
#include"vtkPolyData.h"
#include"vtkCellArray.h"
#include "vtkImageReslice.h"

//#include "vtkFrenetSerretFrame.h"
#include "vtkPlaneSource.h"
#include "vtkImageData.h"
#include "vtkProbeFilter.h"
#include "vtkMatrix4x4.h"
#include "vtkImageAppend.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkMath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSplineFilter.h"

vtkStandardNewMacro(CBCTSplineDrivenImageSlicer);

CBCTSplineDrivenImageSlicer::CBCTSplineDrivenImageSlicer()
{
    this->local_frenetFrames_ = CBCTFrenetSerretFrame::New();
    this->reslicer_ = vtkImageReslice::New();
    this->slice_extent_[0] = 256;
    this->slice_extent_[1] = 256;
    this->slice_spacing_[0] = 0.2;
    this->slice_spacing_[1] = 0.2;
    this->slice_thickness_ = 0.2;
    this->offset_point_ = 0;
    this->offset_line_ = 0;
    this->incidence_ = 0;
    this->probe_input_ = 0;
    this->SetNumberOfInputPorts(2);
    this->SetNumberOfOutputPorts(2);
    this->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,  vtkDataSetAttributes::SCALARS);
}

CBCTSplineDrivenImageSlicer::~CBCTSplineDrivenImageSlicer()
{
    this->local_frenetFrames_->Delete();
    this->reslicer_->Delete();
}


void CBCTSplineDrivenImageSlicer::SetPathConnection(int id, vtkAlgorithmOutput *algOutput)
{
    if (id < 0) 
    {
        vtkErrorMacro("Bad index " << id << " for source.");
        return;
    }
    int numConnections = this->GetNumberOfInputConnections(1);
    if (id < numConnections) 
    {
        this->SetNthInputConnection(1, id, algOutput);
    }
    else if (id == numConnections && algOutput) 
    {
        this->AddInputConnection(1, algOutput);
    }
    else if (algOutput) 
    {
        vtkWarningMacro("The source id provided is larger than the maximum "
            "source id, using " << numConnections << " instead.");
        this->AddInputConnection(1, algOutput);
    }
}

void CBCTSplineDrivenImageSlicer::SetPathConnection(vtkAlgorithmOutput *algOutput) 
{
    this->SetPathConnection(0, algOutput);
}

vtkAlgorithmOutput *CBCTSplineDrivenImageSlicer::GetPathConnection() 
{
    return(this->GetInputConnection(1, 0));
}

int CBCTSplineDrivenImageSlicer::FillInputPortInformation(  int port, vtkInformation *info) 
{
    if (port == 0) 
    {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    }
    else 
    {
        info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    }
    return 1;
}


int CBCTSplineDrivenImageSlicer::FillOutputPortInformation( int port, vtkInformation *info) 
{
    if (port == 0) 
    {
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    }
    if (port == 1) 
    {
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    }
    return 1;
}


int CBCTSplineDrivenImageSlicer::RequestInformation( vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector, vtkInformationVector *outputVector) 
{
    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    int extent[6] = { 0, this->slice_extent_[0] - 1,
                     0, this->slice_extent_[1] - 1,
                     0, 1
    };
    double spacing[3] = { this->slice_spacing_[0], this->slice_spacing_[1], this->slice_thickness_ };
    outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);
    return 1;
}

int CBCTSplineDrivenImageSlicer::RequestData( vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector,  vtkInformationVector *outputVector) 
{
    // 获取信息对象
    vtkInformation *inInfo       = inputVector[0]->GetInformationObject(0);
    vtkInformation *pathInfo     = inputVector[1]->GetInformationObject(0);
    vtkInformation *outImageInfo = outputVector->GetInformationObject(0);
    vtkInformation *outPlaneInfo = outputVector->GetInformationObject(1);
    // 获取输入和输出
    vtkImageData *input          = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageData *inputCopy      = vtkImageData::New();
    inputCopy->ShallowCopy(input);
    vtkPolyData *inputPath       = vtkPolyData::SafeDownCast(  pathInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkImageData *outputImage    = vtkImageData::SafeDownCast(  outImageInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *outputPlane     = vtkPolyData::SafeDownCast( outPlaneInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkSmartPointer<vtkPolyData> pathCopy = vtkSmartPointer<vtkPolyData>::New();
    pathCopy->ShallowCopy(inputPath);
    // 计算路径的局部法线和切线
    this->local_frenetFrames_->SetInputData(pathCopy);
    this->local_frenetFrames_->Setview_up_(this->incidence_);
    this->local_frenetFrames_->Update();
    // 路径将包含点数据数组“切线”和“向量”
    vtkPolyData *path = static_cast<vtkPolyData *>  (this->local_frenetFrames_->GetOutputDataObject(0));
    // 计算单元格中使用了多少个点。如果循环，点可以使用多次
    // 不使用NumberOfPoints，因为我们只需要线条和点
    vtkCellArray *lines = path->GetLines();
    lines->InitTraversal();
    vtkIdType nbCellPoints;     
    vtkIdType *points; 
    vtkIdType cellId = -1;
    lines->GetNextCell(nbCellPoints, (vtkIdType const*&)points);
    do
    {
        lines->GetNextCell(nbCellPoints, (vtkIdType const*&)points);
        //lines->GetNextCell(nbCellPoints, points);
        cellId++;
    } 
    while (cellId != this->offset_line_);

    vtkIdType ptId = this->offset_point_;
    if (ptId >= nbCellPoints) 
    {
        ptId = nbCellPoints - 1;
    }
    // 建立一个新的reslicer与图像输入作为输入。
    this->reslicer_->SetInputData(inputCopy);
    // 获取ptId点的Frenet-Serret图表：
    // - position (center)
    // - tangent T
    // - normal N
    double center[3];
    path->GetPoints()->GetPoint(ptId, center);
    vtkDoubleArray *pathTangents = static_cast<vtkDoubleArray *> (path->GetPointData()->GetArray("FSTangents"));
    double tangent[3];
    pathTangents->GetTuple(ptId, tangent);
    vtkDoubleArray *pathNormals  = static_cast<vtkDoubleArray *> (path->GetPointData()->GetArray("FSNormals"));
    double normal[3];
    pathNormals->GetTuple(ptId, normal);
    // Frenet-Serret 图表由 T, N and B = T ^ N
    double crossProduct[3];
    vtkMath::Cross(tangent, normal, crossProduct);
    // 构建平面输出，该输出将表示三维视图中的切片位置
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
    double planeOrigin[3];
    double planePoint1[3];
    double planePoint2[3];
    for (int comp = 0; comp < 3; comp++) 
    {
        planeOrigin[comp] = center[comp] - normal[comp] * this->slice_extent_[1] * this->slice_spacing_[1] / 2.0
            - crossProduct[comp] * this->slice_extent_[0] * this->slice_spacing_[0] / 2.0;
        planePoint1[comp] = planeOrigin[comp] + crossProduct[comp] * this->slice_extent_[0] * this->slice_spacing_[0];
        planePoint2[comp] = planeOrigin[comp] + normal[comp] * this->slice_extent_[1] * this->slice_spacing_[1];
    }
    plane->SetOrigin(planeOrigin);
    plane->SetPoint1(planePoint1);
    plane->SetPoint2(planePoint2);
    plane->SetResolution(this->slice_extent_[0],  this->slice_extent_[1]);
    plane->Update();
    if (this->probe_input_ == 1) 
    {
        vtkSmartPointer<vtkProbeFilter> probe = vtkSmartPointer<vtkProbeFilter>::New();
        probe->SetInputConnection(plane->GetOutputPort());
        probe->SetSourceData(inputCopy);
        probe->Update();
        outputPlane->DeepCopy(probe->GetOutputDataObject(0));
    }
    else 
    {
        outputPlane->DeepCopy(plane->GetOutputDataObject(0));
    }
    // 构建转换矩阵
    vtkMatrix4x4 *resliceAxes = vtkMatrix4x4::New();
    resliceAxes->Identity();
    double origin[4];
    // 仿照 vtkImageReslice:
    // - 1st column contains the resliced image x-axis
    // - 2nd column contains the resliced image y-axis
    // - 3rd column contains the normal of the resliced image plane
    // -> 1st column is normal to the path
    // -> 3nd column is tangent to the path
    // -> 2nd column is B = T^N
    for (int comp = 0; comp < 3; comp++)
    {
        resliceAxes->SetElement(0, comp, crossProduct[comp]);
        resliceAxes->SetElement(1, comp, normal[comp]);
        resliceAxes->SetElement(2, comp, tangent[comp]);
        origin[comp] = center[comp] - normal[comp] * this->slice_extent_[1] * this->slice_spacing_[1] / 2.0 -
            crossProduct[comp] * this->slice_extent_[0] * this->slice_spacing_[0] / 2.0;
    }
    origin[3] = 1.0;
    double originXYZW[4];
    resliceAxes->MultiplyPoint(origin, originXYZW);
    resliceAxes->Transpose();
    double neworiginXYZW[4];
    resliceAxes->MultiplyPoint(originXYZW, neworiginXYZW);
    resliceAxes->SetElement(0, 3, neworiginXYZW[0]);
    resliceAxes->SetElement(1, 3, neworiginXYZW[1]);
    resliceAxes->SetElement(2, 3, neworiginXYZW[2]);
    this->reslicer_->SetResliceAxes(resliceAxes);
    this->reslicer_->SetInformationInput(input);
    this->reslicer_->SetInterpolationModeToCubic();
    this->reslicer_->SetOutputDimensionality(2);
    this->reslicer_->SetOutputOrigin(0, 0, 0);
    this->reslicer_->SetOutputExtent(0, this->slice_extent_[0] - 1, 0, this->slice_extent_[1] - 1, 0, 1);
    this->reslicer_->SetOutputSpacing(this->slice_spacing_[0],  this->slice_spacing_[1], this->slice_thickness_);
    this->reslicer_->Update();
    resliceAxes->Delete();
    outputImage->DeepCopy(this->reslicer_->GetOutputDataObject(0));
    outputImage->GetPointData()->GetScalars()->SetName("ReslicedImage");
    return 1;
}
*/
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

///+++++++++++++++++++++++*******C-P-R-++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    double dir[3] = { 0,0,1 };// { direction[0], direction[1], direction[2] };
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

vtkSmartPointer<vtkPolyData> SweepLine(const std::vector<std::array<double, 3>>& allpoints, double const direction[3], double distance, unsigned int cols)
{
    vtkNew<vtkPoints> vtkPoints, points;
    int size = allpoints.size();
    for (int i = 0; i < size; i++)
    {
        vtkPoints->InsertNextPoint(allpoints[i][0], allpoints[i][1], allpoints[i][2]);
    }
    vtkNew<vtkPolyData> line;
    line->SetPoints(vtkPoints);

    unsigned int rows = line->GetNumberOfPoints();
    double spacing = distance / cols;

    // Generate the points.
    cols++;
    unsigned int numberOfPoints = rows * cols;
    unsigned int numberOfPolys = (rows - 1) * (cols - 1);

    points->Allocate(numberOfPoints);
    vtkNew<vtkCellArray> polys;
    polys->Allocate(numberOfPolys * 4);

    double x[3];
    unsigned int cnt = 0;
    for (unsigned int row = 0; row < rows; row++)
    {
        for (unsigned int col = 0; col < cols; col++)
        {
            double p[3];
            line->GetPoint(row, p);
            x[0] = p[0] + direction[0] * col * spacing;
            x[1] = p[1] + direction[1] * col * spacing;
            x[2] = p[2] + direction[2] * col * spacing;
            points->InsertPoint(cnt++, x);
        }
    }
    // Generate the quads.
    vtkIdType pts[4];
    for (unsigned int row = 0; row < rows - 1; row++)
    {
        for (unsigned int col = 0; col < cols - 1; col++)
        {
            pts[0] = col + row * (cols);
            pts[1] = pts[0] + 1;
            pts[2] = pts[0] + cols + 1;
            pts[3] = pts[0] + cols;
            polys->InsertNextCell(4, pts);
        }
    }
    vtkNew<vtkPolyData> surface;
    surface->SetPoints(points);
    surface->SetPolys(polys);

    return surface;
}


//https://blog.csdn.net/juluwangriyue/article/details/127272402
vtkSmartPointer<vtkImageData> GenerateCPRImageWithThickness(
    vtkImageData* volume, const std::vector<std::array<double, 3>>& pathPoints, int outputSliceSize = 200,
    int thickness = 1, double spacing = 0.1, const std::string& mode = "NULL" // "mean" or "mip"
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
        if (mode == "mip")
        {
            auto mip = vtkSmartPointer<vtkImageMathematics>::New();
            mip->SetOperationToMax();
            mip->SetInput1Data(sliceLayers[0]);
            for (size_t j = 1; j < sliceLayers.size(); ++j) {
                mip->SetInput2Data(sliceLayers[j]);
                mip->Update();
                mip->SetInput1Data(mip->GetOutput());
            }
            mip->Update();
            combined = mip->GetOutput();
        }
        else if (mode == "mean")
        {
            auto sum = vtkSmartPointer<vtkImageMathematics>::New();
            sum->SetOperationToAdd();
            sum->SetInput1Data(sliceLayers[0]);
            for (size_t j = 1; j < sliceLayers.size(); ++j)
            {
                sum->SetInput2Data(sliceLayers[j]);
                sum->Update();
                sum->SetInput1Data(sum->GetOutput());
            }
            sum->Update();

            auto avg = vtkSmartPointer<vtkImageShiftScale>::New();
            avg->SetInputData(sum->GetOutput());
            avg->SetShift(0);
            avg->SetScale(1.0 / sliceLayers.size());
            avg->SetOutputScalarTypeToFloat();
            avg->Update();

            combined = avg->GetOutput();
        }
        else
        {
            // 默认返回中间层
            combined = sliceLayers[thickness];
        }

        finalStack->AddInputData(combined);
    }
    finalStack->Update();
    return finalStack->GetOutput();
}

vtkSmartPointer<vtkImageData> GenerateCPRImage(vtkResliceImageViewer* viewer, const std::vector<std::array<double, 3>>& points, unsigned int cols) // 法向量方向的采样数
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

//
vtkSmartPointer<vtkImageData> MPRCPRImageWithThickness(vtkImageData* volume, const std::vector<std::array<double, 3>>& pathPoints, int outputSliceSize = 256, 
    int thickness = 1,  double spacing = 1.0,  const std::string& mode = "mean" // "mean" or "mip"
)
{
    if (!volume || pathPoints.size() < 2)
        return nullptr;

    auto finalStack = vtkSmartPointer<vtkImageAppend>::New();
    finalStack->SetAppendAxis(0); // Stack along Y
    int size = pathPoints.size() - 1;
    for (size_t i = 0; i < size; ++i)
    {
        std::array<double, 3> p0 = pathPoints[i];
        std::array<double, 3> p1 = pathPoints[i + 1];

        double tangent[3] = {p1[0] - p0[0],  p1[1] - p0[1], p1[2] - p0[2]};
        vtkMath::Normalize(tangent);

        // Build orthogonal coordinate system
        double ref[3] = { 0, 0, 1 };
        if (std::abs(vtkMath::Dot(tangent, ref)) > 0.99)
            ref[0] = 0, ref[1] = 1, ref[2] = 0;

        double normal[3];
        vtkMath::Cross(tangent, ref, normal);
        vtkMath::Normalize(normal);

        double binormal[3];
        vtkMath::Cross(tangent, normal, binormal);
        vtkMath::Normalize(binormal);

        // Midpoint center
        double center[3] = { 0.5 * (p0[0] + p1[0]), 0.5 * (p0[1] + p1[1]), 0.5 * (p0[2] + p1[2]) };

        // Store thickness slices
        std::vector<vtkSmartPointer<vtkImageData>> sliceLayers;
        for (int t = -thickness/2; t <= thickness/2; ++t) 
        {
            double offsetCenter[3] =  { center[0] + spacing * t * normal[0], 
                center[1] + spacing * t * normal[1],
                center[2] + spacing * t * normal[2] };

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
            reslice->SetInterpolationModeToLinear();
            reslice->SetOutputSpacing(1.0, 1.0, 1.0);
            //reslice->SetOutputExtent(0, outputSliceSize - 1, 0, outputSliceSize - 1, 0, 0);
            reslice->SetBackgroundLevel(0);
            reslice->Update();

            sliceLayers.push_back(reslice->GetOutput());
        }

        vtkSmartPointer<vtkImageData> combined;

        if (mode == "mip") 
        {
            auto mip = vtkSmartPointer<vtkImageMathematics>::New();
            mip->SetOperationToMax();
            mip->SetInput1Data(sliceLayers[0]);
            for (size_t j = 1; j < sliceLayers.size(); ++j) {
                mip->SetInput2Data(sliceLayers[j]);
                mip->Update();
                mip->SetInput1Data(mip->GetOutput());
            }
            mip->Update();
            combined = mip->GetOutput();
        }
        else if (mode == "mean") 
        {
            auto sum = vtkSmartPointer<vtkImageMathematics>::New();
            sum->SetOperationToAdd();
            sum->SetInput1Data(sliceLayers[0]);
            for (size_t j = 1; j < sliceLayers.size(); ++j) {
                sum->SetInput2Data(sliceLayers[j]);
                sum->Update();
                sum->SetInput1Data(sum->GetOutput());
            }
            sum->Update();

            auto avg = vtkSmartPointer<vtkImageShiftScale>::New();
            avg->SetInputData(sum->GetOutput());
            avg->SetShift(0);
            avg->SetScale(1.0 / sliceLayers.size());
            avg->SetOutputScalarTypeToFloat();
            avg->Update();

            combined = avg->GetOutput();
        }
        else 
        {
            // 默认返回中间层
            combined = sliceLayers[thickness];
        }

        finalStack->AddInputData(combined);
    }

    finalStack->Update();
    return finalStack->GetOutput();
}


//0617++++
#include <vtkCardinalSpline.h>
// 计算两点间距离
double Distance(const double p1[3], const double p2[3]) {
    return std::sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
}

// ---------- 计算每个切片方向矩阵 ----------
vtkSmartPointer<vtkMatrix4x4> ComputeResliceAxes(const double origin[3], const double direction[3])
{
    double z[3] = { direction[0], direction[1], direction[2] };
    vtkMath::Normalize(z);

    double arbitrary[3] = { 0.0, 0.0, 1.0 };
    if (std::fabs(vtkMath::Dot(z, arbitrary)) > 0.9)
        arbitrary[0] = 1.0;

    double x[3], y[3];
    vtkMath::Cross(arbitrary, z, x); vtkMath::Normalize(x);
    vtkMath::Cross(z, x, y); vtkMath::Normalize(y);

    auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    for (int i = 0; i < 3; ++i)
    {
        matrix->SetElement(i, 0, x[i]);
        matrix->SetElement(i, 1, y[i]);
        matrix->SetElement(i, 2, z[i]);
        matrix->SetElement(i, 3, origin[i]);
    }
    matrix->SetElement(3, 0, 0);
    matrix->SetElement(3, 1, 0);
    matrix->SetElement(3, 2, 0);
    matrix->SetElement(3, 3, 1);

    return matrix;
}

#define DIS(p1, p2) (sqrt((p1[0]-p2[0])*(p1[0]-p2[0]) + (p1[1]-p2[1])*(p1[1]-p2[1]) + (p1[2]-p2[2])*(p1[2]-p2[2])))
// ---------- 生成CPR图像 ----------
//           ↑ normal(Y轴)
//           |
//  ← binormal       tangent(Z轴, 法向)
//  (X轴方向)         (切面法线方向)
vtkSmartPointer<vtkImageData> NewGenerateCPRImage( vtkImageData* volume,  const std::vector<std::array<double, 3>>& splinePoints,  double thickness, double *spacing)
{
    vtkNew<vtkPoints> vtkpoints;
    vtkNew<vtkCellArray> lines;
    lines->InsertNextCell(splinePoints.size());
    for (int i = 0; i < splinePoints.size(); i++)
    {
        vtkpoints->InsertNextPoint(splinePoints[i][0], splinePoints[i][1], splinePoints[i][2]);
        lines->InsertCellPoint(i);
    }
    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(vtkpoints);
    polyData->SetLines(lines);
    vtkNew<vtkSplineFilter> spline_filter;
    spline_filter->SetSubdivideToLength();
    spline_filter->SetLength(1.0);
    spline_filter->SetInputData(polyData);//(poly_data);
    spline_filter->Update();
    vtkPoints* vtP = spline_filter->GetOutput()->GetPoints();
    long long nbpoints = spline_filter->GetOutput()->GetNumberOfPoints();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++S
    auto imageAppend = vtkSmartPointer<vtkImageAppend>::New();
    imageAppend->SetAppendAxis(0); // 横向拼接
    double voxelSpacing = volume->GetSpacing()[0]; // X方向spacing，假设横向是X轴
    int* dims = volume->GetDimensions();
    double spac = 1.0; // 每像素 1mm
    //spac = spacing[0];
    int height     = 200;     // 图像高度，单位像素
    
    //for (size_t i = 0; i < splinePoints.size() - 1; ++i)
    for (size_t i = 0; i < nbpoints - 1; ++i)
    {
        //double origin[3] = 
        //{   (splinePoints[i][0] + splinePoints[i + 1][0]) / 2,
        //    (splinePoints[i][1] + splinePoints[i + 1][1]) / 2,
        //    (splinePoints[i][2] + splinePoints[i + 1][2]) / 2
        //};
        //auto next           = splinePoints[i + 1];
        double origin[3] = { (vtP->GetPoint(i)[0] + vtP->GetPoint(i+1)[0]) / 2,(vtP->GetPoint(i)[1] + vtP->GetPoint(i + 1)[1]) / 2,(vtP->GetPoint(i)[2] + vtP->GetPoint(i + 1)[2]) / 2 };
        double*pre   = vtP->GetPoint(i);
        double* next = vtP->GetPoint(i + 1);
        double direction[3] = {
            next[0] - origin[0],
            next[1] - origin[1],
            next[2] - origin[2]
        };
        double width_mm = vtkMath::Distance2BetweenPoints(vtP->GetPoint(i)/*splinePoints[i]*/, next);
        int width = 4*static_cast<int>(sqrt(width_mm));// / spac);
        //auto axes = ComputeResliceAxes(origin, direction);
        double z[3] = { direction[0], direction[1], direction[2] };
        vtkMath::Normalize(z);

        double arbitrary[3] = { 0.0, 0.0, 1.0 };
        if (std::fabs(vtkMath::Dot(z, arbitrary)) > 0.9)
            arbitrary[0] = 1.0;

        double x[3], y[3];
        vtkMath::Cross(arbitrary, z, x); vtkMath::Normalize(x);
        vtkMath::Cross(z, x, y); vtkMath::Normalize(y);

        auto matrix = vtkSmartPointer<vtkMatrix4x4>::New();
        for (int i = 0; i < 3; ++i)
        {
            matrix->SetElement(i, 0, x[i]);
            matrix->SetElement(i, 1, y[i]);
            matrix->SetElement(i, 2, z[i]);
            matrix->SetElement(i, 3, origin[i]);
        }
        matrix->SetElement(3, 0, 0);
        matrix->SetElement(3, 1, 0);
        matrix->SetElement(3, 2, 0);
        matrix->SetElement(3, 3, 1);

        auto reslice = vtkSmartPointer<vtkImageReslice>::New();
        reslice->SetInputData(volume);
        reslice->SetResliceAxes(matrix);
        reslice->SetOutputDimensionality(2);
        //reslice->SetInterpolationModeToLinear();
        reslice->SetInterpolationModeToCubic();
        reslice->SetOutputSpacing(1, 1, 1);
        //reslice->SetOutputExtent(0, resliceSize - 1, 0, resliceSize - 1, 0, 0);
        //reslice->SetOutputExtent( -width / 2, width / 2,  -height / 2, height / 2, 0, 0);
        // 投影设置：厚度方向最大值
        reslice->SetSlabModeToMax();
        //reslice->SetSlabModeToMean(); // 可选：ToMean / ToMin
        reslice->SetSlabNumberOfSlices(static_cast<int>(thickness));
        //reslice->SetSlabSliceSpacing(1.0);
        reslice->Update();

        imageAppend->AddInputData(reslice->GetOutput());
    }

    imageAppend->Update();
    return imageAppend->GetOutput();
}
//+++
//样条曲线离散
int SplineCPRImage(vtkResliceImageViewer* viewer, const std::vector<std::array<double, 3>>& points)
{
    vtkNew<vtkPoints> vtkPoints;
    int size = points.size();
    for (int i = 0; i < size; i++)
    {
        vtkPoints->InsertNextPoint(points[i][0], points[i][1], points[i][2]);
    }
    auto spline = vtkSmartPointer<vtkParametricSpline>::New();
    spline->SetPoints(vtkPoints);
    // 将样条曲线离散为多点
    auto splineSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
    splineSource->SetParametricFunction(spline);
    splineSource->SetUResolution(1000); // 更高 = 更平滑
    splineSource->Update();

    auto polyline = splineSource->GetOutput();
    auto polylinevtkPoints = polyline->GetPoints();

    // 构建一幅输出图像，沿样条路径切片
    const int sliceHeight = 50; // 可视厚度
    const int sliceLength = polylinevtkPoints->GetNumberOfPoints();
    auto outputImage = vtkSmartPointer<vtkImageData>::New();
    outputImage->SetDimensions(sliceLength, sliceHeight, 1);
    outputImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    // 设置vtkImageReslice
    for (int i = 0; i < sliceLength; ++i)
    {
        double pt[3];
        polylinevtkPoints->GetPoint(i, pt);

        double tangent[3];
        if (i < sliceLength - 1)
        {
            double nextPt[3];
            polylinevtkPoints->GetPoint(i + 1, nextPt);
            vtkMath::Subtract(nextPt, pt, tangent);
        }
        else
        {
            double prevPt[3];
            polylinevtkPoints->GetPoint(i - 1, prevPt);
            vtkMath::Subtract(pt, prevPt, tangent);
        }
        vtkMath::Normalize(tangent);

        double normal[3] = { 0.0, 0.0, 1.0 }; // 任意法向量
        double binormal[3];
        vtkMath::Cross(tangent, normal, binormal);
        vtkMath::Normalize(binormal);
        vtkMath::Cross(binormal, tangent, normal);

        auto reslicer = vtkSmartPointer<vtkImageReslice>::New();
        reslicer->SetInputData(viewer->GetInput());
        reslicer->SetInterpolationModeToLinear();
        reslicer->SetOutputExtent(0, 0, 0, sliceHeight - 1, 0, 0);
        reslicer->SetOutputSpacing(1, 1, 1);

        auto axes = vtkSmartPointer<vtkMatrix4x4>::New();
        for (int j = 0; j < 3; ++j)
        {
            axes->SetElement(j, 0, binormal[j]);
            axes->SetElement(j, 1, normal[j]);
            axes->SetElement(j, 2, tangent[j]);
            axes->SetElement(j, 3, pt[j]);
        }
        reslicer->SetResliceAxes(axes);
        reslicer->Update();

        for (int y = 0; y < sliceHeight; ++y)
        {
            unsigned char* pixel = static_cast<unsigned char*>(reslicer->GetOutput()->GetScalarPointer(0, y, 0));
            unsigned char* outPixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(i, y, 0));
            outPixel[0] = pixel[0];
        }
    }
    // 显示切片结果
    auto showviewer = vtkSmartPointer<vtkImageViewer2>::New();
    showviewer->SetInputData(outputImage);

    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    showviewer->SetupInteractor(interactor);
    showviewer->Render();
    showviewer->GetRenderer()->ResetCamera();
    showviewer->Render();
    interactor->Start();

    return 0;
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
/// 图像中有三条线 https://gitlab.kitware.com/vtk/vtk/-/issues/18441
///https://github.com/SJTUzhh/Dental-Pre-Processing/tree/master/scripts
#include <vtkSphereSource.h>
#include <vtkGlyph3DMapper.h>
#include <vtkContourWidget.h>
#include <vtkKochanekSpline.h>

#include "vtkXMLPolyDataReader.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLImageDataWriter.h"
#include "vtkXMLPolyDataWriter.h"

#include "vtkImageEllipsoidSource.h"
#include "vtkLineSource.h"

#include "vtkSplineFilter.h"
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
        //https://notes.beyondxin.top/Vtk/%E9%87%8D%E5%BB%BA/CPR.html?h=cpr#4
        else if (ev == vtkCommand::LeftButtonDoubleClickEvent)
        {
            m_starSpline   = false;
            int pointsize  = m_points.size();
            m_points[0][2] = m_points[1][2];
            if (pointsize > 1)
            {
                int icase = 4;
                if (1 == icase)
                {
                    //???什么地方错误
                    SplineCPRImage(currentViewer, m_points);
                }
                else if (2 == icase)
                {
                    //参考VTKdemo的曲面改,目前效果完全无法使用
                    //auto data = GenerateCPRImage(currentViewer, m_points, 60);
                    auto data = GenerateCPRImageWithThickness(currentViewer->GetInput(), m_points);
                    //auto data = GenerateCPRImageFromViewer(currentViewer, m_points);
                    auto viewer = vtkSmartPointer<vtkImageViewer2>::New();
                    viewer->SetInputData(data);
                    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
                    viewer->SetupInteractor(interactor);
                    viewer->GetRenderWindow()->SetWindowName("title.c_str()");
                    viewer->Render();
                    interactor->Start();
                    //https://blog.csdn.net/a15005784320/article/details/117248736
                    //
                }
                else if (3 == icase)
                {                              
                    VTKRCP* repOblique = VTKRCP::SafeDownCast(currentViewer->GetResliceCursorWidget()->GetRepresentation());
                    vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(repOblique->GetReslice());
                    double *spacing = reslice->GetOutputSpacing();
                    auto data = NewGenerateCPRImage(currentViewer->GetInput(), m_points, 10, spacing);
                    //auto data = MPRCPRImageWithThickness(currentViewer->GetInput(), m_points);
                    //auto data = GenerateCPRImageFromViewer(currentViewer, m_points, 10, 200,"NULL");
                    auto viewer = vtkSmartPointer<vtkImageViewer2>::New();
                    viewer->SetInputData(data);
                    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
                    viewer->SetupInteractor(interactor);
                    viewer->GetRenderWindow()->SetWindowName("title.c_str()");
                    viewer->Render();
                    interactor->Start();
                }
                ////************************************************************\\//
                else if (4 == icase)
                {
                    //目前是白色的线带.暂时不知问题地方
                    vtkNew<vtkPoints> vtkpoints;
                    vtkNew<vtkCellArray> lines;
                    lines->InsertNextCell(m_points.size());
                    for (int i = 0; i < m_points.size(); i++)
                    {
                        vtkpoints->InsertNextPoint(m_points[i][0], m_points[i][1], m_points[i][2]);
                        lines->InsertCellPoint(i);
                    }
                    vtkNew<vtkPolyData> polyData;
                    polyData->SetPoints(vtkpoints);
                    polyData->SetLines(lines);
                    vtkNew<vtkSplineFilter> spline_filter;                                    
                    spline_filter->SetSubdivideToLength();
                    spline_filter->SetLength(1);
                    spline_filter->SetInputData(polyData);//(poly_data);
                    spline_filter->Update(); 
                    //vtkPoints* vtP = spline_filter->GetOutput()->GetPoints();
                    //long long nbpoints = spline_filter->GetOutput()->GetNumberOfPoints();
                    //___
                    //vtkSmartPointer<vtkLineSource> linSource = vtkSmartPointer<vtkLineSource>::New();
                    //linSource->SetPoints(vtkpoints);
                    //linSource->SetResolution(20);
                    //
                    //vtkNew <vtkSplineFilter> splineFilter;
                    //splineFilter->SetSubdivideToLength();
                    //splineFilter->SetLength(1.0);
                    //splineFilter->SetInputData(polyData);
                    //splineFilter->Update();
                    //++              

                    vtkSmartPointer<vtkXMLPolyDataReader> pathReader =  vtkSmartPointer<vtkXMLPolyDataReader>::New();
                    pathReader->SetFileName("F:\\temp\\HealthApp\\Tools\\MPR3DVTK94\\closed_curve.vtp");
                    vtkNew<vtkImageAppend> append;
                    append->SetAppendAxis(2);
                    vtkNew<vtkSplineDrivenImageSlicer> reslicer;
                    reslicer->SetInputData(currentViewer->GetInput());
                    reslicer->SetPathConnection(pathReader->GetOutputPort());//spline_filter->GetOutputPort());
                    //reslicer->SetPathConnection(linSource->GetOutputPort());
                    //reslicer->SetPathConnection(splineFilter->GetOutputPort());
                    reslicer->SetSliceExtent(256, 256);
                    //reslicer->SetSliceSpacing(0.2, 0.1);
                    reslicer->SetSliceThickness(4);

                    long long nb_points = spline_filter->GetOutput()->GetNumberOfPoints();
                    for (int pt_id = 0; pt_id < nb_points; pt_id++)
                    {
                        reslicer->SetOffsetPoint(pt_id);
                        reslicer->Update();
                        vtkNew<vtkImageData> tempSlice;
                        tempSlice->DeepCopy(reslicer->GetOutput());
                        append->AddInputData(tempSlice);
                    }
                    append->Update();
                    //vtkNew<vtkImagePermute> permute_filter;
                    //permute_filter->SetInputData(append->GetOutput());
                    //permute_filter->SetFilteredAxes(2, 0, 1);
                    //permute_filter->Update();
                    //vtkNew<vtkImageFlip> flip_filter;
                    //flip_filter->SetInputData(permute_filter->GetOutput());
                    //flip_filter->SetFilteredAxes(1);
                    //flip_filter->Update();
                    auto viewer = vtkSmartPointer<vtkImageViewer2>::New();
                    //viewer->SetInputData(flip_filter->GetOutput());
                    viewer->SetInputData(append->GetOutput());
                    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
                    viewer->SetupInteractor(interactor);
                    viewer->GetRenderWindow()->SetWindowName("title.c_str()");
                    viewer->Render();
                    interactor->Start();
                }              
                //******************************************************************\\
                 // Create an ellipsoid
                //vtkSmartPointer<vtkImageEllipsoidSource> imgSource = vtkSmartPointer<vtkImageEllipsoidSource>::New();
                //imgSource->SetWholeExtent(0, 255, 0, 255, 0, 255);
                //imgSource->SetCenter(127, 127, 127);
                //imgSource->SetRadius(50, 50, 50);
                //imgSource->Update();
                //double imgBB[6];
                //imgSource->GetOutput()->GetBounds(imgBB);
                //
                //// Create a spline
                //vtkSmartPointer<vtkLineSource> linSource = vtkSmartPointer<vtkLineSource>::New();
                //linSource->SetPoint1(imgBB[0], imgBB[2], imgBB[4]);
                //linSource->SetPoint2(imgBB[1], imgBB[3], imgBB[5]);
                //linSource->SetResolution(20);
                //
                //// tested object
                //vtkSmartPointer<vtkSplineDrivenImageSlicer> sdis;
                //sdis = vtkSmartPointer<vtkSplineDrivenImageSlicer>::New();
                //sdis->SetInputConnection(0, imgSource->GetOutputPort());
                //sdis->SetInputConnection(1, linSource->GetOutputPort());
                //sdis->SetOffsetPoint(10);
                //sdis->SetSliceExtent(200, 200);
                //sdis->SetSliceSpacing(0.5, 0.5);
                //sdis->Update();
                //+++++++
                ///+++++++++++++++
                /*
                //double const direction[3] = {0,0,1};
                //auto surface = SweepLine(m_points, direction, 10, 900);
                //vtkNew<vtkProbeFilter> sampleVolume;
                //sampleVolume->SetInputConnection(1, g_vtkAlgorithmOutput);
                //sampleVolume->SetInputData(0, surface);
                //vtkNew<vtkWindowLevelLookupTable> wlLut;
                //vtkImageData *data = currentViewer->GetInput();
                //double range = data->GetScalarRange()[1] - data->GetScalarRange()[0];
                //double level = (data->GetScalarRange()[1] + data->GetScalarRange()[0]) / 2.0;
                //wlLut->SetWindow(range);
                //wlLut->SetLevel(level);

                //vtkNew<vtkDataSetMapper> mapper;
                //mapper->SetInputConnection(sampleVolume->GetOutputPort());
                //mapper->SetLookupTable(wlLut);
                //mapper->SetScalarRange(0, 255);
                //vtkNew<vtkActor> actor;
                //actor->SetMapper(mapper);
                //
                //// Create a renderer, render window, and interactor.
                //vtkNew<vtkRenderer> renderer;
                //vtkNew<vtkRenderWindow> renderWindow;
                //renderWindow->AddRenderer(renderer);
                //renderWindow->SetWindowName("CurvedReformation");
                //
                //vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
                //renderWindowInteractor->SetRenderWindow(renderWindow);
                //
                //vtkNew<vtkNamedColors> colors;
                //// Add the actors to the scene.
                //renderer->AddActor(actor);
                //renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());
                //
                //// Set the camera for viewing medical images.
                //renderer->GetActiveCamera()->SetViewUp(0, 0, 1);
                //renderer->GetActiveCamera()->SetPosition(0, 0, 0);
                //renderer->GetActiveCamera()->SetFocalPoint(0, 1, 0);
                //renderer->ResetCamera();
                //
                //// Render and interact
                //renderWindow->Render();
                //renderWindowInteractor->Start();
                */
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

            ///
            //vtkContourWidget *wid = vtkContourWidget::New();
            //wid->SetInteractor(renderWindow->GetInteractor());
            //wid->CreateDefaultRepresentation();
            //wid->On();
            //m_scrollConnection->Connect( wid, vtkCommand::EndInteractionEvent, this, 
            //    SLOT(  SlotContourEndInteractionEvent( vtkObject *, unsigned long, void *, void *)) , nullptr, 0.0, Qt::UniqueConnection);
            //
            //auto widget = dynamic_cast<vtkContourWidget *>(t_obj);
            //auto rep = dynamic_cast<vtkOrientedGlyphContourRepresentation *>(widget->GetContourRepresentation());
            //if (rep) 
            //{
            //    vtkMatrix4x4 *sourceMatrix = m_resliceWidget->getImageReslicers()[2]->GetResliceAxes();
            //    qint32 n = rep->GetNumberOfNodes();
            //    for (qint32 i = 0; i < n; ++i)
            //    {
            //        double p[3];
            //        rep->GetNthNodeWorldPosition(i, p);
            //        vtkNew<vtkTransform> transform1;
            //        transform1->SetMatrix(sourceMatrix);
            //        transform1->Translate(p[0], p[1], 0);
            //        qDebug() << i
            //            << transform1->GetMatrix()->GetElement(0, 3)
            //            << transform1->GetMatrix()->GetElement(1, 3)
            //            << transform1->GetMatrix()->GetElement(2, 3);
            //    }
            //}
/////////
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
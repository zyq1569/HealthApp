#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"
#include <QDir>
#include <QEnterEvent>

#include "vtkBoundedPlanePointPlacer.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDICOMImageReader.h"
#include "vtkDistanceRepresentation.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkDistanceWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
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
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkRenderWindowInteractor.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkResliceCursorWidget.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursor.h"
#include "vtkResliceImageViewerMeasurements.h"

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
///
#include "itkImageToVTKImageFilter.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkVTKImageToImageFilter.h"
#include "itkImageSeriesWriter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkCastImageFilter.h"
#include "itksys/SystemTools.hxx"
#include "itkImageRegionIterator.h"
#include <itkConnectedThresholdImageFilter.h>
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryFillholeImageFilter.h"
#include "itkGrayscaleFillholeImageFilter.h"
#include "itkMeanImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkExtractImageFilter.h"
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkPNGImageIOFactory.h>
#include "itkGDCMImageIO.h"
#include <itkJPEGImageIOFactory.h>
///
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
//----------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand
{
public:
  static vtkResliceCursorCallback *New()
  {
      return new vtkResliceCursorCallback; 
  }

  void Execute( vtkObject *caller, unsigned long ev, void *callData ) override
  {
	 if (ev == vtkCommand::MouseMoveEvent )
	 {
		 for (int i = 0; i < 3; i++)
		 {
			 if (SIV[i])
			 {
				 SIV[i]->GetRenderWindow()->Render();
			 }
		 }
		 vtkSmartPointer<vtkInteractorStyleImage> interator = vtkInteractorStyleImage::SafeDownCast(caller);
		 interator->OnMouseMove();
	 }
	 else if (ev == vtkCommand::MiddleButtonPressEvent)
	 {
		 vtkSmartPointer<vtkInteractorStyleImage> interator = vtkInteractorStyleImage::SafeDownCast(caller);
		 interator->OnMiddleButtonDown();
	 }
	else  if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent || ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
    {
      // Render everything
      for (int i = 0; i < 3; i++)
      {
        this->RCW[i]->Render();
      }
      this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
      return;
    }

    vtkImagePlaneWidget* ipw = dynamic_cast< vtkImagePlaneWidget* >( caller );
    if (ipw)
    {
      double* wl = static_cast<double*>( callData );

      if ( ipw == this->IPW[0] )
      {
        this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
        this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
      }
      else if( ipw == this->IPW[1] )
      {
        this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
        this->IPW[2]->SetWindowLevel(wl[0],wl[1],1);
      }
      else if (ipw == this->IPW[2])
      {
        this->IPW[0]->SetWindowLevel(wl[0],wl[1],1);
        this->IPW[1]->SetWindowLevel(wl[0],wl[1],1);
      }
    }

    vtkResliceCursorWidget *rcw = dynamic_cast< vtkResliceCursorWidget * >(caller);
    if (rcw)
    {
      vtkResliceCursorLineRepresentation *rep = dynamic_cast<vtkResliceCursorLineRepresentation * >(rcw->GetRepresentation());
      // Although the return value is not used, we keep the get calls
      // in case they had side-effects
      rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
      for (int i = 0; i < 3; i++)
      {
        vtkPlaneSource *ps = static_cast< vtkPlaneSource * >( this->IPW[i]->GetPolyDataAlgorithm());
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

  vtkResliceCursorCallback() {}
  vtkImagePlaneWidget* IPW[3];
  vtkResliceCursorWidget *RCW[3];
  vtkResliceImageViewer *SIV[3];
};


using namespace std;
typedef short PixelType;
const unsigned int   Dimension = 3;
typedef itk::Image< PixelType, Dimension > Input2dImageType;
typedef itk::Image< PixelType, 3 > Input3dImageType;

typedef itk::Image< PixelType, Dimension > Output2dImageType;
typedef itk::Image< PixelType, 3 > Output3dImageType;

bool getFileNames(QString dir, std::vector< std::string > &fileNames)
{
	using NamesGeneratorType = itk::GDCMSeriesFileNames;
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetUseSeriesDetails(true);
	nameGenerator->AddSeriesRestriction("0008|0021");
	nameGenerator->SetDirectory(dir.toUtf8().data());//bool SystemTools::FileIsDirectory(const std::string& inName)
	using SeriesIdContainer = std::vector< std::string >;
	const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
	auto seriesItr = seriesUID.begin();
	auto seriesEnd = seriesUID.end();
	std::string seriesIdentifier;
	while (seriesItr != seriesEnd)
	{
		seriesIdentifier = seriesItr->c_str();
		fileNames = nameGenerator->GetFileNames(seriesIdentifier);
		++seriesItr;
	}
	nameGenerator = NULL;
	return true;
}

Input3dImageType::Pointer GdcmRead3dImage(std::string path)
{
	QString dir;
	std::vector< std::string > fileNames;
	dir = path.c_str();
	getFileNames(dir, fileNames);

	using ReaderType3d = itk::ImageSeriesReader< Input3dImageType >;
	ReaderType3d::Pointer reader3d = ReaderType3d::New();

	typedef itk::GDCMImageIO   ImageIOType;//GDCMImageIO读DICOM
	ImageIOType::Pointer gdcmImageIO = ImageIOType::New();

	reader3d->SetImageIO(gdcmImageIO);
	reader3d->SetFileNames(fileNames);
	reader3d->Update();
	Input3dImageType::Pointer image = reader3d->GetOutput();
	reader3d = NULL;
	gdcmImageIO = NULL;
	//using ReaderType3d = itk::ImageSeriesWriter< Input3dImageType >;
	return image;
}

vtkSmartPointer<vtkImageData> ImageDataItkToVtk(Input3dImageType::Pointer image)
{
	typedef itk::ImageToVTKImageFilter< Input3dImageType> itkTovtkFilterType;
	itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
	itkTovtkImageFilter->SetInput(image);//设置图像数据从ITK转向VTK
	itkTovtkImageFilter->Update();

	//vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
	//ImageFlip->SetInputData(itkTovtkImageFilter->GetOutput());
    //
	//ImageFlip->SetFilteredAxes(1);
	//ImageFlip->Update();
	//vtkSmartPointer<vtkImageData> vtkdata = ImageFlip->GetOutput();
	//ImageFlip = NULL;
	//itkTovtkImageFilter = NULL;
	//return vtkdata;
    return itkTovtkImageFilter->GetOutput();
}

void saveHDMdata(QString DicomDir)
{
	Input3dImageType::Pointer image = GdcmRead3dImage(qPrintable(DicomDir));
	vtkSmartPointer<vtkImageData> ImageVTKData = ImageDataItkToVtk(image);

	vtkImageData * itkImageData = ImageVTKData;
	std::string Input_Name = qPrintable(DicomDir);
	std::string path = Input_Name + "/VTKMetaData.mhd";

	vtkMetaImageWriter *vtkdatawrite = vtkMetaImageWriter::New();
	vtkdatawrite->SetInputData(itkImageData);
	vtkdatawrite->SetFileName(path.c_str());
	path = Input_Name + "/VTKMetaData.raw";
	vtkdatawrite->SetRAWFileName(path.c_str());
	vtkdatawrite->Write();
	vtkdatawrite->Delete();

}

QtVTKRenderWindows::QtVTKRenderWindows( int vtkNotUsed(argc), char *argv[])
{
  this->ui = new Ui_QtVTKRenderWindows;
  this->ui->setupUi(this);

  MprInit();
};

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
}

void QtVTKRenderWindows::SetBlendMode(int m)
{
  for (int i = 0; i < 3; i++)
  {
	vtkWidgetRepresentation *tation              = riw[i]->GetResliceCursorWidget()->GetRepresentation();
	vtkResliceCursorThickLineRepresentation *rep = vtkResliceCursorThickLineRepresentation::SafeDownCast(tation);
	vtkImageSlabReslice *thickSlabReslice        = vtkImageSlabReslice::SafeDownCast(rep->GetReslice());
	vtkResliceCursorThickLineRepresentation::SafeDownCast( riw[i]->GetResliceCursorWidget()->GetRepresentation())->GetReslice();
    thickSlabReslice->SetBlendMode(m);
    riw[i]->Render();
  }
}

void QtVTKRenderWindows::SetBlendModeToMaxIP()
{
  this->SetBlendMode(VTK_IMAGE_SLAB_MAX);
}

void QtVTKRenderWindows::SetBlendModeToMinIP()
{
  this->SetBlendMode(VTK_IMAGE_SLAB_MIN);
}

void QtVTKRenderWindows::SetBlendModeToMeanIP()
{
  this->SetBlendMode(VTK_IMAGE_SLAB_MEAN);
}

void QtVTKRenderWindows::ResetViews()
{
  // Reset the reslice image views
  for (int i = 0; i < 3; i++)
  {
    riw[i]->Reset();
    riw[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
  }

  // Also sync the Image plane widget on the 3D top right view with any
  // changes to the reslice cursor.
  for (int i = 0; i < 3; i++)
  {
    vtkPlaneSource *ps = static_cast< vtkPlaneSource * >( planeWidget[i]->GetPolyDataAlgorithm());
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
    riw[i]->Render();
  }
  this->ui->view3->GetRenderWindow()->Render();
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
  this->DistanceWidget[i] = vtkSmartPointer< vtkDistanceWidget >::New();
  this->DistanceWidget[i]->SetInteractor( this->riw[i]->GetResliceCursorWidget()->GetInteractor());

  // Set a priority higher than our reslice cursor widget
  this->DistanceWidget[i]->SetPriority( this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

  vtkSmartPointer< vtkPointHandleRepresentation2D > handleRep =   vtkSmartPointer< vtkPointHandleRepresentation2D >::New();
  vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep  =   vtkSmartPointer< vtkDistanceRepresentation2D >::New();
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

void QtVTKRenderWindows::viewRender(QEvent* event)
{
    //if (event-> == Qt::mousemi)
    QPoint delt;// = event->angleDelta();
    if (event->type() == QEvent::Wheel)
    {
        //qDebug() << "delt:" << delt.y();
        int a = delt.y();
    }
}
void QtVTKRenderWindows::mouseEvent(QMouseEvent* event)
{
    QPoint delt;// = event->angleDelta();
    if (event->type() == QEvent::Wheel)
    {
        //qDebug() << "delt:" << delt.y();
        int a = delt.y();
    }
}
bool QtVTKRenderWindows::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type())
    {
        if (event->type() == QEvent::Wheel)
        {
            //qDebug() << "delt:" << delt.y();
            int a = 0;
        }
    }

    return true;
}
class QeventMouse:public QObject
{
public:
    QeventMouse();
    ~QeventMouse();
    bool eventFilter(QObject *object, QEvent *event)
    {
        //switch (event->type())
        {
            if (event->type() == QEvent::Wheel)
            {
                //qDebug() << "delt:" << delt.y();               
                for (int i = 0; i < 3; i++)
                {
                    if (m_riw[i])
                    {
                        m_riw[i]->GetRenderWindow()->Render();
                    }
                }
                QString name = object->objectName();
                if (name.contains("view"))
                {                    
                    int i = name.replace("view", "").toInt() - 1;
                    
                    int now = m_riw[i]->GetSlice()+1;
                    int max = m_riw[i]->GetSliceMax()+1;
                    QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
                    m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
                }
            }
        }
        return 0;
    }
public:
    vtkCornerAnnotation *m_cornerAnnotations[3];
    vtkResliceImageViewer *m_riw[3];
private:

};

QeventMouse::QeventMouse()
{
}

QeventMouse::~QeventMouse()
{
}

void QtVTKRenderWindows::MprInit()
{
	vtkNew<vtkMetaImageReader> reader;
	//std::string dir = qPrintable());//argv[1];
	
	QString dir = this->ui->m_dcmDIR->toPlainText();
	dir += "\\VTKMetaData.mhd";
	QDir mhdDir;
	QFileInfo file(dir);
	if (!file.isFile())
	{
		saveHDMdata(this->ui->m_dcmDIR->toPlainText());
	}
	std::string stddir = qPrintable(dir);
	reader->SetFileName(stddir.c_str());
	reader->Update();
	int imageDims[3];
	reader->GetOutput()->GetDimensions(imageDims);

	for (int i = 0; i < 3; i++)
	{
		riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
		vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
		riw[i]->SetRenderWindow(renderWindow);
	}


	this->ui->view1->SetRenderWindow(riw[0]->GetRenderWindow());
	riw[0]->SetupInteractor(this->ui->view1->GetRenderWindow()->GetInteractor());

	this->ui->view2->SetRenderWindow(riw[1]->GetRenderWindow());
	riw[1]->SetupInteractor(this->ui->view2->GetRenderWindow()->GetInteractor());

	this->ui->view3->SetRenderWindow(riw[2]->GetRenderWindow());
	riw[2]->SetupInteractor(this->ui->view3->GetRenderWindow()->GetInteractor());

	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation *rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
		riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());
		rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
        //if (i <2)
        //{
           //// 创建vtkImageReslice
           //vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
           //reslice->SetInputData(reader->GetOutput());
           
           //// 设置插值方式为线性插值
           //reslice->SetInterpolationModeToLinear();
           //
           //// 创建绕X轴旋转180度的矩阵
          // vtkSmartPointer<vtkMatrix4x4> rotationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
           //rotationMatrix->Identity();  // 初始化为单位矩阵
           //
           //// 绕X轴旋转180度（变换Y轴和Z轴）
           //rotationMatrix->SetElement(1, 1, -1);  // Y轴翻转
           //rotationMatrix->SetElement(2, 2, -1);  // Z轴翻转

            //vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
            //reslice->SetInputData(reader->GetOutput());
            //reslice->SetInterpolationModeToLinear();  // 设置插值方式为线性插值
            //// 创建 vtkTransform 对象
            //vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
            //
            //// 设置绕 X 轴旋转 180 度
            //transform->RotateX(180);
            //
            //// 获取旋转矩阵
            //vtkSmartPointer<vtkMatrix4x4> transformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
            //transformMatrix->Identity();
            //transformMatrix->DeepCopy(transform->GetMatrix());
            //
            //// 将变换应用到 vtkImageReslice
            //reslice->SetResliceAxes(transformMatrix);
           
            //
            //riw[i]->SetInputData(reslice->GetOutput());

            //vtkSmartPointer<vtkMatrix4x4> rotationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
            //rotationMatrix->Identity();  // 初始化为单位矩阵
            //
            //// 绕 X 轴旋转 180 度（Y 和 Z 翻转）
            //rotationMatrix->SetElement(1, 1, -1);  // Y轴翻转
            //rotationMatrix->SetElement(2, 2, -1);  // Z轴翻转
            //
            //// 获取 vtkImageReslice 对象（通过内部的 vtkResliceImageViewer）
            //riw[i]->GetRenderer()->ResetCamera();

            // 应用旋转矩阵到 vtkResliceImageViewer
            //riw[i]->SetResliceAxes(rotationMatrix);  // 设置旋转矩阵
            //riw[i]->SetReslice()
            //typedef itk::ImageToVTKImageFilter< Input3dImageType> itkTovtkFilterType;
            //itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
            ////itkTovtkImageFilter->SetInput(image);//设置图像数据从ITK转向VTK
            //itkTovtkImageFilter->Update();

            //vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
            ////vtkSmartPointer<vtkImageData>  data = vtkSmartPointer <vtkImageData>::New();
            ////data->CopyStructure((vtkDataSet*)(reader->GetOutputPort()));           
            ////ImageFlip->SetInputConnection(reader->GetOutputPort());
            //ImageFlip->SetInputData(reader->GetOutput());
            ////int x = ImageFlip->GetFilteredAxes();
            //ImageFlip->SetFilteredAxes(1);
            //ImageFlip->Update();
            ////x = ImageFlip->GetFilteredAxes();
            ////ImageFlip->Update();
            //vtkSmartPointer<vtkImageData> vtkdata =  ImageFlip->GetOutput();
            //riw[i]->SetInputData(vtkdata);
            ////vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
            ////ImageFlip->SetInputData(reader->GetOutput());
            ////ImageFlip->SetFilteredAxes(2);
            ////ImageFlip->Update();
            ////riw[i]->SetInputData(ImageFlip->GetOutput());
            //
            //vtkSmartPointer<vtkImageReslice> ImageReslice = vtkSmartPointer<vtkImageReslice >::New();
            //vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
            //
            //// 设置绕 X 轴旋转 180 度
            //transform->RotateX(180);
            //ImageReslice->SetInputData(reader->GetInput());
            //ImageReslice->SetResliceTransform(transform);
            //ImageReslice->SetOutputDimensionality(3);
            //ImageReslice->Update();
            //riw[i]->SetInputData(reader->GetOutput());
        //}
        //else
        //{
		//   riw[i]->SetInputData(reader->GetOutput());
        //}
        riw[i]->SetInputData(reader->GetOutput());
		riw[i]->SetSliceOrientation(i);
		riw[i]->SetResliceModeToAxisAligned();
	}

	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);

	vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

	vtkSmartPointer< vtkRenderer > ren = vtkSmartPointer< vtkRenderer >::New();

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
	this->ui->view4->SetRenderWindow(renderWindow);
	this->ui->view4->GetRenderWindow()->AddRenderer(ren);
	vtkRenderWindowInteractor *iren = this->ui->view4->GetInteractor();

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
    //connect(ui->view1, SIGNAL(widgetEvent(QEvent*)), this, SLOT(viewRender(QEvent*)));
    //connect(ui->view1, SIGNAL(mouseEvent(QMouseEvent*)), this, SLOT(mouseEvent(QMouseEvent*)));
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
    //riw[0]->GetRenderWindow()->SetSize(w, h);
    //riw[0]->GetRenderer()->GetActiveCamera()->Zoom(1);
    ui->view2->installEventFilter(&filter);
    //riw[1]->SetSize(w, h);
    ui->view3->installEventFilter(&filter);
    //riw[2]->SetSize(w, h);
    for (int i = 0; i < 3; i++)
    {
        riw[i]->SetResliceMode(1);
        riw[i]->GetRenderer()->ResetCamera();
        riw[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
        riw[i]->Render();
    }
    //riw[2]->GetRenderer()->GetActiveCamera()->SetViewUp(1, 0, 0);
    //riw[2]->GetRenderer()->GetActiveCamera()->Azimuth(-90.0);
    //riw[2]->GetRenderer()->GetActiveCamera()->Azimuth(-90.0);
    //riw[2]->GetRenderer()->GetActiveCamera()->Pitch(90.0);
    
    //riw[2]->GetRenderer()->GetActiveCamera()->Roll(90.0);
    //riw[2]->GetRenderer()->GetActiveCamera()->Elevation(360.0);
    //vtkNew<vtkTransform> vtktransform;
    //vtktransform->RotateX(180);
    //vtkNew<vtkActor> vtkactor;
    //vtkactor->RotateX(180);
    //riw[0]->GetRenderer()->AddActor(vtkactor);
    //riw[0]->GetImageActor()->SetUserTransform(vtktransform);
    //riw[0]->GetImageActor()->RotateX(180);
    //riw[0]->GetRenderer()->GetActiveCamera()->Roll(270);
    //riw[0]->GetRenderer()->GetActiveCamera()->Azimuth(180);
    //vtkImageReslice
    //riw[0]->GetImageActor()->Update();
    //riw[0]->GetRenderer()->AddViewProp(vtktransform);
    //riw[0]->GetImageActor()->SetUserMatrix();
    //vtkVolume *m_vtkVolume;
    //m_vtkVolume = vtkVolume::New();
    //riw[0]->GetRenderer()->AddViewProp(m_vtkVolume);
    //vtkMatrix4x4 *projectionMatrix = vtkMatrix4x4::New();
    //projectionMatrix->Identity();
    //int x[3] = { 1,0,0 }, y[3] = { 0,1,0 }, z[3] = { 0,0,1 };
    //for (int row = 0; row < 3; row++)
    //{
    //    projectionMatrix->SetElement(row, 0, x[row]);
    //    projectionMatrix->SetElement(row, 1, y[row]);
    //    projectionMatrix->SetElement(row, 2, z[row]);
    //}
    //m_vtkVolume->SetUserMatrix(projectionMatrix);
    //vtkSmartPointer<vtkImageReslice> reslice =  riw[0]->GetReslice();
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

		// Make them all share the same color map.
		riw[i]->SetLookupTable(riw[0]->GetLookupTable());
		planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
		//planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
		planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
		cbk->SIV[i] = riw[i];
	}

	this->ui->view1->show();
	this->ui->view2->show();
	this->ui->view3->show();
    ///
    this->ui->thickModeCheckBox->setEnabled( 1 );
    this->ui->blendModeGroupBox->setEnabled( 1 );
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
        //m_cornerAnnotations = vtkCornerAnnotation::New();
        //m_cornerAnnotations->GetTextProperty()->SetFontFamilyToArial();
        //m_cornerAnnotations->GetTextProperty()->ShadowOn();
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
void QtVTKRenderWindows::on_DcmDIr_clicked()
{
	this->hide();
	MprInit();
	this->show();
}


#include "ui_QtVTKRenderWindows.h"
#include "QtVTKRenderWindows.h"

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
  { return new vtkResliceCursorCallback; }

  void Execute( vtkObject *caller, unsigned long ev,
                void *callData ) override
  {

    if (ev == vtkResliceCursorWidget::WindowLevelEvent ||
        ev == vtkCommand::WindowLevelEvent ||
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

    vtkImagePlaneWidget* ipw =
      dynamic_cast< vtkImagePlaneWidget* >( caller );
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

    vtkResliceCursorWidget *rcw = dynamic_cast<
      vtkResliceCursorWidget * >(caller);
    if (rcw)
    {
      vtkResliceCursorLineRepresentation *rep = dynamic_cast<
        vtkResliceCursorLineRepresentation * >(rcw->GetRepresentation());
      // Although the return value is not used, we keep the get calls
      // in case they had side-effects
      rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
      for (int i = 0; i < 3; i++)
      {
        vtkPlaneSource *ps = static_cast< vtkPlaneSource * >(
            this->IPW[i]->GetPolyDataAlgorithm());
        ps->SetOrigin(this->RCW[i]->GetResliceCursorRepresentation()->
                                          GetPlaneSource()->GetOrigin());
        ps->SetPoint1(this->RCW[i]->GetResliceCursorRepresentation()->
                                          GetPlaneSource()->GetPoint1());
        ps->SetPoint2(this->RCW[i]->GetResliceCursorRepresentation()->
                                          GetPlaneSource()->GetPoint2());

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

Input3dImageType::Pointer GdcmRead3dImage(std::string path, QString dir)
{
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

	vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
	ImageFlip->SetInputData(itkTovtkImageFilter->GetOutput());

	ImageFlip->SetFilteredAxes(1);
	ImageFlip->Update();
	vtkSmartPointer<vtkImageData> vtkdata = ImageFlip->GetOutput();
	ImageFlip = NULL;
	itkTovtkImageFilter = NULL;
	return vtkdata;
}

#include "vtkMetaImageReader.h"
QtVTKRenderWindows::QtVTKRenderWindows( int vtkNotUsed(argc), char *argv[])
{
  this->ui = new Ui_QtVTKRenderWindows;
  this->ui->setupUi(this);
  
  
  
 // vtkSmartPointer< vtkDICOMImageReader > reader =
 // vtkSmartPointer< vtkDICOMImageReader >::New();
  vtkNew<vtkMetaImageReader> reader;
  std::string dir = argv[1];
  dir += "\\VTKdata.mhd";
  //reader->SetDirectoryName(dir.c_str());//argv[1]);
  reader->SetFileName(dir.c_str());
  reader->Update();
  int imageDims[3];
  reader->GetOutput()->GetDimensions(imageDims);
/**/
  ///
  //int imageDims[3];
  //Input3dImageType::Pointer image = GdcmRead3dImage(argv[1], "");
  //vtkSmartPointer<vtkImageData> ImageVTKData = ImageDataItkToVtk(image);
  ///


  for (int i = 0; i < 3; i++)
  {
    riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    riw[i]->SetRenderWindow(renderWindow);
  }


  //vtkNew<vtkRenderer> renS;
  //renS->SetViewport(0, 0, 0.5, 0.5);
  //riw[0]->GetRenderWindow()->AddRenderer(renS);
  this->ui->view1->SetRenderWindow(riw[0]->GetRenderWindow());
  riw[0]->SetupInteractor( this->ui->view1->GetRenderWindow()->GetInteractor());

  this->ui->view2->SetRenderWindow(riw[1]->GetRenderWindow());
  riw[1]->SetupInteractor( this->ui->view2->GetRenderWindow()->GetInteractor());

  this->ui->view3->SetRenderWindow(riw[2]->GetRenderWindow());
  riw[2]->SetupInteractor( this->ui->view3->GetRenderWindow()->GetInteractor());

  for (int i = 0; i < 3; i++)
  {
    // make them all share the same reslice cursor object.
    vtkResliceCursorLineRepresentation *rep =  vtkResliceCursorLineRepresentation::SafeDownCast( riw[i]->GetResliceCursorWidget()->GetRepresentation());
    riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());

    rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

    riw[i]->SetInputData(reader->GetOutput());
	//riw[i]->SetInputData(ImageVTKData);
    riw[i]->SetSliceOrientation(i);
    riw[i]->SetResliceModeToAxisAligned();
  }

  vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
  picker->SetTolerance(0.005);

  vtkSmartPointer<vtkProperty> ipwProp  = vtkSmartPointer<vtkProperty>::New();

  vtkSmartPointer< vtkRenderer > ren    =  vtkSmartPointer< vtkRenderer >::New();

  vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
  this->ui->view4->SetRenderWindow(renderWindow);
  this->ui->view4->GetRenderWindow()->AddRenderer(ren);
  vtkRenderWindowInteractor *iren = this->ui->view4->GetInteractor();

  for (int i = 0; i < 3; i++)
  {
    planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
    planeWidget[i]->SetInteractor( iren );
    planeWidget[i]->SetPicker(picker);
    planeWidget[i]->RestrictPlaneToVolumeOn();
    double color[3] = {0, 0, 0};
    color[i] = 1;
    planeWidget[i]->GetPlaneProperty()->SetColor(color);

    color[0] /= 4.0;
    color[1] /= 4.0;
    color[2] /= 4.0;
    riw[i]->GetRenderer()->SetBackground( color );

    planeWidget[i]->SetTexturePlaneProperty(ipwProp);
    planeWidget[i]->TextureInterpolateOff();
    planeWidget[i]->SetResliceInterpolateToLinear();
	///
	//vtkMarchingCubes *marchingcube = vtkMarchingCubes::New();
	//marchingcube->SetInputData(ImageVTKData);
	//marchingcube->SetValue(0, 200);//Setting the threshold;
	//marchingcube->ComputeNormalsOn();//计算表面法向量;
	///
    planeWidget[i]->SetInputConnection(reader->GetOutputPort());
	//planeWidget[i]->SetInputConnection(marchingcube->GetOutputPort());
	
    planeWidget[i]->SetPlaneOrientation(i);
    planeWidget[i]->SetSliceIndex(imageDims[i]/2);
    planeWidget[i]->DisplayTextOn();
    planeWidget[i]->SetDefaultRenderer(ren);
    planeWidget[i]->SetWindowLevel(1358, -27);
    planeWidget[i]->On();
    planeWidget[i]->InteractionOn();
  }

  vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

  for (int i = 0; i < 3; i++)
  {
    cbk->IPW[i] = planeWidget[i];
    cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
    riw[i]->GetResliceCursorWidget()->AddObserver( vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk );
    riw[i]->GetResliceCursorWidget()->AddObserver( vtkResliceCursorWidget::WindowLevelEvent, cbk );
    riw[i]->GetResliceCursorWidget()->AddObserver( vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk );
    riw[i]->GetResliceCursorWidget()->AddObserver( vtkResliceCursorWidget::ResetCursorEvent, cbk );
    riw[i]->GetInteractorStyle()->AddObserver( vtkCommand::WindowLevelEvent, cbk );

    // Make them all share the same color map.
    riw[i]->SetLookupTable(riw[0]->GetLookupTable());
    planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
    //planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
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

void QtVTKRenderWindows::on_DcmDIr_clicked()
{
	int a = 0;
}


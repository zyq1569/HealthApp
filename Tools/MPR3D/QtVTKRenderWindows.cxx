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

	void Execute(vtkObject *caller, unsigned long ev, void *callData) override
	{
		if (ev == vtkCommand::MouseMoveEvent)
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

		vtkResliceCursorWidget *rcw = dynamic_cast<vtkResliceCursorWidget *>(caller);
		if (rcw)
		{
			vtkResliceCursorLineRepresentation *rep = dynamic_cast<vtkResliceCursorLineRepresentation *>(rcw->GetRepresentation());
			// Although the return value is not used, we keep the get calls
			// in case they had side-effects
			rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
			for (int i = 0; i < 3; i++)
			{
				vtkPlaneSource *ps = static_cast<vtkPlaneSource *>(this->IPW[i]->GetPolyDataAlgorithm());
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

class vtkRCLP :public vtkResliceCursorLineRepresentation {

public:
	//vtkRCLP()
	//{
	//
	//}

	static vtkRCLP* New()
	{
		return new vtkRCLP;
	}
	//irtual void CreateDefaultResliceAlgorithm() override
	//
	//	// Allows users to optionally use their own reslice filters or other
	//	// algorithms here.
	//	if (!this->Reslice)
	//	{
	//		this->Reslice = vtkImageReslice::New();			
	//	}
	//	auto flip = vtkSmartPointer< vtkImageFlip>::New();
	//	flip->SetFilteredAxis(1);
	//	flip->SetInputConnection(this->Reslice->GetOutputPort());
	//	this->Reslice->SetInputConnection(flip->GetOutputPort());
	//
	void SetResliceParameters(double outputSpacingX, double outputSpacingY, int extentX, int extentY)override
	{
		vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(this->Reslice);
	
		if (reslice)
		{
			// Set the default color the minimum scalar value
			double range[2];
			vtkImageData::SafeDownCast(reslice->GetInput())->GetScalarRange(range);
			reslice->SetBackgroundLevel(range[0]);
			//-------------------------
			auto flip = vtkSmartPointer< vtkImageFlip>::New();
			flip->SetFilteredAxis(1);
			flip->SetInputConnection(reslice->GetOutputPort());
			flip->Update();
			//reslice->SetInputConnection(flip->GetOutputPort());
			//--------------------------

			//???
			//???
			if (0)	this->ColorMap->SetInputConnection(flip->GetOutputPort());
			else    this->ColorMap->SetInputConnection(reslice->GetOutputPort());

			reslice->TransformInputSamplingOff();
			reslice->AutoCropOutputOn();
			if (1)
			{
				vtkSmartPointer<vtkMatrix4x4> sagittalMatrix =	vtkSmartPointer<vtkMatrix4x4>::New();
				sagittalMatrix->DeepCopy(this->ResliceAxes);
				// 设置 X 轴镜像翻转
				//sagittalMatrix->SetElement(0, 0, -1);  // 水平镜像
				//sagittalMatrix->SetElement(0, 3, sagittalMatrix->GetElement(0, 3));  // 调整偏移
				reslice->SetResliceAxes(sagittalMatrix);
			}
			else
			reslice->SetResliceAxes(this->ResliceAxes);

			//reslice->SetResliceTransform(Transform);
			reslice->SetOutputSpacing(outputSpacingX, outputSpacingY, 1);
			reslice->SetOutputOrigin(0.5 * outputSpacingX, 0.5 * outputSpacingY, 0);
			reslice->SetOutputExtent(0, extentX - 1, 0, extentY - 1, 0, 0);		
		}
	}

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
	return itkTovtkImageFilter->GetOutput();

	//vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
	//ImageFlip->SetInputData(itkTovtkImageFilter->GetOutput());
	//
	//ImageFlip->SetFilteredAxes(1);
	//ImageFlip->Update();
	//vtkSmartPointer<vtkImageData> vtkdata = ImageFlip->GetOutput();
	//ImageFlip = NULL;
	//itkTovtkImageFilter = NULL;
	//return vtkdata;

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

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char *argv[])
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
		vtkWidgetRepresentation *tation = riw[i]->GetResliceCursorWidget()->GetRepresentation();
		vtkResliceCursorThickLineRepresentation *rep = vtkResliceCursorThickLineRepresentation::SafeDownCast(tation);
		vtkImageSlabReslice *thickSlabReslice = vtkImageSlabReslice::SafeDownCast(rep->GetReslice());
		vtkResliceCursorThickLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation())->GetReslice();
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
		vtkPlaneSource *ps = static_cast<vtkPlaneSource *>(planeWidget[i]->GetPolyDataAlgorithm());
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
	this->DistanceWidget[i]->SetInteractor(this->riw[i]->GetResliceCursorWidget()->GetInteractor());

	// Set a priority higher than our reslice cursor widget
	this->DistanceWidget[i]->SetPriority(this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

	vtkSmartPointer< vtkPointHandleRepresentation2D > handleRep = vtkSmartPointer< vtkPointHandleRepresentation2D >::New();
	vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep = vtkSmartPointer< vtkDistanceRepresentation2D >::New();
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
class QeventMouse :public QObject
{
public:
	QeventMouse() {	};
	~QeventMouse() { };
	bool eventFilter(QObject *object, QEvent *event)
	{
		if (event->type() == QEvent::Wheel)
		{
			QString name = object->objectName();
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

void QtVTKRenderWindows::MprInit()
{
	//return GPTMRP3D();
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

	///012  021 120 102  201 210  
	vtkImageData *imageData  = reader->GetOutput();
	//if (10)
	//{
		vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
		ImageFlip->SetInputData(reader->GetOutput());
		ImageFlip->SetFilteredAxes(0);
		ImageFlip->Update();
		imageData = ImageFlip->GetOutput();
	//}
	//vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
	//ImageFlip->SetInputData(imageData);
	//ImageFlip->SetFilteredAxes(2);
	//ImageFlip->Update();
	//imageData = ImageFlip->GetOutput();
	//vtkSmartPointer< vtkImageFlip > ImageFlip1 = vtkSmartPointer< vtkImageFlip >::New();
	//ImageFlip1->SetInputData(imageData);
	//ImageFlip1->SetFilteredAxes(1);
	//ImageFlip1->Update();
	//imageData = ImageFlip1->GetOutput();
	//vtkSmartPointer< vtkImageFlip > ImageFlip2 = vtkSmartPointer< vtkImageFlip >::New();
	//ImageFlip2->SetInputData(imageData);
	//ImageFlip2->SetFilteredAxes(0);
	//ImageFlip2->Update();
	//imageData = ImageFlip2->GetOutput();
	//vtkSmartPointer< vtkImageFlip > ImageFlip3 = vtkSmartPointer< vtkImageFlip >::New();
	//ImageFlip3->SetInputData(imageData);
	//ImageFlip3->SetFilteredAxes(2);
	//ImageFlip3->Update();
	//imageData = ImageFlip3->GetOutput();


	//vtkTransform* transform = vtkTransform::New();
	//transform->RotateZ(180);
	//vtkImageReslice* reslice = vtkImageReslice::New();
	//reslice->SetInputData(imageData);
	//reslice->SetInformationInput(imageData);
	//reslice->SetResliceTransform(transform);
	//reslice->SetOutputDimensionality(3);
	//reslice->Update();

	///https://gitlab.kitware.com/vtk/vtk/-/issues/18726
	///https://discourse.vtk.org/t/flip-mpr-image-become-abnormal/9931
	vtkSmartPointer< vtkRenderWindowInteractor >renderWindowInteractor[3];
	vtkSmartPointer< vtkResliceCursorLineRepresentation > vtkrclp[3];
	vtkSmartPointer< vtkResliceCursor > resliceCursor = vtkSmartPointer< vtkResliceCursor >::New();
	///
	if (0)
	{
		for (int i = 0; i < 3; i++)
		{
			riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
			if (i == 2)
			{
				vtkrclp[i] = vtkSmartPointer< vtkRCLP >::New();
			}
			else
			{
				vtkrclp[i] = vtkSmartPointer< vtkResliceCursorLineRepresentation >::New();
			}

			renderWindowInteractor[i] = vtkSmartPointer< vtkRenderWindowInteractor >::New();
			riw[i]->SetupInteractor(renderWindowInteractor[i]);
		}

		for (int i = 0; i < 3; i++)
		{
			riw[i]->GetResliceCursorWidget()->SetRepresentation(vtkrclp[i]);
			riw[i]->SetResliceCursor(resliceCursor);
			vtkrclp[i]->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

			vtkrclp[i]->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
			vtkrclp[i]->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
			vtkrclp[i]->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线

			riw[i]->SetInputData(reader->GetOutput());

			riw[i]->GetRenderWindow()->SetSize(500, 500);
			riw[i]->GetRenderWindow()->SetPosition(800 + i * 500, 500);
			if (i == 2)
				riw[i]->GetRenderWindow()->SetPosition(800, 0);
			riw[i]->SetSliceOrientation(1);
			riw[i]->SetSliceOrientation(i);
		}

		for (int i = 0; i < 3; i++)
		{
			riw[i]->SetResliceMode(1);
			riw[i]->GetRenderer()->ResetCamera();
			riw[i]->Render();
		}
	}
	else
	{
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
			if (10)
			{
				// 假设你已经有了 resliceImageViewer 的实例
				vtkSmartPointer<vtkResliceImageViewer> resliceImageViewer = riw[i];

				// 获取 vtkResliceCursor 对象
				vtkSmartPointer<vtkResliceCursor> resliceCursor = resliceImageViewer->GetResliceCursor();

				// 获取当前光标矩阵
				vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
				// 获取当前切片平面方向
				double xAxis[3], yAxis[3], zAxis[3];
				resliceCursor->GetXAxis(xAxis);
				resliceCursor->GetYAxis(yAxis);
				resliceCursor->GetZAxis(zAxis);

				// 创建一个新的旋转矩阵（绕Z轴旋转90度）
				// 旋转X轴和Y轴 90 度（绕Z轴旋转）
				double newXAxis[3] = { -yAxis[0], -yAxis[1], -yAxis[2] };
				double newYAxis[3] = { xAxis[0], xAxis[1], xAxis[2] };

				// 更新轴方向
				resliceCursor->SetXAxis(newXAxis);
				resliceCursor->SetYAxis(newYAxis);

				// 通知视图更新
				//resliceImageViewer->Render();
			}
		}
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

	if (0)
	{
		vtkSmartPointer<vtkImageReslice> reslicer =	vtkSmartPointer<vtkImageReslice>::New();
		reslicer->SetInputConnection(reader->GetOutputPort());
		vtkSmartPointer<vtkMatrix4x4> flipMatrix =	vtkSmartPointer<vtkMatrix4x4>::New();
		flipMatrix->Identity();            // 初始化为单位矩阵
		flipMatrix->SetElement(0, 0, -1);  // X 轴镜像翻转
		// flipMatrix->SetElement(1, 1, -1);  // Y 轴镜像翻转（如果需要）

		reslicer->SetResliceAxes(flipMatrix);
		reslicer->SetInterpolationModeToLinear();
		reslicer->Update();
		riw[2]->SetInputConnection(reslicer->GetOutputPort());
	//}
	//{
		
		// 获取相机
		vtkCamera* camera = riw[2]->GetRenderer()->GetActiveCamera();

		// 设置相机的位置到图像正后方
		double focalPoint[3] = { 0.0, 0.0, 0.0 };  // 图像中心
		double backPosition[3] = { 0.0, 0.0, -10.0 };  // 图像正后方

		camera->SetFocalPoint(focalPoint);
		camera->SetPosition(backPosition);
		camera->SetViewUp(0.0, 1.0, 0.0);  // 设置 "向上" 的方向

		// 渲染并启动交互
		riw[2]->GetRenderer()->ResetCamera();  // 自动调整相机视角以适应数据
		riw[2]->GetRenderer()->GetRenderWindow()->Render();
		//m_renderWindow->Render();
	}
	for (int i = 0; i < 3; i++)
	{
		riw[i]->SetResliceMode(1);
		riw[i]->GetRenderer()->ResetCamera();
		riw[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
		riw[i]->Render();
	}

	//riw[2]->GetRenderer()->GetActiveCamera()->SetViewUp(0, 1, 0);
	//riw[2]->GetRenderer()->GetActiveCamera()->Azimuth(180);
	//riw[2]->GetRenderer()->GetActiveCamera()->SetViewUp(0,-1, 0);
	//riw[2]->GetRenderer()->GetActiveCamera()->Elevation(180);
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
		cbk->SIV[i] = riw[i];
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
void QtVTKRenderWindows::on_DcmDIr_clicked()
{
	this->hide();
	MprInit();
	this->show();
}

///--------------------------------------------------------VTK9.2
class MPRvtkResliceCursorCallback : public vtkCommand
{
public:
	static MPRvtkResliceCursorCallback* New()
	{
		return new MPRvtkResliceCursorCallback;
	}
	void Execute(vtkObject* caller, unsigned long ev,
		void* callData) override
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
			//return;

		}

		vtkResliceCursorWidget* rcw = dynamic_cast<
			vtkResliceCursorWidget*>(caller);
		if (rcw)
		{
			vtkResliceCursorLineRepresentation* rep = dynamic_cast<
				vtkResliceCursorLineRepresentation*>(rcw->GetRepresentation());
			double window[2];
			rep->GetWindowLevel(window);

			//同步的前提下才能实现
			for (size_t i = 0; i < 3; i++)
			{
				RCW[i]->GetResliceCursorRepresentation()->SetWindowLevel(window[0], window[1]);
			}
			rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
			for (int i = 0; i < 3; i++)
			{
				vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(
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

	MPRvtkResliceCursorCallback() {}
	vtkImagePlaneWidget* IPW[3];
	vtkResliceCursorWidget* RCW[3];

	vtkResliceImageViewer* RIW[3];
};


void QtVTKRenderWindows::GPTMRP3D()
{
	// 1. 读取 DICOM 图像
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


	// 2. 使用 vtkImageReslice 实现矢状面镜像
	vtkSmartPointer<vtkImageReslice> sagittalReslice =
		vtkSmartPointer<vtkImageReslice>::New();
	sagittalReslice->SetInputConnection(reader->GetOutputPort());

	// 获取图像方向的初始轴
	vtkSmartPointer<vtkMatrix4x4> sagittalMatrix =
		vtkSmartPointer<vtkMatrix4x4>::New();
	sagittalMatrix->DeepCopy(sagittalReslice->GetResliceAxes());

	// 设置 X 轴镜像翻转
	sagittalMatrix->SetElement(0, 0, -1);  // 水平镜像
	sagittalMatrix->SetElement(0, 3, sagittalMatrix->GetElement(0, 3));  // 调整偏移

	sagittalReslice->SetResliceAxes(sagittalMatrix);
	sagittalReslice->SetInterpolationModeToLinear();  // 设置插值模式
	sagittalReslice->Update();

	// 3. 创建 vtkResliceImageViewer 用于矢状面
	vtkSmartPointer<vtkResliceImageViewer> sagittalViewer =
		vtkSmartPointer<vtkResliceImageViewer>::New();
	sagittalViewer->SetInputConnection(sagittalReslice->GetOutputPort());
	sagittalViewer->SetSliceOrientationToYZ();  // 矢状面

	// 4. 配置渲染窗口和交互器
	vtkSmartPointer<vtkRenderWindowInteractor> sagittalInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	sagittalViewer->SetupInteractor(sagittalInteractor);
	sagittalViewer->GetRenderWindow()->SetSize(400, 400);

	// 5. 渲染并启动交互
	sagittalViewer->Render();
	sagittalInteractor->Start();

}
void QtVTKRenderWindows::MRP3D()
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

	vtkSmartPointer< vtkResliceImageViewer >riw[3];
	vtkSmartPointer< vtkRenderWindow >renderWindow[3];
	vtkSmartPointer< vtkRenderWindowInteractor >renderWindowInteractor[3];
	vtkSmartPointer< vtkResliceCursor > resliceCursor = vtkSmartPointer< vtkResliceCursor >::New();
	vtkSmartPointer< vtkRCLP > rep[3];

	resliceCursor->SetCenter(reader->GetOutput()->GetCenter());

	resliceCursor->SetImage(reader->GetOutput());
	for (int i = 0; i < 3; i++)
	{
		riw[i] = vtkSmartPointer< vtkResliceImageViewer >::New();
		rep[i] = vtkSmartPointer< vtkRCLP >::New();
		renderWindowInteractor[i] = vtkSmartPointer< vtkRenderWindowInteractor >::New();
		riw[i]->SetupInteractor(renderWindowInteractor[i]);
	}

	for (int i = 0; i < 3; i++)
	{
		riw[i]->GetResliceCursorWidget()->SetRepresentation(rep[i]);
		riw[i]->SetResliceCursor(resliceCursor);
		rep[i]->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

		rep[i]->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep[i]->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep[i]->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线

		riw[i]->SetInputData(reader->GetOutput());

		riw[i]->GetRenderWindow()->SetSize(500, 500);
		riw[i]->GetRenderWindow()->SetPosition(800 + i * 500, 500);
		if (i == 2)
			riw[i]->GetRenderWindow()->SetPosition(800, 0);
		riw[i]->SetSliceOrientation(1);
		riw[i]->SetSliceOrientation(i);
	}

	for (int i = 0; i < 3; i++)
	{
		riw[i]->SetResliceMode(1);
		riw[i]->GetRenderer()->ResetCamera();
		riw[i]->Render();
	}

	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();
	vtkSmartPointer< vtkRenderer > ren = vtkSmartPointer< vtkRenderer >::New();
	auto renderWindow4 = vtkSmartPointer<vtkRenderWindow>::New();;//第四个窗口
	auto iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();//第四个窗口的vtkRenderWindowInteractor

	renderWindow4->SetInteractor(iren);
	renderWindow4->AddRenderer(ren);
	vtkSmartPointer<vtkImagePlaneWidget>planeWidget[3];
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
		riw[i]->GetRenderer()->SetBackground(0, 0, 0);

		planeWidget[i]->SetTexturePlaneProperty(ipwProp);
		planeWidget[i]->TextureInterpolateOff();
		planeWidget[i]->SetResliceInterpolateToLinear();
		planeWidget[i]->SetInputConnection(reader->GetOutputPort());
		planeWidget[i]->SetPlaneOrientation(i);
		planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
		planeWidget[i]->DisplayTextOn();
		planeWidget[i]->SetDefaultRenderer(ren);
		planeWidget[i]->SetWindowLevel(1358, -27);
		planeWidget[i]->On();
		planeWidget[i]->InteractionOn();
	}
	auto cbk = vtkSmartPointer<MPRvtkResliceCursorCallback>::New();

	for (int i = 0; i < 3; i++)
	{
		cbk->RIW[i] = riw[i];
		cbk->IPW[i] = planeWidget[i];
		cbk->RCW[i] = riw[i]->GetResliceCursorWidget();

		vtkWidgetEventTranslator* WidgetTrans = riw[i]->GetResliceCursorWidget()->GetEventTranslator();

		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);

		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);

		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);

		riw[i]->SetLookupTable(riw[0]->GetLookupTable());
		planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
		// planeWidget[i]->GetColorMap()->(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
		planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());

	}

	auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	iren->SetInteractorStyle(style);
	renderWindow4->SetSize(500, 500);
	renderWindow4->SetPosition(1300, 0);
	renderWindow4->Render();

	iren->Initialize();
	iren->Start();
}


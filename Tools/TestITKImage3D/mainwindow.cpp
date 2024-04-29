#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkPNGImageIOFactory.h>
#include "itkGDCMImageIO.h"
#include <itkJPEGImageIOFactory.h>
#include <vtkAutoInit.h> 
#include <vtkRenderingVolumeOpenGL2ObjectFactory.h>
#include <vtkRenderingOpenGL2ObjectFactory.h>
#include <vtkOutlineFilter.h>
#include <vtkOBJExporter.h>
#include <vtkMetaImageReader.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageData.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkRendererCollection.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkMarchingCubes.h"
#include "vtkStripper.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkImageShrink3D.h"
#include "vtkDecimatePro.h"
#include <vtkInteractorStyleImage.h>
#include "itkImageToVTKImageFilter.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include<vtkDICOMImageReader.h>
#include<vtkCamera.h>
#include<vtkActor.h>
#include<vtkVolumeProperty.h>
#include<vtkPolyDataNormals.h>
#include<vtkImageShiftScale.h>
#include "vtkSmartVolumeMapper.h"
#include<vtkPiecewiseFunction.h>
#include<vtkColorTransferFunction.h>
#include<vtkImageCast.h>
#include<vtkLODProp3D.h>
#include <vtkAxesActor.h>
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>
#include <vtkImageViewer2.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageFlip.h>
#include <vtkImageGradient.h>
#include <vtkImageMagnitude.h>
#include <vtkImageHybridMedian2D.h>
#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkSliderWidget.h>
#include <vtkCommand.h>
#include <vtkWidgetEvent.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkSliderRepresentation2D.h>
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
#include <string>
//体绘制加速
//Gpu光照映射
#include<vtkGPUVolumeRayCastMapper.h>
#include<iostream>
#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/fileap.h"
#include "dcmtk/ofstd/ofstd.h"
#include <vector>
#include <map>

// On Solaris with Sun Workshop 11, <signal.h> declares signal() but <csignal> does not
#include <signal.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);


LPCWSTR stringLPCWSTR(std::string str)
{
	size_t size = str.length();
	int wlen = ::MultiByteToWideChar(CP_UTF8, 0,
		str.c_str(),
		-1,
		NULL,
		0);
	wchar_t *buffer = new wchar_t[wlen + 1];
	memset(buffer, 0, (wlen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), size, (LPWSTR)buffer, wlen);
	std::wstring wstr = buffer;
	delete[] buffer;
	return wstr.c_str();
}

using namespace std;
typedef short PixelType;
const unsigned int   Dimension = 3;
typedef itk::Image< PixelType, Dimension > Input2dImageType;
typedef itk::Image< PixelType, 3 > Input3dImageType;

typedef itk::Image< PixelType, Dimension > Output2dImageType;
typedef itk::Image< PixelType, 3 > Output3dImageType;



Input3dImageType::Pointer GdcmRead3dImage(std::string path, QString dir = "");
QStringList generateFilenames(const QString &dirPath);
void scanDirectories(const QString &rootPath, QStringList &dirsList);
void scanDirectories(const QString &rootPath, QStringList &dirsList)
{
	QDir rootDir(rootPath);
	if (rootDir.exists())
	{
		// We add the current directory to the list
		dirsList << rootPath;
		// We look to see if we have more directories
		QStringList subdirs = rootDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
		if (!subdirs.isEmpty())
		{
			// For each subdirectory we scan recursively
			foreach(const QString &subDir, subdirs)
			{
				scanDirectories(rootPath + "/" + subDir, dirsList);
			}
		}
	}
}
QStringList generateFilenames(const QString &dirPath)
{
	QStringList list;
	//We check that the directory has files
	QDir dir(dirPath);
	QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);

	QString suffix;
	//We add to the list each of the absolute paths of the files contained in the directory
	foreach(const QFileInfo &fileInfo, fileInfoList)
	{
		suffix = fileInfo.suffix();
		if ((suffix.length() > 0 && suffix.toLower() == "dcm") || suffix.length() == 0)
		{
			list << fileInfo.absoluteFilePath();
		}
	}

	return list;
}

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
#ifndef QSACN
	getFileNames(dir, fileNames);
#else
	std::vector< std::string > fileNames2;
	QStringList filenames;
	if (dir.length() > 0)
	{
		QStringList dirList;
		scanDirectories(dir, dirList);
		//For each directory, we get the files we can handle
		foreach(const QString &dirName, dirList)
		{
			filenames << generateFilenames(dirName);
		}
	}
	for (int i = 0; i < filenames.size(); i++)
	{
		fileNames2.push_back(std::string(filenames.at(i).toUtf8()));
	}
#endif
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


class vtkSliderCallback : public vtkCommand
{
public:
    static vtkSliderCallback *New()
    {
        return new vtkSliderCallback;
    }
    virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
        vtkSliderWidget *sliderWidget =   reinterpret_cast<vtkSliderWidget*>(caller);
        this->viewer->SetSlice(static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue());
    }
    vtkSliderCallback() {}
    vtkSmartPointer<vtkImageViewer2> viewer = nullptr;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
	vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());
}

MainWindow::~MainWindow()
{
    delete ui;
}

//https://blog.csdn.net/Ericohe/article/details/114368594

void MainWindow::on_pBDir_clicked()
{
    QString dlg;
    dlg = ui->m_dcmDir->toPlainText();
    //第三个参数 如果是"./" 代表当前应用的目录. QString()空为上次打开的目录
    QString  path = QFileDialog::getExistingDirectory(this,"select dicom dir...",dlg/*QString()*/,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    //QString path(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = ShowDirsOnly);
    //ui->cbDcmDir->setCurrentText(path);
    ui->m_dcmDir->setText(path);
    ui->m_dcmDir->update();
}

void showImage2D(Input3dImageType::Pointer image)
{
	vtkSmartPointer<vtkImageViewer2> viewer = vtkSmartPointer<vtkImageViewer2>::New();
	viewer->SetInputData(ImageDataItkToVtk(image));
	//设置基本属性
	viewer->SetSize(640, 480);
	viewer->SetColorLevel(500);
	viewer->SetColorWindow(2000);
	viewer->SetSlice(40);
	viewer->SetSliceOrientationToXY();
	viewer->Render();
	viewer->GetRenderer()->SetBackground(0, 0, 0);
	viewer->GetRenderWindow()->SetWindowName("ImageViewer2D");

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> vtkInteractorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> RenderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//设置交互属性
	viewer->SetupInteractor(RenderWindowInteractor);
	viewer->GetRenderer()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(vtkInteractorStyle);

	vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
	sliderRep->SetMinimumValue(viewer->GetSliceMin());
	sliderRep->SetMaximumValue(viewer->GetSliceMax());
	sliderRep->SetValue(5.0);
	sliderRep->GetSliderProperty()->SetColor(1, 0, 0);//red
	sliderRep->GetTitleProperty()->SetColor(1, 0, 0);//red
	sliderRep->GetLabelProperty()->SetColor(1, 0, 0);//red
	sliderRep->GetSelectedProperty()->SetColor(0, 1, 0);//green
	sliderRep->GetTubeProperty()->SetColor(1, 1, 0);//yellow
	sliderRep->GetCapProperty()->SetColor(1, 1, 0);//yellow
	sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
	sliderRep->GetPoint1Coordinate()->SetValue(40, 40);
	sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
	sliderRep->GetPoint2Coordinate()->SetValue(500, 40);
	vtkSmartPointer<vtkSliderWidget> sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
	sliderWidget->SetInteractor(RenderWindowInteractor);
	sliderWidget->SetRepresentation(sliderRep);
	sliderWidget->SetAnimationModeToAnimate();
	sliderWidget->EnabledOn();

	vtkSmartPointer<vtkSliderCallback> callback = vtkSmartPointer<vtkSliderCallback>::New();
	callback->viewer = viewer;

	sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);
	vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
	double axesSize[3] = { 100,100,100 };
	axes->SetTotalLength(axesSize);
	axes->SetConeRadius(0.1);
	axes->SetShaftTypeToLine();
	axes->SetAxisLabels(false);
	viewer->GetRenderer()->AddActor(axes);
	RenderWindowInteractor->Start();
	viewer->Render();
}
//showDcmImage2D
void MainWindow::on_pBITK_clicked()/////https://blog.csdn.net/l1783111653/article/details/108457374
{
	QString dir = ui->m_dcmDir->toPlainText();
	std::string Input_Name = qPrintable(dir);

	Input3dImageType::Pointer image = GdcmRead3dImage(Input_Name, dir);
	showImage2D(image);
	return;
}



void MainWindow::initImage3D_ITK_VTK(vtkActor *vtkactor)
{
	//https://developer.aliyun.com/article/933543
	QString dir = ui->m_dcmDir->toPlainText();

	Input3dImageType::Pointer image = GdcmRead3dImage(qPrintable(dir), dir);

	vtkMarchingCubes *marchingcube = vtkMarchingCubes::New();
	vtkSmartPointer<vtkImageData> ImageVTKData = ImageDataItkToVtk(image);
	image = NULL;

	marchingcube->SetInputData(ImageVTKData);
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
	vtkactor->GetProperty()->SetDiffuseColor(.50, .49, .25);//设置皮肤颜色；
	vtkactor->GetProperty()->SetSpecular(0.3);//反射率；
	vtkactor->GetProperty()->SetOpacity(1.0);//透明度；
	vtkactor->GetProperty()->SetSpecularPower(20);//反射光强度；
	vtkactor->GetProperty()->SetColor(0.45, 0.23, 0.26);//设置角的颜色；
	vtkactor->GetProperty()->SetRepresentationToWireframe();//线框；

	vtkOutlineFilter * outfilterline = vtkOutlineFilter::New();
	outfilterline->SetInputData(ImageVTKData);
	ImageVTKData = NULL;
	
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
}

void MainWindow::showImage3D_ITK_VTK()
{
	vtkActor *vtkactor = vtkActor::New();
	initImage3D_ITK_VTK(vtkactor);

	vtkRenderer *vtkrenderer = vtkRenderer::New();
	vtkRenderWindow* show3DWinow = vtkRenderWindow::New();
	show3DWinow->AddRenderer(vtkrenderer);

	vtkRenderWindowInteractor *show3DInteractor = vtkRenderWindowInteractor::New();
	show3DInteractor->SetRenderWindow(show3DWinow);
	vtkrenderer->AddActor(vtkactor);

	//vtkrenderer->AddActor(OutlineActor);
	//OutlineActor = NULL;
	vtkrenderer->SetBackground(1, 1, 1);//设置背景颜色；

	show3DWinow->SetSize(1000, 600);

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


void MainWindow::on_pBITK3D_clicked()//return show3DIVTK(Input_Name);
{
	showImage3D_ITK_VTK();
}


void MainWindow::on_pBVolume3D_clicked()
{
	QString dir = ui->m_dcmDir->toPlainText();
	std::string Input_Name = qPrintable(dir);
	Input3dImageType::Pointer dicomimage = GdcmRead3dImage(Input_Name, dir);
	static bool init = false;
	if (!init)
	{
		vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
		vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());
		init = true;
	}

	//定义绘制器；
	vtkRenderer *rendererViewer = vtkRenderer::New();//指向指针；
	vtkSmartPointer<vtkRenderWindow> RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	RenderWindow->AddRenderer(rendererViewer);

	//透明度映射函数定义；
	vtkPiecewiseFunction *opacityTransform = vtkPiecewiseFunction::New();
	opacityTransform->AddPoint(0, 0.0);
	opacityTransform->AddPoint(20, 0.0);
	opacityTransform->AddPoint(200, 1.0);
	opacityTransform->AddPoint(300, 1.0);

	//颜色映射函数定义,梯度上升的
	vtkColorTransferFunction *colorTransformFunction = vtkColorTransferFunction::New();
	colorTransformFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	colorTransformFunction->AddRGBPoint(64.0, 0.0, 0.0, 0.0);
	colorTransformFunction->AddRGBPoint(128.0, 1.0, 0.0, 0.0);
	colorTransformFunction->AddRGBPoint(192.0, 1.0, 0.0, 0.0);
	colorTransformFunction->AddRGBPoint(255.0, 1.0, 0.0, 0.0);

	vtkPiecewiseFunction *gradientTransform = vtkPiecewiseFunction::New();
	gradientTransform->AddPoint(0, 0.0);

	gradientTransform->AddPoint(20, 2.0);
	gradientTransform->AddPoint(200, 0.1);
	gradientTransform->AddPoint(300, 0.1);


	//体数据属性；
	vtkVolumeProperty *volumeProperty = vtkVolumeProperty::New();
	volumeProperty->SetColor(colorTransformFunction);
	volumeProperty->SetScalarOpacity(opacityTransform);
	volumeProperty->SetGradientOpacity(gradientTransform);
	volumeProperty->ShadeOn();//应用
	volumeProperty->SetInterpolationTypeToLinear();//直线间样条插值；
	volumeProperty->SetAmbient(0.4);//环境光系数；
	volumeProperty->SetDiffuse(0.69996);//漫反射；
	volumeProperty->SetSpecular(0.2);
	volumeProperty->SetSpecularPower(10);//高光强度；

	//光纤映射类型定义：
	//Mapper定义,
	//vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	vtkSmartVolumeMapper *volumeMapper = vtkSmartVolumeMapper::New();
	volumeMapper->SetInputData(ImageDataItkToVtk(dicomimage));//;cast_file->GetOutput());
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetRequestedRenderModeToDefault();
	//vtkSmartPointer<vtkLODProp3D> lodProp3D = vtkSmartPointer<vtkLODProp3D>::New();
	vtkLODProp3D *lodProp3D =  vtkLODProp3D::New();
	lodProp3D->AddLOD(volumeMapper, volumeProperty, 0.0);


	vtkVolume *volume = vtkVolume::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);//设置体属性；

	double volumeView[4] = { 0,0,0.5,1 };

	vtkOutlineFilter *outlineData = vtkOutlineFilter::New();//线框；
	outlineData->SetInputData(ImageDataItkToVtk(dicomimage));
	vtkPolyDataMapper *mapOutline = vtkPolyDataMapper::New();
	mapOutline->SetInputConnection(outlineData->GetOutputPort());
	vtkActor *outline = vtkActor::New();
	outline->SetMapper(mapOutline);
	outline->GetProperty()->SetColor(0, 0, 0);//背景纯黑色；

	rendererViewer->AddVolume(volume);
	rendererViewer->AddActor(outline);
	rendererViewer->SetBackground(1, 1, 1);
	rendererViewer->ResetCamera();


	//重设相机的剪切范围；
	rendererViewer->ResetCameraClippingRange();
	RenderWindow->SetSize(500, 500);
	RenderWindow->SetWindowName("Volume-3D");

	vtkRenderWindowInteractor *RenderWindowInteractor = vtkRenderWindowInteractor::New();
	RenderWindowInteractor->SetRenderWindow(RenderWindow);

	//设置相机跟踪模式
	vtkInteractorStyleTrackballCamera *Interactorstyle = vtkInteractorStyleTrackballCamera::New();
	RenderWindowInteractor->SetInteractorStyle(Interactorstyle);

	RenderWindow->Render();
	RenderWindowInteractor->Initialize();
	RenderWindowInteractor->Start();

	RenderWindowInteractor->Delete();
	lodProp3D->Delete();
	volumeMapper->Delete();
	colorTransformFunction->Delete();
	gradientTransform->Delete();
	volumeProperty->Delete();
	volume->Delete();
	outlineData->Delete();
	outline->Delete();
	mapOutline->Delete();
	Interactorstyle->Delete();
	rendererViewer->Delete();

}
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
#include<vtkRenderingVolumeOpenGL2ObjectFactory.h>
#include<vtkRenderingOpenGL2ObjectFactory.h>
#include<vtkOutlineFilter.h>
#include<vtkOBJExporter.h>
#include<vtkMetaImageReader.h>
#include<vtkInteractorStyleTrackballCamera.h>
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
//!判断目录是否为空
OFString CheckDirPath(const OFString dir)
{
#ifdef HAVE_WINDOWS_H
	OFString path = dir;
	if (path == "")
	{
		return "";
	}
	if (path[path.length() - 1] != '\\'&& path[path.length() - 1] != '/')
	{
		path += "\\";
	}
	return path;
#else
	//to do add!

#endif
}

void SearchDirFile(const OFString Dir, const OFString FileExt, std::vector< std::string > &datas, const bool Not = false, const int Count = 200);
void SearchDirectory(const OFString Dir, std::vector< std::string > &datas);
void SearchDirectory(const OFString Dir, std::vector< std::string > &datas)
{
	OFString dir;
#ifdef HAVE_WINDOWS_H
	dir = CheckDirPath(Dir);
	if (dir == "")
	{
		return;
	}
	OFString pach = dir + "*.*";
	WIN32_FIND_DATA sr;
	HANDLE h;
	if ((h = FindFirstFile(pach.c_str(), &sr)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			OFString name = sr.cFileName;
			if ((sr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				if (name != "." && name != "..")
				{
					SearchDirectory(dir + name, datas);
					datas.push_back((dir + name).c_str());
				}
			}
		} while (FindNextFile(h, &sr) != 0);
		FindClose(h);
	}
#else
	//to do add!
#endif
}

void SearchDirFile(const OFString Dir, const OFString FileExt, std::vector< std::string > &datas, const bool Not, const int Count)
{
	if (datas.size() >= Count)
	{
		return;
	}
#ifdef HAVE_WINDOWS_H
	OFString dir = CheckDirPath(Dir);
	if (dir == "")
	{
		return;
	}
	OFString pach = dir + "*.*";
	WIN32_FIND_DATA sr;
	HANDLE h;
	if ((h = FindFirstFile(pach.c_str(), &sr)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			OFString name = sr.cFileName;
			if ((sr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				if (name != "." && name != "..")
				{
					SearchDirFile(dir + name, FileExt, datas);
				}
			}
			else
			{
				OFString findExt;
				int pos = name.find_last_of('.');
				if (pos > 0)
				{
					findExt = name.substr(pos + 1);
				}
				OFString temp, up1, up2;
				up1 = OFStandard::toUpper(temp, FileExt);
				up2 = OFStandard::toUpper(temp, findExt);
				if (FileExt == "")
				{
					datas.push_back((dir + name).c_str());
				}
				else if (Not == false && up1 == up2)
				{
					datas.push_back((dir + name).c_str());
				}
				else if (Not == true && up1 != up2)
				{
					datas.push_back((dir + name).c_str());
				}
			}
		} while (FindNextFile(h, &sr) != 0);
		FindClose(h);
	}
#else
	//to do add!

#endif
}
using namespace std;
typedef short PixelType;
const unsigned int   Dimension = 3;
typedef itk::Image< PixelType, Dimension > Input2dImageType;
typedef itk::Image< PixelType, 3 > Input3dImageType;

typedef itk::Image< PixelType, Dimension > Output2dImageType;
typedef itk::Image< PixelType, 3 > Output3dImageType;

typedef itk::GDCMImageIO   ImageIOType;//GDCMImageIO读DICOM
ImageIOType::Pointer gdcmImageIO = ImageIOType::New();

using ReaderType3d = itk::ImageSeriesReader< Input3dImageType >;
ReaderType3d::Pointer reader3d = ReaderType3d::New();

typedef itk::ImageFileReader< Input2dImageType > ReaderType2d;
ReaderType2d::Pointer reader2d = ReaderType2d::New();

using NamesGeneratorType = itk::GDCMSeriesFileNames;
NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

Input3dImageType::Pointer read3dImage(std::string path, QString dir = "");
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

Input3dImageType::Pointer read3dImage(std::string path, QString dir)
{
 
	reader3d->SetImageIO(gdcmImageIO);

	using FileNamesContainer = std::vector< std::string >;
	FileNamesContainer fileNames;
	if (1)
	{
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
	}
	else
	{
		//SearchDirFile(path.c_str(), "dcm", fileNames);
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
			fileNames.push_back(std::string(filenames.at(i).toUtf8()));
		}
	}
    reader3d->SetFileNames(fileNames);
    reader3d->Update();
    return reader3d->GetOutput();
}


vtkSmartPointer<vtkImageData> itkToVtk(Input3dImageType::Pointer image)
{
    typedef itk::ImageToVTKImageFilter< Input3dImageType> itkTovtkFilterType;
    itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
    itkTovtkImageFilter->SetInput(image);//设置图像数据从ITK转向VTK
    itkTovtkImageFilter->Update();

    vtkSmartPointer< vtkImageFlip > flip1 = vtkSmartPointer< vtkImageFlip >::New();
    flip1->SetInputData(itkTovtkImageFilter->GetOutput());

    flip1->SetFilteredAxes(1);
    flip1->Update();
    return flip1->GetOutput();
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

void show3dImage(Input3dImageType::Pointer image)
{
    vtkSmartPointer<vtkImageViewer2> viewer =   vtkSmartPointer<vtkImageViewer2>::New();
    viewer->SetInputData(itkToVtk(image));
    //设置基本属性
    viewer->SetSize(640, 480);
    viewer->SetColorLevel(500);
    viewer->SetColorWindow(2000);
    viewer->SetSlice(40);
    viewer->SetSliceOrientationToXY();
    viewer->Render();
    viewer->GetRenderer()->SetBackground(0, 0, 0);
    viewer->GetRenderWindow()->SetWindowName("ImageViewer2D");

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =    vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> rwi =  vtkSmartPointer<vtkRenderWindowInteractor>::New();
    //设置交互属性
    viewer->SetupInteractor(rwi);
    //viewer->GetRenderer()->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);

    vtkSmartPointer<vtkSliderRepresentation2D> sliderRep =  vtkSmartPointer<vtkSliderRepresentation2D>::New();
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
    vtkSmartPointer<vtkSliderWidget> sliderWidget =     vtkSmartPointer<vtkSliderWidget>::New();
    sliderWidget->SetInteractor(rwi);
    sliderWidget->SetRepresentation(sliderRep);
    sliderWidget->SetAnimationModeToAnimate();
    sliderWidget->EnabledOn();

    vtkSmartPointer<vtkSliderCallback> callback =   vtkSmartPointer<vtkSliderCallback>::New();
    callback->viewer = viewer;

    sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);
    vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
    double axesSize[3] = { 100,100,100 };
    axes->SetTotalLength(axesSize);
    axes->SetConeRadius(0.1);
    axes->SetShaftTypeToLine();
    axes->SetAxisLabels(false);
    viewer->GetRenderer()->AddActor(axes);
    rwi->Start();
    viewer->Render();
}


void write3dImage(const char *path, Input3dImageType::Pointer image)
{
    itksys::SystemTools::MakeDirectory(path);
    typedef itk::ImageSeriesWriter < Input3dImageType, Output2dImageType >	WriterType;
    WriterType::Pointer writer = WriterType::New();

    nameGenerator->SetOutputDirectory(path);
    writer->SetMetaDataDictionaryArray(reader3d->GetMetaDataDictionaryArray());
    writer->SetImageIO(gdcmImageIO);
    writer->SetFileNames(nameGenerator->GetOutputFileNames());
    writer->SetInput(image);
    writer->Update();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//https://blog.csdn.net/Ericohe/article/details/114368594

bool dcm2jpeg(std::string InputDcmName, std::string OutputJpegName)
{
	using InputPixelType = signed short;
	const unsigned int InputDimension = 2;
	using InputImageType = itk::Image<InputPixelType, InputDimension>;
	//创建reader，设置读取的文件名
	using ReaderType = itk::ImageFileReader<InputImageType>;
	ReaderType::Pointer reader = ReaderType::New();
	//
	reader->SetFileName(InputDcmName);
	//创建读取DCM的GDCMIOImage类
	using ImageIOType = itk::GDCMImageIO;
	ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
	reader->SetImageIO(gdcmImageIO);
	//调用Update()触发过程,放置在try-catch模块
	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cerr << "exception in file reader" << std::endl;
		std::cerr << e << std::endl;
		return false;
	}
	using WriterPixelType = unsigned char;
	using WriteImageType = itk::Image<WriterPixelType, 2>;
	using RescaleFilterType = itk::RescaleIntensityImageFilter<InputImageType, WriteImageType>;
	RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
	rescaler->SetOutputMinimum(0);
	rescaler->SetOutputMaximum(255);
	//再次创建一个writer，写入到文件,这个时候文件以及变成了一个灰度图
	using Writer2Type = itk::ImageFileWriter<WriteImageType>;
	Writer2Type::Pointer writer2 = Writer2Type::New();
	writer2->SetFileName(OutputJpegName);
	rescaler->SetInput(reader->GetOutput());
	writer2->SetInput(rescaler->GetOutput());
	//itk::JPEGImageIOFactory::RegisterOneFactory();
	//itk::ObjectFactoryBase::RegisterFactory(itk::JPEGImageIOFactory::New());
	try
	{
		writer2->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cerr << "Exception in file writer " << std::endl;
		std::cerr << e << std::endl;
		return false;
	}
	return true;
}

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


//using PixelType = float;

void show3DIVTK(std::string const & path)//error!
{
	using ImageType = itk::Image< PixelType, 3 >;
	using ReaderType = itk::ImageSeriesReader< ImageType >;
	using ImageIOType = itk::GDCMImageIO;
	ReaderType::Pointer reader = ReaderType::New();
	ImageIOType::Pointer dicomIO = ImageIOType::New();
	reader->SetImageIO(dicomIO);
	//获取读取序列名称
	using NamesGeneratorType = itk::GDCMSeriesFileNames;
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetUseSeriesDetails(true);
	nameGenerator->SetDirectory(path);
	using SeriesIdContainer = std::vector< std::string >;
	const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
	auto seriesItr = seriesUID.begin();
	auto seriesEnd = seriesUID.end();
	using FileNamesContainer = std::vector< std::string >;
	FileNamesContainer fileNames;
	std::string seriesIdentifier;
	while (seriesItr != seriesEnd)
	{
		seriesIdentifier = seriesItr->c_str();
		fileNames = nameGenerator->GetFileNames(seriesIdentifier);
		++seriesItr;
	}
	//读取序列
	reader->SetFileNames(fileNames);
	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
		return ;
	}
	ImageType::SizeType imgSize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
	cout << "read done！Original size: " << imgSize << endl;

	typedef itk::ImageToVTKImageFilter< ImageType> itkTovtkFilterType;
	itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
	itkTovtkImageFilter->SetInput(reader->GetOutput());
	itkTovtkImageFilter->Update();

	vtkSmartPointer<vtkMarchingCubes> vesselExtractor = vtkMarchingCubes::New();
	vesselExtractor->SetInputData(itkTovtkImageFilter->GetOutput());
	vesselExtractor->SetNumberOfContours(10);
	vesselExtractor->SetValue(0, 1);   //轮廓

									   //将提取的等值面拼接成连续的
	vtkSmartPointer<vtkStripper> vesselStripper = vtkStripper::New();                           //建立三角带对象
	vesselStripper->SetInputConnection(vesselExtractor->GetOutputPort());


	vtkSmartPointer<vtkPolyDataMapper>  vesselMapper = vtkPolyDataMapper::New();     //建立一个数据映射对象
	vesselMapper->SetInputConnection(vesselStripper->GetOutputPort());                     //将三角带映射为几何数据
	vesselMapper->SetScalarRange(0, 7);

	//对象和对象属性等设置
	vtkSmartPointer<vtkActor> vessel = vtkActor::New();
	vessel->SetMapper(vesselMapper);
	vessel->GetProperty()->SetColor(0, 0, 1);

	// A renderer and render window
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(600, 600);
	renderer->AddActor(vessel);
	// An interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor  = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindow->Render();
	renderWindow->SetWindowName("SetWindowName");
	renderWindowInteractor->Initialize();
	renderWindowInteractor->Start();
	return ;
}


//using namespace std;
void MainWindow::on_pBITK_clicked()/////https://blog.csdn.net/l1783111653/article/details/108457374
{

	QString dir = ui->m_dcmDir->toPlainText();
	std::string Input_Name = qPrintable(dir);

	Input3dImageType::Pointer image = read3dImage(Input_Name, dir);
	show3dImage(image);
	//导出dicom序列图
	//write3dImage(output_Name ,image);
	return;
}

void MainWindow::on_pBITK3D_clicked()//return show3DIVTK(Input_Name);
{
	//https://developer.aliyun.com/article/933543
	QString dir = ui->m_dcmDir->toPlainText();
	std::string Input_Name = qPrintable(dir);
	vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
	vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());

	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();//WINDOW；

	renWin->AddRenderer(ren);

	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();//wininteratcor;
	iren->SetRenderWindow(renWin);

	Input3dImageType::Pointer image = read3dImage(Input_Name, dir);
	vtkSmartPointer<vtkImageViewer2> viewer = vtkSmartPointer<vtkImageViewer2>::New();
	viewer->SetInputData(itkToVtk(image));

	vtkSmartPointer<vtkMarchingCubes> marchingcube = vtkSmartPointer<vtkMarchingCubes>::New();
	marchingcube->SetInputData(itkToVtk(image));
	marchingcube->SetValue(0, 200);//Setting the threshold;
	marchingcube->ComputeNormalsOn();//计算表面法向量;

	vtkSmartPointer<vtkStripper> Stripper = vtkSmartPointer<vtkStripper>::New();
	Stripper->SetInputConnection(marchingcube->GetOutputPort());//获取三角片

	vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();//将三角片映射为几何数据；
	Mapper->SetInputConnection(Stripper->GetOutputPort());
	Mapper->ScalarVisibilityOff();//


	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();//Created a actor;
	actor->SetMapper(Mapper);//获得皮肤几何数据
	actor->GetProperty()->SetDiffuseColor(.50, .49, .25);//设置皮肤颜色；
	actor->GetProperty()->SetSpecular(0.3);//反射率；
	actor->GetProperty()->SetOpacity(1.0);//透明度；
	actor->GetProperty()->SetSpecularPower(20);//反射光强度；
	actor->GetProperty()->SetColor(0.45, 0.23, 0.26);//设置角的颜色；
	actor->GetProperty()->SetRepresentationToWireframe();//线框；

	vtkSmartPointer<vtkOutlineFilter> outfilterline = vtkSmartPointer<vtkOutlineFilter>::New();
	outfilterline->SetInputData(itkToVtk(image));
	vtkSmartPointer<vtkPolyDataMapper> outmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	outmapper->SetInputConnection(outfilterline->GetOutputPort());
	vtkSmartPointer<vtkActor> OutlineActor = vtkSmartPointer<vtkActor>::New();
	OutlineActor->SetMapper(outmapper);
	OutlineActor->GetProperty()->SetColor(0, 0, 0);//线框颜色

	ren->AddActor(actor);
	ren->AddActor(OutlineActor);

	ren->SetBackground(1, 1, 1);//设置背景颜色；
	renWin->SetSize(1000, 600);


	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	iren->SetInteractorStyle(style);

	renWin->Render();
	renWin->SetWindowName("SetWindowName");
	iren->Initialize();
	iren->Start();
}


void MainWindow::on_pBVolume3D_clicked()
{
	QString dir = ui->m_dcmDir->toPlainText();
	std::string Input_Name = qPrintable(dir);
	Input3dImageType::Pointer dicomimage = read3dImage(Input_Name, dir);

	vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
	vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());

	//定义绘制器；
	vtkRenderer *aRenderer = vtkRenderer::New();//指向指针；
	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(aRenderer);

	vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renWin);

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
	volumeProperty->SetDiffuse(0.6);//漫反射；
	volumeProperty->SetSpecular(0.2);
	volumeProperty->SetSpecularPower(10);//高光强度；

	//光纤映射类型定义：
	//Mapper定义,
	vtkSmartPointer<vtkSmartVolumeMapper> hiresMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	hiresMapper->SetInputData(itkToVtk(dicomimage));//;cast_file->GetOutput());

	vtkSmartPointer<vtkLODProp3D> prop = vtkSmartPointer<vtkLODProp3D>::New();
	prop->AddLOD(hiresMapper, volumeProperty, 0.0);


	vtkVolume *volume = vtkVolume::New();
	volume->SetMapper(hiresMapper);
	volume->SetProperty(volumeProperty);//设置体属性；

	double volumeView[4] = { 0,0,0.5,1 };

	vtkOutlineFilter *outlineData = vtkOutlineFilter::New();//线框；
	outlineData->SetInputData(itkToVtk(dicomimage));
	vtkPolyDataMapper *mapOutline = vtkPolyDataMapper::New();
	mapOutline->SetInputConnection(outlineData->GetOutputPort());
	vtkActor *outline = vtkActor::New();
	outline->SetMapper(mapOutline);
	outline->GetProperty()->SetColor(0, 0, 0);//背景纯黑色；

	aRenderer->AddVolume(volume);
	aRenderer->AddActor(outline);
	aRenderer->SetBackground(1, 1, 1);
	aRenderer->ResetCamera();


	//重设相机的剪切范围；
	aRenderer->ResetCameraClippingRange();
	renWin->SetSize(800, 800);
	renWin->SetWindowName("vtkVolume-3D");

	vtkRenderWindowInteractor *iren2 = vtkRenderWindowInteractor::New();
	iren2->SetRenderWindow(renWin);

	//设置相机跟踪模式
	vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
	iren2->SetInteractorStyle(style);

	renWin->Render();
	iren2->Initialize();
	iren2->Start();

}


// 定义图像类型
/*

	using PixelType = unsigned short;
	constexpr unsigned int Dimension = 3;
	using ImageType = itk::Image<PixelType, Dimension>;

	// DICOM 文件读取器
	using ReaderType = itk::ImageFileReader<ImageType>;
	ImageIOType::Pointer m_dcmtkIO = ImageIOType::New();
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetImageIO(m_dcmtkIO);
	std::string dcmfile = "F:/temp/starviewer/SDK/ITK/out/1.dcm";
	reader->SetFileName(dcmfile);

	try
	{
		// 执行读取操作
		reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cerr << "Exception caught while reading DICOM file: " << ex << std::endl;
		//return EXIT_FAILURE;
	}

	// 获取 DICOM 图像的方向信息
	ImageType::DirectionType direction = reader->GetOutput()->GetDirection();
	std::cout << "Image Direction:\n" << direction << std::endl;

	// 获取 DICOM 图像的坐标信息
	ImageType::PointType origin = reader->GetOutput()->GetOrigin();
	std::cout << "Image Origin:\n" << origin << std::endl;

	// Rescale Intensity 过滤器（设置对比度为默认值）
	using RescaleFilterType = itk::RescaleIntensityImageFilter<ImageType, ImageType>;
	RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
	rescaler->SetInput(reader->GetOutput());

	// PNG 文件写入器
	using WriterType = itk::ImageFileWriter<ImageType>;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName("F:/temp/starviewer/SDK/ITK/out/1.png");
	writer->SetInput(rescaler->GetOutput());
	ImageIOType::Pointer m_IO = ImageIOType::New();
	writer->SetImageIO(m_dcmtkIO);
	try
	{
		// 执行写入操作
		writer->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cerr << "Exception caught while writing PNG file: " << ex << std::endl;
		//return EXIT_FAILURE;
	}
*/
//设置读取图像类型
// 设置DICOM文件夹路径
/*	const char* dicomDirectory = output_Name;
	ReaderType::Pointer reader = ReaderType::New();
	ImageIOType::Pointer dicomIO = ImageIOType::New();
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetDirectory(dicomDirectory);
	const ReaderType::FileNamesContainer & filenames = nameGenerator->GetInputFileNames();
	reader->SetFileNames(filenames);
	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cerr << "Exception caught: " << ex << std::endl;
		return;////EXIT_FAILURE;
	}
	// 将ITK图像保存为PNG
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName("output.png");
	writer->SetInput(reader->GetOutput());
	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cerr << "Exception caught: " << ex << std::endl;
		return;// EXIT_FAILURE;
	}
	*/
	/*
		itksys::SystemTools::MakeDirectory(path);
		typedef itk::ImageSeriesWriter < Input3dImageType, Output2dImageType >	WriterType;
		WriterType::Pointer writer = WriterType::New();

		nameGenerator->SetOutputDirectory(path);
		writer->SetMetaDataDictionaryArray(reader3d->GetMetaDataDictionaryArray());
		writer->SetImageIO(gdcmImageIO);
		writer->SetFileNames(nameGenerator->GetOutputFileNames());
		writer->SetInput(image);
		writer->Update();
		/*	std::wstring wl = dir2.toStdWString();
	DWORD attr = GetFileAttributesW(wl.c_str());
	if (attr != INVALID_FILE_ATTRIBUTES)
	{
	}
#if defined(_WIN32)
	std::string ss = qPrintable(dir);
	//std::wstring wl = converter;
	wstring temp = wstring(ss.begin(), ss.end());
	LPCWSTR pcw = stringLPCWSTR(qPrintable(dir));// temp.c_str();// to_wide_string(ss).c_str();
	DWORD attr = GetFileAttributesW(pcw);
	DWORD w = INVALID_FILE_ATTRIBUTES;
*/	//DWORD attr = GetFileAttributesW(dir2.toStdWString().c_str()/*Encoding::ToWindowsExtendedPath(Input_Name).c_str()*/);
/*	if (attr != INVALID_FILE_ATTRIBUTES)
	{
		bool falg;

		falg = (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
#else
	struct stat fs;
	if (stat(name, &fs) == 0)
	{
		return S_ISDIR(fs.st_mode);
	}
#endif
*/




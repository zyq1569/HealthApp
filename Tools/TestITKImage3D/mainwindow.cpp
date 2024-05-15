#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "transferfunctionio.h"
#include "renderingstyle.h"


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

#include <vtkDICOMImageReader.h>
#include <vtkCamera.h>
#include <vtkActor.h>
#include <vtkVolumeProperty.h>
#include <vtkPolyDataNormals.h>
#include <vtkImageShiftScale.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageCast.h>
#include <vtkLODProp3D.h>
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
#include <vtkMetaImageWriter.h>
#include <vtkSliderRepresentation2D.h>
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

///https://shenchunxu.blog.csdn.net/article/details/53022151
// On Solaris with Sun Workshop 11, <signal.h> declares signal() but <csignal> does not
#include <signal.h>

#include <qmessagebox.h>

// Qt
#include <QAction>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QTimer>


using namespace udg;

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

	QString dir = QCoreApplication::applicationDirPath();
	ui->m_dcmDir->setText(dir+"/Dicom Data");

    m_rendererViewer=NULL;
    m_renderWindow=NULL;
    m_opacityTransform=NULL;
    m_colorTransformFunction=NULL;
    m_gradientTransform=NULL;
    m_volumeProperty=NULL;
    m_volumeMapper=NULL;
    m_volume=NULL;
    m_outlineData=NULL;
    m_mapOutline=NULL;
    m_outline=NULL;
    m_renderWindowInteractor=NULL;
    m_interactorstyle=NULL;
    m_lodProp3D=NULL;

    loadRenderingStyles();

}

MainWindow::~MainWindow()
{
    delete ui;
    free3Dviewer();
}

//https://blog.csdn.net/Ericohe/article/details/114368594

void MainWindow::on_pBDir_clicked()
{
    QString dlg;
    dlg = ui->m_dcmDir->toPlainText();
    //第三个参数 如果是"./" 代表当前应用的目录. QString()空为上次打开的目录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
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

void MainWindow::loadRenderingStyles()
{
    m_renderingStyleModel = new QStandardItemModel(this);

    // For the first version we create all this by default.
    // For each style we create an item and assign some data that will be used to apply it.
    QStandardItem *item;
    RenderingStyle renderingStyle;
    TransferFunction *transferFunction;

    item = new QStandardItem(QIcon(":/renderingstyles/spine2.png"), tr("Spine"));
    item->setToolTip("Spine");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/spine2.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/thorax1.png"), tr("Thorax"));
    item->setToolTip("Thorax");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/thorax1.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/pelvis2.png"), tr("Pelvis"));
    item->setToolTip("Pelvis");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/pelvis2.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/cow1.png"), tr("Circle of Willis"));
    item->setToolTip("Circle of Willis");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(0.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/cow1.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/carotids2.png"), tr("Carotids"));
    item->setToolTip("Carotids");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/carotids2.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/bones4.png"), tr("Bones"));
    item->setToolTip("Bones");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/bones4.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/bonehires.png"), tr("Bones 2"));
    item->setToolTip("Bones 2");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(0.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/bonehires.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/abdomenbones.png"), tr("Abdomen bones"));
    item->setToolTip("Abdomen bones");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/abdomenbones.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/abdomenrunoff1.png"), tr("Abdomen run-off"));
    item->setToolTip("Abdomen run-off");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/abdomenrunoff1.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    item = new QStandardItem(QIcon(":/renderingstyles/abdomenslab.png"), tr("Abdomen slab"));
    item->setToolTip("Abdomen slab");
    renderingStyle.setMethod(RenderingStyle::RayCasting);
    renderingStyle.setShading(true);
    renderingStyle.setAmbientCoefficient(0.1);
    renderingStyle.setDiffuseCoefficient(0.7);
    renderingStyle.setSpecularCoefficient(1.0);
    renderingStyle.setSpecularPower(64.0);
    transferFunction = TransferFunctionIO::fromXmlFile(":/renderingstyles/abdomenslab.xml");
    renderingStyle.setTransferFunction(*transferFunction);
    delete transferFunction;
    item->setData(renderingStyle.toVariant());
    m_renderingStyleModel->appendRow(item);

    //ui->m_renderingStyleListView->setSpacing(6);
    //ui->m_renderingStyleListView->setIconSize(QSize(64, 64));
    //m_renderingStyleListView->setModel(m_renderingStyleModel);
    ui->m_renderingStyleListView->setModel(m_renderingStyleModel);

    // Rendering styles
    connect(ui->m_renderingStyleListView, SIGNAL(activated(const QModelIndex&)), SLOT(applyRenderingStyle(const QModelIndex&)));
}

void MainWindow::applyRenderingStyle(const QModelIndex &index)
{
    if (m_volumeProperty)
    {
        QStandardItem *item = m_renderingStyleModel->itemFromIndex(index);
        RenderingStyle renderingStyle = RenderingStyle::fromVariant(item->data());


        m_volumeProperty->SetScalarOpacity(renderingStyle.getTransferFunction().vtkOpacityTransferFunction());
        m_volumeProperty->SetColor(renderingStyle.getTransferFunction().vtkColorTransferFunction());

        m_renderWindow->Render();
        m_renderWindow->SetWindowName("Volume-3D");

    }


/*
    switch (renderingStyle.getMethod())
    {
        case RenderingStyle::RayCasting:
            break;

        case RenderingStyle::MIP:
            break;

        case RenderingStyle::IsoSurface:
            break;
    }
*/

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
	vtkactor->GetProperty()->SetDiffuseColor(.50, .50, .50);//设置皮肤颜色；
	vtkactor->GetProperty()->SetSpecular(0.3);//反射率；
	vtkactor->GetProperty()->SetOpacity(1.0);//透明度；
	vtkactor->GetProperty()->SetSpecularPower(20);//反射光强度；
	vtkactor->GetProperty()->SetColor(0.23, 0.23, 0.23);//设置角的颜色；
	vtkactor->GetProperty()->SetRepresentationToWireframe();//线框；

	vtkOutlineFilter * outfilterline = vtkOutlineFilter::New();
	outfilterline->SetInputData(ImageVTKData);

	
	vtkPolyDataMapper *outmapper = vtkPolyDataMapper::New();
	outmapper->SetInputConnection(outfilterline->GetOutputPort());
	outfilterline->Delete();
	outfilterline = NULL;

	ImageVTKData = NULL;

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
	QString DicomDir = ui->m_dcmDir->toPlainText();
	QDir dir;
	if (!dir.exists(DicomDir))
	{
		QMessageBox::information(NULL, "Dicom3D", "No dicom files!");
		return;
	}


	vtkActor *vtkactor = vtkActor::New();
	initImage3D_ITK_VTK(vtkactor);

	vtkRenderer *vtkrenderer = vtkRenderer::New();
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


void MainWindow::on_pBITK3D_clicked()//return show3DIVTK(Input_Name);
{
	showImage3D_ITK_VTK();
}


void MainWindow::free3Dviewer()
{
    if (m_rendererViewer)
    {
        m_rendererViewer->Delete();
        m_renderWindow->Delete();
        //m_opacityTransform->Delete();
        //m_colorTransformFunction->Delete();
        m_gradientTransform->Delete();
        m_volumeProperty->Delete();
        m_volumeMapper->Delete();
        m_volume->Delete();
        m_outlineData->Delete();
        m_mapOutline->Delete();
        m_outline->Delete();
        m_renderWindowInteractor->Delete();
        m_interactorstyle->Delete();
        m_lodProp3D->Delete();


        m_rendererViewer = NULL;
        m_renderWindow = NULL;
        m_opacityTransform = NULL;
        m_colorTransformFunction = NULL;
        m_gradientTransform = NULL;
        m_volumeProperty = NULL;
        m_volumeMapper = NULL;
        m_volume = NULL;
        m_outlineData = NULL;
        m_mapOutline = NULL;
        m_outline = NULL;
        m_renderWindowInteractor = NULL;
        m_interactorstyle = NULL;
        m_lodProp3D = NULL;
    }
}

void MainWindow::on_pBVolume3D_clicked()
{
    static bool init = true;
    if (init)
    {
        init = false;
        ui->pBVolume3D->setText("Close 3DViewer");
        QString DicomDir = ui->m_dcmDir->toPlainText();
        QDir dir;
        if (!dir.exists(DicomDir))
        {
            QMessageBox::information(NULL, "Dicom3D", "No dicom files!");
            return;
        }

        std::string Input_Name = qPrintable(DicomDir);
        Input3dImageType::Pointer dicomimage = GdcmRead3dImage(Input_Name, DicomDir);
        static bool init = false;
        if (!init)
        {
            vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
            vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());
            init = true;
        }

        //定义绘制器；
        m_rendererViewer = vtkRenderer::New();//指向指针；
        m_renderWindow = vtkRenderWindow::New();
        m_renderWindow->AddRenderer(m_rendererViewer);

        //透明度映射函数定义；
        vtkPiecewiseFunction *opacityTransform = vtkPiecewiseFunction::New();
        opacityTransform->AddPoint(-1024, 0.0);
        opacityTransform->AddPoint(-24, 0.0);
        opacityTransform->AddPoint(167.00000000000000, 0.16862745098039220);
        opacityTransform->AddPoint(218.00000000000000, 0.41960784313725491);
        opacityTransform->AddPoint(218.00000000000000, 0.41960784313725491);
        opacityTransform->AddPoint(445.00000000000000, 0.57254901960784310);
        opacityTransform->AddPoint(1455.0000000000000, 0.87450980392156863);
        opacityTransform->AddPoint(2784.0000000000000, 0.88235294117647056);

        //颜色映射函数定义,梯度上升的
        vtkColorTransferFunction *colorTransformFunction = vtkColorTransferFunction::New();
        colorTransformFunction->AddRGBPoint(-1024.0, 1.0, 0.13725490196078433, 0.17254901960784313);
        colorTransformFunction->AddRGBPoint(24.0, 1.0, 0.13725490196078433, 0.17254901960784313);
        colorTransformFunction->AddRGBPoint(163.0, 1.0, 0.13725490196078433, 0.17254901960784313);
        colorTransformFunction->AddRGBPoint(167.0, 1.0, 0.35294117647058826, 0.16862745098039217);
        colorTransformFunction->AddRGBPoint(218.0, 1.0, 0.63921568627450975, 0.11372549019607843);

        colorTransformFunction->AddRGBPoint(445.0, 1.0, 1.0, 1.0);
        colorTransformFunction->AddRGBPoint(1455.0, 1.0, 1.0, 1.0);
        colorTransformFunction->AddRGBPoint(2784.0, 1.0, 1.0, 1.0);

        m_gradientTransform = vtkPiecewiseFunction::New();
        m_gradientTransform->AddPoint(1, 0.0);
        m_gradientTransform->AddPoint(70, 0.5);
        m_gradientTransform->AddPoint(130, 1.0);
        //gradientTransform->AddPoint(300, 0.1);


        //体数据属性；
        m_volumeProperty = vtkVolumeProperty::New();
        m_volumeProperty->SetColor(colorTransformFunction);
        m_volumeProperty->SetScalarOpacity(opacityTransform);
        m_volumeProperty->SetGradientOpacity(m_gradientTransform);
        m_volumeProperty->ShadeOn();//应用
        m_volumeProperty->SetInterpolationTypeToLinear();//直线间样条插值；
        m_volumeProperty->SetAmbient(0.4);//环境光系数；
        m_volumeProperty->SetDiffuse(0.69996);//漫反射；
        m_volumeProperty->SetSpecular(0.2);
        m_volumeProperty->SetSpecularPower(10);//高光强度；

        vtkSmartPointer<vtkImageData> itkImageData = ImageDataItkToVtk(dicomimage);
        /*
        vtkMetaImageWriter *vtkdatawrite = vtkMetaImageWriter::New();
        vtkdatawrite->SetInputData(itkImageData);
        std::string path = Input_Name + "/VTKdata.mhd";
        vtkdatawrite->SetFileName(path.c_str());
        path = Input_Name + "/VTKdata.raw";
        vtkdatawrite->SetRAWFileName(path.c_str());
        vtkdatawrite->Write();
        vtkdatawrite->Delete();
        */


        /*
        typedef VolumePixelData::ItkImageType ItkImageType;
        typedef itk::ImageFileReader<Volume::ItkImageType> ReaderType;
        ReaderType::Pointer reader = ReaderType::New();
        reader->SetFileName(qPrintable(fileName));
        reader->SetImageIO(m_gdcmIO);
        emit progress(0);
        try
        {
            reader->Update();
        }
        catch (const itk::ProcessAborted&)
        {
            errorCode = ReadAborted;
        }
        catch (itk::ExceptionObject &e)
        {
            WARN_LOG(QString("Exception reading the file [%1] Description: [%2]").arg(fileName).arg(e.GetDescription()));
            //DEBUG_LOG(QString("Exception reading the file [%1] Description: [%2]").arg(fileName).arg(e.GetDescription()));
            //We read the error message to find out what the error is
            errorCode = identifyErrorMessage(QString(e.GetDescription()));
        }
        */


        //光纤映射类型定义：
        //Mapper定义,
        m_volumeMapper = vtkSmartVolumeMapper::New();

        m_volumeMapper->SetInputData(itkImageData);//;cast_file->GetOutput());
        m_volumeMapper->SetBlendModeToComposite();
        m_volumeMapper->SetRequestedRenderModeToDefault();
        m_lodProp3D = vtkLODProp3D::New();
        m_lodProp3D->AddLOD(m_volumeMapper, m_volumeProperty, 0.0);


        m_volume = vtkVolume::New();
        m_volume->SetMapper(m_volumeMapper);
        m_volume->SetProperty(m_volumeProperty);//设置体属性；

        double volumeView[4] = { 0,0,0.5,1 };

        m_outlineData = vtkOutlineFilter::New();//线框；
        m_outlineData->SetInputData(itkImageData);
        m_mapOutline = vtkPolyDataMapper::New();
        m_mapOutline->SetInputConnection(m_outlineData->GetOutputPort());
        m_outline = vtkActor::New();
        m_outline->SetMapper(m_mapOutline);
        m_outline->GetProperty()->SetColor(0, 0, 0);//背景纯黑色；

        m_rendererViewer->AddVolume(m_volume);
        m_rendererViewer->AddActor(m_outline);
        m_rendererViewer->SetBackground(1, 1, 1);
        m_rendererViewer->ResetCamera();


        //重设相机的剪切范围；
        m_rendererViewer->ResetCameraClippingRange();
        m_renderWindow->SetSize(500, 500);

        m_renderWindowInteractor = vtkRenderWindowInteractor::New();
        m_renderWindowInteractor->SetRenderWindow(m_renderWindow);

        //设置相机跟踪模式
        m_interactorstyle = vtkInteractorStyleTrackballCamera::New();
        m_renderWindowInteractor->SetInteractorStyle(m_interactorstyle);

        m_renderWindow->Render();
        m_renderWindow->SetWindowName("Volume-3D");

        //RenderWindowInteractor->Initialize();
        //RenderWindowInteractor->Start();
    }
    else
    {
      init = true;
      ui->pBVolume3D->setText("Volume-3D");
      free3Dviewer();
    }

}
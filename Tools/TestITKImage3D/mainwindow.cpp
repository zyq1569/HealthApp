#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "transferfunctionio.h"
#include "renderingstyle.h"

#include <QFileDialog>
#include "Header.h"
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
//https://blog.csdn.net/liushao1031177/article/details/118419221
//https://github.com/orgs/morphomuseum/repositories
// test data https://morphomuseum.com/tutorialsMorphodig
//https://blog.csdn.net/wxflamy/article/details/86509177
//https://examples.vtk.org/site/Cxx/PolyData/ImplicitSelectionLoop/
//https://blog.csdn.net/weixin_42834523/article/details/108329303
// On Solaris with Sun Workshop 11, <signal.h> declares signal() but <csignal> does not
#include <signal.h>

#include <qmessagebox.h>

// Qt
#include <QAction>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QTimer>
#include <QBitmap>
#include <QCursor>
#include <QPixmap>
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

//QCursor g_cursor(QPixmap(":/images/cursors/contrast.svg"));
//https://www.cnblogs.com/ybqjymy/p/14244545.html
QPoint g_windowLevelStartPosition;
QPoint g_windowLevelCurrentPosition;
#include <vtkPointPicker.h>
#include "voilut.h"
#include <vtkEventQtSlotConnect.h>
#include <vtkPropPicker.h>
#include <vtkCallbackCommand.h>
#include <vtkPolygon.h>
#include <vtkPlane.h>
#include <vtkCutter.h>

#include <vtkTriangle.h>
static constexpr double MinimumWindowWidth = 0.0001;
class vtkInteractorStyleTrackballCameraWindowleve : public vtkInteractorStyleTrackballCamera
{
public:
    static vtkInteractorStyleTrackballCameraWindowleve *New()
    {
        return new vtkInteractorStyleTrackballCameraWindowleve;
    }
    void OnRightButtonDown() override
    {
        if (m_bFlagWindowLeve)
        {
            m_startWindowLeve = true;
            startWindowLevel();
        }
        vtkInteractorStyleTrackballCamera::OnRightButtonDown();
    }
    void OnRightButtonUp() override
    {
        if (m_bFlagWindowLeve)
        {
            m_startWindowLeve = false;
            endWindowLevel();
        }
        vtkInteractorStyleTrackballCamera::OnRightButtonUp();
    }
    void OnLeftButtonUp() override
    {
        if (0)
        {
            int* clickPos = this->GetInteractor()->GetEventPosition();
            vtkRenderer* renderer = this->GetCurrentRenderer();
            vtkRenderWindow* renderWindow = this->GetInteractor()->GetRenderWindow();

            // 获取鼠标点击位置的世界坐标
            double worldPos[4];
            renderer->SetDisplayPoint(clickPos[0], clickPos[1], 0);
            renderer->DisplayToWorld();
            renderer->GetWorldPoint(worldPos);

            if (this->PointCount < 3)
            {
                this->Points->InsertNextPoint(worldPos[0], worldPos[1], worldPos[2]);
                this->PointCount++;

                if (this->PointCount == 3)
                {
                    vtkNew<vtkTriangle> triangle;
                    triangle->GetPointIds()->SetId(0, 0);
                    triangle->GetPointIds()->SetId(1, 1);
                    triangle->GetPointIds()->SetId(2, 2);
                    this->Triangles->InsertNextCell(triangle);

                    renderer->AddActor(this->TriangleActor);
                }

                this->PolyData->Modified();
                renderWindow->Render();
            }
        }
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }
    void OnMouseMove() override
    {
        if (m_bFlagWindowLeve && m_startWindowLeve)
        {
            doWindowLevel();          
        }
        else
        {
            vtkInteractorStyleTrackballCamera::OnMouseMove();
        }
    }
    void OnMiddleButtonUp() override
    {
        m_bFlagWindowLeve = !m_bFlagWindowLeve;
        vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();
    }

    void startWindowLevel()
    {
        VoiLut voiLut;
        voiLut                = m_initialLut;
        m_initialWindow       = voiLut.getWindowLevel().getWidth();
        m_initialLevel        = voiLut.getWindowLevel().getCenter();
        m_initialLut          = voiLut.getLut();
        m_initialLut.setName(voiLut.getOriginalLutExplanation());
        int *vtkpoint         = m_vtkRenderWindow->GetInteractor()->GetEventPosition();
        m_windowLevelStartPosition = QPoint(vtkpoint[0], vtkpoint[1]);
    }
    void endWindowLevel()
    {
        m_vtkRenderWindow->Render();
    }
    void doWindowLevel()
    {
        if (m_vtkRenderWindow)
        {
            //m_vtkRenderWindow->SetCurrentCursor(g_cursor);
            //m_viewer->setCursor(QCursor(QPixmap(":/images/cursors/contrast.svg")));
            int *vtkpoint = m_vtkRenderWindow->GetInteractor()->GetEventPosition();
            m_windowLevelCurrentPosition = QPoint(vtkpoint[0], vtkpoint[1]);
            int *vtksize = m_vtkRenderWindow->GetSize();
            QSize size = QSize(vtksize[0], vtksize[1]);

            double dx = 4.0 * (m_windowLevelCurrentPosition.x() - m_windowLevelStartPosition.x()) / size.width();
            double dy = 4.0 * (m_windowLevelStartPosition.y() - m_windowLevelCurrentPosition.y()) / size.height();

            double initialWindowWidth = std::max(std::abs(m_initialWindow), MinimumWindowWidth);

            dx *= initialWindowWidth;
            dy *= initialWindowWidth;

            double newWindow = dx + m_initialWindow;

            if (newWindow > -MinimumWindowWidth && newWindow < MinimumWindowWidth)
            {
                newWindow = std::copysign(MinimumWindowWidth, newWindow);
            }
            double newLevel = m_initialLevel - dy;

            VoiLut voiLut;
            double oldX1 = m_initialLut.keys().first();
            double oldX2 = m_initialLut.keys().last();
            double newX1 = newLevel - newWindow / 2.0;
            double newX2 = newLevel + newWindow / 2.0;
            voiLut = VoiLut(m_initialLut.toNewRange(oldX1, oldX2, newX1, newX2), m_initialLut.name());
            voiLut.setExplanation("Custom");
            m_initialLut = voiLut.getLut();
            m_volumeProperty->SetScalarOpacity(voiLut.getLut().vtkOpacityTransferFunction());
            m_volumeProperty->SetColor(voiLut.getLut().vtkColorTransferFunction());
            m_vtkRenderWindow->Render();
        }
 
    }
    void setWindowLeve(bool b)
    {
        m_bFlagWindowLeve = b;
        m_startWindowLeve = false;
    }

    virtual void OnKeyPress() override
    {
        vtkRenderWindowInteractor *interactor = this->GetInteractor();
        int keyCode = m_vtkRenderWindow->GetInteractor()->GetKeyCode();
        if (keyCode == 27)//ESC
        {
            //MessageBox(NULL, "keycode 27", "Error", MB_OK);
            m_volumeProperty->SetScalarOpacity(m_oldTransferFunction.vtkOpacityTransferFunction());
            m_volumeProperty->SetColor(m_oldTransferFunction.vtkColorTransferFunction());
            m_vtkRenderWindow->Render();
        }
        /*
            std::string key = interactor->GetKeySym();
            // 打印按下的键
            // 处理特定按键
            if (key == "Up")
            {
                std::cout << "Up arrow key pressed." << std::endl;            
            }
            else if (key == "Down")
            {
                std::cout << "Down arrow key pressed." << std::endl;
            }        
        */
        // 调用父类的 OnKeyPress 方法
        vtkInteractorStyleTrackballCamera::OnKeyPress();
    }

private:
    bool m_bFlagWindowLeve;
    TransferFunction m_initialLut,m_oldTransferFunction;
    QPoint m_windowLevelStartPosition;
    QPoint m_windowLevelCurrentPosition;
    double m_initialWindow, m_initialLevel, m_currentWindow, m_currentLevel;
    vtkRenderWindow   *m_vtkRenderWindow;
    vtkVolumeProperty *m_volumeProperty;
    MainWindow        *m_MainWindow;
    bool m_startWindowLeve;
    vtkEventQtSlotConnect *m_vtkQtConnections;
    HCURSOR m_hCursor;
    void *m_void;
 public:
     void init()
     {
         m_vtkRenderWindow = NULL;
         m_volumeProperty = NULL;
         m_MainWindow = NULL;
         m_startWindowLeve = false;
         m_vtkQtConnections = NULL;
         m_void = NULL;
     }
     void setMainwindowsHand(MainWindow *m_main)
     {
         m_MainWindow = m_main;
     }
     void setHCURSOR(HCURSOR hcursor)
     {
         m_hCursor = hcursor;
     }
     void setMainwindowsVTKParms(vtkVolumeProperty* vp, vtkRenderWindow* vr, TransferFunction tr, vtkDataObject *vtkdata)
     {
         m_vtkRenderWindow = vr;
         m_volumeProperty  = vp;
         m_initialLut      = tr;

         //m_vtkQtConnections = vtkEventQtSlotConnect::New();
         //m_vtkQtConnections->Connect(m_vtkRenderWindow->GetInteractor(), vtkCommand::AnyEvent, m_MainWindow, SLOT(eventHandler(vtkObject*, unsigned long, void*, void*, vtkCommand*)));
     }

     void setSaveTransferFunction(TransferFunction tr)
     {
         VoiLut voiLut;
         voiLut = tr;
         m_oldTransferFunction = voiLut.getLut();;
         m_oldTransferFunction.setName(voiLut.getOriginalLutExplanation());
     }
     void setVoid(void * p)
     {
         m_void = p;
        
     }
protected:
    vtkInteractorStyleTrackballCameraWindowleve() : Points(vtkSmartPointer<vtkPoints>::New()),
        Triangles(vtkSmartPointer<vtkCellArray>::New()),
        PolyData(vtkSmartPointer<vtkPolyData>::New()),
        TriangleMapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
        TriangleActor(vtkSmartPointer<vtkActor>::New()),
        PointCount(0)
    {
        this->PolyData->SetPoints(this->Points);
        this->PolyData->SetPolys(this->Triangles);
        this->TriangleMapper->SetInputData(this->PolyData);
        this->TriangleActor->SetMapper(this->TriangleMapper);
        this->TriangleActor->GetProperty()->SetEdgeColor(0.0, 1.0, 0.0); // 绿色
        this->TriangleActor->GetProperty()->EdgeVisibilityOn();

        init();
    }
    ~vtkInteractorStyleTrackballCameraWindowleve() //override
    {

    }

private:
    vtkInteractorStyleTrackballCameraWindowleve(const vtkInteractorStyleTrackballCameraWindowleve&) = delete;
    void operator=(const vtkInteractorStyleTrackballCameraWindowleve&) = delete;
private:
    ////
    vtkSmartPointer<vtkPoints> Points;
    vtkSmartPointer<vtkCellArray> Triangles;
    vtkSmartPointer<vtkPolyData> PolyData;
    vtkSmartPointer<vtkPolyDataMapper> TriangleMapper;
    vtkSmartPointer<vtkActor> TriangleActor;
    int PointCount;
   };
//#include <QCursor>
HCURSOR MainWindow::QCursorToHCursor(const QCursor &qCursor)
{
    QPixmap pixmap = qCursor.pixmap();

    // 获取光标位图和蒙版数据
    QBitmap mask = pixmap.mask();
    QSize size = pixmap.size();
    int width = size.width();
    int height = size.height();

    QImage img = pixmap.toImage();
    QImage maskImg = mask.toImage();

    // 创建DIB位图
    HBITMAP hBitmapColor = CreateBitmap(width, height, 1, 32, img.bits());
    HBITMAP hBitmapMask = CreateBitmap(width, height, 1, 1, maskImg.bits());

    ICONINFO iconinfo;
    iconinfo.fIcon = FALSE;
    iconinfo.xHotspot = qCursor.hotSpot().x();
    iconinfo.yHotspot = qCursor.hotSpot().y();
    iconinfo.hbmMask = hBitmapMask;
    iconinfo.hbmColor = hBitmapColor;

    // 创建HCURSOR
    HCURSOR hCursor = CreateIconIndirect(&iconinfo);

    // 释放资源
    DeleteObject(hBitmapColor);
    DeleteObject(hBitmapMask);

    return hCursor;
}
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

    //m_vtkWidget = new QVTKOpenGLNativeWidget(this);

    QCursor g_cursor(QPixmap(":/images/cursors/contrast.svg"));
    //setCursor(g_cursor);
    loadRenderingStyles();
    ui->m_labeltip->setStyleSheet("color:red");

    //透明度映射函数定义；   
    m_transferFunction.setScalarOpacity(-1024, 0.0);
    m_transferFunction.setScalarOpacity(-24, 0.0);
    m_transferFunction.setScalarOpacity(167.00000000000000, 0.16862745098039220);
    m_transferFunction.setScalarOpacity(218.00000000000000, 0.41960784313725491);
    m_transferFunction.setScalarOpacity(218.00000000000000, 0.41960784313725491);
    m_transferFunction.setScalarOpacity(445.00000000000000, 0.57254901960784310);
    m_transferFunction.setScalarOpacity(1455.0000000000000, 0.87450980392156863);
    m_transferFunction.setScalarOpacity(2784.0000000000000, 0.88235294117647056);
    //颜色映射函数定义,梯度上升的

    m_transferFunction.setColor(-1024.0, 1.0, 0.13725490196078433, 0.17254901960784313);
    m_transferFunction.setColor(24.0, 1.0, 0.13725490196078433, 0.17254901960784313);
    m_transferFunction.setColor(163.0, 1.0, 0.13725490196078433, 0.17254901960784313);
    m_transferFunction.setColor(167.0, 1.0, 0.35294117647058826, 0.16862745098039217);
    m_transferFunction.setColor(218.0, 1.0, 0.63921568627450975, 0.11372549019607843);
    m_transferFunction.setColor(445.0, 1.0, 1.0, 1.0);
    m_transferFunction.setColor(1455.0, 1.0, 1.0, 1.0);
    m_transferFunction.setColor(2784.0, 1.0, 1.0, 1.0);

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
        m_transferFunction = renderingStyle.getTransferFunction();
        m_volumeProperty->SetScalarOpacity(m_transferFunction.vtkOpacityTransferFunction());
        m_volumeProperty->SetColor(m_transferFunction.vtkColorTransferFunction());

        m_volumeProperty->SetAmbient(0.1);
        m_volumeProperty->SetDiffuse(0.7);

        m_volumeProperty->SetSpecular(renderingStyle.getSpecularCoefficient());

        m_volumeProperty->SetSpecularPower(64.0);

        m_renderWindow->Render();
        m_renderWindow->SetWindowName("鼠标左键旋转 右键:WW/WL(ESC还原）|(鼠标中间单击切换)Zoom 鼠标中键移动图像");
        m_interactorstyle->setMainwindowsVTKParms( m_volumeProperty, m_renderWindow, m_transferFunction, 0);
        m_interactorstyle->setSaveTransferFunction(m_transferFunction);

    }

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
        //m_gradientTransform->Delete();
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
        //m_opacityTransform = NULL;
        //m_colorTransformFunction = NULL;
        //m_gradientTransform = NULL;
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
    QList<void*> list;
    //return test();
    //return testAll(0, list);
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
        static bool init = true ;
        if (init)
        {
            vtkObjectFactory::RegisterFactory(vtkRenderingOpenGL2ObjectFactory::New());
            vtkObjectFactory::RegisterFactory(vtkRenderingVolumeOpenGL2ObjectFactory::New());
            init = false;
        }
        
        vtkSmartPointer<vtkImageData> itkImageData = ImageDataItkToVtk(dicomimage);

        //定义绘制器；
        m_rendererViewer = vtkRenderer::New();//指向指针；
        m_renderWindow = vtkRenderWindow::New(); 
        m_renderWindow->AddRenderer(m_rendererViewer);

        //体数据属性；
        m_volumeProperty = vtkVolumeProperty::New();      
        m_volumeProperty->SetScalarOpacity(m_transferFunction.vtkOpacityTransferFunction());
        m_volumeProperty->SetColor(m_transferFunction.vtkColorTransferFunction());
        //m_volumeProperty->SetGradientOpacity(m_gradientTransform);
        m_volumeProperty->SetIndependentComponents(true);
        m_volumeProperty->ShadeOn();//应用
        m_volumeProperty->SetInterpolationTypeToLinear();//直线间样条插值；
        m_volumeProperty->SetAmbient(0.4);//环境光系数；
        m_volumeProperty->SetDiffuse(0.69996);//漫反射；
        m_volumeProperty->SetSpecular(0.2);
        m_volumeProperty->SetSpecularPower(10);//高光强度；
        /**/
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
        m_renderWindow->SetSize(600, 600);

        m_renderWindowInteractor = vtkRenderWindowInteractor::New();
        m_renderWindowInteractor->SetRenderWindow(m_renderWindow);

        //设置相机跟踪模式
        m_interactorstyle = vtkInteractorStyleTrackballCameraWindowleve::New();
        m_interactorstyle->init();
        m_interactorstyle->setWindowLeve(true);
        m_interactorstyle->setMainwindowsVTKParms(m_volumeProperty, m_renderWindow, m_transferFunction, itkImageData);
        m_interactorstyle->setSaveTransferFunction(m_transferFunction);
    
        m_renderWindowInteractor->SetInteractorStyle(m_interactorstyle);

        m_interactorstyle->SetCurrentRenderer(m_rendererViewer);

        m_renderWindow->Render();
        m_renderWindow->SetWindowName("鼠标左键旋转 右键:WW/WL(ESC还原）|(鼠标中间单击切换)Zoom 鼠标中键移动图像");
    }
    else
    {
          init = true;
          ui->pBVolume3D->setText("Volume-3D");
          free3Dviewer();
    }

}

void MainWindow::eventHandler(vtkObject *object, unsigned long vtkEvent, void *clientData, void *callData, vtkCommand *command)
{
    Q_UNUSED(object);
    Q_UNUSED(clientData);
    Q_UNUSED(callData);
    Q_UNUSED(command);
    if (vtkEvent == vtkCommand::MouseWheelForwardEvent || vtkEvent == vtkCommand::MouseWheelBackwardEvent)
    {
        QWheelEvent *event = (QWheelEvent*)callData; //WARNING: I don't like that casting here, may become dangerous.
        if (event)
        {
        }
    }

    switch (vtkEvent)
    {
        case vtkCommand::LeftButtonPressEvent:
        {

        }
        //case QVTKInteractor::vtkCustomEvents::ContextMenuEvent:
        //case vtkCommand::RightButtonPressEvent:
        //case vtkCommand::MiddleButtonPressEvent:
        //case vtkCommand::MouseWheelForwardEvent:
        //case vtkCommand::MouseWheelBackwardEvent:
        case vtkCommand::KeyPressEvent:
        {
            int keyCode = m_renderWindowInteractor->GetKeyCode();
            if (keyCode == 27) // ESC
            {
            }
        }
            break;

        case vtkCommand::MouseMoveEvent:
            break;
        case vtkCommand::RightButtonReleaseEvent:
            break;
    }
}

void MainWindow::saveHDMdata(vtkImageData * itkImageData, bool read)
{
    QString DicomDir = ui->m_dcmDir->toPlainText();
    QDir dir;
    if (!dir.exists(DicomDir))
    {
        QMessageBox::information(NULL, "Dicom3D", "No dicom files!");
        return;
    }
    std::string Input_Name = qPrintable(DicomDir);
    std::string path = Input_Name + "/VTKdata.mhd";
    if (read)
    {
        vtkMetaImageWriter *vtkdatawrite = vtkMetaImageWriter::New();
        vtkdatawrite->SetInputData(itkImageData);
        vtkdatawrite->SetFileName(path.c_str());
        path = Input_Name + "/VTKdata.raw";
        vtkdatawrite->SetRAWFileName(path.c_str());
        vtkdatawrite->Write();
        vtkdatawrite->Delete();
    }
    else
    {
        vtkMetaImageReader *vtkdataread = vtkMetaImageReader::New();
        vtkdataread->SetFileName(path.c_str());
        vtkdataread->Update();
        itkImageData = vtkdataread->GetOutput();
    }
}
void MainWindow::testAll(int n, QList<void *> list)
{
    vtkNew<vtkNamedColors> colors;

    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetPhiResolution(100);
    sphereSource->SetThetaResolution(100);
    sphereSource->Update();

    vtkNew<vtkPoints> selectionPoints;

    selectionPoints->InsertPoint(0, -0.16553, 0.135971, 0.451972);
    selectionPoints->InsertPoint(1, -0.0880123, -0.134952, 0.4747);
    selectionPoints->InsertPoint(2, 0.00292618, -0.134604, 0.482459);
    selectionPoints->InsertPoint(3, 0.0641941, 0.067112, 0.490947);
    selectionPoints->InsertPoint(4, 0.15577, 0.0734765, 0.469245);
    selectionPoints->InsertPoint(5, 0.166667, -0.129217, 0.454622);
    selectionPoints->InsertPoint(6, 0.241259, -0.123363, 0.420581);
    selectionPoints->InsertPoint(7, 0.240334, 0.0727106, 0.432555);
    selectionPoints->InsertPoint(8, 0.308529, 0.0844311, 0.384357);
    selectionPoints->InsertPoint(9, 0.32672, -0.121674, 0.359187);
    selectionPoints->InsertPoint(10, 0.380721, -0.117342, 0.302527);
    selectionPoints->InsertPoint(11, 0.387804, 0.0455074, 0.312375);
    selectionPoints->InsertPoint(12, 0.43943, -0.111673, 0.211707);
    selectionPoints->InsertPoint(13, 0.470984, -0.0801913, 0.147919);
    selectionPoints->InsertPoint(14, 0.436777, 0.0688872, 0.233021);
    selectionPoints->InsertPoint(15, 0.44874, 0.188852, 0.109882);
    selectionPoints->InsertPoint(16, 0.391352, 0.254285, 0.176943);
    selectionPoints->InsertPoint(17, 0.373274, 0.154162, 0.294296);
    selectionPoints->InsertPoint(18, 0.274659, 0.311654, 0.276609);
    selectionPoints->InsertPoint(19, 0.206068, 0.31396, 0.329702);
    selectionPoints->InsertPoint(20, 0.263789, 0.174982, 0.387308);

    selectionPoints->InsertPoint(21, 0.213034, 0.175485, 0.417142);
    selectionPoints->InsertPoint(22, 0.169113, 0.261974, 0.390286);
    selectionPoints->InsertPoint(23, 0.102552, 0.25997, 0.414814);
    selectionPoints->InsertPoint(24, 0.131512, 0.161254, 0.454705);
    selectionPoints->InsertPoint(25, 0.000192443, 0.156264, 0.475307);
    selectionPoints->InsertPoint(26, -0.0392091, 0.000251724, 0.499943);
    selectionPoints->InsertPoint(27, -0.096161, 0.159646, 0.46438);

    vtkNew<vtkImplicitSelectionLoop> loop;
    loop->SetLoop(selectionPoints);

    vtkNew<vtkClipPolyData> clip;
    clip->SetInputConnection(sphereSource->GetOutputPort());
    clip->SetClipFunction(loop);
    clip->SetValue(0.0);

    vtkNew<vtkPolyDataMapper> clipMapper;
    clipMapper->SetInputConnection(clip->GetOutputPort());
    clipMapper->ScalarVisibilityOff();

    vtkNew<vtkProperty> backProp;
    backProp->SetColor(colors->GetColor3d("Tomato").GetData());

    vtkNew<vtkLODActor> clipActor;
    clipActor->SetMapper(clipMapper);
    clipActor->SetBackfaceProperty(backProp);
    clipActor->GetProperty()->SetColor(colors->GetColor3d("Banana").GetData());

    vtkNew<vtkRenderer> renderer;

    vtkNew<vtkRenderWindow> renderWindow;
    //vtkRenderWindow *renderWindow = vtkRenderWindow::New();
    renderWindow->AddRenderer(renderer);

    vtkNew<vtkRenderWindowInteractor> interactor;
    interactor->SetRenderWindow(renderWindow);

    //vtkInteractorStyleTrackballCamera *interactorstyle = vtkInteractorStyleTrackballCamera::New();
    vtkNew< vtkInteractorStyleTrackballCameraWindowleve> interactorstyle;
    interactor->SetInteractorStyle(interactorstyle);
    interactorstyle->setVoid((void*)loop);
    interactorstyle->SetCurrentRenderer(renderer);

    // Add the actors to the renderer, set the background and size
    renderer->AddActor(clipActor);
    renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

    renderWindow->SetSize(500, 500);
    renderWindow->SetWindowName("ImplicitSelectionLoop");

    renderWindow->Render();
    interactor->Start();
}
void MainWindow::on_pBRemoveBed_clicked()
{
    if (m_volumeMapper && m_renderWindow)
    {
        vtkImageData* imageData = m_volumeMapper->GetInput();
        if (m_removeBed != imageData)
        {   
            m_removeBed == imageData;
            vtkImageThreshold* threshold = vtkImageThreshold::New();
            threshold->SetInputData(imageData);
            threshold->ThresholdBetween(-700, 700); // Adjust the threshold value based on your data
            threshold->ReplaceInOn();
            threshold->SetInValue(-100); // Set the value of the bed pixels to 0
            threshold->Update();
            imageData->DeepCopy(threshold->GetOutput());
            m_renderWindow->Render();
            threshold->Delete();
        }
    }

}

//void RemoveBed(vtkSmartPointer<vtkImageData> imageData);
//void RemoveBed(vtkSmartPointer<vtkImageData> imageData)
//{
//    vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
//    threshold->SetInputData(imageData);
//    threshold->ThresholdByLower(400); // Adjust the threshold value based on your data
//    threshold->ReplaceInOn();
//    threshold->SetInValue(0); // Set the value of the bed pixels to 0
//    threshold->Update();
//    imageData->DeepCopy(threshold->GetOutput());
//}

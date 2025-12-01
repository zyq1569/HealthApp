#include "mainwindow.h"

#include <Windows.h>
#include <QApplication>
#include <QFileInfo>

RECT GetMonitorRectBounds(int i)
{
    DISPLAY_DEVICEW d;
    d.cb = sizeof(d);
    bool flag = EnumDisplayDevicesW(nullptr, i, &d, 0);
    DEVMODEW dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    flag = EnumDisplaySettingsExW(d.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0);
    RECT rect = { dm.dmPosition.x,dm.dmPosition.y, dm.dmPosition.x + dm.dmPelsWidth,dm.dmPosition.y + dm.dmPelsHeight };
    return rect;
}

/*
#include <vtkMetaImageReader.h>
#include <vtkImageReslice.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkBMPWriter.h>
#include <vtkImageCast.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkCamera.h>
#include <vtkImageResample.h>
#include <vtkImageShiftScale.h>
void saveSlice(vtkSmartPointer<vtkImageReslice> reslice, const QSize& sz, const QString& strPath);
int TestExtract2DImageWithoutUI(const QString& strMhdPath, const QString& strAxialPath, const QString& strCoronalPath, const QString& strSagittalPath);
void saveSlice(vtkSmartPointer<vtkImageReslice> reslice, const QSize& sz, const QString& strPath)
{
    vtkSmartPointer<vtkLookupTable> colorTable = vtkSmartPointer<vtkLookupTable>::New();
    colorTable->SetRange(0, 2048);
    colorTable->SetValueRange(0.0, 1.0);
    colorTable->SetSaturationRange(0.0, 0.0);
    colorTable->SetRampToLinear();
    colorTable->Build();

    vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
    colorMap->SetLookupTable(colorTable);
    colorMap->SetInputConnection(reslice->GetOutputPort());
    colorMap->Update();

    // 3. 创建演员和渲染器
    vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInputData(colorMap->GetOutput());

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->ResetCamera();

    // 调整相机位置确保没有四周填充
    double bounds[6];
    actor->GetBounds(bounds);
    double x_range = bounds[1] - bounds[0];
    double y_range = bounds[3] - bounds[2];
    double dataAspect = x_range / y_range;
    double windowAspect = static_cast<double>(sz.width()) / sz.height();

    vtkCamera* camera = renderer->GetActiveCamera();
    camera->ParallelProjectionOn();
    if (windowAspect > dataAspect) // 窗口比数据宽，以高度为基准
    {
        camera->SetParallelScale(y_range / 2.0);
    }
    else // 窗口比数据高，以宽度为基准并考虑宽高比
    {
        camera->SetParallelScale(x_range / windowAspect / 2.0);
    }
    //camera->SetRoll(180);

    // 4. 创建离屏渲染窗口
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetOffScreenRendering(1); // 关键步骤：启用离屏渲染
    renderWindow->SetSize(sz.width(), sz.height()); // 设置输出图像尺寸
    renderWindow->Render(); // 执行渲染

    // 5. 捕获窗口内容并保存为图片
    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(renderWindow);
    // windowToImageFilter->SetInputBufferTypeToRGBA(); // 捕获带透明度的RGB图像
    windowToImageFilter->Update();

    vtkSmartPointer<vtkBMPWriter> writer = vtkSmartPointer<vtkBMPWriter>::New();
    QByteArray byteArray = strPath.toLocal8Bit();
    writer->SetFileName(std::string(byteArray).c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();

    ///+++++++
    double* spacing = reslice->GetOutput()->GetSpacing();
    vtkImageData* data = reslice->GetOutput();
    vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
    resample->SetInputData(data);
    resample->SetOutputSpacing(spacing);
    resample->Update();


    double ww(1528), wl(862);
    //ww = rep->GetWindow();
    //wl = rep->GetLevel();
    auto shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
    shiftScale->SetInputData(resample->GetOutput());
    shiftScale->SetShift(-1.0 * (wl - ww / 2.0));
    shiftScale->SetScale(255.0 / ww);
    shiftScale->SetOutputScalarTypeToUnsignedChar();
    shiftScale->ClampOverflowOn();

    auto lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetRange(0, 255);
    lookupTable->SetValueRange(0.0, 1.0);
    lookupTable->SetSaturationRange(0.0, 0.0);  // 灰度
    lookupTable->SetRampToSCurve();
    lookupTable->Build();

    auto imageMapToColor = vtkSmartPointer<vtkImageMapToColors>::New();
    imageMapToColor->SetInputConnection(shiftScale->GetOutputPort());
    imageMapToColor->SetLookupTable(lookupTable);
    imageMapToColor->Update();
    vtkSmartPointer<vtkBMPWriter>  imagewriter = vtkSmartPointer<vtkBMPWriter>::New();
    std::string str = qPrintable(strPath+".bmp");
    imagewriter->SetFileName(str.c_str());
    imagewriter->SetInputConnection(imageMapToColor->GetOutputPort());
    imagewriter->Update();
    imagewriter->Write();


}
int TestExtract2DImageWithoutUI(const QString& strMhdPath, const QString& strAxialPath, const QString& strCoronalPath, const QString& strSagittalPath)
{
    QByteArray byteArray = strMhdPath.toLocal8Bit();

    vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
    reader->SetFileName(std::string(byteArray).c_str());
    reader->Update();

    vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputConnection(reader->GetOutputPort());
    reslice->SetOutputDimensionality(2); // 输出二维图像

    // int extent[6];
    // reader->GetOutput()->GetExtent(extent);
    //double spacing[3];
    //reader->GetOutput()->GetSpacing(spacing);
    // double origin[3];
    // reader->GetOutput()->GetOrigin(origin);
    double center[3];
    reader->GetOutput()->GetCenter(center);
    int dims[3];
    reader->GetOutput()->GetDimensions(dims);

    // 提取横截面
    vtkSmartPointer<vtkMatrix4x4> axialMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    axialMatrix->Identity();
    axialMatrix->SetElement(0, 3, center[0]); // X 方向平移到中心
    axialMatrix->SetElement(1, 3, center[1]); // Y 方向平移到中心
    axialMatrix->SetElement(2, 3, center[2]); // Z 方向平移到中心
    reslice->SetResliceAxes(axialMatrix);
    reslice->Update();
    saveSlice(reslice, QSize(dims[0], dims[1]), strAxialPath);

    // 2. 提取冠状面
    vtkSmartPointer<vtkMatrix4x4> coronalMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    coronalMatrix->Zero();
    coronalMatrix->SetElement(0, 0, 1);
    coronalMatrix->SetElement(1, 2, 1);
    coronalMatrix->SetElement(2, 1, 1);
    coronalMatrix->SetElement(3, 3, 1);
    coronalMatrix->SetElement(0, 3, center[0]); // X 方向平移到中心
    coronalMatrix->SetElement(1, 3, center[1]); // Y 方向平移到中心
    coronalMatrix->SetElement(2, 3, center[2]); // Z 方向平移到中心
    reslice->SetResliceAxes(coronalMatrix);
    reslice->Update();
    saveSlice(reslice, QSize(dims[1], dims[2]), strCoronalPath);

    // 3. 提取矢状面
    vtkSmartPointer<vtkMatrix4x4> sagittalMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    sagittalMatrix->Zero();
    sagittalMatrix->SetElement(0, 2, 1);
    sagittalMatrix->SetElement(1, 1, 1);
    sagittalMatrix->SetElement(2, 0, -1);//1---->-1 修正镜像方向
    sagittalMatrix->SetElement(3, 3, 1);

    sagittalMatrix->SetElement(0, 3, center[0]); // X 方向平移到中心
    sagittalMatrix->SetElement(1, 3, center[1]); // Y 方向平移到中心
    sagittalMatrix->SetElement(2, 3, center[2]); // Z 方向平移到中心
    reslice->SetResliceAxes(sagittalMatrix);
    reslice->Update();
    saveSlice(reslice, QSize(dims[2], dims[1]), strSagittalPath);

    return 0;
}
*/
int main(int argc, char *argv[])
{
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
    if (argc = 2)
    {
        QString dir = argv[1];
        QFileInfo finfo(dir);
        if (finfo.exists())
        {
            QString strMhdPath = dir;
            QString strAxialPath(strMhdPath +"_pXY.bmp");
            QString strCoronalPath(strMhdPath + "_pXZ.bmp");
            QString strSagittalPath(strMhdPath + "_pYZ.bmp");
            TestExtract2DImageWithoutUI(strMhdPath, strAxialPath, strCoronalPath, strSagittalPath);
        }
    }    
    */
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    int monitorCount = ::GetSystemMetrics(SM_CMONITORS);
    QString scaleFactor;
    for (int i = 0; i < monitorCount; i++)
    {
        RECT dims = GetMonitorRectBounds(i);
        int h = dims.bottom - dims.top;
        double scale = (h > 1080) ? ((double)h / 1080.0) : 1.0;
        scaleFactor += (i == 0 ? "" : ";") + QString::number(scale, 'f', 1);
    }

    QByteArray env = scaleFactor.toUtf8();
    qputenv("QT_SCREEN_SCALE_FACTORS", env);

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication a(argc, argv);
	MainWindow w;
    if (argc == 3)
    {
        QString filepath = argv[1];
        QFileInfo finfo(filepath);
        if (!finfo.exists())
        {
            w.show();
        }
        else
        {
            w.showMinimized();
            w.miniTrayIcon();
            w.setWindowFlags(Qt::Tool);
            w.hide();
            w.show();
            w.m_commdFilePath        = filepath;
            w.m_commdParmIniFilePath = argv[2];
            w.autoRun();
        }
    }
    else if (argc > 1)
	{
		QString filepath = argv[1];
		QFileInfo finfo(filepath);
		if (!finfo.exists())
		{
			w.show();
		}
		else
		{
            
			w.m_commdFilePath = filepath;
			w.starViewer();          
		}
	}
	else
	{
		w.show();
	}

	return a.exec();
}

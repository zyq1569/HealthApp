#include "q3dviewer.h"
#include "ui_q3dviewer.h"

#include "mainwindow.h"

#include <vtkImageData.h>
#include <vtkActor.h>
#include <vtkImageMarchingCubes.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <vtkMetaImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <vtkRayCastImageDisplayHelper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

Q3dviewer::Q3dviewer(QWidget *parent, vtkMetaImageReader* metaReader) :
    QWidget(parent), m_MetaReader(metaReader), ui(new Ui::Q3dviewer)
{
    ui->setupUi(this);

	m_pieceF     = nullptr;
	m_pieceGradF = nullptr;
	m_colorTranF = nullptr;
	m_mainwindow = (MainWindow*)parent;

	m_volumeMapper     = vtkSmartVolumeMapper::New();

	m_isosurfaceFilter = vtkImageMarchingCubes::New();

	m_volumeProperty   = vtkVolumeProperty::New();
	

	m_isosurfaceActor  = vtkActor::New();

	m_renderer  = vtkRenderer::New();

	m_vtkVolume = vtkVolume::New();
	m_vtkVolume->SetProperty(m_volumeProperty);
	m_vtkVolume->SetMapper(m_volumeMapper);

	vtkNew<vtkPolyDataMapper> isosurfaceMapper;
	isosurfaceMapper->SetInputConnection(m_isosurfaceFilter->GetOutputPort());
	m_isosurfaceActor->SetMapper(isosurfaceMapper);

	m_renderer->AddViewProp(m_vtkVolume);

	INimage3D();
}

Q3dviewer::~Q3dviewer()
{
	m_volumeMapper->Delete();
	m_volumeProperty->Delete();
	m_vtkVolume->Delete();

	m_isosurfaceFilter->Delete();
	m_isosurfaceActor->Delete();
	m_renderer->Delete();

	m_pieceF ? m_pieceF->Delete(): m_pieceF = nullptr;
	m_pieceF ? m_pieceGradF->Delete() : m_pieceF = nullptr;
	m_pieceF ? m_colorTranF->Delete() : m_pieceF = nullptr;

    delete ui;
}

void Q3dviewer::INimage3D()
{
	if (!m_MetaReader)
		return;

	//QString fileMhd = m_fileMHd;
	//std::string filename = qPrintable(fileMhd);
	//
	//vtkNew<vtkMetaImageReader> reader;
	//reader->SetFileName(filename.c_str());
	//reader->Update();
	int imageDims[3];


	vtkImageData *imageData = m_MetaReader->GetOutput();
	imageData->GetDimensions(imageDims);


	m_pieceF = vtkPiecewiseFunction::New();//	m_pieceF->AddPoint(20.0, 0.0);	m_pieceF->AddPoint(255, 0.0);
	//m_pieceF->AddPoint(-1024, 0.0);
	//m_pieceF->AddPoint(-24, 0.0);
	m_pieceF->AddPoint(167.00000000000000, 0.16862745098039220);
	m_pieceF->AddPoint(218.00000000000000, 0.41960784313725491);
	m_pieceF->AddPoint(218.00000000000000, 0.41960784313725491);
	m_pieceF->AddPoint(445.00000000000000, 0.57254901960784310);
	m_pieceF->AddPoint(1455.0000000000000, 0.87450980392156863);
	m_pieceF->AddPoint(2784.0000000000000, 0.88235294117647056);

	m_colorTranF = vtkColorTransferFunction::New();
	/*
	m_colorTranF->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	m_colorTranF->AddRGBPoint(64.0, 1.0, 0.0, 0.0);
	m_colorTranF->AddRGBPoint(128.0, 0.0, 0.0, 1.0);
	m_colorTranF->AddRGBPoint(192.0, 0.0, 1.0, 0.0);
	m_colorTranF->AddRGBPoint(255.0, 0.0, 0.2, 0.0);	
	*/

	m_colorTranF->AddRGBPoint(-1024.0, 1.0, 0.13725490196078433, 0.17254901960784313);
	m_colorTranF->AddRGBPoint(24.0, 1.0, 0.13725490196078433, 0.17254901960784313);
	m_colorTranF->AddRGBPoint(163.0, 1.0, 0.13725490196078433, 0.17254901960784313);
	m_colorTranF->AddRGBPoint(167.0, 1.0, 0.35294117647058826, 0.16862745098039217);
	m_colorTranF->AddRGBPoint(218.0, 1.0, 0.63921568627450975, 0.11372549019607843);
	m_colorTranF->AddRGBPoint(445.0, 1.0, 1.0, 1.0);
	m_colorTranF->AddRGBPoint(1455.0, 1.0, 1.0, 1.0);
	m_colorTranF->AddRGBPoint(2784.0, 1.0, 1.0, 1.0);

	m_pieceGradF = vtkPiecewiseFunction::New();
	m_pieceGradF->AddPoint(1,0.0);
	m_pieceGradF->AddPoint(70,0.5);
	m_pieceGradF->AddPoint(130,1.0);
	m_pieceGradF->AddPoint(300,0.1);

	if (m_mainwindow->m_check3Dcolor)
	{
		m_volumeProperty->SetColor(m_colorTranF);
	}

	m_volumeProperty->SetScalarOpacity(m_pieceF);
	m_volumeProperty->SetGradientOpacity(m_pieceGradF);
	m_volumeProperty->ShadeOn();
	m_volumeProperty->SetInterpolationTypeToLinear();
	m_volumeProperty->SetAmbient(0.4);//环境光系数
	m_volumeProperty->SetDiffuse(0.69996);//漫反射
	m_volumeProperty->SetSpecular(0.2);
	m_volumeProperty->SetSpecularPower(10);//高光强度
	


	m_volumeMapper->SetInputData(imageData);
	m_volumeMapper->SetBlendModeToComposite();
	m_volumeMapper->SetRequestedRenderModeToDefault();
	// force the mapper to compute a sample distance based on data spacing
	m_volumeMapper->SetSampleDistance(-1.0);
	m_isosurfaceFilter->SetInputData(imageData);

	m_renderer->SetBackground(0, 0, 0);
	m_renderer->ResetCamera();
	//重设相机的剪切范围；
	m_renderer->ResetCameraClippingRange();
	ui->m_3DView->renderWindow()->AddRenderer(m_renderer);
	//ui->m_3DView->renderWindow()->Set
	ui->m_3DView->show();


}

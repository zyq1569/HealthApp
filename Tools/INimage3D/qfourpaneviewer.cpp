#include "qfourpaneviewer.h"
#include "ui_qfourpaneviewer.h"

#include "mainwindow.h"
///---------------------------
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
#include <vtkCamera.h>

#include <QAction>


//-------------showVolume3D
#include <vtkSmartVolumeMapper.h>
#include <vtkRayCastImageDisplayHelper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageMarchingCubes.h>
#include <vtkVolumeProperty.h>
#include <vtkPolyDataMapper.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

//--------------


#define vtkREP vtkResliceCursorLineRepresentation 

class vtkResliceCursorCallback : public vtkCommand
{
public:
	static vtkResliceCursorCallback* New()
	{ 
		return new vtkResliceCursorCallback; 
	}

	void Execute(vtkObject* caller, unsigned long ev, void* callData) override
	{
		if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||
			ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
		{
			// Render everything
			for (int i = 0; i < 3; i++)
			{
				RCW[i]->Render();
			}
			//IPW[0]->GetInteractor()->GetRenderWindow()->Render();
			return;
		}

		//vtkImagePlaneWidget* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
		//if (ipw)
		//{
		//	double* wl = static_cast<double*>(callData);
		//
		//	if (ipw == IPW[0])
		//	{
		//		IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
		//		IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
		//	}
		//	else if (ipw == IPW[1])
		//	{
		//		IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
		//		IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
		//	}
		//	else if (ipw == IPW[2])
		//	{
		//		IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
		//		IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
		//	}
		//}

		//vtkResliceCursorWidget* rcw = dynamic_cast<vtkResliceCursorWidget*>(caller);
		//if (rcw)
		//{
		//	vtkREP* rep =	dynamic_cast<vtkREP*>(rcw->GetRepresentation());
		//	// Although the return value is not used, we keep the get calls
		//	// in case they had side-effects
		//	rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
		//	for (int i = 0; i < 3; i++)
		//	{
		//		vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(IPW[i]->GetPolyDataAlgorithm());
		//		ps->SetOrigin(RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetOrigin());
		//		ps->SetPoint1(RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint1());
		//		ps->SetPoint2(RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint2());
		//
		//		// If the reslice plane has modified, update it on the 3D widget
		//		//IPW[i]->UpdatePlacement();
		//	}
		//}

		// Render everything
		for (int i = 0; i < 3; i++)
		{
			RCW[i]->Render();
		}
		//IPW[0]->GetInteractor()->GetRenderWindow()->Render();
	}

	vtkResliceCursorCallback() {}
	//vtkImagePlaneWidget* IPW[3];
	vtkResliceCursorWidget* RCW[3];
};

//--------------

QFourpaneviewer::QFourpaneviewer(QWidget *parent, vtkMetaImageReader* metaReader) :
    QWidget(parent), m_MetaReader(metaReader), ui(new Ui::QFourpaneviewer)
{
    ui->setupUi(this);

	for (int i = 0; i < 3; i++)
	{
		m_planeWidget[i]        = nullptr;
		m_resliceImageViewer[i] = nullptr;
		m_renderWindow[i]       = nullptr;
	}
	m_2DViewRenderWindow = nullptr;
	m_resliceCallback    = nullptr;
	m_cellPicker         = nullptr;
	//
	m_ipwProp            = nullptr;
	m_ren                = nullptr;
	m_actionReset        = nullptr;
				         

	m_mainwindow         = (MainWindow*)parent;

	if (m_mainwindow->m_checkDefaultWL)
	{
		m_defaultLevel  = m_mainwindow->m_DefaultLevel;
		m_defaultWindow = m_mainwindow->m_DefaultWindow;
	}
	else
	{
		m_defaultLevel  = 862;
		m_defaultWindow = 1528;
	}


	INimage3D();

	m_actionReset = new QAction(this);
	addAction(m_actionReset);
	m_actionReset->setShortcut(tr("ctrl+s"));

	connect(m_actionReset, SIGNAL(triggered()), SLOT(ResetViewer()));//Open an existing DICOM folder


	//---------------------showVolume3D----------------------------------
	m_pieceF = nullptr;
	m_pieceGradF = nullptr;
	m_colorTranF = nullptr;
	m_mainwindow = (MainWindow*)parent;

	m_volumeMapper = vtkSmartVolumeMapper::New();

	m_isosurfaceFilter = vtkImageMarchingCubes::New();

	m_volumeProperty = vtkVolumeProperty::New();


	m_isosurfaceActor = vtkActor::New();

	m_renderer = vtkRenderer::New();

	m_vtkVolume = vtkVolume::New();
	m_vtkVolume->SetProperty(m_volumeProperty);
	m_vtkVolume->SetMapper(m_volumeMapper);

	m_isosurfaceMapper = vtkPolyDataMapper::New();// isosurfaceMapper;
	m_isosurfaceMapper->SetInputConnection(m_isosurfaceFilter->GetOutputPort());
	m_isosurfaceActor->SetMapper(m_isosurfaceMapper);

	m_renderer->AddViewProp(m_vtkVolume);

	INshowVolume3D();

	//-----------------------------------------------------------
}

void QFourpaneviewer::ResetViewer()
{
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->SetResliceMode(1);
		//m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
	    //DefaultLevel = 862		DefaultWindow = 1528
		vtkREP::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation())->SetWindowLevel(1528, 862);
	}
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->Reset();
		m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
		m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
		m_resliceImageViewer[i]->Render();
	}

}

void QFourpaneviewer::INimage3D()
{
	if (!m_MetaReader)
		return;

	int imageDims[3];
	vtkImageData *imageData = m_MetaReader->GetOutput();
	imageData->GetDimensions(imageDims);

	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i] = vtkResliceImageViewer::New();
		m_renderWindow[i]       = vtkGenericOpenGLRenderWindow::New(); //vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
		m_resliceImageViewer[i]->SetRenderWindow(m_renderWindow[i]);
	}

	ui->m_sagital2DView->setRenderWindow(m_resliceImageViewer[0]->GetRenderWindow());
	m_resliceImageViewer[0]->SetupInteractor(ui->m_sagital2DView->renderWindow()->GetInteractor());

	ui->m_coronal2DView->setRenderWindow(m_resliceImageViewer[1]->GetRenderWindow());
	m_resliceImageViewer[1]->SetupInteractor(ui->m_coronal2DView->renderWindow()->GetInteractor());

	ui->m_axial2DView->setRenderWindow(m_resliceImageViewer[2]->GetRenderWindow());
	m_resliceImageViewer[2]->SetupInteractor(ui->m_axial2DView->renderWindow()->GetInteractor());

	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkREP *rep  = vtkREP::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation());
		m_resliceImageViewer[i]->SetResliceCursor(m_resliceImageViewer[0]->GetResliceCursor());
		rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
		//-----------------------------------------------------------------------------------------------------
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->RenderLinesAsTubesOn();
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetLineWidth(2);
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetLineWidth(2);
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetLineWidth(2);
		//-------------------------------------------------------------------------------------------------------
		m_resliceImageViewer[i]->SetInputData(imageData);
		m_resliceImageViewer[i]->SetSliceOrientation(i);
		m_resliceImageViewer[i]->SetResliceModeToAxisAligned();
		//DefaultLevel = 862		DefaultWindow = 1528
		rep->SetWindowLevel(1528, 862);
	}

	//mpr2DView--->hide- to->showVolume3D
	//m_cellPicker = vtkCellPicker::New();
	//m_cellPicker->SetTolerance(0.005);

	//m_ipwProp = vtkProperty::New();
	//m_ren     = vtkRenderer::New();

	//m_2DViewRenderWindow = vtkGenericOpenGLRenderWindow::New();
	//ui->m_mpr2DView->setRenderWindow(m_2DViewRenderWindow);
	//ui->m_mpr2DView->renderWindow()->AddRenderer(m_ren);
	//vtkRenderWindowInteractor* iren = ui->m_mpr2DView->interactor();

	for (int i = 0; i < 3; i++)
	{
		m_planeWidget[i] = vtkImagePlaneWidget::New();
		//m_planeWidget[i]->SetInteractor(iren);
		//m_planeWidget[i]->SetPicker(m_cellPicker);
		m_planeWidget[i]->RestrictPlaneToVolumeOn();
		double color[3] = { 0, 0, 0 };
		color[i] = 1;
		m_planeWidget[i]->GetPlaneProperty()->SetColor(color);

		color[0] /= 4.0;
		color[1] /= 4.0;
		color[2] /= 4.0;
		m_resliceImageViewer[i]->GetRenderer()->SetBackground(color);

		//m_planeWidget[i]->SetTexturePlaneProperty(m_ipwProp);
		m_planeWidget[i]->TextureInterpolateOff();
		m_planeWidget[i]->SetResliceInterpolateToLinear();
		m_planeWidget[i]->SetInputConnection(m_MetaReader->GetOutputPort());
		m_planeWidget[i]->SetPlaneOrientation(i);
		m_planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
		m_planeWidget[i]->DisplayTextOn();
		//m_planeWidget[i]->SetDefaultRenderer(m_ren);
		m_planeWidget[i]->SetWindowLevel(1528, 862);//DefaultLevel = 862		DefaultWindow = 1528
		//m_planeWidget[i]->On();
		//m_planeWidget[i]->InteractionOn();
	}

	m_resliceCallback = vtkResliceCursorCallback::New();

	vtkScalarsToColors *LookupTablecolor = m_resliceImageViewer[0]->GetLookupTable();
	for (int i = 0; i < 3; i++)
	{
		//m_resliceCallback->IPW[i] = m_planeWidget[i];
		m_resliceCallback->RCW[i] = m_resliceImageViewer[i]->GetResliceCursorWidget();
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, m_resliceCallback);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, m_resliceCallback);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, m_resliceCallback);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, m_resliceCallback);
		m_resliceImageViewer[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, m_resliceCallback);
		m_resliceImageViewer[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, m_resliceCallback);

		// Make them all share the same color map.
		m_resliceImageViewer[i]->SetLookupTable(LookupTablecolor);
		m_planeWidget[i]->GetColorMap()->SetLookupTable(LookupTablecolor);
		m_planeWidget[i]->SetColorMap(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
	}
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->SetResliceMode(1);
		m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
		m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
		m_resliceImageViewer[i]->Render();
	}
	ui->m_axial2DView->show();
	ui->m_sagital2DView->show();
	ui->m_coronal2DView->show();

	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
		m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
		m_resliceImageViewer[i]->Render();
	}
}

void QFourpaneviewer::INshowVolume3D()
{
	if (!m_MetaReader)
		return;

	vtkImageData *imageData = m_MetaReader->GetOutput();

	m_pieceF = vtkPiecewiseFunction::New();
	m_pieceF->AddPoint(167.00000000000000, 0.16862745098039220);
	m_pieceF->AddPoint(218.00000000000000, 0.41960784313725491);
	m_pieceF->AddPoint(218.00000000000000, 0.41960784313725491);
	m_pieceF->AddPoint(445.00000000000000, 0.57254901960784310);
	m_pieceF->AddPoint(1455.0000000000000, 0.87450980392156863);
	m_pieceF->AddPoint(2784.0000000000000, 0.88235294117647056);

	m_colorTranF = vtkColorTransferFunction::New();

	m_colorTranF->AddRGBPoint(-1024.0, 1.0, 0.13725490196078433, 0.17254901960784313);
	m_colorTranF->AddRGBPoint(24.0, 1.0, 0.13725490196078433, 0.17254901960784313);
	m_colorTranF->AddRGBPoint(163.0, 1.0, 0.13725490196078433, 0.17254901960784313);
	m_colorTranF->AddRGBPoint(167.0, 1.0, 0.35294117647058826, 0.16862745098039217);
	m_colorTranF->AddRGBPoint(218.0, 1.0, 0.63921568627450975, 0.11372549019607843);
	m_colorTranF->AddRGBPoint(445.0, 1.0, 1.0, 1.0);
	m_colorTranF->AddRGBPoint(1455.0, 1.0, 1.0, 1.0);
	m_colorTranF->AddRGBPoint(2784.0, 1.0, 1.0, 1.0);

	m_pieceGradF = vtkPiecewiseFunction::New();
	m_pieceGradF->AddPoint(1, 0.0);
	m_pieceGradF->AddPoint(70, 0.5);
	m_pieceGradF->AddPoint(130, 1.0);
	m_pieceGradF->AddPoint(300, 0.1);

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
	ui->m_mpr2DView->renderWindow()->AddRenderer(m_renderer);
	ui->m_mpr2DView->show();
}

QFourpaneviewer::~QFourpaneviewer()
{
	for (int i = 0; i < 3; i++)
	{
		if (m_resliceImageViewer[i])
		{
			m_resliceImageViewer[i]->Delete();
		}

		if (m_planeWidget[i])
		{
			m_planeWidget[i]->Delete();
		}

		if (m_renderWindow[i])
		{
			m_renderWindow[i]->Delete();
		}
	}

	if (m_resliceCallback)
	{
		m_resliceCallback->Delete();
	}

	if (m_cellPicker)
	{
		m_cellPicker->Delete();
	}

	if (m_ipwProp)
	{
		m_ipwProp->Delete();
	}

	if (m_ren)
	{
		m_ren->Delete();
	}

	if (m_2DViewRenderWindow)
	{
		m_2DViewRenderWindow->Delete();
	}


	///--------------showVolume3D--------
	m_volumeMapper->Delete();
	m_volumeProperty->Delete();
	m_vtkVolume->Delete();

	m_isosurfaceFilter->Delete();
	m_isosurfaceActor->Delete();
	m_renderer->Delete();

	m_pieceF ? m_pieceF->Delete()         : m_pieceF = nullptr;
	m_pieceGradF ? m_pieceGradF->Delete() : m_pieceF = nullptr;
	m_colorTranF ? m_colorTranF->Delete() : m_pieceF = nullptr;
	m_pieceF = nullptr;
	m_pieceF = nullptr;
	m_pieceF = nullptr;
	if (m_isosurfaceMapper)
	{
		m_isosurfaceMapper->Delete();
	}
	//----------------------------------
    delete ui;
}

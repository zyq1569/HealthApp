#include "qfourpaneviewer.h"
#include "ui_qfourpaneviewer.h"


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
class vtkResliceCursorCallback : public vtkCommand
{
public:
	static vtkResliceCursorCallback* New() { return new vtkResliceCursorCallback; }

	void Execute(vtkObject* caller, unsigned long ev, void* callData) override
	{

		if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||
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

		vtkResliceCursorWidget* rcw = dynamic_cast<vtkResliceCursorWidget*>(caller);
		if (rcw)
		{
			vtkResliceCursorLineRepresentation* rep =
				dynamic_cast<vtkResliceCursorLineRepresentation*>(rcw->GetRepresentation());
			// Although the return value is not used, we keep the get calls
			// in case they had side-effects
			rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
			for (int i = 0; i < 3; i++)
			{
				vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(this->IPW[i]->GetPolyDataAlgorithm());
				ps->SetOrigin(
					this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetOrigin());
				ps->SetPoint1(
					this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint1());
				ps->SetPoint2(
					this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint2());

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
	vtkResliceCursorWidget* RCW[3];
};




//--------------

QFourpaneviewer::QFourpaneviewer(QWidget *parent, vtkMetaImageReader* metaReader) :
    QWidget(parent), m_MetaReader(metaReader), ui(new Ui::QFourpaneviewer)
{
    ui->setupUi(this);

	INimage3D();

	m_actionReset = new QAction(this);
	addAction(m_actionReset);
	m_actionReset->setShortcut(tr("ctrl+s"));

	connect(m_actionReset, SIGNAL(triggered()), SLOT(ResetViewer()));//Open an existing DICOM folder
}

void QFourpaneviewer::ResetViewer()
{
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->SetResliceMode(1);
		//m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
	    //DefaultLevel = 862		DefaultWindow = 1528
		vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation())->SetWindowLevel(1528, 862);
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
	//QString fileMhd = m_fileNameMhd;
	//std::string filename = qPrintable(fileMhd);
	//
	//vtkNew<vtkMetaImageReader> reader;
	//reader->SetFileName(filename.c_str());
	//reader->Update();
	int imageDims[3];
	vtkImageData *imageData = m_MetaReader->GetOutput();
	imageData->GetDimensions(imageDims);

	//vtkImageData *imageData = reader->GetOutput();
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i] = vtkResliceImageViewer::New();
		m_renderWindow[i] = vtkGenericOpenGLRenderWindow::New(); //vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
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
		vtkResliceCursorLineRepresentation* rep;
		rep  = vtkResliceCursorLineRepresentation::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation());
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

	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);

	vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
	ui->m_mpr2DView->setRenderWindow(renderWindow);
	ui->m_mpr2DView->renderWindow()->AddRenderer(ren);
	vtkRenderWindowInteractor* iren = ui->m_mpr2DView->interactor();

	for (int i = 0; i < 3; i++)
	{
		m_planeWidget[i] = vtkImagePlaneWidget::New();
		m_planeWidget[i]->SetInteractor(iren);
		m_planeWidget[i]->SetPicker(picker);
		m_planeWidget[i]->RestrictPlaneToVolumeOn();
		double color[3] = { 0, 0, 0 };
		color[i] = 1;
		m_planeWidget[i]->GetPlaneProperty()->SetColor(color);

		color[0] /= 4.0;
		color[1] /= 4.0;
		color[2] /= 4.0;
		m_resliceImageViewer[i]->GetRenderer()->SetBackground(color);

		m_planeWidget[i]->SetTexturePlaneProperty(ipwProp);
		m_planeWidget[i]->TextureInterpolateOff();
		m_planeWidget[i]->SetResliceInterpolateToLinear();
		m_planeWidget[i]->SetInputConnection(m_MetaReader->GetOutputPort());
		m_planeWidget[i]->SetPlaneOrientation(i);
		m_planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
		m_planeWidget[i]->DisplayTextOn();
		m_planeWidget[i]->SetDefaultRenderer(ren);
		m_planeWidget[i]->SetWindowLevel(1528, 862);//DefaultLevel = 862		DefaultWindow = 1528
		m_planeWidget[i]->On();
		m_planeWidget[i]->InteractionOn();
	}

	vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

	for (int i = 0; i < 3; i++)
	{
		cbk->IPW[i] = m_planeWidget[i];
		cbk->RCW[i] = m_resliceImageViewer[i]->GetResliceCursorWidget();
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
		m_resliceImageViewer[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
		m_resliceImageViewer[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);
		m_resliceImageViewer[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, cbk);

		// Make them all share the same color map.
		m_resliceImageViewer[i]->SetLookupTable(m_resliceImageViewer[0]->GetLookupTable());
		m_planeWidget[i]->GetColorMap()->SetLookupTable(m_resliceImageViewer[0]->GetLookupTable());
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
QFourpaneviewer::~QFourpaneviewer()
{
    delete ui;
}

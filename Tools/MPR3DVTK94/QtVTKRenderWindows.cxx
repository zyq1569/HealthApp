// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "QtVTKRenderWindows.h"
#include "ui_QtVTKRenderWindows.h"

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


#include <qevent.h>

const double PI = -3.141592653589793238462643383279502884197169399375105820974944;

class QeventMouse :public QObject
{
public:
	QeventMouse() {	};
	~QeventMouse() { };
	bool eventFilter(QObject *object, QEvent *event)
	{
		if (event->type() == QEvent::Wheel)
		{
			QWheelEvent *wheelEvent = (QWheelEvent*)(event);
			QString name = object->objectName();
			int delta = wheelEvent->delta();
			if (delta  > 0)
			{
				name = object->objectName();
			}
			else
			{
				name = object->objectName();
			}
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
class vtkResliceImageViewerScrollCallback : public vtkCommand
{
public:
	static vtkResliceImageViewerScrollCallback* New()
	{
		return new vtkResliceImageViewerScrollCallback;
	}

	void Execute(vtkObject*, unsigned long ev, void*) override
	{
		if (!this->Viewer->GetSliceScrollOnMouseWheel())
		{
			return;
		}

		// Do not process if any modifiers are ON
		if (this->Viewer->GetInteractor()->GetShiftKey() ||
			this->Viewer->GetInteractor()->GetControlKey() || this->Viewer->GetInteractor()->GetAltKey())
		{
			return;
		}

		// forwards or backwards
		//int sign = (ev == vtkCommand::MouseWheelForwardEvent) ? 1 : -1;
		int sign = (ev == vtkCommand::MouseWheelForwardEvent) ? SliceInc : -SliceInc;

		this->Viewer->IncrementSlice(sign);

		//this->Viewer->SetSlice(this->Viewer->GetSlice() + static_cast<int>(std::round(sign * 1.0)));
		// Abort further event processing for the scroll.
		this->SetAbortFlag(1);
	}

	vtkResliceImageViewerScrollCallback()
		: Viewer(nullptr), SliceInc (1)
	{

	}
	void ChangeSliceScrollOnMouseWheel(bool ChangeSliceScrollOnMouseWheel = false)
	{
		SliceInc = (ChangeSliceScrollOnMouseWheel) ? -1 : 1;
	}
	vtkResliceImageViewer* Viewer;
	int SliceInc;
};
class vtkMPRResliceImageViewer :public vtkResliceImageViewer
{
public:
	static vtkMPRResliceImageViewer* New()
	{
		return new vtkMPRResliceImageViewer;
	}
	vtkMPRResliceImageViewer()
	{
		init();
	}
	void init(bool ChangeSliceScrollOnMouseWheel = false)
	{
		this->ScrollCallback = vtkResliceImageViewerScrollCallback::New();
		this->ScrollCallback->Viewer = this;
		this->ScrollCallback->ChangeSliceScrollOnMouseWheel(ChangeSliceScrollOnMouseWheel);
	}

};
class vtkRCLP :public vtkResliceCursorLineRepresentation
{
public:
	static vtkRCLP* New()
	{
		return new vtkRCLP;
	}

	//void SetResliceParameters(double outputSpacingX, double outputSpacingY, int extentX, int extentY)override
	//{
	//	vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(this->Reslice);
	//
	//	if (reslice)
	//	{
	//		// Set the default color the minimum scalar value
	//		double range[2];
	//		vtkImageData::SafeDownCast(reslice->GetInput())->GetScalarRange(range);
	//		reslice->SetBackgroundLevel(range[0]);
	//		this->ColorMap->SetInputConnection(flip->GetOutputPort());
	//		reslice->TransformInputSamplingOff();
	//		reslice->AutoCropOutputOn();
	//		reslice->SetResliceAxes(this->ResliceAxes);
	//		reslice->SetOutputSpacing(outputSpacingX, outputSpacingY, 1);
	//		reslice->SetOutputOrigin(0.5 * outputSpacingX, 0.5 * outputSpacingY, 0);
	//		reslice->SetOutputExtent(0, extentX - 1, 0, extentY - 1, 0, 0);
	//	}
	//}
	void TransformPlane(vtkPlaneSource* planeToTransform, double targetCenter[3], double targetNormal[3], double targetViewUp[3])
	{
		planeToTransform->SetNormal(targetNormal);
		double currentViewUp[3];
		vtkMath::Subtract(planeToTransform->GetPoint2(), planeToTransform->GetOrigin(), currentViewUp);
		double angle = vtkMath::SignedAngleBetweenVectors(currentViewUp, targetViewUp, targetNormal);
		planeToTransform->Rotate(vtkMath::DegreesFromRadians(angle), targetNormal);
		planeToTransform->SetCenter(targetCenter);
	}
	void RotateASC(int axis, double angle)
	{
		//		vtkResliceCursor* rc = this->GetResliceCursor();
		//		vtkPlane* planeToBeRotated = rc->GetPlane(axis);
		//		double* viewUp = rc->GetViewUp(axis);
		//
		//		const int rcPlaneIdx = this->ResliceCursorActor->GetCursorAlgorithm()->GetReslicePlaneNormal();
		//
		//		vtkPlane* normalPlane = rc->GetPlane(rcPlaneIdx);
		//
		//		double vectorToBeRotated[3], aboutAxis[3], rotatedVector[3];
		//		planeToBeRotated->GetNormal(vectorToBeRotated);
		//		normalPlane->GetNormal(aboutAxis);
		//
		//		this->RotateVectorAboutVector(vectorToBeRotated, aboutAxis, angle, rotatedVector);
		//		this->RotateVectorAboutVector(viewUp, aboutAxis, angle, viewUp);
		//		planeToBeRotated->SetNormal(rotatedVector);
				//RotateAxis
		this->RotateAxis(axis, angle);
	}
};
//------------------------------------------------------------------------------
class vtkResliceCursorCallback : public vtkCommand
{
public:
	static vtkResliceCursorCallback* New() { return new vtkResliceCursorCallback; }

	void Execute(vtkObject* caller, unsigned long ev, void* callData) override
	{
		AbortFlagOff();
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
		else if (ev == vtkResliceImageViewer::SliceChangedEvent)//vtkResliceCursorWidget::ResliceAxesChangedEvent)
		{
			for (int i = 0; i < 3; i++)
			{
				int now = m_riw[i]->GetSlice() + 1;
				int max = m_riw[i]->GetSliceMax() + 1;
				QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
				m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
			}
		}
		else if (ev == vtkCommand::MouseMoveEvent)//vtkResliceCursorWidget::ResliceAxesChangedEvent)
		{
			for (int i = 0; i < 3; i++)
			{
				int now = m_riw[i]->GetSlice() + 1;
				int max = m_riw[i]->GetSliceMax() + 1;
				QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
				m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
			}
		}
		else if (ev == vtkCommand::MouseWheelForwardEvent || ev == vtkCommand::MouseWheelBackwardEvent)
		{
			for (int i = 0; i < 3; i++)
			{
				int now = m_riw[i]->GetSlice() + 1;
				int max = m_riw[i]->GetSliceMax() + 1;
				QString sliceInfo = QObject::tr("im: %1 / %2").arg(now).arg(max);
				m_cornerAnnotations[i]->SetText(2, sliceInfo.toLatin1().constData());
			}
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
			vtkResliceCursorLineRepresentation* rep = dynamic_cast<vtkResliceCursorLineRepresentation*>(rcw->GetRepresentation());
			// Although the return value is not used, we keep the get calls
			// in case they had side-effects
			rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
			for (int i = 0; i < 3; i++)
			{
				vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(this->IPW[i]->GetPolyDataAlgorithm());
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
	vtkResliceCursorWidget* RCW[3];
	vtkCornerAnnotation *m_cornerAnnotations[3];
	vtkResliceImageViewer *m_riw[3];
};

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char* argv[])
{
	this->ui = new Ui_QtVTKRenderWindows;
	this->ui->setupUi(this);

	//MprInit();
	//return;
	//vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	//reader->SetDirectoryName(argv[1]);
	//reader->Update();
	//int imageDims[3];
	//reader->GetOutput()->GetDimensions(imageDims);
	//--
	vtkNew<vtkMetaImageReader> reader;
	//std::string dir = qPrintable());//argv[1];	
	QString dir = argv[1];
	dir = "D:\\Test_DICOM\\jp2k\\VTKMetaData.mhd";
	std::string stddir = qPrintable(dir);
	reader->SetFileName(stddir.c_str());
	reader->Update();
	int imageDims[3];
	reader->GetOutput()->GetDimensions(imageDims);

	vtkImageData *imageData = reader->GetOutput();
	//vtkSmartPointer< vtkImageFlip > ImageFlip = vtkSmartPointer< vtkImageFlip >::New();
	//ImageFlip->SetInputData(reader->GetOutput());
	//ImageFlip->SetFilteredAxes(0);
	//ImageFlip->Update();
	//imageData = ImageFlip->GetOutput();


	for (int i = 0; i < 3; i++)
	{
		riw[i] = vtkSmartPointer<vtkMPRResliceImageViewer>::New();
		//
		if (i == 1)
		{
			riw[i]->init(true);
		}
		else
		{
			riw[i]->init();
		}
		//
		vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
		riw[i]->SetRenderWindow(renderWindow);
	}

	////
		//--
	for (int i = 0; i < 3; i++)
	{
		m_cornerAnnotations[i] = vtkCornerAnnotation::New();
		m_cornerAnnotations[i]->GetTextProperty()->SetFontFamilyToArial();
		m_cornerAnnotations[i]->GetTextProperty()->ShadowOn();
		riw[i]->GetRenderer()->AddViewProp(m_cornerAnnotations[i]);
	}

	this->ui->view2->setRenderWindow(riw[0]->GetRenderWindow());
	riw[0]->SetupInteractor(this->ui->view2->renderWindow()->GetInteractor());

	this->ui->view3->setRenderWindow(riw[1]->GetRenderWindow());
	riw[1]->SetupInteractor(this->ui->view3->renderWindow()->GetInteractor());

	this->ui->view1->setRenderWindow(riw[2]->GetRenderWindow());
	riw[2]->SetupInteractor(this->ui->view1->renderWindow()->GetInteractor());
	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());

		riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());
		rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
		//
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
		//
		riw[i]->SetInputData(imageData/*reader->GetOutput()*/);
		riw[i]->SetSliceOrientation(i);
		riw[i]->SetResliceModeToAxisAligned();
	}
	//----adjust ---
	double a = -0.12037446071029209*26.1;//-3.1417734245386240 ~~~
	a = -3.1415926535897931;//pi
	const double pi = -3.141592653589793238462643383279502884197169399375105820974944;
	vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, pi);
	riw[0]->GetRenderWindow()->Render();
	//---
	////
	vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, PI);
	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);

	vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
	this->ui->view4->setRenderWindow(renderWindow);
	this->ui->view4->renderWindow()->AddRenderer(ren);
	vtkRenderWindowInteractor* iren = this->ui->view4->interactor();

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
		double black[3] = { 0, 0, 0 };
		riw[i]->GetRenderer()->SetBackground(color);// black/*color*/);

		planeWidget[i]->SetTexturePlaneProperty(ipwProp);
		planeWidget[i]->TextureInterpolateOff();
		planeWidget[i]->SetResliceInterpolateToLinear();
		planeWidget[i]->SetInputConnection(reader->GetOutputPort());
		planeWidget[i]->SetPlaneOrientation(i);
		planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
		planeWidget[i]->DisplayTextOn();
		planeWidget[i]->SetDefaultRenderer(ren);
		planeWidget[i]->SetWindowLevel(260, 50);
		planeWidget[i]->On();
		planeWidget[i]->InteractionOn();

		riw[i]->GetRenderer()->ResetCamera();
		riw[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
	}

	///

	resliceMode(1);
	double bounds[6];
	imageData->GetCellBounds(imageData->GetNumberOfCells(), bounds);
	double w = bounds[0] - bounds[0];

	///

	vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

	for (int i = 0; i < 3; i++)
	{
		///------------------
		cbk->m_cornerAnnotations[i] = m_cornerAnnotations[i];
		cbk->m_riw[i] = riw[i];
		///----------------
		cbk->IPW[i] = planeWidget[i];
		cbk->RCW[i] = riw[i]->GetResliceCursorWidget();

		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
		riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
		riw[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);
		riw[i]->AddObserver(vtkResliceImageViewer::SliceChangedEvent, cbk);

		riw[i]->GetInteractor()->AddObserver(vtkCommand::MouseWheelForwardEvent, cbk);
		riw[i]->GetInteractor()->AddObserver(vtkCommand::MouseWheelBackwardEvent, cbk);
		riw[i]->AddObserver(vtkCommand::MouseWheelForwardEvent, cbk);

		// Make them all share the same color map.
		riw[i]->SetLookupTable(riw[0]->GetLookupTable());
		planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
		// planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
		planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
	}

	/////
		//add
	static QeventMouse filter;
	for (int i = 0; i < 3; i++)
	{
		filter.m_cornerAnnotations[i] = m_cornerAnnotations[i];
		filter.m_riw[i] = riw[i];
	}
	ui->view1->installEventFilter(&filter);
	ui->view2->installEventFilter(&filter);
	ui->view3->installEventFilter(&filter);
	////
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
	//resliceMode(2);
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
	ui->radioButton_Max->setEnabled(mode);
	ui->radioButton_Min->setEnabled(mode);
	ui->radioButton_Mean->setEnabled(mode);
}

void QtVTKRenderWindows::SetBlendMode(int m)
{
	for (int i = 0; i < 3; i++)
	{
		vtkImageSlabReslice* thickSlabReslice = vtkImageSlabReslice::SafeDownCast(vtkResliceCursorThickLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation())->GetReslice());
		thickSlabReslice->SetBlendMode(m);
		riw[i]->Render();
	}
}

void QtVTKRenderWindows::SetBlendModeToMaxIP()
{
	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
		//
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
		//
	}
	this->SetBlendMode(VTK_IMAGE_SLAB_MAX);
}

void QtVTKRenderWindows::SetBlendModeToMinIP()
{
	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
		//
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
		//
	}
	this->SetBlendMode(VTK_IMAGE_SLAB_MIN);
}

void QtVTKRenderWindows::SetBlendModeToMeanIP()
{
	for (int i = 0; i < 3; i++)
	{
		// make them all share the same reslice cursor object.
		vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
		//
		rep->GetResliceCursorActor()->GetCenterlineProperty(0)->SetRepresentationToWireframe();//代表12窗口竖线
		rep->GetResliceCursorActor()->GetCenterlineProperty(1)->SetRepresentationToWireframe();//0竖线，2横线
		rep->GetResliceCursorActor()->GetCenterlineProperty(2)->SetRepresentationToWireframe();//01横线
		//
	}
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
		vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(planeWidget[i]->GetPolyDataAlgorithm());
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
	this->ui->view3->renderWindow()->Render();
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
	this->DistanceWidget[i] = vtkSmartPointer<vtkDistanceWidget>::New();
	this->DistanceWidget[i]->SetInteractor(this->riw[i]->GetResliceCursorWidget()->GetInteractor());

	// Set a priority higher than our reslice cursor widget
	this->DistanceWidget[i]->SetPriority(this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

	vtkSmartPointer<vtkPointHandleRepresentation2D> handleRep = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
	vtkSmartPointer<vtkDistanceRepresentation2D> distanceRep = vtkSmartPointer<vtkDistanceRepresentation2D>::New();
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

void QtVTKRenderWindows::MprInit()
{
	//return GPTMRP3D();
	vtkNew<vtkMetaImageReader> reader;
	//std::string dir = qPrintable());//argv[1];	
	QString dir = this->ui->m_dcmDIR->toPlainText();
	dir += "\\VTKMetaData.mhd";
	//QDir mhdDir;
	//QFileInfo file(dir);
	//if (!file.isFile())
	//{
	//	saveHDMdata(this->ui->m_dcmDIR->toPlainText());
	//}
	std::string stddir = qPrintable(dir);
	reader->SetFileName(stddir.c_str());
	reader->Update();
	int imageDims[3];
	reader->GetOutput()->GetDimensions(imageDims);

	for (int i = 0; i < 3; i++)
	{
		riw[i] = vtkSmartPointer< vtkMPRResliceImageViewer >::New();
		vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
		riw[i]->SetRenderWindow(renderWindow);
	}


	this->ui->view1->setRenderWindow(riw[0]->GetRenderWindow());
	riw[0]->SetupInteractor(this->ui->view1->renderWindow()->GetInteractor());

	this->ui->view2->setRenderWindow(riw[1]->GetRenderWindow());
	riw[1]->SetupInteractor(this->ui->view2->renderWindow()->GetInteractor());

	this->ui->view3->setRenderWindow(riw[2]->GetRenderWindow());
	riw[2]->SetupInteractor(this->ui->view3->renderWindow()->GetInteractor());

	///012  021 120 102  201 210  
	vtkImageData *imageData = reader->GetOutput();

	///https://gitlab.kitware.com/vtk/vtk/-/issues/18726
	///https://discourse.vtk.org/t/flip-mpr-image-become-abnormal/9931
	vtkSmartPointer< vtkRenderWindowInteractor >renderWindowInteractor[3];
	vtkSmartPointer< vtkResliceCursorLineRepresentation > vtkrclp[3];
	vtkSmartPointer< vtkResliceCursor > resliceCursor = vtkSmartPointer< vtkResliceCursor >::New();
	///

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
	}



	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);

	vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

	vtkSmartPointer< vtkRenderer > ren = vtkSmartPointer< vtkRenderer >::New();

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
	this->ui->view4->setRenderWindow(renderWindow);
	this->ui->view4->renderWindow()->AddRenderer(ren);
	vtkRenderWindowInteractor *iren = this->ui->view4->interactor();

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

	for (int i = 0; i < 3; i++)
	{
		riw[i]->SetResliceMode(1);
		riw[i]->GetRenderer()->ResetCamera();
		riw[i]->GetRenderer()->GetActiveCamera()->Zoom(1.6);
		riw[i]->Render();
	}
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

void QtVTKRenderWindows::on_pushButton_clicked()
{
	double a = -0.12037446071029209*26.1;//-3.1417734245386240 ~~~
	a = -3.1415926535897931;//pi
	const double pi = -3.141592653589793238462643383279502884197169399375105820974944;
	vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, pi);
	riw[0]->GetRenderWindow()->Render();
}


void QtVTKRenderWindows::on_Sagittal_clicked()
{
    /*
    this->ui->view2->setRenderWindow(riw[0]->GetRenderWindow());
	riw[0]->SetupInteractor(this->ui->view2->renderWindow()->GetInteractor());

	this->ui->view3->setRenderWindow(riw[1]->GetRenderWindow());
	riw[1]->SetupInteractor(this->ui->view3->renderWindow()->GetInteractor());

	this->ui->view1->setRenderWindow(riw[2]->GetRenderWindow());
	riw[2]->SetupInteractor(this->ui->view1->renderWindow()->GetInteractor());
    */
	//    //m_Sagittal
	const double pi = -3.141592653589793238462643383279502884197169399375105820974944;

    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, 30 * pi/180);
    //riw[0]->GetRenderWindow()->Render();
	//vtkResliceCursorLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, 30 * pi / 180);
    //riw[0]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(2, 30 * pi / 180);
    //riw[0]->GetRenderWindow()->Render();
    //
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, 30 * pi / 180);
    //riw[2]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, 30 * pi / 180);
    //riw[2]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[2]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(2, 30 * pi / 180);
    //riw[2]->GetRenderWindow()->Render();
    //
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[1]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(0, 30 * pi / 180);
    //riw[1]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[1]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, 30 * pi / 180);
    //riw[1]->GetRenderWindow()->Render();
    //vtkResliceCursorLineRepresentation::SafeDownCast(riw[1]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(2, 30 * pi / 180);
	//riw[1]->GetRenderWindow()->Render();
}

//常见人体组织的CT值（HU）
//
//组织          CT值            组织        CT值
//骨组织      ＞400              肝脏      50～70
//钙值       80～300             脾脏      35～60
//血块       64～84             胰腺      30～55
//脑白质     25～34              肾脏      25～50
//脑灰质     28～44              肌肉      40～55
//脑脊液      3～8               胆囊      10～30
//血液       13～32            甲状腺      50～90
//血浆       3～14              脂肪     －20～－100
//渗出液      ＞15                水           0
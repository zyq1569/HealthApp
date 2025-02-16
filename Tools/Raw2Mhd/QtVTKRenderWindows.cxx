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

#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkFloatArray.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkByteSwap.h>
#include <fstream>
#include <iostream>

#include<Qdir>
void QtVTKRenderWindows::raw2mhd()
{	
	QString dir = ui->m_dcmDIR->toPlainText();
	QString w = ui->m_width->toPlainText();
	QString h = ui->m_height->toPlainText();
	QString number = ui->m_number->toPlainText();
	QString filenameExtra="";
	if (ui->m_extraCheck->isChecked())
	{
		filenameExtra = ui->m_extra->toPlainText();
	}
	int width = w.toInt();
	int height = h.toInt();
	int depth = number.toInt();  // 0~200 共 201 张
	// 逐个读取 0～200 的 RAW 文件
	QString raw = dir + "VTK_Data.raw";
	std::string rawFilename = qPrintable(raw);// "D:/TEMP/dingliang/volume_1/volume_VTKData600.raw";  // `.raw` 文件
	QString mhd = dir + "VTK_Data.mhd";
	std::string mhdFilename = qPrintable(mhd);// "D:/TEMP/dingliang/volume_1/volume_VTKData600.mhd";  // `.mhd` 文件
	std::ofstream rawFile(rawFilename, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!rawFile.is_open())
	{
		//std::cerr << "无法打开 .raw 文件进行写入！" << std::endl;
		return ;
	}
	QString fn = ui->m_filename->toPlainText();
	QString fa = ui->m_fmn->toPlainText();
	int formateN = fa.toInt();
	int index_s = ui->m_start->toPlainText().toInt();
	int index_e = index_s + depth - 1;

	for (int i = index_s; i < index_e; i++)
	{
		QString index = dir + "\\" + fn;//
		if (ui->m_formate->isChecked())
		{
			index += QString("%1").arg(i, formateN, 10, QLatin1Char('0'));
			index += filenameExtra;
		}
		else
		{
			index += QString("%1").arg(i);
			index += filenameExtra;
		}

		std::string filename = qPrintable(index);
		QFileInfo file(index);
		if (!file.isFile())
		{
			return;
		}
		vtkSmartPointer<vtkImageReader2> reader = vtkSmartPointer<vtkImageReader2>::New();
		reader->SetFileName(filename.c_str());
		reader->SetFileDimensionality(2);  // 2D 图像
		reader->SetDataExtent(0, width - 1, 0, height - 1, 0, 0);  // 2D 读取
		reader->SetDataScalarTypeToUnsignedShort();  // 16-bit 无符号整型
		reader->SetNumberOfScalarComponents(1);
		reader->SetFileLowerLeft(true);  // 数据从左下角开始
		// 设置像素间距
		reader->SetDataSpacing(0.025, 0.025, 0.025); // X, Y, Z 间距匹配 Scale_x/y/z	
		reader->Update();

		vtkSmartPointer<vtkImageData> imageData = reader->GetOutput();
		int* dims = imageData->GetDimensions();
		int dataSize = dims[0] * dims[1] * sizeof(unsigned short);

		rawFile.write(static_cast<char*>(imageData->GetScalarPointer()), dataSize);
		//std::cout << "已写入切片 " << 10 << " 到 " << std::endl;

	}

	// 生成 MHD 头文件（只执行一次）
	std::ofstream mhdFile(mhdFilename);
	if (!mhdFile.is_open())
	{
		//std::cerr << "无法创建 .mhd 文件！" << std::endl;
		return ;
	}
	mhdFile << "ObjectType = Image\n";
	mhdFile << "NDims = 3\n";
	mhdFile << "DimSize = " << width << " " << height << " " << depth << "\n";
	mhdFile << "ElementSpacing = " << 0.025 << " " << 0.025 << " " << 0.025 << "\n";
	mhdFile << "ElementType = MET_USHORT\n";
	mhdFile << "ElementDataFile = " << rawFilename << "\n";
	mhdFile.close();

	//std::cout << "MHD 文件已生成：" << mhdFilename << std::endl;

}

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char* argv[])
{
	this->ui = new Ui_QtVTKRenderWindows;
	this->ui->setupUi(this);

	ui->Sagittal->hide();
	ui->pushButton->hide();
	ui->frame->hide();
	ui->view1->hide();
	ui->view2->hide();
	ui->view3->hide();
	ui->view4->hide();
	ui->pushButton->hide();
	ui->pushButton->hide();
	ui->pushButton->hide();

	connect(ui->m_pbRaw2Mhd, SIGNAL(clicked()), this, SLOT(raw2mhd()));

	QString dir = QCoreApplication::applicationDirPath();
	ui->m_dcmDIR->setText(dir);
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
	//    //m_Sagittal
	const double pi = -3.141592653589793238462643383279502884197169399375105820974944;
	vtkResliceCursorLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation())->UserRotateAxis(1, pi);
	riw[2]->GetRenderWindow()->Render();
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
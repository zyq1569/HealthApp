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

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkMarchingCubes.h>
//-------------showVolume3D
#include <vtkSmartVolumeMapper.h>
#include <vtkRayCastImageDisplayHelper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageMarchingCubes.h>
#include <vtkVolumeProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkBMPWriter.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

///////////////////////////////////
#include <ThreadWeaver/ThreadWeaver>
//--------------
#include <QAction>
#include <QDateTime>
#include <QFileDialog>

#include "vtkcolorgradient.h"
using namespace ThreadWeaver;

class Volume3DJob : public Job 
{
public:
	Volume3DJob(QFourpaneviewer *fourpaneviewer = nullptr) : m_fourPaneviewer(fourpaneviewer)
	{}
protected:
	void run(JobPointer, Thread*) Q_DECL_OVERRIDE
	{
		if (!m_fourPaneviewer)
			return;

        m_fourPaneviewer->ShowImage3D();
	}
private:
	QFourpaneviewer *m_fourPaneviewer;
};

class VolumeGrayHistogramJob :public Job
{
public:
    VolumeGrayHistogramJob(QFourpaneviewer *parent = nullptr, vtkImageData *imageData = nullptr):m_fourPaneviewer(parent), m_imageData(imageData)
    {}

protected:
    void run(JobPointer, Thread*) Q_DECL_OVERRIDE
    {
        if (!m_fourPaneviewer || !m_imageData)
            return;

        //加载灰值数据--[-1,4096]
        vtkImageData *image = m_imageData;

        int vl, L = 0, H = 0, max = 0, min = 0;
        double pixelvalue;
        int *dims  = image->GetDimensions();
        m_fourPaneviewer->m_numberPixels = dims[0] * dims[1] * dims[2];;
        for (int z = 0; z < dims[2]; z++)
        {
            for (int y = 0; y < dims[1]; y++)
            {
                for (int x = 0; x < dims[0]; x++)
                {
                    pixelvalue = image->GetScalarComponentAsDouble(x, y, z, 0);
                    vl = static_cast<int>(pixelvalue);

                    if (vl < min)
                    {  
                        min = vl;
                    }
                        
                    if (vl > max)
                    {
                        max = vl;
                    }

                    if (vl < 0)
                    {
                        L++;
                    }
                    else if (vl > 4096)
                    {
                        H++;
                    }
                    else
                    {
                        m_fourPaneviewer->m_imageGrayHis[vl]++;
                    }
                }
            }
        }
        int star = min, end = max + 1,grayNumbers = 0;
        if (min < 1)
        {
            star = 1;
        }
        for (int i = star; i < end; i++)
        {
            if (m_fourPaneviewer->m_imageGrayHis[i] > grayNumbers)
            {
                m_fourPaneviewer->m_grayMaxOfNumbersIndex = i;
                grayNumbers = m_fourPaneviewer->m_imageGrayHis[i];
            }
        }
        m_fourPaneviewer->m_grayMaxOfNumbers = grayNumbers;
        if (m_fourPaneviewer->m_imageGrayHis[0] > grayNumbers)
        {
            m_fourPaneviewer->m_grayMaxOfNumbers = m_fourPaneviewer->m_imageGrayHis[0] + grayNumbers/10;
        }
        m_fourPaneviewer->m_lValue  = L;
        m_fourPaneviewer->m_hValue  = H;
        m_fourPaneviewer->m_maxGray = max;
        m_fourPaneviewer->m_minGray = min;
    };
private:
    QFourpaneviewer *m_fourPaneviewer;
    vtkImageData *m_imageData;
};

//class VolumePlaneJob : public Job
//{
//public:
//	VolumePlaneJob(QFourpaneviewer *Fourpaneviewer = nullptr) : m_Fourpaneviewer(Fourpaneviewer)
//	{}
//protected:
//	void run(JobPointer, Thread*) Q_DECL_OVERRIDE
//	{
//		if (!m_Fourpaneviewer)
//			return;
//
//		m_Fourpaneviewer->INimage3D();
//	}
//private:
//	QFourpaneviewer *m_Fourpaneviewer;
//};
//
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

QFourpaneviewer::QFourpaneviewer(QWidget *parent) : QWidget(parent),  ui(new Ui::QFourpaneviewer)
{
    ui->setupUi(this);
	m_MainWindow = (MainWindow*)parent;

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
    m_vtkColorGradient   = nullptr;

    m_numberPixels = 0;

	if (m_MainWindow->m_checkDefaultWL)
	{
		m_defaultLevel  = m_MainWindow->m_DefaultLevel;
		m_defaultWindow = m_MainWindow->m_DefaultWindow;
	}
	else
	{
		m_defaultLevel  = 862;
		m_defaultWindow = 1528;
	}
	//////////////////////////////////////////////////////////////////
	//color 窗体颜色编辑值信号
	connect(ui->m_editorByValues, &QEditorByValues::signalsColorValue,this, &QFourpaneviewer::ResetColor3D);
    //connect(ui->m_editorByValues, &QEditorByValues::show, ui->m_editorByValues, &QFourpaneviewer::ResetColor3D);
    connect(this, &QFourpaneviewer::LoadConfigFiles, ui->m_editorByValues, &QEditorByValues::loadFileValues);
    
	/////////////////////////////////////////////////////////////////////

	m_actionReset = new QAction(this);
	addAction(m_actionReset);
	m_actionReset->setShortcut(tr("ctrl+s"));

	connect(m_actionReset, SIGNAL(triggered()), SLOT(ResetViewer()));//Open an existing DICOM folder
	//---------------------showVolume3D----------------------------------
	m_pieceF = nullptr;
	m_pieceGradF = nullptr;
	m_colorTranF = nullptr;

    //m_volumeMapper = vtkGPUVolumeRayCastMapper::New();
    m_volumeMapper     = vtkSmartVolumeMapper::New();

	m_isosurfaceFilter = vtkImageMarchingCubes::New();

	m_volumeProperty   = vtkVolumeProperty::New();

	m_isosurfaceActor  = vtkActor::New();
    m_isosurfaceActor->GetProperty()->SetColor(1.0, 0.85, 0.7);  // 设置皮肤色（RGB）

	m_renderer  = vtkRenderer::New();

	m_vtkVolume = vtkVolume::New();
	m_vtkVolume->SetProperty(m_volumeProperty);
	m_vtkVolume->SetMapper(m_volumeMapper);

	m_isosurfaceMapper = vtkPolyDataMapper::New();// isosurfaceMapper;
	m_isosurfaceMapper->SetInputConnection(m_isosurfaceFilter->GetOutputPort());
	m_isosurfaceActor->SetMapper(m_isosurfaceMapper);

	m_renderer->AddViewProp(m_vtkVolume);

    //add ----ISO
    m_renderer->AddActor(m_vtkVolume);  // 添加体绘制到渲染器
    //m_renderer->AddActor(m_isosurfaceActor);  // 添加等值面到渲染器
    m_bremoveActor = false;
	//-----------------------------------------------------------
	ui->m_SplitterLR->widget(1)->setMaximumWidth(650);
	ui->m_SplitterLR->widget(1)->setMinimumWidth(320);
	//QSplitterHandle* hand = ui->m_SplitterLR->handle(1);
	//if (hand)
	//{
	//	hand->setDisabled(true);
	//}

	ui->m_editorsWidget->hide();
	m_showEditors = false;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	ui->m_editorsWidget->setCurrentIndex(1);//设置坐标系方式选择点,另外一个是具体点坐标输入方式设置颜色及透明度
	ui->m_editorsWidget->setStyleSheet("background-color:rgb(240,240,240)}");
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

#define VTKRCP vtkResliceCursorRepresentation
void QFourpaneviewer::SavePaneImage()
{
    QString dir = QCoreApplication::applicationDirPath()+"\\";
    vtkSmartPointer<vtkPNGWriter> writer        = vtkSmartPointer<vtkPNGWriter>::New();
    vtkSmartPointer<vtkJPEGWriter> writerJ      = vtkSmartPointer<vtkJPEGWriter>::New();
    vtkSmartPointer<vtkBMPWriter> writerB       = vtkSmartPointer<vtkBMPWriter>::New();
    vtkSmartPointer<vtkImageWriter> imagewriter;

    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetTableRange(0.0, 2048.0);  // 适用于 8 位灰度图像 (0-255)
    lookupTable->SetValueRange(0.0, 1.0);   // 0 = 黑色, 1 = 白色// 设置颜色范围 (黑 -> 白)
    lookupTable->SetSaturationRange(0.0, 0.0); // 0 = 无彩色 (纯灰度)
    lookupTable->Build();
    vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "",  tr("Images(*.png );;Images(*.bmp);;images(*.jpg)"));
    QString upperstr = fileName.toUpper();
    if (upperstr.contains("PNG"))
    {
        imagewriter = writer;
    }
    else if (upperstr.contains("BMP"))
    {
        imagewriter = writerB;
    }
    else if (upperstr.contains("JPG"))
    {
        imagewriter = writerJ;
    }
    else
    {
        fileName = dir + QString::number(QDateTime::currentDateTime().toTime_t()) + ".png";
    }
    int pos = fileName.length()-4;
    for (int i = 0; i < 3; i++)
    {
        if (VTKRCP* rep = VTKRCP::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation()))
        {
            if (vtkImageReslice* reslice = vtkImageReslice::SafeDownCast(rep->GetReslice()))
            {
                // default background color is the min value of the image scalar range
                vtkImageData* data = reslice->GetOutput();
                mapToColors->SetInputData(data);
                mapToColors->SetLookupTable(lookupTable); // 需要提前设置适当的查找表
                mapToColors->Update();
                QString imagefileName = fileName;// dir + QString::number(QDateTime::currentDateTime().toTime_t()) + QString::number(i) + ".png";
                QString insertStr     = "_" + QString::number(i) + ".";
                imagefileName = imagefileName.replace(pos,1, insertStr);
                std::string str  = qPrintable(imagefileName);
                writer->SetFileName(str.c_str());
                writer->SetInputData(mapToColors->GetOutput());
                writer->Update();
                writer->Write();
            }
        }
    }
}

void QFourpaneviewer::ShowEditorsWidget()
{
    if (m_MainWindow->m_colorGradient)
    {
        if (!m_vtkColorGradient)
        {
	        m_vtkColorGradient = new VtkColorGradient(this);
            m_vtkColorGradient->setGeometry(this->width()/2, 5, m_vtkColorGradient->width(), m_vtkColorGradient->height());
        }
        if (m_vtkColorGradient)
        {
            (m_vtkColorGradient->isHidden())?(m_vtkColorGradient->show()):(m_vtkColorGradient->hide());
        }
        ui->m_editorsWidget->hide();
    }
    else
    {
        m_showEditors ? (ui->m_editorsWidget->hide()) : (ui->m_editorsWidget->show());
	    m_showEditors = !m_showEditors;

        //+++++++++++++++++++++++++++++++++++
        //只有第一次加载文件,需要时再修改
        static bool bfirst = true;
        if (bfirst)
        {
            if (m_showEditors)
            {
                ui->m_editorByValues->setMaximum(m_maxGray + 1);
                ui->m_editorByValues->setMinimum(m_minGray - 1);
                emit LoadConfigFiles();
                bfirst = false;
            }
        }
        //++++++++++++++++++++++++++++++++++++++++

        if (m_vtkColorGradient)
        {
            m_vtkColorGradient->hide();
        }
    }
}

void QFourpaneviewer::ShowImagePlaneAnd3D()
{
    Queue queue3D;

    //读取灰度值信息
    QSharedPointer<Job> jobgray(new VolumeGrayHistogramJob(this, m_MainWindow->m_vtkImageData));
    queue3D.enqueue(jobgray);// 将任务加入队列

	QSharedPointer<Job> job(new Volume3DJob(this));
	queue3D.enqueue(job);// 将任务加入队列

    ShowImagePlane();
	ResetViewer();//第一个切图窗口
	ui->m_mpr2DView->renderWindow()->Render();

}

void QFourpaneviewer::ResetViewer()
{
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->SetResliceMode(1);
		vtkREP::SafeDownCast(m_resliceImageViewer[i]->GetResliceCursorWidget()->GetRepresentation())->SetWindowLevel(m_defaultWindow, m_defaultLevel);
	}
	for (int i = 0; i < 3; i++)
	{
		m_resliceImageViewer[i]->Reset();
		m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
		m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
		m_resliceImageViewer[i]->Render();
	}
}

void QFourpaneviewer::UpdateColorGradient3D(VtkColorStyle colorValue)
{
    if(m_volumeProperty)
    {
        //设置透明度和梯度透明度
        if (colorValue.m_colorOpacity)
        {
            m_volumeProperty->DisableGradientOpacityOff();
            QList<QPointF> points = colorValue.m_gradientPoinst;
            if (m_pieceGradF)
            {
                m_pieceGradF->RemoveAllPoints();
                int size = points.size();
                int slope = colorValue.m_slope;
                for (unsigned short i = 0; i < size; i++)
                {
                    m_pieceGradF->AddPoint(points[i].rx(), (points[i].ry()*slope)/100.0);
                }
                /**
                 * Set the opacity of a volume to an opacity transfer function based
                 * on gradient magnitude for the given component.
                 */
                m_volumeProperty->SetGradientOpacity(m_pieceGradF);
            }
            if (m_pieceF)
            {
                m_pieceF->RemoveAllPoints();
                int size = points.size();
                for (unsigned short i = 0; i < size; i++)
                {
                    m_pieceF->AddPoint(points[i].rx(), points[i].ry());
                }
                /**
                 * Set the opacity of a volume to an opacity transfer function based
                 * on scalar value for the component indicated by index.
                 */
                m_volumeProperty->SetScalarOpacity(m_pieceF);
            }
        }
        else
        {
            m_pieceGradF->RemoveAllPoints();
            m_pieceF->RemoveAllPoints();
            m_pieceF->AddPoint(-2000, 1.0); m_pieceF->AddPoint(2000, 1.0);  // 低梯度区域完全不透明

            m_volumeProperty->SetGradientOpacity(m_pieceGradF);
            m_volumeProperty->SetScalarOpacity(m_pieceF);

            m_volumeProperty->DisableGradientOpacityOn();//关闭梯度透明度
        }

        if (colorValue.m_colorAdd)
        {
            QList<VtkColorPoint> points = colorValue.m_colorPoint;
            if (m_colorTranF)
            {
                m_colorTranF->RemoveAllPoints();
                for (unsigned short i = 0; i < points.size(); i++)
                {
                    double x = points.at(i).m_X;
                    QColor color = points.at(i).m_Color;
                    m_colorTranF->AddRGBPoint(x, color.redF(), color.greenF(), color.blueF());
                }
                m_volumeProperty->SetColor(m_colorTranF);
            }
        }
        else
        {
            m_colorTranF->RemoveAllPoints();
            m_volumeProperty->SetColor(m_colorTranF);
        }

        //lightShade
        if (colorValue.m_lightShade)
        {
            m_volumeProperty->ShadeOn();
            m_volumeProperty->SetAmbient(colorValue.m_Ambient);//环境光系数
            m_volumeProperty->SetDiffuse(colorValue.m_Diffuse);//散射光系数
            m_volumeProperty->SetSpecular(colorValue.m_Specular);//反射光系数
            m_volumeProperty->SetSpecularPower(colorValue.m_SpecularPower);//高光强度
        }
        else
        {
            m_volumeProperty->ShadeOff();
        }
        //+++IsoSurface test set color
        //m_isosurfaceMapper->ScalarVisibilityOff();
        //m_isosurfaceFilter->ComputeScalarsOff(); //ScalarsOff 数据（否则 Mapper 会默认启用 Scalar）
        //m_isosurfaceActor->GetProperty()->SetColor(0.9, 0.9, 0.9);

        m_volumeMapper->SetBlendMode(colorValue.m_blendMode);
        //+++++++++++++
        int blendMode = colorValue.m_blendMode;
        if (blendMode == 5)
        {
            m_volumeMapper->SetBlendMode(0);//目前简化使用混合面值方式
            m_renderer->AddActor(m_isosurfaceActor);
            m_bremoveActor = true;
            m_isosurfaceMapper->ScalarVisibilityOff();
            m_isosurfaceFilter->ComputeScalarsOff(); //ScalarsOff 数据（否则 Mapper 会默认启用 Scalar）
            QList<VtkColorPoint> points = colorValue.m_colorPoint;
            int len = points.size();    
            QColor color = points.at(len-1).m_Color;//使用最后一个点颜色作为面的颜色
            m_isosurfaceActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
            m_isosurfaceActor->GetProperty()->SetAmbient(colorValue.m_Ambient);//环境光系数
            m_isosurfaceActor->GetProperty()->SetDiffuse(colorValue.m_Diffuse);//散射光系数
            m_isosurfaceActor->GetProperty()->SetSpecular(colorValue.m_Specular);//反射光系数
            m_isosurfaceActor->GetProperty()->SetSpecularPower(colorValue.m_SpecularPower);//高光强度
            m_isosurfaceActor->GetProperty()->SetOpacity(colorValue.m_slope);            
        }
        else if(m_bremoveActor)
        {
            m_renderer->RemoveActor(m_isosurfaceActor);
            m_bremoveActor = false;
        }    
        //+++++++++++++

        ui->m_mpr2DView->renderWindow()->Render();
    }
}

void QFourpaneviewer::ResetColor3D(VtkColorStyle colorValue)
{
	if (m_volumeProperty)
	{
		if (colorValue.m_colorOpacity)
		{
			m_volumeProperty->DisableGradientOpacityOff();
			QList<VtkColorPoint> points = colorValue.m_colorPoint;
			if (m_pieceGradF)
			{
				m_pieceGradF->RemoveAllPoints();
				for (unsigned short i = 0; i < points.size(); i++)
				{
					double x = points.at(i).m_X;
					QColor color = points.at(i).m_Color;
					m_pieceGradF->AddPoint(x, color.alphaF());	
				}
				m_volumeProperty->SetGradientOpacity(m_pieceGradF);
			}
			if (m_pieceF)
			{
				m_pieceF->RemoveAllPoints();
				for (unsigned short i = 0; i < points.size(); i++)
				{
					double x = points.at(i).m_X;
					QColor color = points.at(i).m_Color;
					m_pieceF->AddPoint(x, color.alphaF());
				}
				m_volumeProperty->SetScalarOpacity(m_pieceF);
			}
		}
		else
		{
			m_pieceGradF->RemoveAllPoints();
			m_pieceF->RemoveAllPoints();
			m_pieceF->AddPoint(-2000, 1.0); m_pieceF->AddPoint(2000, 1.0);  // 低梯度区域完全不透明

			m_volumeProperty->SetGradientOpacity(m_pieceGradF);
			m_volumeProperty->SetScalarOpacity(m_pieceF);

			m_volumeProperty->DisableGradientOpacityOn();//关闭梯度透明度
		}

		if (colorValue.m_colorAdd)
		{
			QList<VtkColorPoint> points = colorValue.m_colorPoint;		
			if (m_colorTranF)
			{
				m_colorTranF->RemoveAllPoints();
				for (unsigned short i = 0; i < points.size(); i++)
				{
					double x = points.at(i).m_X;
					QColor color = points.at(i).m_Color;
					m_colorTranF->AddRGBPoint(x, color.redF(), color.greenF(), color.blueF());
				}
				m_volumeProperty->SetColor(m_colorTranF);
			}
		}
		else
		{
			m_colorTranF->RemoveAllPoints();
			m_volumeProperty->SetColor(m_colorTranF);
		}

		//lightShade
		if (colorValue.m_lightShade)
		{
			m_volumeProperty->ShadeOn();
			m_volumeProperty->SetAmbient(colorValue.m_Ambient);//环境光系数
			m_volumeProperty->SetDiffuse(colorValue.m_Diffuse);//散射光系数
			m_volumeProperty->SetSpecular(colorValue.m_Specular);//反射光系数
			m_volumeProperty->SetSpecularPower(colorValue.m_SpecularPower);//高光强度
		}
		else
		{
			m_volumeProperty->ShadeOff();
		}

        //++++++++++++++++++++++++++++
        if (m_MainWindow->m_sampleDistanceCheck)
        {
            m_volumeMapper->SetRequestedRenderModeToRayCast();
            m_volumeMapper->SetSampleDistance(m_MainWindow->m_sampleDistance);
        }
        else///ToDefault
        {
            m_volumeMapper->SetRequestedRenderModeToDefault();
            m_volumeMapper->SetSampleDistance(-1.0);
        }
        //+++++++++++++++++++++++++++

        m_renderer->RemoveActor(m_isosurfaceActor); // AddActor(m_isosurfaceActor);  // 添加等值面到渲染器
		ui->m_mpr2DView->renderWindow()->Render();
	}
}
void QFourpaneviewer::ShowImagePlane()
{
	if (!m_MainWindow->m_vtkImageData)
	{
		return;
	}

	int imageDims[3];
	vtkImageData *imageData = m_MainWindow->m_vtkImageData;
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
		//-------------------------------------------------------------------------------------------------------
		m_resliceImageViewer[i]->SetInputData(imageData);
		m_resliceImageViewer[i]->SetSliceOrientation(i);
		m_resliceImageViewer[i]->SetResliceModeToAxisAligned();
		//DefaultLevel = 862		DefaultWindow = 1528
		rep->SetWindowLevel(m_defaultWindow, m_defaultLevel);
	}

	//mpr2DView--->hide- to->showVolume3D
	//m_cellPicker = vtkCellPicker::New();	//m_cellPicker->SetTolerance(0.005);
	//m_ipwProp = vtkProperty::New();	//m_ren     = vtkRenderer::New();
	//m_2DViewRenderWindow = vtkGenericOpenGLRenderWindow::New();	//ui->m_mpr2DView->setRenderWindow(m_2DViewRenderWindow);
	//ui->m_mpr2DView->renderWindow()->AddRenderer(m_ren);	//vtkRenderWindowInteractor* iren = ui->m_mpr2DView->interactor();
	double color[3] = { 0, 0, 0 };
	for (int i = 0; i < 3; i++)
	{
		m_planeWidget[i] = vtkImagePlaneWidget::New();
		//m_planeWidget[i]->SetInteractor(iren);//m_planeWidget[i]->SetPicker(m_cellPicker);
		m_planeWidget[i]->RestrictPlaneToVolumeOn();
		//color[i] = 1;
		m_planeWidget[i]->GetPlaneProperty()->SetColor(color);
		//color[0] /= 4.0;		color[1] /= 4.0;		color[2] /= 4.0;
		m_resliceImageViewer[i]->GetRenderer()->SetBackground(color);

		//m_planeWidget[i]->SetTexturePlaneProperty(m_ipwProp);
		m_planeWidget[i]->TextureInterpolateOff();
		m_planeWidget[i]->SetResliceInterpolateToLinear();
		m_planeWidget[i]->SetInputConnection(m_MainWindow->m_vtkAlgorithmOutput);
		m_planeWidget[i]->SetPlaneOrientation(i);
		m_planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
		m_planeWidget[i]->DisplayTextOn();
		//m_planeWidget[i]->SetDefaultRenderer(m_ren);
		m_planeWidget[i]->SetWindowLevel(m_defaultWindow, m_defaultLevel);
		//m_planeWidget[i]->On();//m_planeWidget[i]->InteractionOn();
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
		m_resliceImageViewer[i]->Reset();
		m_resliceImageViewer[i]->SetResliceMode(1);
		m_resliceImageViewer[i]->GetRenderer()->ResetCamera();
		m_resliceImageViewer[i]->GetRenderer()->GetActiveCamera()->Zoom(1.2);
		m_resliceImageViewer[i]->Render();
	}	//ui->m_axial2DView->show();	//ui->m_sagital2DView->show();	//ui->m_coronal2DView->show();
}

void QFourpaneviewer::ShowImage3D()
{
	if (!m_MainWindow->m_vtkImageData)
		return;

	vtkImageData *imageData = m_MainWindow->m_vtkImageData;

	m_pieceF = vtkPiecewiseFunction::New();//	m_pieceF->AddPoint(0, 1.0); m_pieceF->AddPoint(255, 1.0);  // 低梯度区域完全不透明 // 高梯度区域同样不透明（相当于禁用梯度影响）
	m_MainWindow->m_checkOpacity ?(m_pieceF->AddPoint(167.00000000000000, 0.16862745098039220),m_pieceF->AddPoint(218.00000000000000, 0.41960784313725491)):(m_pieceF->AddPoint(255, 1.0));
	 // 完全不透明	m_pieceF->AddPoint(167.00000000000000, 0.16862745098039220);	m_pieceF->AddPoint(218.00000000000000, 0.41960784313725491);
	//m_pieceF->AddPoint(445.00000000000000, 0.57254901960784310);
	m_pieceF->AddPoint(1455.0000000000000, 0.87450980392156863);
	//m_pieceF->AddPoint(2784.0000000000000, 0.88235294117647056);
	/**/
	m_volumeProperty->SetScalarOpacity(m_pieceF);

	if (m_MainWindow->m_check3Dcolor)
	{
		m_colorTranF = vtkColorTransferFunction::New();	//m_colorTranF->AddRGBPoint(0, 0.0, 0.0, 0.0);  m_colorTranF->AddRGBPoint(255, 1.0, 1.0, 1.0);  // 黑色// 白色
		//气孔 半透明/黑色，可见但不影响主体观察。		//金属正常区域 白色，结构清晰可见。		//夹杂物 红色 + 高不透明度，突出异常区域。
		m_colorTranF->AddRGBPoint(-1000.0, 0.0, 0.0, 0.0); // 空气 -> 黑色
		m_colorTranF->AddRGBPoint(200.0, 0.8, 0.8, 0.8);   // 低密度区域 -> 灰色
		m_colorTranF->AddRGBPoint(800.0, 1.0, 1.0, 1.0);   // 金属主体 -> 白色
		//m_colorTranF->AddRGBPoint(1500.0, 1.0, 0.0, 0.0);  // 夹杂物 -> 红色（高密度）
        m_colorTranF->AddRGBPoint(1500.0, 190, 143, 74);  // 夹杂物 -> 红色（高密度）
		m_volumeProperty->SetColor(m_colorTranF);

	}

	//关闭梯度透明
	m_pieceGradF = vtkPiecewiseFunction::New();
	m_pieceGradF->AddPoint(1, 0.0);
	m_pieceGradF->AddPoint(70, 0.5);
	m_pieceGradF->AddPoint(130, 1.0);
	m_pieceGradF->AddPoint(300, 0.1);
	m_volumeProperty->SetGradientOpacity(m_pieceGradF);


	m_volumeProperty->ShadeOn();
	//m_volumeProperty->ShadeOff();
	m_volumeProperty->SetInterpolationType(m_MainWindow->m_cbInterType);//m_volumeProperty->SetInterpolationTypeToNearest();m_volumeProperty->SetInterpolationTypeToLinear();//m_volumeProperty->SetInterpolationTypeToCubic();


	//-----一般 环境光系数+散射光系数+反射光系数=1.0,  提供亮度可以大于1.0
	m_volumeProperty->SetAmbient(0.4);//环境光系数
	m_volumeProperty->SetDiffuse(0.5);//散射光系数
	m_volumeProperty->SetSpecular(0.2);//反射光系数
	m_volumeProperty->SetSpecularPower(1.0);//高光强度
	//------------------------------------------------------------------
	if (m_MainWindow->m_checkOpacity)
	{
		m_volumeProperty->DisableGradientOpacityOff();//关闭梯度透明度
	}
	else
	{
		m_volumeProperty->DisableGradientOpacityOn();//关闭梯度透明度
	}


	m_volumeMapper->SetInputData(imageData);
    //m_volumeMapper->SetBlendModeToIsoSurface();//
    m_volumeMapper->SetBlendModeToComposite();
	m_volumeMapper->SetRequestedRenderModeToDefault();

	// force the mapper to compute a sample distance based on data spacing
	m_volumeMapper->SetSampleDistance(-1.0);

	//m_volumeMapper->SetRequestedRenderModeToGPU(); // 强制使用 GPU
	m_isosurfaceFilter->SetInputData(imageData);
    if (m_MainWindow->m_sampleDistanceCheck)
    {
        //m_volumeMapper->SetRequestedRenderModeToRayCast();
        m_volumeMapper->SetSampleDistance(m_MainWindow->m_sampleDistance);
    }

	m_renderer->SetBackground(0, 0, 0);
	m_renderer->ResetCamera();
	m_renderer->GetActiveCamera()->Zoom(1.5);
	//重设相机的剪切范围；
	m_renderer->ResetCameraClippingRange();

	//透明度参数
	m_renderer->SetUseDepthPeeling(1);
	m_renderer->SetMaximumNumberOfPeels(100);
	m_renderer->SetOcclusionRatio(0.1);
    //
	ui->m_image3DView->renderWindow()->AddRenderer(m_renderer);
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

    if (m_vtkColorGradient)
    {
        m_vtkColorGradient->hide();
        delete m_vtkColorGradient;
    }
    delete ui;
}

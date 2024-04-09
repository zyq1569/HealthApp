#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



//https://blog.csdn.net/l1783111653/article/details/108457374
///
///
///
#pragma once
//#include "pch.h"
//有些头文件可能没用到
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

#include <vtkImageData.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkRendererCollection.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkDICOMImageReader.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkAxesActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageViewer2.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageFlip.h>
#include <vtkImageGradient.h>
#include <vtkImageMagnitude.h>
#include <vtkImageShiftScale.h>
#include <vtkImageHybridMedian2D.h>
#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkSliderWidget.h>
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkWidgetEvent.h>
#include <vtkCallbackCommand.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSliderRepresentation2D.h>

#include "itkGDCMSeriesFileNames.h"
#include "itkVTKImageToImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageSeriesWriter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkCastImageFilter.h"
#include "itksys/SystemTools.hxx"
#include "itkImageRegionIterator.h"
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkPNGImageIOFactory.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryFillholeImageFilter.h"
#include "itkGrayscaleFillholeImageFilter.h"
#include "itkMeanImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkExtractImageFilter.h"

#include <vector>
#include <string>



///
///
///



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pBITK_clicked();

    void on_pBDir_clicked();

    void on_pBITK3D_clicked();

    void on_pBVolume3D_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H




#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "transferfunction.h"
//https://blog.csdn.net/l1783111653/article/details/108457374
///
///
///不畏人知畏己知
#pragma once

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class vtkActor;
class QStandardItemModel;
class vtkRenderer;
class vtkRenderWindow;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkVolumeProperty;
class vtkSmartVolumeMapper;
class vtkVolume;
class vtkOutlineFilter;
class vtkPolyDataMapper;
class vtkActor; class vtkRenderWindowInteractor;
//class vtkInteractorStyleTrackballCamera;
class vtkLODProp3D;
class vtkInteractorStyleTrackballCameraWindowleve;
class QVTKOpenGLNativeWidget;
class vtkObject;
class vtkCommand;
using namespace udg;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
	void initImage3D_ITK_VTK(vtkActor *vtkactor);
	void showImage3D_ITK_VTK();
    void loadRenderingStyles();
    HCURSOR QCursorToHCursor(const QCursor &qCursor);

private:
    /// Model that saves rendering styles.
    QStandardItemModel *m_renderingStyleModel;
private:
    vtkRenderer *m_rendererViewer;
    vtkRenderWindow *m_renderWindow;
    vtkPiecewiseFunction *m_opacityTransform;
    vtkColorTransferFunction *m_colorTransformFunction;
    vtkPiecewiseFunction *m_gradientTransform;
    vtkVolumeProperty *m_volumeProperty;
    vtkSmartVolumeMapper *m_volumeMapper;
    vtkVolume *m_volume;
    vtkOutlineFilter *m_outlineData;
    vtkPolyDataMapper *m_mapOutline;
    vtkActor *m_outline;
    vtkRenderWindowInteractor *m_renderWindowInteractor;
    //vtkInteractorStyleTrackballCamera *m_interactorstyle;
    vtkInteractorStyleTrackballCameraWindowleve *m_interactorstyle;
    vtkLODProp3D *m_lodProp3D;
    /// The widget to display a vtk window with qt
    QVTKOpenGLNativeWidget *m_vtkWidget;
    bool m_bWL;
    void free3Dviewer();
public:
    TransferFunction m_transferFunction;

public:
    void saveHDMdata(vtkSmartPointer<vtkImageData> itkImageData);

private slots:
    void applyRenderingStyle(const QModelIndex &index);

private slots:
    void on_pBITK_clicked();

    void on_pBDir_clicked();

    void on_pBITK3D_clicked();

    void on_pBVolume3D_clicked();

    void eventHandler(vtkObject * object, unsigned long vtkEvent, void * clientData, void * callData, vtkCommand * command);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H




// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#ifndef QtVTKRenderWindows_H
#define QtVTKRenderWindows_H

#include "vtkDistanceWidget.h"
#include "vtkImagePlaneWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkSmartPointer.h"
#include <QMainWindow>
#include <QFutureWatcher>

class vtkCornerAnnotation;
class vtkResliceCursorLineRepresentation;
class vtkMPRResliceImageViewer;
class vtkResliceCursorCallback;
class vtkMetaImageReader;

// Forward Qt class declarations
class Ui_QtVTKRenderWindows;

class QtVTKRenderWindows : public QMainWindow
{
  Q_OBJECT
public:
  // Constructor/Destructor
  QtVTKRenderWindows(int argc, char* argv[]);
  ~QtVTKRenderWindows() override {}

public:
	vtkCornerAnnotation *m_cornerAnnotations[3];

public Q_SLOTS:

  virtual void slotExit();
  virtual void resliceMode(int);
  virtual void thickMode(int);
  virtual void SetBlendModeToMaxIP();
  virtual void SetBlendModeToMinIP();
  virtual void SetBlendModeToMeanIP();
  virtual void SetBlendMode(int);
  virtual void ResetViews();
  virtual void Render();
  virtual void AddDistanceMeasurementToView1();
  virtual void AddDistanceMeasurementToView(int);
  virtual void StarCPR();
  virtual void initDir();

protected:
  //vtkSmartPointer<vtkResliceImageViewer> riw[3];
  vtkSmartPointer<vtkMPRResliceImageViewer> riw[3];
  vtkSmartPointer<vtkImageViewer2> m_cprViewer;
	
  vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
  vtkSmartPointer<vtkDistanceWidget> DistanceWidget[3];
  vtkSmartPointer<vtkResliceImageViewerMeasurements> ResliceMeasurements;

public:
    void initMPR();
    void showCPRimageSlicer(vtkImageData * itkImageData);
    void showSRVimageSlicer(vtkImageData * itkImageData);
    void showVolumeImageSlicer(vtkImageData * itkImageData);
    void processing( vtkResliceImageViewer *viewer, std::vector<std::array<double, 3>> m_points, int channel = 0);
    vtkSmartPointer<vtkMetaImageReader> m_mHDreader;

//signals:
//    void mappingFinished(const std::vector<vtkSmartPointer<vtkImageData>> &result);

private:
   QFutureWatcher<std::vector<vtkSmartPointer<vtkImageData>>> m_watcher;
   QFuture<std::vector<vtkSmartPointer<vtkImageData>>> m_future;
    //QFutureWatcher<std::vector<vtkSmartPointer<vtkImageData>>> m_watcher;

public:
	vtkSmartPointer< vtkResliceCursorLineRepresentation > vtkrclp[3];

protected Q_SLOTS:

private slots:

private:
  // Designer form
  Ui_QtVTKRenderWindows* ui;
public:
    vtkSmartPointer <vtkResliceCursorCallback> m_cbk;
private:
    vtkImageData * m_itkImageData;
};

#endif // QtVTKRenderWindows_H

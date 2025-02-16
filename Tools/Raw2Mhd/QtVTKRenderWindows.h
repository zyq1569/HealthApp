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

class vtkCornerAnnotation;
class vtkResliceCursorLineRepresentation;
class vtkMPRResliceImageViewer;
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

protected:
  //vtkSmartPointer<vtkResliceImageViewer> riw[3];
  vtkSmartPointer<vtkMPRResliceImageViewer> riw[3];
	
  vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
  vtkSmartPointer<vtkDistanceWidget> DistanceWidget[3];
  vtkSmartPointer<vtkResliceImageViewerMeasurements> ResliceMeasurements;

public:
	vtkSmartPointer< vtkResliceCursorLineRepresentation > vtkrclp[3];

protected Q_SLOTS:

private slots:
  void on_pushButton_clicked();
  void on_Sagittal_clicked();
  void raw2mhd();
private:
  // Designer form
  Ui_QtVTKRenderWindows* ui;
};

#endif // QtVTKRenderWindows_H

#ifndef QFOURPANEVIEWER_H
#define QFOURPANEVIEWER_H


#include "vtkDistanceWidget.h"
#include "vtkImagePlaneWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkSmartPointer.h"

class vtkGenericOpenGLRenderWindow;
class vtkMetaImageReader;

#include <QWidget>

namespace Ui {
class QFourpaneviewer;
}

class QFourpaneviewer : public QWidget
{
    Q_OBJECT

public:
    explicit QFourpaneviewer(QWidget *parent = nullptr, vtkMetaImageReader* metaReader = nullptr);

    ~QFourpaneviewer();

private:
    Ui::QFourpaneviewer *ui;

public slots:
	void ResetViewer();

public:
	QString m_fileNameMhd;
	vtkResliceImageViewer*        m_resliceImageViewer[3];
	vtkImagePlaneWidget*          m_planeWidget[3];
	vtkGenericOpenGLRenderWindow* m_renderWindow[3];
	QAction *m_actionReset;
	vtkMetaImageReader* m_MetaReader;
	void INimage3D();
};

#endif // QFOURPANEVIEWER_H

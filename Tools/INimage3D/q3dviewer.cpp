#include "q3dviewer.h"
#include "ui_q3dviewer.h"

#include <vtkImageData.h>
#include <vtkActor.h>
#include <vtkImageMarchingCubes.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPlanes.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <vtkMetaImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

Q3dviewer::Q3dviewer(QWidget *parent, QString fileMhd) :
    QWidget(parent), m_fileMHd(fileMhd), ui(new Ui::Q3dviewer)
{
    ui->setupUi(this);

	m_volumeMapper = vtkSmartVolumeMapper::New();

	m_isosurfaceFilter = vtkImageMarchingCubes::New();

	m_volumeProperty = vtkVolumeProperty::New();
	m_volumeProperty->SetInterpolationTypeToLinear();

	m_isosurfaceActor = vtkActor::New();

	m_renderer = vtkRenderer::New();

	vtkNew<vtkPolyDataMapper> isosurfaceMapper;
	isosurfaceMapper->SetInputConnection(m_isosurfaceFilter->GetOutputPort());
	m_isosurfaceActor->SetMapper(isosurfaceMapper);

	INimage3D();
}

Q3dviewer::~Q3dviewer()
{
	m_volumeMapper->Delete();
	m_volumeProperty->Delete();
	m_vtkVolume->Delete();

	m_isosurfaceFilter->Delete();
	m_isosurfaceActor->Delete();
	m_renderer->Delete();

    delete ui;
}

void Q3dviewer::INimage3D()
{
	QString fileMhd = m_fileMHd;
	std::string filename = qPrintable(fileMhd);

	vtkNew<vtkMetaImageReader> reader;
	reader->SetFileName(filename.c_str());
	reader->Update();
	int imageDims[3];
	vtkImageData *imageData = reader->GetOutput();
	imageData->GetDimensions(imageDims);

	m_volumeMapper->SetInputData(reader->GetOutput());

	// force the mapper to compute a sample distance based on data spacing
	m_volumeMapper->SetSampleDistance(-1.0);
	m_isosurfaceFilter->SetInputData(imageData);


}

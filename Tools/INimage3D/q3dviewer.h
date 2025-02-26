#ifndef Q3DVIEWER_H
#define Q3DVIEWER_H


class vtkActor;
class vtkImageData;
class vtkImageMarchingCubes;
class vtkPlanes;
class vtkSmartVolumeMapper;
class vtkVolume;
class vtkVolumeProperty;
class vtkRenderer;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkMetaImageReader;
class MainWindow;
class vtkPolyDataMapper;

#include <QWidget>

namespace Ui {
class Q3dviewer;
}

class Q3dviewer : public QWidget
{
    Q_OBJECT

public:
    explicit Q3dviewer(QWidget *parent = nullptr, vtkMetaImageReader* metaReader = nullptr);
    ~Q3dviewer();

private:
    Ui::Q3dviewer *ui;

public:
	/// The main mapper for volume rendering.
	vtkSmartVolumeMapper *m_volumeMapper;
	/// Properties of volume rendering.
	vtkVolumeProperty *m_volumeProperty;
	/// The volume actor.
	vtkVolume *m_vtkVolume;

	/// The filter to compute isosurfaces.
	vtkImageMarchingCubes *m_isosurfaceFilter;
	/// The actor for isosurfaces.
	vtkActor *m_isosurfaceActor;

	QString m_fileMHd;

	/// Renderer per defecte
	vtkRenderer *m_renderer;
	vtkPiecewiseFunction *m_pieceF, *m_pieceGradF;
	vtkColorTransferFunction *m_colorTranF;
	vtkPolyDataMapper* m_isosurfaceMapper;
	vtkMetaImageReader* m_MetaReader;
	MainWindow* m_mainwindow;

	void INimage3D();
};

#endif // Q3DVIEWER_H

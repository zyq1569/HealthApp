#ifndef SHOWBOUNDS3D_H
#define SHOWBOUNDS3D_H

#include <QWidget>

#include "volumeparams.h"

namespace Ui {
    class ShowBounds3D;
}


enum class VolumeBlendMode
{
    Composite,
    MIP
};

struct ColorPoint
{
    double scalar;
    double r;
    double g;
    double b;
};

struct OpacityPoint
{
    double scalar;
    double opacity;
};

class VolumeRenderParams
{
public:
    // 顏色傳輸函數
    std::vector<ColorPoint> colorPoints;
    // 透明度傳輸函數
    std::vector<OpacityPoint> opacityPoints;
    // 光照
    bool shade = true;
    double ambient = 0.3;
    double diffuse = 0.7;
    double specular = 0.2;
    double specularPower = 10.0;
    // 插值
    bool linearInterpolation = true;
    // 混合模式
    VolumeBlendMode blendMode = VolumeBlendMode::Composite;
    // 採樣距離
    double sampleDistance = 1.0;
    // 可見性
    bool visible = true;
};
class VolumeErrorObserver;
class VolumeRenderResource;

class vtkImageData;
class vtkRenderer;
class vtkRenderWindow;
class ShowBounds3D : public QWidget
{
    Q_OBJECT
public:
    explicit ShowBounds3D(QWidget *parent = nullptr);
    ~ShowBounds3D();


public:
    void ReadVolumes(vtkImageData *imageData);
    void ShowSegmentedVolumes(vtkImageData* imageData, const std::vector<Bounds3D>& partitions, vtkRenderer* renderer);

    bool RenderAllVolumes(const std::vector<vtkImageData*>& imageDataList, const std::vector<VolumeRenderParams>& paramsList,
        vtkRenderer* renderer, vtkRenderWindow* renderWindow, std::vector<std::string>& errorMessages, std::vector<std::shared_ptr<VolumeRenderResource>>& resources);
    static bool CreateOneVolume(int index, vtkImageData* imageData, const VolumeRenderParams& params, vtkRenderer* renderer,
        std::shared_ptr<VolumeRenderResource>& resource, std::string& errorMessage);
    bool UpdateOneVolume(int volumeIndex, VolumeRenderResource& resource, const VolumeRenderParams& newParams, vtkRenderWindow* renderWindow, std::string& errorMessage);

public slots:
    bool UpdateParams(Bounds3D *bounds3D, VolumeRenderParams *params);
public:
    std::vector<vtkImageData*> m_imageDataList;
    std::vector<VolumeRenderParams> m_paramsList;
    std::vector<std::string> m_errorMessages;
    std::vector<std::shared_ptr<VolumeRenderResource>> m_resources;

public:
    VolumeParams m_volumeParamsUI;
private:
    Ui::ShowBounds3D *ui;
};

#endif // SHOWBOUNDS3D_H

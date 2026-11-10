#include "showbounds3d.h"
#include "ui_showbounds3d.h"

///+++++++++++++++++++++
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkCommand.h>

#include <vtkRenderWindow.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageData.h>
#include <vtkCommand.h>
#include <vtkPointData.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkExtractVOI.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>

#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <string>
#include <stdexcept>
#include <QColor>
#include <array>
#include <random>
#include <cstdint>

class VolumeErrorObserver : public vtkCommand
{
public:

    static VolumeErrorObserver* New()
    {
        return new VolumeErrorObserver;
    }
    vtkTypeMacro(VolumeErrorObserver, vtkCommand);
    void SetErrorMessage(std::string* message)
    {
        m_message = message;
    }
    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override
    {
        if (!m_message)
            return;
        if (eventId != vtkCommand::ErrorEvent &&            eventId != vtkCommand::WarningEvent)
            return;
        std::string message;
        if (callData)
        {
            message = static_cast<const char*>(callData);
        }
        else
        {
            message = "Unknown VTK error.";
        }
        if (caller)
        {
            message = std::string(caller->GetClassName()) + ": " + message;
        }
        *m_message += message + "\n";
    }
private:
    std::string* m_message = nullptr;
};

class VolumeRenderResource
{
public:
    int volumeIndex = -1;

    vtkSmartPointer<vtkImageData> imageData;

    vtkSmartPointer<vtkSmartVolumeMapper> mapper;

    vtkSmartPointer<vtkVolumeProperty> property;

    vtkSmartPointer<vtkColorTransferFunction> color;

    vtkSmartPointer<vtkPiecewiseFunction> opacity;

    vtkSmartPointer<vtkVolume> volume;

    vtkSmartPointer<VolumeErrorObserver> observer;

    std::string errorMessage;

    bool valid = false;
};

// Bounds:
// {xmin, xmax, ymin, ymax, zmin, zmax}

// 判斷兩個長方體是否重疊
static bool IsOverlap(const Bounds3D& a, const Bounds3D& b)
{
    return !(a[1] < b[0] || a[0] > b[1] || a[3] < b[2] || a[2] > b[3] || a[5] < b[4] || a[4] > b[5]);
}

// 隨機生成不重疊的三維分割體
std::vector<Bounds3D> GenerateRandomPartitions(const int DimSize[3], int N, bool coverAll = true, uint32_t seed = std::random_device{}())
{
    if (!DimSize || N <= 0)
        throw std::invalid_argument("Invalid parameters");

    for (int i = 0; i < 3; ++i)
    {
        if (DimSize[i] <= 0)
            throw std::invalid_argument("Invalid DimSize");
    }

    int64_t totalVoxels = static_cast<int64_t>(DimSize[0]) *        DimSize[1] * DimSize[2];

    if (N > totalVoxels)
        throw std::invalid_argument("N too large");

    std::mt19937 rng(seed);

    Bounds3D full = {
        0, DimSize[0] - 1,
        0, DimSize[1] - 1,
        0, DimSize[2] - 1
    };

    std::vector<Bounds3D> result;

    // =================================================
    // 模式一：完整覆蓋原始影像
    // =================================================
    if (coverAll)
    {
        result.push_back(full);

        while ((int)result.size() < N)
        {
            std::vector<int> candidates;

            for (int i = 0; i < (int)result.size(); ++i)
            {
                const auto& b = result[i];

                if (b[0] < b[1] || b[2] < b[3] || b[4] < b[5])
                {
                    candidates.push_back(i);
                }
            }

            if (candidates.empty())
                throw std::runtime_error("Cannot split");

            std::uniform_int_distribution<int> boxDist(0, (int)candidates.size() - 1);

            int index = candidates[boxDist(rng)];

            Bounds3D box = result[index];

            std::vector<int> axes;

            for (int axis = 0; axis < 3; ++axis)
            {
                if (box[axis * 2] < box[axis * 2 + 1])
                    axes.push_back(axis);
            }

            std::uniform_int_distribution<int> axisDist(0, (int)axes.size() - 1);

            int axis = axes[axisDist(rng)];

            int minIndex = axis * 2;
            int maxIndex = axis * 2 + 1;

            std::uniform_int_distribution<int> splitDist(box[minIndex], box[maxIndex] - 1);

            int split = splitDist(rng);

            Bounds3D box1 = box;
            Bounds3D box2 = box;

            box1[maxIndex] = split;
            box2[minIndex] = split + 1;

            result[index] = box1;
            result.push_back(box2);
        }
    }
    // =================================================
    // 模式二：允許存在空閒體素
    // =================================================
    else
    {
        // 每個分割體隨機生成
        // 並檢查是否與已生成區域重疊

        std::uniform_int_distribution<int> xDist(0, DimSize[0] - 1);

        std::uniform_int_distribution<int> yDist(0, DimSize[1] - 1);

        std::uniform_int_distribution<int> zDist(0, DimSize[2] - 1);

        // 隨機長度不超過對應維度
        std::uniform_int_distribution<int> dxDist(1, DimSize[0]);

        std::uniform_int_distribution<int> dyDist(1, DimSize[1]);

        std::uniform_int_distribution<int> dzDist(1, DimSize[2]);

        int maxAttempts = 10000;

        for (int i = 0; i < N; ++i)
        {
            bool success = false;

            for (int attempt = 0; attempt < maxAttempts; ++attempt)
            {
                int x0 = xDist(rng);
                int y0 = yDist(rng);
                int z0 = zDist(rng);

                int x1 = std::min(x0 + dxDist(rng) - 1, DimSize[0] - 1);

                int y1 = std::min(y0 + dyDist(rng) - 1, DimSize[1] - 1);

                int z1 = std::min(z0 + dzDist(rng) - 1, DimSize[2] - 1);

                Bounds3D candidate = { x0, x1,  y0, y1,   z0, z1
                };
                bool overlap = false;
                for (const auto& old : result)
                {
                    if (IsOverlap(candidate, old))
                    {
                        overlap = true;
                        break;
                    }
                }

                if (!overlap)
                {
                    result.push_back(candidate);
                    success = true;
                    break;
                }
            }

            if (!success)
            {
                throw std::runtime_error("Unable to generate enough non-overlapping partitions");
            }
        }
    }
    std::shuffle(result.begin(), result.end(), rng);
    return result;
}///------------------------------------------------

// Bounds:
// {xmin, xmax, ymin, ymax, zmin, zmax}

ShowBounds3D::ShowBounds3D(QWidget *parent) : QWidget(parent), ui(new Ui::ShowBounds3D)
{
    ui->setupUi(this);
}

ShowBounds3D::~ShowBounds3D()
{
    delete ui;
}

void ShowBounds3D::ReadVolumes(vtkImageData *imageData)
{
    int DimSize[3];
    int *xyz = imageData->GetDimensions();
    DimSize[0] = xyz[0];
    DimSize[1] = xyz[1];
    DimSize[2] = xyz[2];
    std::vector<Bounds3D> bounds = GenerateRandomPartitions(DimSize, 5, false);
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    ui->m_bound3DView->renderWindow()->AddRenderer(renderer);
    ShowSegmentedVolumes(imageData, bounds, renderer);

    m_volumeParamsUI.UpdateUI(this, bounds);
    m_volumeParamsUI.show();
}

//展示自動分割的體數據
void ShowBounds3D::ShowSegmentedVolumes(vtkImageData* imageData, const std::vector<Bounds3D>& partitions, vtkRenderer* renderer)
{
    if (!imageData || !renderer)
        throw std::invalid_argument("Invalid input");

    int dims[3];
    imageData->GetDimensions(dims);

    if (partitions.empty())
        return;

    // 不同分割體使用不同顏色
    const int colorCount = 12;

    const double colors[colorCount][3] = {
        {1.0, 0.0, 0.0},        {0.0, 1.0, 0.0},        {0.0, 0.0, 1.0},        {1.0, 1.0, 0.0},        {1.0, 0.0, 1.0},
        {0.0, 1.0, 1.0},        {1.0, 0.5, 0.0},        {0.5, 0.0, 1.0},        {0.5, 1.0, 0.0},        {1.0, 0.5, 0.5},      {0.5, 0.5, 1.0},        {0.0, 0.8, 0.5} };

    if (1)
    {
        m_imageDataList.clear();
        m_paramsList.clear();
        for (size_t i = 0; i < partitions.size(); ++i)
        {
            const auto& b = partitions[i];
            // 邊界檢查
            if (b[0] < 0 || b[1] >= dims[0] || b[2] < 0 || b[3] >= dims[1] || b[4] < 0 || b[5] >= dims[2] || b[0] > b[1] || b[2] > b[3] || b[4] > b[5])
            {
                throw std::out_of_range("Partition bounds out of image range");
            }
            // 提取分割體
            auto extract = vtkExtractVOI::New();// vtkSmartPointer<vtkExtractVOI>::New();
            extract->SetInputData(imageData);
            extract->SetVOI(b[0], b[1], b[2], b[3], b[4], b[5]);
            extract->Update();
            m_imageDataList.push_back(extract->GetOutput());

            VolumeRenderParams params;
            ColorPoint point;
            point.scalar = 0.0;
            point.r = point.g = point.b = 0.0;
            params.colorPoints.push_back(point);
            point.scalar = 255;
            point.r = colors[i % colorCount][0];
            point.g = colors[i % colorCount][1];
            point.b = colors[i % colorCount][2];
            params.colorPoints.push_back(point);

            OpacityPoint opoint;
            opoint.opacity = 0;
            opoint.scalar = 0;

            OpacityPoint opacitypoint;
            opacitypoint.opacity = 0.0;
            opacitypoint.scalar = 0.0;
            params.opacityPoints.push_back(opacitypoint);
            opacitypoint.opacity = 0.0;
            opacitypoint.scalar = 1.0;
            params.opacityPoints.push_back(opacitypoint);
            opacitypoint.opacity = 0.35;
            opacitypoint.scalar = 255.0;
            params.opacityPoints.push_back(opacitypoint);

            m_paramsList.push_back(params);
        }
        bool flag = RenderAllVolumes(m_imageDataList, m_paramsList, renderer, renderer->GetRenderWindow(), m_errorMessages, m_resources);
    }
    else  //簡單繪製每個小體數據
    {
        for (size_t i = 0; i < partitions.size(); ++i)
        {
            const auto& b = partitions[i];
            // 邊界檢查
            if (b[0] < 0 || b[1] >= dims[0] || b[2] < 0 || b[3] >= dims[1] || b[4] < 0 || b[5] >= dims[2] || b[0] > b[1] || b[2] > b[3] || b[4] > b[5])
            {
                throw std::out_of_range("Partition bounds out of image range");
            }
            // 提取分割體
            auto extract = vtkSmartPointer<vtkExtractVOI>::New();
            extract->SetInputData(imageData);
            extract->SetVOI(b[0], b[1], b[2], b[3], b[4], b[5]);
            extract->Update();

            // 體繪製 Mapper
            auto mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
            mapper->SetInputConnection(extract->GetOutputPort());
            // 顏色映射
            auto color = vtkSmartPointer<vtkColorTransferFunction>::New();
            color->AddRGBPoint(0, 0.0, 0.0, 0.0);
            color->AddRGBPoint(255, colors[i % colorCount][0], colors[i % colorCount][1], colors[i % colorCount][2]);
            // 透明度映射
            auto opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
            opacity->AddPoint(0, 0.0);
            opacity->AddPoint(1, 0.0);
            opacity->AddPoint(255, 0.35);
            // Volume Property
            auto property = vtkSmartPointer<vtkVolumeProperty>::New();
            property->SetColor(color);
            property->SetScalarOpacity(opacity);
            property->SetInterpolationTypeToLinear();
            property->ShadeOn();
            // Volume
            auto volume = vtkSmartPointer<vtkVolume>::New();

            volume->SetMapper(mapper);
            volume->SetProperty(property);

            renderer->AddVolume(volume);
        }

        renderer->ResetCamera();
        renderer->GetRenderWindow()->Render();
    }

}

bool ShowBounds3D::CreateOneVolume(int index, vtkImageData* imageData, const VolumeRenderParams& params, vtkRenderer* renderer, std::shared_ptr<VolumeRenderResource>& resource, std::string& errorMessage)
{
    try
    {
        if (!imageData)
        {
            errorMessage = "ImageData is null.";
            return false;
        }
        if (!renderer)
        {
            errorMessage = "Renderer is null.";
            return false;
        }
        if (!imageData->GetPointData() || !imageData->GetPointData()->GetScalars())
        {
            errorMessage = "ImageData Scalars are invalid.";
            return false;
        }
        if (params.colorPoints.empty() || params.opacityPoints.empty())
        {
            errorMessage = "Transfer function points are empty.";

            return false;
        }
        if (params.sampleDistance <= 0.0 || !std::isfinite(params.sampleDistance))
        {
            errorMessage = "Invalid sample distance.";
            return false;
        }
        if (!std::isfinite(params.ambient) || !std::isfinite(params.diffuse) ||
            !std::isfinite(params.specular) || !std::isfinite(params.specularPower))
        {
            errorMessage = "Invalid lighting parameters.";
            return false;
        }
        if (params.ambient < 0.0 || params.ambient > 1.0 || params.diffuse < 0.0 || params.diffuse > 1.0 ||
            params.specular < 0.0 || params.specular > 1.0 || params.specularPower < 0.0)
        {
            errorMessage = "Lighting parameters out of range.";
            return false;
        }
        auto item = std::make_shared<VolumeRenderResource>();
        item->volumeIndex = index;
        item->imageData = imageData;
        item->mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
        item->property = vtkSmartPointer<vtkVolumeProperty>::New();
        item->color = vtkSmartPointer<vtkColorTransferFunction>::New();
        item->opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
        item->volume = vtkSmartPointer<vtkVolume>::New();
        item->mapper->SetInputData(imageData);
        // 顏色傳輸函數
        for (const auto& p : params.colorPoints)
        {
            if (!std::isfinite(p.scalar) || !std::isfinite(p.r) || !std::isfinite(p.g) || !std::isfinite(p.b) ||
                p.r < 0.0 || p.r > 1.0 || p.g < 0.0 || p.g > 1.0 || p.b < 0.0 || p.b > 1.0)
            {
                errorMessage = "Invalid color transfer point.";
                return false;
            }
            item->color->AddRGBPoint(p.scalar, p.r, p.g, p.b);
        }
        // 透明度傳輸函數
        for (const auto& p : params.opacityPoints)
        {
            if (!std::isfinite(p.scalar) || !std::isfinite(p.opacity) || p.opacity < 0.0 || p.opacity > 1.0)
            {
                errorMessage = "Invalid opacity transfer point.";
                return false;
            }
            item->opacity->AddPoint(p.scalar, p.opacity);
        }
// Property
        item->property->SetColor(item->color);
        item->property->SetScalarOpacity(item->opacity);
        item->property->SetInterpolationType(params.linearInterpolation ? VTK_LINEAR_INTERPOLATION : VTK_NEAREST_INTERPOLATION);
        item->property->SetShade(params.shade);
        item->property->SetAmbient(params.ambient);
        item->property->SetDiffuse(params.diffuse);
        item->property->SetSpecular(params.specular);
        item->property->SetSpecularPower(params.specularPower);
        // Mapper
        item->mapper->SetSampleDistance(params.sampleDistance);
        if (params.blendMode == VolumeBlendMode::Composite)
        {
            item->mapper->SetBlendModeToComposite();
        }
        else
        {
            item->mapper->SetBlendModeToMaximumIntensity();
        }
        // Volume
        item->volume->SetMapper(item->mapper);
        item->volume->SetProperty(item->property);
        item->volume->SetVisibility(params.visible);
        // Observer
        item->observer = vtkSmartPointer<VolumeErrorObserver>::New();
        item->observer->SetErrorMessage(&item->errorMessage);
        item->mapper->AddObserver(vtkCommand::ErrorEvent, item->observer);
        item->mapper->AddObserver(vtkCommand::WarningEvent, item->observer);
        item->valid = true;
        resource = item;
        return true;
    }
    catch (const std::exception& e)
    {
        errorMessage = e.what();
        return false;
    }
    catch (...)
    {
        errorMessage = "Unknown exception while creating Volume.";
        return false;
    }
}

//繪製分割的體數據.支持每個體數據單獨調整參數.顏色 透明度 .等參數
bool ShowBounds3D::RenderAllVolumes(const std::vector<vtkImageData*>& imageDataList, const std::vector<VolumeRenderParams>& paramsList, vtkRenderer* renderer,
    vtkRenderWindow* renderWindow, std::vector<std::string>& errorMessages, std::vector<std::shared_ptr<VolumeRenderResource>>& resources)
{
    errorMessages.clear();
    resources.clear();
    if (!renderer || !renderWindow)
    {
        errorMessages.push_back("Renderer or RenderWindow is null.");
        return false;
    }
    if (imageDataList.empty())
    {
        errorMessages.push_back("ImageData list is empty.");
        return false;
    }
    if (imageDataList.size() != paramsList.size())
    {
        errorMessages.push_back("ImageData count does not match params count.");
        return false;
    }
    bool allSuccess = true;
    // 第一階段：獨立創建所有 Volume Pipeline
    for (size_t i = 0; i < imageDataList.size(); ++i)
    {
        std::string error;
        std::shared_ptr<VolumeRenderResource> resource;
        bool success = CreateOneVolume(static_cast<int>(i), imageDataList[i], paramsList[i], renderer, resource, error);
        if (!success)
        {
            allSuccess = false;
            errorMessages.push_back("Volume[" + std::to_string(i) + "] initialization failed: " + error);
            continue;
        }
        resources.push_back(resource);
    }
    // 第二階段：將成功創建的 Volume 添加到場景
    for (const auto& resource : resources)
    {
        if (!resource || !resource->valid)
            continue;
        try
        {
            renderer->AddVolume(resource->volume);
        }
        catch (const std::exception& e)
        {
            allSuccess = false;
            errorMessages.push_back("Volume[" + std::to_string(resource->volumeIndex) + "] AddVolume failed: " + e.what());
            resource->valid = false;
            renderer->RemoveVolume(resource->volume);
        }
        catch (...)
        {
            allSuccess = false;
            errorMessages.push_back("Volume[" + std::to_string(resource->volumeIndex) + "] AddVolume unknown exception.");
            resource->valid = false;
            renderer->RemoveVolume(resource->volume);
        }
    }
    // 第三階段：統一渲染
    try
    {
        renderWindow->Render();
    }
    catch (const std::exception& e)
    {
        allSuccess = false;
        errorMessages.push_back(std::string("Render exception: ") + e.what());
    }
    catch (...)
    {
        allSuccess = false;
        errorMessages.push_back("Unknown Render exception.");
    }
    // 第四階段：檢查每個 Pipeline 的錯誤信息
    for (const auto& resource : resources)
    {
        if (!resource)
            continue;
        if (!resource->errorMessage.empty())
        {
            allSuccess = false;
            errorMessages.push_back("Volume[" + std::to_string(resource->volumeIndex) + "] VTK error: " + resource->errorMessage);
            // 只移除當前異常 Volume
            if (resource->volume)
            {
                renderer->RemoveVolume(resource->volume);
            }
            resource->valid = false;
        }
    }
    // 最後刷新一次，保留正常 Volume
    if (!allSuccess)
    {
        try
        {
            renderWindow->Render();
        }
        catch (...)
        {
            errorMessages.push_back("Final Render failed.");
        }
    }

    return allSuccess;
}

bool ShowBounds3D::UpdateOneVolume(int volumeIndex, VolumeRenderResource& resource, const VolumeRenderParams& newParams, vtkRenderWindow* renderWindow, std::string& errorMessage)
{
    errorMessage.clear();

    // 1. 檢查 Volume 索引
    if (volumeIndex < 0)
    {
        errorMessage = "Invalid volume index.";
        return false;
    }

    if (resource.volumeIndex != volumeIndex)
    {
        errorMessage = "Volume index does not match resource.";
        return false;
    }

    // 2. 檢查資源有效性
    if (!resource.valid)
    {
        errorMessage = "Volume resource is not valid.";
        return false;
    }

    if (!resource.imageData || !resource.mapper || !resource.property || !resource.color || !resource.opacity || !resource.volume)
    {
        errorMessage = "Volume resource contains null VTK objects.";
        return false;
    }

    if (!renderWindow)
    {
        errorMessage = "Render window is null.";
        return false;
    }

    // 3. 檢查採樣距離
    if (!std::isfinite(newParams.sampleDistance) || newParams.sampleDistance <= 0.0)
    {
        errorMessage = "Invalid sample distance.";
        return false;
    }

    // 4. 檢查光照參數
    if (!std::isfinite(newParams.ambient) || !std::isfinite(newParams.diffuse) || !std::isfinite(newParams.specular) ||
        !std::isfinite(newParams.specularPower))
    {
        errorMessage = "Invalid lighting parameters.";
        return false;
    }

    if (newParams.ambient < 0.0 || newParams.diffuse < 0.0 || newParams.specular < 0.0 || newParams.specularPower < 0.0)
    {
        errorMessage = "Lighting parameters cannot be negative.";
        return false;
    }

    // 5. 檢查顏色控制點
    if (newParams.colorPoints.empty())
    {
        errorMessage = "Color transfer points are empty.";
        return false;
    }

    for (const auto& point : newParams.colorPoints)
    {
        if (!std::isfinite(point.scalar) || !std::isfinite(point.r) || !std::isfinite(point.g) || !std::isfinite(point.b))
        {
            errorMessage = "Invalid color transfer point.";
            return false;
        }

        if (point.r < 0.0 || point.r > 1.0 || point.g < 0.0 || point.g > 1.0 || point.b < 0.0 || point.b > 1.0) {
            errorMessage = "Color values must be in [0, 1].";
            return false;
        }
    }

    // 6. 檢查透明度控制點
    if (newParams.opacityPoints.empty())
    {
        errorMessage = "Opacity transfer points are empty.";
        return false;
    }

    for (const auto& point : newParams.opacityPoints)
    {
        if (!std::isfinite(point.scalar) || !std::isfinite(point.opacity))
        {
            errorMessage = "Invalid opacity transfer point.";
            return false;
        }

        if (point.opacity < 0.0 || point.opacity > 1.0)
        {
            errorMessage = "Opacity must be in [0, 1].";
            return false;
        }
    }

    // 7. 更新顏色傳輸函數
    resource.color->RemoveAllPoints();

    for (const auto& point : newParams.colorPoints)
    {
        resource.color->AddRGBPoint(point.scalar, point.r, point.g, point.b);
    }

    // 8. 更新透明度傳輸函數
    resource.opacity->RemoveAllPoints();

    for (const auto& point : newParams.opacityPoints)
    {
        resource.opacity->AddPoint(point.scalar, point.opacity);
    }

    // 9. 更新 Volume Property
    resource.property->SetColor(resource.color);
    resource.property->SetScalarOpacity(resource.opacity);

    resource.property->SetShade(newParams.shade ? 1 : 0);

    resource.property->SetAmbient(newParams.ambient);

    resource.property->SetDiffuse(newParams.diffuse);

    resource.property->SetSpecular(newParams.specular);

    resource.property->SetSpecularPower(newParams.specularPower);

    // 插值方式
    if (newParams.linearInterpolation)
    {
        resource.property->SetInterpolationTypeToLinear();
    }
    else
    {
        resource.property->SetInterpolationTypeToNearest();
    }

    // 10. 更新混合模式
    switch (newParams.blendMode)
    {
    case VolumeBlendMode::Composite:
        resource.mapper->SetBlendModeToComposite();
        break;

        // case VolumeBlendMode::MaximumIntensity:
        //     resource.mapper->SetBlendModeToMaximumIntensity();
        //     break;
        //
        // case VolumeBlendMode::MinimumIntensity:
        //     resource.mapper->SetBlendModeToMinimumIntensity();
        //     break;
        //
        // case VolumeBlendMode::Additive:
        //     resource.mapper->SetBlendModeToAdditive();
        //     break;

    default:
        errorMessage = "Unsupported volume blend mode.";
        return false;
    }

    // 11. 更新採樣距離
    resource.mapper->SetSampleDistance(static_cast<float>(newParams.sampleDistance));

    // 12. 更新可見性
    resource.volume->SetVisibility(newParams.visible ? 1 : 0);

    // 13. 確保 Volume 使用更新後的 Property
    resource.volume->SetProperty(resource.property);

    // 14. 標記 VTK 資源修改
    resource.color->Modified();
    resource.opacity->Modified();
    resource.property->Modified();
    resource.mapper->Modified();
    resource.volume->Modified();

    // 15. 請求重新渲染
    renderWindow->Render();

    return true;
}

bool ShowBounds3D::UpdateParams(Bounds3D bounds3D, const VolumeRenderParams& newParams)
{


    return true;

}
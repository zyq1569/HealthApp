#include "showbounds3d.h"
#include "ui_showbounds3d.h"


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
#include <array>
#include <algorithm>
#include <stdexcept>

#include <QColor>

#include <vector>
#include <array>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cstdint>



// Bounds:
// {xmin, xmax, ymin, ymax, zmin, zmax}

// 判斷兩個長方體是否重疊
static bool IsOverlap(const Bounds3D& a, const Bounds3D& b)
{
    return !(   a[1] < b[0] || a[0] > b[1] ||        a[3] < b[2] || a[2] > b[3] ||        a[5] < b[4] || a[4] > b[5]        );
}

// 隨機生成不重疊的三維分割體
std::vector<Bounds3D> GenerateRandomPartitions(const int DimSize[3],  int N,  bool coverAll = true,    uint32_t seed = std::random_device{}())
{
    if (!DimSize || N <= 0)
        throw std::invalid_argument("Invalid parameters");

    for (int i = 0; i < 3; ++i)
    {
        if (DimSize[i] <= 0)
            throw std::invalid_argument("Invalid DimSize");
    }

    int64_t totalVoxels =        static_cast<int64_t>(DimSize[0]) *        DimSize[1] * DimSize[2];

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

                if (b[0] < b[1] ||   b[2] < b[3] ||    b[4] < b[5])
                {
                    candidates.push_back(i);
                }
            }

            if (candidates.empty())
                throw std::runtime_error("Cannot split");

            std::uniform_int_distribution<int> boxDist(  0, (int)candidates.size() - 1);

            int index = candidates[boxDist(rng)];

            Bounds3D box = result[index];

            std::vector<int> axes;

            for (int axis = 0; axis < 3; ++axis)
            {
                if (box[axis * 2] < box[axis * 2 + 1])
                    axes.push_back(axis);
            }

            std::uniform_int_distribution<int> axisDist( 0, (int)axes.size() - 1);

            int axis = axes[axisDist(rng)];

            int minIndex = axis * 2;
            int maxIndex = axis * 2 + 1;

            std::uniform_int_distribution<int> splitDist(  box[minIndex], box[maxIndex] - 1);

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

        std::uniform_int_distribution<int> xDist( 0, DimSize[0] - 1);

        std::uniform_int_distribution<int> yDist( 0, DimSize[1] - 1);

        std::uniform_int_distribution<int> zDist( 0, DimSize[2] - 1);

        // 隨機長度不超過對應維度
        std::uniform_int_distribution<int> dxDist( 1, DimSize[0]);

        std::uniform_int_distribution<int> dyDist( 1, DimSize[1]);

        std::uniform_int_distribution<int> dzDist( 1, DimSize[2]);

        int maxAttempts = 10000;

        for (int i = 0; i < N; ++i)
        {
            bool success = false;

            for (int attempt = 0;  attempt < maxAttempts;  ++attempt)
            {
                int x0 = xDist(rng);
                int y0 = yDist(rng);
                int z0 = zDist(rng);

                int x1 = std::min( x0 + dxDist(rng) - 1,  DimSize[0] - 1);

                int y1 = std::min( y0 + dyDist(rng) - 1,  DimSize[1] - 1);

                int z1 = std::min( z0 + dzDist(rng) - 1,  DimSize[2] - 1);

                Bounds3D candidate = {  x0, x1,  y0, y1,   z0, z1
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
                throw std::runtime_error(
                    "Unable to generate enough "
                    "non-overlapping partitions");
            }
        }
    }
    std::shuffle(result.begin(), result.end(), rng);
    return result;
}///------------------------------------------------

// Bounds:
// {xmin, xmax, ymin, ymax, zmin, zmax}


ShowBounds3D::ShowBounds3D(QWidget *parent) :    QWidget(parent),    ui(new Ui::ShowBounds3D)
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
    int *xyz  = imageData->GetDimensions();
    DimSize[0] = xyz[0];
    DimSize[1] = xyz[1];
    DimSize[2] = xyz[2];
    std::vector<Bounds3D> bounds = GenerateRandomPartitions(DimSize, 3,false);
    vtkSmartPointer<vtkRenderer> renderer =        vtkSmartPointer<vtkRenderer>::New();
    ui->m_bound3DView->renderWindow()->AddRenderer(renderer);
    ShowSegmentedVolumes(imageData, bounds, renderer);
}

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
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0},
        {1.0, 1.0, 0.0},
        {1.0, 0.0, 1.0},
        {0.0, 1.0, 1.0},
        {1.0, 0.5, 0.0},
        {0.5, 0.0, 1.0},
        {0.5, 1.0, 0.0},
        {1.0, 0.5, 0.5},
        {0.5, 0.5, 1.0},
        {0.0, 0.8, 0.5}
    };

    for (size_t i = 0; i < partitions.size(); ++i)
    {
        const auto& b = partitions[i];

        // 邊界檢查
        if (b[0] < 0 || b[1] >= dims[0] ||
            b[2] < 0 || b[3] >= dims[1] ||
            b[4] < 0 || b[5] >= dims[2] ||
            b[0] > b[1] ||
            b[2] > b[3] ||
            b[4] > b[5])
        {
            throw std::out_of_range(
                "Partition bounds out of image range");
        }

        // 提取分割體
        auto extract =
            vtkSmartPointer<vtkExtractVOI>::New();

        extract->SetInputData(imageData);

        extract->SetVOI(
            b[0], b[1],
            b[2], b[3],
            b[4], b[5]);

        extract->Update();

        // 體繪製 Mapper
        auto mapper =
            vtkSmartPointer<vtkSmartVolumeMapper>::New();

        mapper->SetInputConnection(
            extract->GetOutputPort());

        // 顏色映射
        auto color =
            vtkSmartPointer<vtkColorTransferFunction>::New();

        color->AddRGBPoint(
            0, 0.0, 0.0, 0.0);

        color->AddRGBPoint(
            255,
            colors[i % colorCount][0],
            colors[i % colorCount][1],
            colors[i % colorCount][2]);

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

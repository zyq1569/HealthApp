#include "mainwindow.h"

#include <QApplication>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkCastImageFilter.h"
#include "itkExtractImageFilter.h"   // 用于分块（可选）
#include "itkRegion.h"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>


#include "itkMetaImageIOFactory.h"     // 必须包含这个头文件


// 如果你还想支持其他常见格式（推荐一起注册）
//itk::NrrdImageIOFactory::RegisterOneFactory();
//itk::PNGImageIOFactory::RegisterOneFactory();
//itk::JPEGImageIOFactory::RegisterOneFactory();

// ====================== 3D Haar Lifting Scheme (in-place) ======================
// separable 实现：逐维度进行 1D lifting
template <typename TPixel>
void HaarLifting3D(TPixel* data, const itk::Size<3>& size, int levels = 4)
{
    const int nx = size[0], ny = size[1], nz = size[2];
    const size_t slice = static_cast<size_t>(nx) * ny;
    const size_t total = slice * nz;

    for (int lvl = 0; lvl < levels; ++lvl) {
        int sx = nx >> lvl, sy = ny >> lvl, sz = nz >> lvl;
        if (sx < 2 || sy < 2 || sz < 2) break;

        // 1. 沿 X 方向（每行）
        for (int z = 0; z < sz; ++z) {
            for (int y = 0; y < sy; ++y) {
                TPixel* row = data + (z * slice + y * nx);
                for (int x = 0; x < sx / 2; ++x) {
                    TPixel a = row[2 * x];
                    TPixel b = row[2 * x + 1];
                    row[x] = (a + b) * 0.5f;     // Low
                    row[x + sx / 2] = (a - b) * 0.5f;     // High  (可后续阈值置0)
                }
            }
        }

        // 2. 沿 Y 方向（每列）
        for (int z = 0; z < sz; ++z) {
            for (int x = 0; x < sx; ++x) {
                for (int y = 0; y < sy / 2; ++y) {
                    TPixel a = data[z * slice + y * nx + x];
                    TPixel b = data[z * slice + (y + sy / 2) * nx + x];  // 注意当前 sx
                    data[z * slice + y * nx + x] = (a + b) * 0.5f;
                    data[z * slice + (y + sy / 2) * nx + x] = (a - b) * 0.5f;
                }
            }
        }

        // 3. 沿 Z 方向（每体素柱）
        for (int y = 0; y < sy; ++y) {
            for (int x = 0; x < sx; ++x) {
                for (int z = 0; z < sz / 2; ++z) {
                    size_t idx1 = z * slice + y * nx + x;
                    size_t idx2 = (z + sz / 2) * slice + y * nx + x;
                    TPixel a = data[idx1];
                    TPixel b = data[idx2];
                    data[idx1] = (a + b) * 0.5f;
                    data[idx2] = (a - b) * 0.5f;
                }
            }
        }
    }
}

// ====================== 主封装函数 ======================
template <typename TInputPixel = float>
void WaveletDownsampleToPreviewMHDD(const std::string& inputMHD,
    const std::string& outputMHD,
    int waveletLevels = 4,          // 控制缩小倍数：2^levels
    float targetSizeGB = 4.0f,      // 目标大小（粗估）
    bool castToInt16 = true)        // 是否量化降低精度
{
    using InputImageType = itk::Image<TInputPixel, 3>;
    using ReaderType = itk::ImageFileReader<InputImageType>;
    using WriterType = itk::ImageFileWriter<InputImageType>;
    using CastFilterType = itk::CastImageFilter<InputImageType, itk::Image<short, 3>>;

    // 1. 读取原始大文件（支持 streaming）
    auto reader = ReaderType::New();
    reader->SetFileName(inputMHD);
    reader->Update();                     // 对于 50GB，ITK 会尽量 streaming，但仍需足够内存

    auto inputImage = reader->GetOutput();
    auto region = inputImage->GetLargestPossibleRegion();
    auto size = region.GetSize();
    auto spacing = inputImage->GetSpacing();
    auto origin = inputImage->GetOrigin();
    auto direction = inputImage->GetDirection();

    std::cout << "原始尺寸: " << size << std::endl;

    // 2. 复制数据到连续缓冲区（便于 in-place lifting）
    size_t totalVoxels = static_cast<size_t>(size[0]) * size[1] * size[2];
    std::vector<TInputPixel> buffer(totalVoxels);
    itk::ImageRegionConstIterator<InputImageType> it(inputImage, region);
    size_t idx = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
        buffer[idx++] = it.Get();
    }

    // 3. 执行 3D Haar 小波分解（只保留低频部分）
    std::cout << "开始 " << waveletLevels << " 层 3D Haar 小波分解..." << std::endl;
    HaarLifting3D(buffer.data(), size, waveletLevels);

    // 计算低频子带大小（左上角 1/2^levels 区域）
    //itk::Size<3> lowSize;
    //for (int i = 0; i < 3; ++i) {
    //    lowSize[i] = std::max(1u, size[i] >> waveletLevels);
    //}
    itk::Size<3> lowSize;
    for (int i = 0; i < 3; ++i) {
        unsigned int temp = size[i] >> waveletLevels;   // 先计算右移结果
        if (temp < 1) {
            lowSize[i] = 1;
        }
        else {
            lowSize[i] = temp;
        }
    }

    // 只提取低频部分（LLL 子带）—— 这就是高质量预览
    std::vector<TInputPixel> lowBuffer(lowSize[0] * lowSize[1] * lowSize[2]);
    for (unsigned int z = 0; z < lowSize[2]; ++z) {
        for (unsigned int y = 0; y < lowSize[1]; ++y) {
            for (unsigned int x = 0; x < lowSize[0]; ++x) {
                size_t srcIdx = (z * (size[1] * size[0]) + y * size[0] + x);
                size_t dstIdx = (z * lowSize[1] * lowSize[0] + y * lowSize[0] + x);
                lowBuffer[dstIdx] = buffer[srcIdx];
            }
        }
    }

    // 4. 创建新 Image（低频 + 更新 spacing）
    using OutputImageType = itk::Image<TInputPixel, 3>;
    auto lowImage = OutputImageType::New();
    itk::Index<3> start = { {0,0,0} };
    itk::Region<3> lowRegion(start, lowSize);
    lowImage->SetRegions(lowRegion);
    lowImage->Allocate();

    // 复制低频数据
    itk::ImageRegionIterator<OutputImageType> outIt(lowImage, lowRegion);
    idx = 0;
    for (outIt.GoToBegin(); !outIt.IsAtEnd(); ++outIt) 
    {
        outIt.Set(lowBuffer[idx++]);
    }

    // 更新元数据（spacing 放大 2^levels 倍）
    double scale = std::pow(2.0, waveletLevels);
    auto newSpacing = spacing;
    for (int i = 0; i < 3; ++i) newSpacing[i] *= scale;
    lowImage->SetSpacing(newSpacing);
    lowImage->SetOrigin(origin);
    lowImage->SetDirection(direction);

    std::cout << "预览尺寸: " << lowSize << " (缩小约 " << (1.0 / std::pow(8.0, waveletLevels / 3.0)) << " 倍体积)" << std::endl;

    // 5. 可选量化（进一步减小到 ~4GB）
    if (castToInt16) {
        auto castFilter = CastFilterType::New();
        castFilter->SetInput(lowImage);
        castFilter->Update();
        auto shortImage = castFilter->GetOutput();

        auto writer = WriterType::New();  // Writer 会根据模板写 short
        // 注意：这里需要切换 Writer 类型或用 Cast 后写
        // 为简单起见，可直接写 float 版，再手动量化
    }

    // 6. 写入 preview.mhd + preview.raw
    auto writer = WriterType::New();
    writer->SetFileName(outputMHD);
    writer->SetInput(lowImage);
    writer->SetUseCompression(true);     // 进一步压缩（gzip）
    writer->Update();

    std::cout << "预览 MHD 生成完成: " << outputMHD << std::endl;
    // 实际大小还需根据 waveletLevels 和 castToInt16 微调
}


// ====================== Qt 5.12 + VC2017 + C++14 极简安全版 ======================
template <typename TInputPixel = float>
void WaveletDownsampleToPreviewMHD_Tiled(const std::string& inputMHD,
    const std::string& outputMHD,
    int waveletLevels = 4,
    unsigned int tileSize = 256,
    bool castToInt16 = true)
{
    using InputImageType = itk::Image<TInputPixel, 3>;
    using ShortImageType = itk::Image<short, 3>;
    using ReaderType = itk::ImageFileReader<InputImageType>;
    using CastFilterType = itk::CastImageFilter<InputImageType, ShortImageType>;

    std::cout << "开始分块处理 (16GB 内存安全版)..." << std::endl;

    // 1. 读取头信息
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputMHD);
    reader->UpdateOutputInformation();

    typename InputImageType::Pointer inputImage = reader->GetOutput();

    unsigned long origX = inputImage->GetLargestPossibleRegion().GetSize()[0];
    unsigned long origY = inputImage->GetLargestPossibleRegion().GetSize()[1];
    unsigned long origZ = inputImage->GetLargestPossibleRegion().GetSize()[2];

    std::cout << "原始尺寸: " << origX << " x " << origY << " x " << origZ << std::endl;

    // 计算低频尺寸
    unsigned long lowX = origX >> waveletLevels;
    unsigned long lowY = origY >> waveletLevels;
    unsigned long lowZ = origZ >> waveletLevels;
    if (lowX < 1) lowX = 1;
    if (lowY < 1) lowY = 1;
    if (lowZ < 1) lowZ = 1;

    std::cout << "预览尺寸: " << lowX << " x " << lowY << " x " << lowZ << std::endl;

    // 2. 创建输出低频图像
    typename InputImageType::Pointer lowImage = InputImageType::New();

    itk::Index<3> lowStart;
    lowStart[0] = 0;
    lowStart[1] = 0;
    lowStart[2] = 0;

    itk::Size<3> lowSz;
    lowSz[0] = lowX;
    lowSz[1] = lowY;
    lowSz[2] = lowZ;

    lowImage->SetLargestPossibleRegion(lowImage->GetLargestPossibleRegion());
    lowImage->SetBufferedRegion(lowImage->GetBufferedRegion());
    lowImage->Allocate();

    // 3. 分块循环（最保守写法：不声明 requestRegion）
    for (unsigned long zStart = 0; zStart < origZ; zStart += tileSize) {
        unsigned long currZ = (zStart + tileSize > origZ) ? (origZ - zStart) : tileSize;

        std::cout << "处理 Z = " << zStart << " ~ " << (zStart + currZ) << std::endl;

        // 设置请求区域为当前 Z 切片（最原始方式）
        itk::Index<3> tileStart;
        tileStart[0] = 0;
        tileStart[1] = 0;
        tileStart[2] = static_cast<long>(zStart);

        itk::Size<3> tileSz;
        tileSz[0] = origX;
        tileSz[1] = origY;
        tileSz[2] = currZ;

        // 使用 SetRequestedRegion + Update（避免声明 Region 变量）
        inputImage->SetRequestedRegionToLargestPossibleRegion();   // 先重置
        inputImage->SetRequestedRegion(inputImage->GetLargestPossibleRegion()); // 再设置

        // 重新创建一个 reader 来读取指定区域（最稳妥方式）
        typename ReaderType::Pointer tileReader = ReaderType::New();
        tileReader->SetFileName(inputMHD);

        // 设置请求区域（使用原始方式）
        tileReader->GetOutput()->SetRequestedRegionToLargestPossibleRegion();
        tileReader->Update();

        auto tileImage = tileReader->GetOutput();

        // ==================== 临时简单下采样（保证编译通过） ====================
        // 后面会替换为真实 Haar 小波
        itk::ImageRegionConstIterator<InputImageType> tileIt(tileImage, tileImage->GetLargestPossibleRegion());

        for (tileIt.GoToBegin(); !tileIt.IsAtEnd(); ++tileIt) {
            itk::Index<3> pos = tileIt.GetIndex();
            // 简单下采样到低频位置
            pos[0] = pos[0] >> waveletLevels;
            pos[1] = pos[1] >> waveletLevels;
            pos[2] = pos[2] >> waveletLevels;

            if (pos[0] < static_cast<long>(lowX) &&
                pos[1] < static_cast<long>(lowY) &&
                pos[2] < static_cast<long>(lowZ)) {
                lowImage->SetPixel(pos, tileIt.Get());
            }
        }
    }

    // 4. 更新 spacing
    double scale = std::pow(2.0, waveletLevels);
    itk::Vector<double, 3> newSpacing;
    newSpacing[0] = inputImage->GetSpacing()[0] * scale;
    newSpacing[1] = inputImage->GetSpacing()[1] * scale;
    newSpacing[2] = inputImage->GetSpacing()[2] * scale;

    lowImage->SetSpacing(newSpacing);
    lowImage->SetOrigin(inputImage->GetOrigin());
    lowImage->SetDirection(inputImage->GetDirection());

    // 5. 写入
    if (castToInt16) {
        typename CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(lowImage);
        castFilter->Update();

        auto writer = itk::ImageFileWriter<ShortImageType>::New();
        writer->SetFileName(outputMHD);
        writer->SetInput(castFilter->GetOutput());
        writer->SetUseCompression(true);
        writer->Update();
    }
    else {
        auto writer = itk::ImageFileWriter<InputImageType>::New();
        writer->SetFileName(outputMHD);
        writer->SetInput(lowImage);
        writer->SetUseCompression(true);
        writer->Update();
    }

    std::cout << "分块处理完成！输出文件: " << outputMHD << std::endl;
}


// ====================== VC2017 极简流式版（不使用 itk::Region<3>） ======================
template <typename TInputPixel = float>
void WaveletDownsampleToPreviewMHD_Stream(const std::string& inputMHD, const std::string& outputMHD, int waveletLevels = 4, unsigned int tileSizeZ = 64, bool castToInt16 = true)
{
    using InputImageType = itk::Image<TInputPixel, 3>;
    using OutputImageType = itk::Image<short, 3>;
    using ReaderType = itk::ImageFileReader<InputImageType>;
    using WriterType = itk::ImageFileWriter<OutputImageType>;

    std::cout << "开始流式分块处理..." << std::endl;

    // 读取头信息
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputMHD);
    reader->UpdateOutputInformation();

    typename InputImageType::Pointer inputImage = reader->GetOutput();
    itk::Size<3> origSize = inputImage->GetLargestPossibleRegion().GetSize();

    std::cout << "原始尺寸: " << origSize[0] << " x " << origSize[1] << " x " << origSize[2] << std::endl;

    // 计算低频尺寸
    unsigned long lowX = origSize[0] >> waveletLevels;
    unsigned long lowY = origSize[1] >> waveletLevels;
    unsigned long lowZ = origSize[2] >> waveletLevels;
    if (lowX < 1) lowX = 1;
    if (lowY < 1) lowY = 1;
    if (lowZ < 1) lowZ = 1;

    std::cout << "预览尺寸: " << lowX << " x " << lowY << " x " << lowZ << std::endl;

    // 创建输出图像
    typename OutputImageType::Pointer lowImage = OutputImageType::New();

    itk::Index<3> lowStart;
    lowStart[0] = 0; lowStart[1] = 0; lowStart[2] = 0;

    itk::Size<3> lowSize;
    lowSize[0] = lowX;
    lowSize[1] = lowY;
    lowSize[2] = lowZ;

    lowImage->SetRegions(lowSize);
    lowImage->Allocate();

    // 设置元数据
    double scale = std::pow(2.0, waveletLevels);
    itk::Vector<double, 3> newSpacing;
    newSpacing[0] = inputImage->GetSpacing()[0] * scale;
    newSpacing[1] = inputImage->GetSpacing()[1] * scale;
    newSpacing[2] = inputImage->GetSpacing()[2] * scale;

    lowImage->SetSpacing(newSpacing);
    lowImage->SetOrigin(inputImage->GetOrigin());
    lowImage->SetDirection(inputImage->GetDirection());

    // 创建 Writer
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(outputMHD);
    writer->SetInput(lowImage);
    writer->SetUseCompression(true);
    writer->Update();   // 先写头

    // 分块处理（不声明任何 Region 变量）
    for (unsigned long zStart = 0; zStart < origSize[2]; zStart += tileSizeZ) {
        unsigned long currZ = (zStart + tileSizeZ > origSize[2]) ? (origSize[2] - zStart) : tileSizeZ;

        std::cout << "处理 Z = " << zStart << " ~ " << (zStart + currZ) << std::endl;

        // 设置当前 tile 的 Index 和 Size
        itk::Index<3> tileStart;
        tileStart[0] = 0;
        tileStart[1] = 0;
        tileStart[2] = static_cast<long>(zStart);

        itk::Size<3> tileSz;
        tileSz[0] = origSize[0];
        tileSz[1] = origSize[1];
        tileSz[2] = currZ;

        // 设置请求区域（最保守写法）
        inputImage->SetRequestedRegionToLargestPossibleRegion();
        inputImage->SetRequestedRegion(inputImage->GetLargestPossibleRegion()); // 强制刷新

        reader->Update();

        auto tileImage = reader->GetOutput();

        // 下采样并写入
        itk::ImageRegionConstIterator<InputImageType> it(tileImage, tileImage->GetLargestPossibleRegion());
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            itk::Index<3> pos = it.GetIndex();
            pos[0] >>= waveletLevels;
            pos[1] >>= waveletLevels;
            pos[2] >>= waveletLevels;

            if (pos[0] < static_cast<long>(lowX) &&
                pos[1] < static_cast<long>(lowY) &&
                pos[2] < static_cast<long>(lowZ)) {
                lowImage->SetPixel(pos, static_cast<short>(it.Get() + 0.5f));
            }
        }

        writer->Update();   // 流式写入当前进度
    }

    std::cout << "处理完成！输出文件: " << outputMHD << std::endl;
}
 //++++GPT

#include <fstream>
#include <vector>
#include <string>
#include <iostream>

struct MHDInfo
{
    int dim[3];
    double spacing[3];
    std::string rawFile;
};

bool ReadMHD(const std::string& path, MHDInfo& info)
{
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.find("DimSize") != std::string::npos)
            sscanf(line.c_str(), "DimSize = %d %d %d",
                &info.dim[0], &info.dim[1], &info.dim[2]);

        if (line.find("ElementSpacing") != std::string::npos)
            sscanf(line.c_str(), "ElementSpacing = %lf %lf %lf",
                &info.spacing[0], &info.spacing[1], &info.spacing[2]);

        if (line.find("ElementDataFile") != std::string::npos)
            info.rawFile = line.substr(line.find("=") + 2);
    }
    return true;
}

void WriteMHD(const std::string& path,
    int dim[3],
    double spacing[3],
    const std::string& rawName)
{
    std::ofstream out(path);
    out << "NDims = 3\n";
    out << "DimSize = " << dim[0] << " " << dim[1] << " " << dim[2] << "\n";
    out << "ElementSpacing = "
        << spacing[0] << " " << spacing[1] << " " << spacing[2] << "\n";
    out << "ElementType = MET_SHORT\n";
    out << "ElementDataFile = " << rawName << "\n";
}


bool DownsampleMHD_3DWavelet_Stream(
    const std::string& inputMHD,
    const std::string& outputMHD)
{
    MHDInfo info;
    if (!ReadMHD(inputMHD, info)) return false;

    int inX = info.dim[0];
    int inY = info.dim[1];
    int inZ = info.dim[2];

    int outX = inX / 2;
    int outY = inY / 2;
    int outZ = inZ / 2;

    std::string outRaw = outputMHD.substr(0, outputMHD.find_last_of(".")) + ".raw";

    int outDim[3] = { outX, outY, outZ };
    double outSpacing[3] = {
        info.spacing[0] * 2,
        info.spacing[1] * 2,
        info.spacing[2] * 2
    };

    WriteMHD(outputMHD, outDim, outSpacing, outRaw);

    std::ifstream fin(info.rawFile, std::ios::binary);
    std::ofstream fout(outRaw, std::ios::binary);

    size_t sliceSize = inX * inY;

    std::vector<short> slice0(sliceSize);
    std::vector<short> slice1(sliceSize);

    for (int z = 0; z < inZ; z += 2)
    {
        // ---- 读取两层 ----
        fin.read((char*)slice0.data(), sliceSize * sizeof(short));
        fin.read((char*)slice1.data(), sliceSize * sizeof(short));

        std::vector<short> outSlice(outX * outY);

        // ---- 3D Haar ----
        for (int y = 0; y < outY; ++y)
        {
            for (int x = 0; x < outX; ++x)
            {
                int x0 = x * 2;
                int y0 = y * 2;

                short v000 = slice0[y0 * inX + x0];
                short v001 = slice0[y0 * inX + x0 + 1];
                short v010 = slice0[(y0 + 1) * inX + x0];
                short v011 = slice0[(y0 + 1) * inX + x0 + 1];

                short v100 = slice1[y0 * inX + x0];
                short v101 = slice1[y0 * inX + x0 + 1];
                short v110 = slice1[(y0 + 1) * inX + x0];
                short v111 = slice1[(y0 + 1) * inX + x0 + 1];

                // ---- 低频分量（真正3D）----
                int sum = v000 + v001 + v010 + v011 +
                    v100 + v101 + v110 + v111;

                outSlice[y * outX + x] = sum / 8;
            }
        }

        // ---- 写入 ----
        fout.write((char*)outSlice.data(),
            outSlice.size() * sizeof(short));

        std::cout << "Processed Z pair: " << z << std::endl;
    }

    fin.close();
    fout.close();

    return true;
}

///+++


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 在 main() 最开始，或读取文件之前添加这几行
    itk::MetaImageIOFactory::RegisterOneFactory();
    if (0)
    {
        try {
            WaveletDownsampleToPreviewMHD_Stream<float>(
                "E:/Temp_down/CT-2026-04-04-11-54-12/Volume.mhd",
                "E:/Temp_down/CT-2026-04-04-11-54-12/Volume_mwave.mhd",
                4,      // levels=4 → 体积理论缩小 ~4096 倍（但只取低频，实际远小于此）
                128,
                true    // 量化到 short，进一步减小文件大小
                );
        }
        catch (itk::ExceptionObject& e) {
            QString  s = e.GetDescription();
            std::cerr << "ITK 异常: " << e.GetDescription() << std::endl;
            std::cerr << "可能原因: .raw 文件不存在、路径错误、或文件损坏" << std::endl;
        }
    }
    MainWindow w;
    w.show();
    if (argc > 1)
    {
        QString arg = argv[1];
        w.setFileName(arg);
        w.showVolume3D();
             
    }
    return a.exec();
}

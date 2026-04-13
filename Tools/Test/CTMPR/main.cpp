#include "mainwindow.h"

#include <vtkSmartPointer.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkProperty.h>
#include <vtkMatrix4x4.h>
#include <vtkImageMapper3D.h>
#include <vtkCommand.h>

#include <fstream>
#include <vector>
#include <windows.h>
#include <sstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>

#include <QApplication>

// ====================== 支持 X/Y/Z 方向提取 ======================
// orientation: 0=X, 1=Y, 2=Z（默认）
// numLayers: 要提取的层数（默认3）
bool ExtractLayersRaw(const std::string& inputMHDPath, const std::string& outputMHDPath, int orientation = 2, int startIndex = -1, int numLayers = 3)
{
    // 1. 读取 .mhd 头文件
    std::ifstream mhdIn(inputMHDPath);
    if (!mhdIn.is_open())
    {
        std::cerr << "Cannot open .mhd file: " << inputMHDPath << std::endl;
        return false;
    }

    int dims[3] = { 0 };           // X, Y, Z
    double spacing[3] = { 0.0 };
    std::string line;

    while (std::getline(mhdIn, line))
    {
        if (line.find("DimSize") != std::string::npos)
        {
            sscanf(line.c_str(), "DimSize = %d %d %d", &dims[0], &dims[1], &dims[2]);
        }
        else if (line.find("ElementSpacing") != std::string::npos)
        {
            sscanf(line.c_str(), "ElementSpacing = %lf %lf %lf", &spacing[0], &spacing[1], &spacing[2]);
        }
    }
    mhdIn.close();

    if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    {
        std::cerr << "Failed to parse .mhd header!" << std::endl;
        return false;
    }

    // 2. 构造原始 .raw 文件完整路径
    std::string rawFullPath = inputMHDPath;
    if (rawFullPath.size() > 4)
    {
        rawFullPath.replace(rawFullPath.size() - 4, 4, ".raw");
    }

    std::cout << "Reading raw file: " << rawFullPath << std::endl;

    // 3. 计算提取范围
    if (startIndex < 0)
    {
        startIndex = dims[orientation] / 2;
    }
    startIndex = std::max(0, std::min(startIndex, dims[orientation] - numLayers));

    // 计算每层字节数（一层 = 当前方向固定，其他两个方向全尺寸）
    size_t bytesPerSlice = 2ULL;  // unsigned short
    for (int i = 0; i < 3; ++i)
    {
        if (i != orientation)
        {
            bytesPerSlice *= dims[i];
        }
    }

    size_t readOffset = static_cast<size_t>(startIndex) * bytesPerSlice;
    size_t bytesToRead = bytesPerSlice * numLayers;

    std::cout << "Direction: "
        << (orientation == 0 ? "X (Sagittal)" : orientation == 1 ? "Y (Coronal)" : "Z (Axial)")
        << " | Extracting from index " << startIndex << " to " << (startIndex + numLayers - 1) << " (" << numLayers << " layers)" << std::endl;

    // 4. 读取数据
    std::ifstream rawIn(rawFullPath, std::ios::binary);
    if (!rawIn.is_open())
    {
        std::cerr << "Cannot open raw file: " << rawFullPath << std::endl;
        return false;
    }

    rawIn.seekg(readOffset, std::ios::beg);
    if (!rawIn)
    {
        std::cerr << "Seek failed at offset " << readOffset << std::endl;
        return false;
    }

    std::vector<unsigned char> buffer(bytesToRead);
    rawIn.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);

    if (rawIn.gcount() != static_cast<std::streamsize>(bytesToRead))
    {
        std::cerr << "Read incomplete! Expected " << bytesToRead << ", got " << rawIn.gcount() << std::endl;
        return false;
    }
    rawIn.close();

    // 5. 写入新的 .raw 文件（与 outputMHD 同名）
    std::string outRawPath = outputMHDPath;
    if (outRawPath.size() > 4 &&
        (outRawPath.substr(outRawPath.size() - 4) == ".mhd" || outRawPath.substr(outRawPath.size() - 4) == ".MHD"))
    {
        outRawPath.replace(outRawPath.size() - 4, 4, ".raw");
    }
    else
    {
        outRawPath += ".raw";
    }

    std::ofstream rawOut(outRawPath, std::ios::binary);
    rawOut.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    rawOut.close();

    // 6. 写入新的 .mhd 文件
    std::ofstream mhdOut(outputMHDPath);
    mhdOut << "ObjectType = Image\n";
    mhdOut << "NDims = 3\n";
    mhdOut << "DimSize = ";
    for (int i = 0; i < 3; ++i)
    {
        mhdOut << (i == orientation ? numLayers : dims[i]);
        if (i < 2)
            mhdOut << " ";
    }
    mhdOut << "\n";
    mhdOut << "ElementSpacing = " << spacing[0] << " " << spacing[1] << " " << spacing[2] << "\n";
    mhdOut << "ElementType = MET_USHORT\n";
    mhdOut << "ElementByteOrderMSB = False\n";
    mhdOut << "ElementDataFile = " << (outRawPath.find_last_of("\\/") != std::string::npos ?
        outRawPath.substr(outRawPath.find_last_of("\\/") + 1) : outRawPath) << "\n";
    mhdOut.close();

    std::cout << "Success! Saved to:\n" << outputMHDPath << "\n" << outRawPath << std::endl;
    return true;
}

bool ExtractZLayersRaw(const std::string& inputMHDPath, const std::string& outputMHDPath, int zStart = -1, int numLayers = 3)   // -1 表示中心层
{
    // 1. 读取 .mhd 文件头
    std::ifstream mhdIn(inputMHDPath);
    if (!mhdIn.is_open())
    {
        std::cerr << "Cannot open .mhd file: " << inputMHDPath << std::endl;
        return false;
    }

    int dims[3] = { 0 };
    double spacing[3] = { 0.0 };
    std::string rawFileName;
    std::string line;

    while (std::getline(mhdIn, line))
    {
        if (line.find("DimSize") != std::string::npos)
        {
            sscanf(line.c_str(), "DimSize = %d %d %d", &dims[0], &dims[1], &dims[2]);
        }
        else if (line.find("ElementSpacing") != std::string::npos)
        {
            sscanf(line.c_str(), "ElementSpacing = %lf %lf %lf", &spacing[0], &spacing[1], &spacing[2]);
        }
        else if (line.find("ElementDataFile") != std::string::npos)
        {
            size_t pos = line.find('=') + 1;
            rawFileName = line.substr(pos);
            // 去除前后空格和换行
            rawFileName.erase(0, rawFileName.find_first_not_of(" \t"));
            rawFileName.erase(rawFileName.find_last_not_of(" \t\r\n") + 1);
        }
    }
    mhdIn.close();

    if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0 || rawFileName.empty())
    {
        std::cerr << "Failed to parse .mhd header!" << std::endl;
        return false;
    }

    // 2. 构造 .raw 文件的完整路径（关键修改部分）
    std::string rawFullPath;

    // 找到 .mhd 文件的最后一个路径分隔符
    size_t lastSep = inputMHDPath.find_last_of("\\/");
    if (lastSep != std::string::npos)
    {
        // 取 .mhd 所在目录 + rawFileName
        rawFullPath = inputMHDPath.substr(0, lastSep + 1) + rawFileName;
    }
    else
    {
        // .mhd 在当前目录
        rawFullPath = rawFileName;
    }

    std::cout << "Raw file full path: " << rawFullPath << std::endl;

    // 3. 计算层范围和偏移
    if (zStart < 0) zStart = dims[2] / 2;
    zStart = std::max(0, std::min(zStart, dims[2] - numLayers));

    size_t bytesPerSlice = static_cast<size_t>(dims[0]) * dims[1] * 2ULL;  // unsigned short
    size_t readOffset = static_cast<size_t>(zStart) * bytesPerSlice;
    size_t bytesToRead = bytesPerSlice * numLayers;

    std::cout << "Extracting Z layers " << zStart << " to " << (zStart + numLayers - 1) << " (" << numLayers << " layers)" << std::endl;

    // 4. 读取 .raw 文件
    std::ifstream rawIn(rawFullPath, std::ios::binary);
    if (!rawIn.is_open())
    {
        std::cerr << "Cannot open raw file: " << rawFullPath << std::endl;
        return false;
    }

    rawIn.seekg(readOffset, std::ios::beg);
    if (!rawIn)
    {
        std::cerr << "Seek failed at offset " << readOffset << " (file too large?)" << std::endl;
        return false;
    }

    std::vector<unsigned char> buffer(bytesToRead);
    rawIn.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);

    if (rawIn.gcount() != static_cast<std::streamsize>(bytesToRead))
    {
        std::cerr << "Read incomplete! Expected " << bytesToRead << ", actually read " << rawIn.gcount() << std::endl;
        return false;
    }
    rawIn.close();

    // 5. 写入新的 .raw 文件
    std::string outRawPath = outputMHDPath;
    if (outRawPath.size() > 4 && (outRawPath.substr(outRawPath.size() - 4) == ".mhd" || outRawPath.substr(outRawPath.size() - 4) == ".MHD"))
    {
        outRawPath = outRawPath.substr(0, outRawPath.size() - 4) + ".raw";
    }

    std::ofstream rawOut(outRawPath, std::ios::binary);
    rawOut.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    rawOut.close();

    // 6. 写入新的 .mhd 文件
    std::ofstream mhdOut(outputMHDPath);
    mhdOut << "ObjectType = Image\n";
    mhdOut << "NDims = 3\n";
    mhdOut << "DimSize = " << dims[0] << " " << dims[1] << " " << numLayers << "\n";
    mhdOut << "ElementSpacing = " << spacing[0] << " " << spacing[1] << " " << spacing[2] << "\n";
    mhdOut << "ElementType = MET_USHORT\n";
    mhdOut << "ElementByteOrderMSB = False\n";
    mhdOut << "ElementDataFile = " << rawFileName << "\n";   // 只写文件名，更兼容
    mhdOut.close();

    std::cout << "Success! Extracted " << numLayers << " layers from Z=" << zStart << ".\nSaved to: " << outputMHDPath << std::endl;

    return true;
}




//++++++++++++GPT

// ======================
// 自定义 Streaming Reader
// ======================
class RawSliceReader : public vtkImageAlgorithm
{
public:
    static RawSliceReader* New();
    vtkTypeMacro(RawSliceReader, vtkImageAlgorithm);
public:
    std::string m_rawPath;
    int m_Dim[3] = { 0,0,0 };// { 18739, 13714, 100 };
    double m_spacingX = 1.0, m_spacingY = 1.0, m_spacingZ = 1.0;
public:
    int SetFileName(std::string filePath)
    {
        // ---------- 1. 解析 MHD ----------
        int dimX = 0, dimY = 0, dimZ = 0;
        double spacingX = 1.0, spacingY = 1.0, spacingZ = 1.0;
        std::string elementType, rawFile;
        std::ifstream in(filePath);
        if (!in)
        {
            std::cerr << "Failed to open MHD file\n";
            return 0;
        }

        std::string line;
        while (std::getline(in, line))
        {
            if (line.find("DimSize") != std::string::npos)
            {
                sscanf(line.c_str(), "DimSize = %d %d %d", &dimX, &dimY, &dimZ);
            }
            else if (line.find("ElementSpacing") != std::string::npos)
            {
                sscanf(line.c_str(), "ElementSpacing = %lf %lf %lf", &spacingX, &spacingY, &spacingZ);
            }
            else if (line.find("ElementType") != std::string::npos)
            {
                elementType = line.substr(line.find("=") + 2);
            }
            else if (line.find("ElementDataFile") != std::string::npos)
            {
                rawFile = line.substr(line.find("=") + 2);
            }
        }
        m_Dim[0] = dimX;
        m_Dim[1] = dimY;
        m_Dim[2] = dimZ;
        m_spacingX = spacingX, m_spacingY = spacingY, m_spacingZ = spacingZ;
        // ---------- 2. RAW 文件路径 ----------
        std::string baseDir, rawPath;
        size_t pos = rawFile.find_last_of("/\\");
        if (pos > std::string::npos)// 找到 .mhd 文件的最后一个路径分隔符
        {
            m_rawPath = rawFile;
        }
        else
        {
            baseDir = filePath.substr(0, filePath.find_last_of("/\\") + 1);
            rawPath = baseDir + rawFile;
            m_rawPath = rawPath;
        }
    }
protected:
    RawSliceReader()
    {
        this->SetNumberOfInputPorts(0);
    }

    int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector) override
    {
        vtkInformation* outInfo = outputVector->GetInformationObject(0);

        int extent[6] = { 0, m_Dim[0] - 1, 0, m_Dim[1] - 1, 0, m_Dim[2] - 1 };

        outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);
        outInfo->Set(vtkDataObject::SPACING(), 1.0, 1.0, 1.0);
        outInfo->Set(vtkDataObject::ORIGIN(), 0.0, 0.0, 0.0);

        vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_SHORT, 1);

        return 1;
    }

    int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector) override
    {
        vtkInformation* outInfo = outputVector->GetInformationObject(0);
        vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

        int extent[6];
        outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent);

        output->SetExtent(extent);
        output->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

        int xMin = extent[0], xMax = extent[1];
        int yMin = extent[2], yMax = extent[3];
        int zMin = extent[4], zMax = extent[5];

        std::ifstream file(m_rawPath, std::ios::binary);
        size_t sliceSize = m_Dim[0] * m_Dim[1];
        for (int z = zMin; z <= zMax; ++z)
        {
            size_t offset = (size_t)z * sliceSize * sizeof(unsigned short);
            file.seekg(offset, std::ios::beg);

            std::vector<unsigned short> buffer(sliceSize);
            file.read((char*)buffer.data(), sliceSize * sizeof(unsigned short));

            for (int y = yMin; y <= yMax; ++y)
            {
                for (int x = xMin; x <= xMax; ++x)
                {
                    unsigned short* pixel = static_cast<unsigned short*>(output->GetScalarPointer(x, y, z));
                    *pixel = buffer[y * m_Dim[0] + x];
                }
            }
        }
        return 1;
    }

};

vtkStandardNewMacro(RawSliceReader);


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (0)// MHD--->dicom
    {
        MainWindow w0;
        w0.show();
        return a.exec();
    }

    
    //++GPT
    auto reader = vtkSmartPointer<RawSliceReader>::New();
    reader->SetFileName("E:/Temp_down/CT-2026-04-04-11-54-12/Volume.mhd"); // 修改路径

    // ---------- XY ----------
    auto mapperXY = vtkSmartPointer<vtkImageSliceMapper>::New();
    mapperXY->SetInputConnection(reader->GetOutputPort());
    mapperXY->SetOrientationToZ();
    mapperXY->SetSliceNumber(50);
    mapperXY->StreamingOn();

    auto sliceXY = vtkSmartPointer<vtkImageSlice>::New();
    sliceXY->SetMapper(mapperXY);

    // ---------- XZ ----------
    auto mapperXZ = vtkSmartPointer<vtkImageSliceMapper>::New();
    mapperXZ->SetInputConnection(reader->GetOutputPort());
    mapperXZ->SetOrientationToY();
    mapperXZ->SetSliceNumber(256);
    mapperXZ->StreamingOn();

    auto sliceXZ = vtkSmartPointer<vtkImageSlice>::New();
    sliceXZ->SetMapper(mapperXZ);

    // ---------- YZ ----------
    auto mapperYZ = vtkSmartPointer<vtkImageSliceMapper>::New();
    mapperYZ->SetInputConnection(reader->GetOutputPort());
    mapperYZ->SetOrientationToX();
    mapperYZ->SetSliceNumber(256);
    mapperYZ->StreamingOn();

    auto sliceYZ = vtkSmartPointer<vtkImageSlice>::New();
    sliceYZ->SetMapper(mapperYZ);

    // ---------- Renderer ----------
    auto ren1 = vtkSmartPointer<vtkRenderer>::New();
    auto ren2 = vtkSmartPointer<vtkRenderer>::New();
    auto ren3 = vtkSmartPointer<vtkRenderer>::New();

    ren1->SetViewport(0, 0, 0.33, 1);
    ren2->SetViewport(0.33, 0, 0.66, 1);
    ren3->SetViewport(0.66, 0, 1, 1);

    ren1->AddViewProp(sliceXY);
    //ren2->AddViewProp(sliceXZ);
    //ren3->AddViewProp(sliceYZ);

    auto window = vtkSmartPointer<vtkRenderWindow>::New();
    window->SetSize(1200, 400);
    window->AddRenderer(ren1);
   // window->AddRenderer(ren2);
    //window->AddRenderer(ren3);

    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);

    window->Render();
    interactor->Start();

    
    MainWindow w;
    w.show();
    return a.exec();

}

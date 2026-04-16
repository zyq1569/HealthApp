#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
// DCMTK
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcuid.h>

///+++
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>   // for strerror

#ifdef _WIN32
#include <direct.h>   // Windows: _mkdir
#define mkdir(dir, mode) _mkdir(dir)   // Windows 不需要权限参数
#endif
///

// ++++++++++++GPT
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
        return 1;
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
        outInfo->Set(vtkDataObject::SPACING(), m_spacingX, m_spacingY, m_spacingZ);
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
        if (!file)
        {
            std::cerr << "open raw failed\n";
            return 0;
        }
        size_t slicePixels = (size_t)m_Dim[0] * m_Dim[1];
        size_t bytesPerSlice = slicePixels * 2ULL;
        if (file)
        {
            //优化 XY（Z方向连续）
            if (xMin == 0 && xMax == m_Dim[0] - 1 && yMin == 0 && yMax == m_Dim[1] - 1 && zMin == zMax)
            {
                size_t bytesPerSlice = static_cast<size_t>(m_Dim[0]) * m_Dim[1] * 2ULL;  // unsigned short
                size_t readOffset = static_cast<size_t>(zMin) * bytesPerSlice;
                size_t bytesToRead = bytesPerSlice * 1;
                // ---------- 1. 定位 ----------
                size_t offset = (size_t)zMin * bytesPerSlice;
                file.seekg(offset, std::ios::beg);
                // ---------- 2. 直接读 ----------
                unsigned short* outPtr = static_cast<unsigned short*>(output->GetScalarPointer());
                file.read(reinterpret_cast<char*>(outPtr), bytesPerSlice);
                if (file.gcount() != (std::streamsize)bytesPerSlice)
                {
                    std::cerr << "Read incomplete!" << std::endl;
                    return 0;
                }
                return 1;
            }
            // ================= XZ =================
            else if (xMin == 0 && xMax == m_Dim[0] - 1 &&  zMin == 0 && zMax == m_Dim[2] - 1 &&  yMin == yMax)
            {
                int ySlice = yMin;
                unsigned short* outPtr =  static_cast<unsigned short*>(output->GetScalarPointer());
                int outWidth           = m_Dim[0];   // x
                size_t rowBytes        = (size_t)m_Dim[0] * sizeof(unsigned short);
                for (int z = 0; z < m_Dim[2]; ++z)
                {
                    //定位到该行
                    size_t offset = (size_t)z * bytesPerSlice + (size_t)ySlice * rowBytes;  // 第 z 个 slice  + 第 y 行
                    file.seekg(offset, std::ios::beg);
                    //读一整行
                    unsigned short* dst = outPtr + z * outWidth;
                    file.read(reinterpret_cast<char*>(dst), rowBytes);
                    if (file.gcount() != (std::streamsize)rowBytes)
                    {
                        std::cerr << "XZ row read incomplete\n";
                        return 0;
                    }
                }
                return 1;
            }           
            // ================= YZ ================= >>>>>>多存储一组数据 .需要将YZ的切面旋转90度存储，保证和XZ切面至少可以连续一行加载速度快
            else
            {
                // ================= YZ（基于 yx.raw） =================   
                std::ifstream fileYZ(m_rawPath+".yx", std::ios::binary);
                if (!fileYZ)
                {
                    std::cerr << "open raw failed\n";
                    return 0;
                }
                int xSlice = xMin;
                unsigned short* outPtr =    static_cast<unsigned short*>(output->GetScalarPointer());
                int outWidth = m_Dim[1];   // y
                size_t rowBytes = (size_t)m_Dim[1] * sizeof(unsigned short);
                for (int z = 0; z < m_Dim[2]; ++z)
                {
                    // yx.raw 中：slice 内排列为 [x][y] 每个 x 对应一整行 y（连续）
                    size_t offset = (size_t)z * bytesPerSlice +  (size_t)xSlice * rowBytes; // 第 z slice
                    // 第 x 行（连续 y）
                    fileYZ.seekg(offset, std::ios::beg);
                    //直接读整行（连续）
                    unsigned short* dst = outPtr + z * outWidth;
                    fileYZ.read(reinterpret_cast<char*>(dst), rowBytes);
                    if (fileYZ.gcount() != (std::streamsize)rowBytes)
                    {
                        std::cerr << "YZ read incomplete\n";
                        return 0;
                    }
                }
                return 1;
            }
        }
        return 1;
    }

};

vtkStandardNewMacro(RawSliceReader);


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_rendererXY = m_rendererXZ = m_rendererYZ = nullptr;

    m_rendererXY = vtkRenderer::New();
    m_rendererXZ = vtkRenderer::New();
    m_rendererYZ = vtkRenderer::New();

    m_mapperXY =  m_mapperYZ  =  m_mapperXZ = nullptr;
    m_mapperXY = vtkImageSliceMapper::New();
    m_mapperYZ = vtkImageSliceMapper::New();
    m_mapperXZ = vtkImageSliceMapper::New();

    m_imageSliceXY = m_imageSliceXZ = m_imageSliceYZ = nullptr;
    m_imageSliceXY = vtkImageSlice::New();
    m_imageSliceXZ = vtkImageSlice::New();
    m_imageSliceYZ = vtkImageSlice::New();

    m_sliceReader = RawSliceReader::New();

    connect(ui->m_selectPB, SIGNAL(clicked()), SLOT(SelectRaw()));
    connect(ui->m_rawPB, SIGNAL(clicked()), SLOT(Mhd2Stream()));
    connect(ui->m_showImages, SIGNAL(clicked()), SLOT(ShowImages()));
}

// 创建目录（支持多级，类似 mkdir -p）
bool EnsureDirectoryExists(const std::string& dir)
{
    if (dir.empty()) return false;

    struct stat st;
    if (stat(dir.c_str(), &st) == 0)
    {
        if (st.st_mode & S_IFDIR)   // 已存在且是目录
            return true;
        else
            return false;           // 存在但不是目录
    }

    // 逐级创建目录
    size_t pos = 0;
    std::string current;

    // 处理 Windows 和 Linux/Unix 路径分隔符
    std::string path = dir;
    // 可选：统一使用 '/'（Windows 也支持）
    for (char& c : path) {
        if (c == '\\') c = '/';
    }

    while ((pos = path.find('/', pos + 1)) != std::string::npos)
    {
        current = path.substr(0, pos);
        if (!current.empty())
        {
#ifdef _WIN32
            _mkdir(current.c_str());
#else
            mkdir(current.c_str(), 0755);
#endif
        }
    }

    // 创建最后一级目录
#ifdef _WIN32
    int ret = _mkdir(path.c_str());
#else
    int ret = mkdir(path.c_str(), 0755);
#endif

    if (ret == 0 || errno == EEXIST)
        return true;

    std::cerr << "创建目录失败 [" << path << "]: " << strerror(errno) << std::endl;
    return false;
}

void MainWindow::SelectRaw()
{
    m_Mhdfilename = QFileDialog::getOpenFileName(this, "mhd|vti file", QString(), "*.mhd;*.vti");

}

class DataInfo
{
public:
    DataInfo(const std::string& mhdPath, const std::string& outputDir)
    {
        m_inputPath = mhdPath;
        m_outputDir = outputDir;
    }
    std::string m_inputPath;
    std::string m_outputDir;
};

bool MHD_To_CT_Stream(DataInfo info)
{
    std::string mhdPath, outputDir;
    mhdPath = info.m_inputPath;
    outputDir = info.m_outputDir;
    // ------------------------------------------------------------
    // 单函数：MHD + RAW → DICOM 序列（CT）
    // ------------------------------------------------------------
        // ---------- 1. 解析 MHD ----------
    int dimX = 0, dimY = 0, dimZ = 0;
    double spacingX = 1.0, spacingY = 1.0, spacingZ = 1.0;
    std::string elementType;
    std::string rawFile;
    {
        std::ifstream in(mhdPath);
        if (!in)
        {
            std::cerr << "Failed to open MHD file\n";
            return 0;
        }

        std::string line;
        while (std::getline(in, line))
        {
            if (line.find("DimSize") != std::string::npos)
                sscanf(line.c_str(), "DimSize = %d %d %d", &dimX, &dimY, &dimZ);

            else if (line.find("ElementSpacing") != std::string::npos)
                sscanf(line.c_str(), "ElementSpacing = %lf %lf %lf",
                    &spacingX, &spacingY, &spacingZ);

            else if (line.find("ElementType") != std::string::npos)
                elementType = line.substr(line.find("=") + 2);

            else if (line.find("ElementDataFile") != std::string::npos)
                rawFile = line.substr(line.find("=") + 2);
        }
    }

    if (elementType != "MET_USHORT")
    {
        std::cerr << "Only MET_USHORT supported in this demo\n";
        return 0;
    }

    // ---------- 2. RAW 文件路径 ----------
    std::string baseDir = mhdPath.substr(0, mhdPath.find_last_of("/\\") + 1);
    std::string rawPath = baseDir + rawFile;

    std::ifstream raw(rawPath, std::ios::binary);
    if (!raw)
    {
        std::cerr << "Failed to open RAW file\n";
        return 0;
    }

    // ---------- 3. UID ----------
    char studyUID[100];
    char seriesUID[100];
    dcmGenerateUniqueIdentifier(studyUID, SITE_STUDY_UID_ROOT);
    dcmGenerateUniqueIdentifier(seriesUID, SITE_SERIES_UID_ROOT);

    // ---------- 4. slice buffer ----------
    size_t pixelBytes = 2;
    size_t sliceBytes = (size_t)dimX * dimY * pixelBytes;

    std::vector<Uint16> buffer(dimX * dimY);

    // ---------- 5. 循环写每个 slice ----------
    for (int z = 0; z < dimZ; ++z)
    {
        // --- 读取一层 ---
        raw.seekg((size_t)z * sliceBytes, std::ios::beg);
        raw.read(reinterpret_cast<char*>(buffer.data()), sliceBytes);

        if (!raw)
        {
            std::cerr << "Read error at slice " << z << "\n";
            break;
        }

        // --- 创建 DICOM ---
        DcmFileFormat file;
        DcmDataset* ds = file.getDataset();

        // ===== SOP =====
        ds->putAndInsertString(DCM_SOPClassUID, UID_CTImageStorage);

        char sopUID[100];
        dcmGenerateUniqueIdentifier(sopUID, SITE_INSTANCE_UID_ROOT);
        ds->putAndInsertString(DCM_SOPInstanceUID, sopUID);

        // ===== Study / Series =====
        ds->putAndInsertString(DCM_StudyInstanceUID, studyUID);
        ds->putAndInsertString(DCM_SeriesInstanceUID, seriesUID);

        // ===== 基本信息 =====
        ds->putAndInsertString(DCM_PatientName, "Test^CT");
        ds->putAndInsertString(DCM_PatientID, "123456");

        ds->putAndInsertString(DCM_Modality, "CT");

        // ===== 图像尺寸 =====
        ds->putAndInsertUint16(DCM_Rows, dimY);
        ds->putAndInsertUint16(DCM_Columns, dimX);

        ds->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME2");

        // ===== 像素格式 =====
        ds->putAndInsertUint16(DCM_BitsAllocated, 16);
        ds->putAndInsertUint16(DCM_BitsStored, 16);
        ds->putAndInsertUint16(DCM_HighBit, 15);
        ds->putAndInsertUint16(DCM_PixelRepresentation, 0);

        // ===== spacing =====
        {
            std::ostringstream ss;
            ss << spacingY << "\\" << spacingX;
            ds->putAndInsertString(DCM_PixelSpacing, ss.str().c_str());
        }

        ds->putAndInsertString(DCM_SliceThickness,
            std::to_string(spacingZ).c_str());

        // ===== 位置信息 =====
        {
            std::ostringstream pos;
            pos << "0\\0\\" << (z * spacingZ);
            ds->putAndInsertString(DCM_ImagePositionPatient,
                pos.str().c_str());
        }

        ds->putAndInsertString(DCM_ImageOrientationPatient,
            "1\\0\\0\\0\\1\\0");

        ds->putAndInsertUint16(DCM_InstanceNumber, z + 1);

        // ===== CT 特有（简化）=====
        ds->putAndInsertString(DCM_RescaleIntercept, "0");
        ds->putAndInsertString(DCM_RescaleSlope, "1");

        // ===== 写像素 =====
        ds->putAndInsertUint16Array(DCM_PixelData,
            buffer.data(),
            dimX * dimY);

        // --- 文件名 ---
        std::ostringstream filename;
        filename << outputDir << "/slice_"
            << std::setw(6) << std::setfill('0') << z
            << ".dcm";

        // --- 保存 ---
        file.saveFile(filename.str().c_str(), EXS_LittleEndianExplicit);

        if (z % 100 == 0)
        {
            std::cout << "Written slice " << z << std::endl;
        }
    }

    std::cout << "DICOM CT export finished." << std::endl;

    return 1;
}

bool SwapXY_MHD_USHORT(const std::string& inputMhdPath, const std::string& outputMhdPath)
{
    try
    {
        // 1. 解析输入 .mhd 文件头
        std::ifstream mhdIn(inputMhdPath);
        if (!mhdIn.is_open())
        {
            std::cerr << "无法打开输入 MHD 文件: " << inputMhdPath << std::endl;
            return false;
        }

        int dimX = 0, dimY = 0, dimZ = 0;
        double spacingX = 1.0, spacingY = 1.0, spacingZ = 1.0;
        double originX = 0.0, originY = 0.0, originZ = 0.0;
        std::string elementType, rawFileName, byteOrder = "False";

        std::string line;
        while (std::getline(mhdIn, line))
        {
            std::istringstream iss(line);
            std::string key;
            iss >> key;
            if (!key.empty() && key.back() == '=')
            {
                key.pop_back();
            }
            std::string token;
            if (iss >> token && token == "=")
            {
                // 已经跳过 '='，继续读取值
            }
            else if (!token.empty() && token != "=")
            {
                iss.str(line);
                iss.clear();
                iss >> key;
                if (iss >> token && token == "=")
                {
                }
            }
            if (key == "DimSize")
            {
                if (!(iss >> dimX >> dimY >> dimZ))
                {
                    std::cerr << "DimSize 解析失败，原始行: " << line << std::endl;
                }
            }
            else if (key == "ElementType")
            {
                iss >> elementType;
            }
            else if (key == "ElementSpacing")
            {
                iss >> spacingX >> spacingY >> spacingZ;
            }
            else if (key == "Offset" || key == "Origin")
            {
                iss >> originX >> originY >> originZ;
            }
            else if (key == "ElementDataFile")
            {
                iss >> rawFileName;
            }
            else if (key == "ElementByteOrderMSB")
            {
                iss >> byteOrder;
            }
        }
        mhdIn.close();

        // 校验参数
        if (dimX <= 0 || dimY <= 0 || dimZ <= 0)
        {
            std::cerr << "无效的维度信息: " << dimX << "x" << dimY << "x" << dimZ << std::endl;
            return false;
        }
        if (elementType != "MET_USHORT")
        {
            std::cerr << "错误：当前函数仅支持 MET_USHORT，检测到: " << elementType << std::endl;
            return false;
        }
        if (rawFileName.empty())
        {
            std::cerr << "未找到 ElementDataFile 字段" << std::endl;
            return false;
        }

        // 2. 构造输入 .raw 文件的完整路径（与 .mhd 在同一目录）
        std::string inputRawPath;
        size_t pos = inputMhdPath.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            inputRawPath = inputMhdPath.substr(0, pos + 1) + rawFileName;
        }
        else
        {
            inputRawPath = rawFileName;   // 同目录
        }

        std::ifstream rawIn(inputRawPath, std::ios::binary);
        if (!rawIn.is_open())
        {
            std::cerr << "无法打开 RAW 文件: " << inputRawPath << std::endl;
            return false;
        }

        // 3. 构造输出 .raw 文件路径（与 outputMhdPath 同目录）
        std::string outputRawPath;
        pos = outputMhdPath.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            outputRawPath = outputMhdPath.substr(0, pos + 1) +
                outputMhdPath.substr(pos + 1, outputMhdPath.rfind('.') - pos - 1) + ".raw";
        }
        else
        {
            // 无路径，只有文件名
            size_t dotPos = outputMhdPath.rfind('.');
            if (dotPos != std::string::npos)
            {
                outputRawPath = outputMhdPath.substr(0, dotPos) + ".raw";
            }
            else
            {
                outputRawPath = outputMhdPath + ".raw";
            }
        }

        std::ofstream rawOut(outputRawPath, std::ios::binary);
        if (!rawOut.is_open())
        {
            std::cerr << "无法创建输出 RAW 文件: " << outputRawPath << std::endl;
            return false;
        }

        // 4. 按切片流式处理（内存占用仅一层）
        const size_t slicePixels = static_cast<size_t>(dimX) * dimY;
        const size_t sliceBytes = slicePixels * sizeof(uint16_t);

        std::vector<uint16_t> slice(slicePixels);
        std::vector<uint16_t> transposedSlice(slicePixels);

        std::cout << "开始 XY 方向交换转换..." << std::endl;
        std::cout << "原始尺寸: " << dimX << " x " << dimY << " x " << dimZ << std::endl;

        for (int z = 0; z < dimZ; ++z)
        {
            // 读取一层数据
            rawIn.read(reinterpret_cast<char*>(slice.data()), sliceBytes);

            if (static_cast<size_t>(rawIn.gcount()) != sliceBytes)
            {
                std::cerr << "读取第 " << z << " 层数据失败！" << std::endl;
                return false;
            }

            // 执行 X/Y 转置：新(x',y') = 原(y, x)
            for (int y = 0; y < dimY; ++y)
            {
                for (int x = 0; x < dimX; ++x)
                {
                    transposedSlice[y * dimX + x] = slice[x * dimY + y];
                }
            }

            // 写入转置后的一层
            rawOut.write(reinterpret_cast<const char*>(transposedSlice.data()), sliceBytes);

            // 进度提示
            if ((z + 1) % 500 == 0 || z == dimZ - 1)
            {
                std::cout << "已处理 " << (z + 1) << " / " << dimZ << " 层" << std::endl;
            }
        }

        rawIn.close();
        rawOut.close();

        // 5. 生成新的 .mhd 文件
        std::ofstream mhdOut(outputMhdPath);
        if (!mhdOut.is_open())
        {
            std::cerr << "无法创建输出 MHD 文件: " << outputMhdPath << std::endl;
            return false;
        }

        // 输出文件名（不带路径）
        std::string outputRawFileName;
        pos = outputRawPath.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            outputRawFileName = outputRawPath.substr(pos + 1);
        }
        else
        {
            outputRawFileName = outputRawPath;
        }

        mhdOut << "ObjectType = Image\n"
            << "NDims = 3\n"
            << "DimSize = " << dimY << " " << dimX << " " << dimZ << "\n"   // X/Y 已交换
            << "ElementType = MET_USHORT\n"
            << "ElementSpacing = " << spacingY << " " << spacingX << " " << spacingZ << "\n"
            << "Offset = " << originX << " " << originY << " " << originZ << "\n"
            << "ElementByteOrderMSB = " << byteOrder << "\n"
            << "ElementDataFile = " << outputRawFileName << "\n"
            << "BinaryData = True\n"
            << "BinaryDataByteOrderMSB = False\n";

        mhdOut.close();

        std::cout << "转换成功完成！" << std::endl;
        std::cout << "输出 MHD: " << outputMhdPath << std::endl;
        std::cout << "输出 RAW: " << outputRawPath << std::endl;
        std::cout << "新尺寸: " << dimY << " x " << dimX << " x " << dimZ << std::endl;

        return true;

    }
    catch (const std::exception& e)
    {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return false;
    }
}

bool MainWindow::Mhd2Stream()
{

    std::string OutputDir;
    std::string inputMHDPath = qPrintable(m_Mhdfilename);

    if (ui->m_DIcom->isChecked())
    {
        size_t lastSep = inputMHDPath.find_last_of("\\/");
        if (lastSep != std::string::npos)
        {
            OutputDir = inputMHDPath.substr(0, lastSep + 1) + "NewDicom";   // 临时写死，实际应解析
            EnsureDirectoryExists(OutputDir);
        }

        ui->m_Message->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->m_Message->setText("start MHD_To_DICOM_CT_Stream!");

        QElapsedTimer ElapsedTimer;
        ElapsedTimer.start();
        DataInfo datainfo(inputMHDPath, OutputDir);

        auto listFuture = QtConcurrent::run(MHD_To_CT_Stream, datainfo);
        listFuture.waitForFinished();
        QString str = QString::number(ElapsedTimer.elapsed());
        QMessageBox::information(nullptr, "ElapsedTimer", str, QMessageBox::Yes, QMessageBox::Yes);//1384070
    }
    else
    {
         return SwapXY_MHD_USHORT(inputMHDPath, inputMHDPath + ".yx");
    }

    return 1;
}


bool MainWindow::ShowImages()
{
    m_sliceReader->SetFileName(qPrintable(m_Mhdfilename));

    m_mapperXY->SetInputConnection(m_sliceReader->GetOutputPort());
    m_mapperXY->SetOrientationToZ();
    m_mapperXY->SetSliceNumber(50);
    m_mapperXY->StreamingOn();
    m_mapperXY->SliceAtFocalPointOff();
    m_mapperXY->SliceFacesCameraOff();
    m_imageSliceXY->SetMapper(m_mapperXY);
    m_imageSliceXY->GetProperty()->SetColorWindow(4000);
    m_imageSliceXY->GetProperty()->SetColorLevel(1000);
    m_imageSliceXY->GetProperty()->SetInterpolationTypeToNearest();
    m_rendererXY->AddViewProp(m_imageSliceXY);
    m_vtkRenderWindowXY = ui->m_axial2DView->renderWindow(); 
    m_vtkRenderWindowXY->AddRenderer(m_rendererXY);
    ui->m_axial2DView->interactor()->SetRenderWindow(m_vtkRenderWindowXY);
    m_vtkRenderWindowXY->SetDesiredUpdateRate(30.0);   // 交互帧率优先
    m_vtkRenderWindowXY->Render();

    ///
    m_mapperXZ->SetInputConnection(m_sliceReader->GetOutputPort());
    m_mapperXZ->SetOrientationToY();
    m_mapperXZ->SetSliceNumber(50);
    m_mapperXZ->StreamingOn();
    m_mapperXZ->SliceAtFocalPointOff();
    m_mapperXZ->SliceFacesCameraOff();
    m_imageSliceXZ->SetMapper(m_mapperXZ);
    m_imageSliceXZ->GetProperty()->SetColorWindow(4000);
    m_imageSliceXZ->GetProperty()->SetColorLevel(1000);
    m_imageSliceXZ->GetProperty()->SetInterpolationTypeToNearest();
    m_rendererXZ->AddViewProp(m_imageSliceXZ);

    m_vtkRenderWindowXZ = ui->m_sagital2DView->renderWindow();

    m_vtkRenderWindowXZ->AddRenderer(m_rendererXZ);
    ui->m_sagital2DView->interactor()->SetRenderWindow(m_vtkRenderWindowXZ);

    m_vtkRenderWindowXZ->SetDesiredUpdateRate(30.0);   // 交互帧率优先

    m_vtkRenderWindowXZ->Render();

    ///
    m_mapperYZ->SetInputConnection(m_sliceReader->GetOutputPort());
    m_mapperYZ->SetOrientationToX();
    m_mapperYZ->SetSliceNumber(50);
    m_mapperYZ->StreamingOn();
    m_mapperYZ->SliceAtFocalPointOff();
    m_mapperYZ->SliceFacesCameraOff();

    m_imageSliceYZ->SetMapper(m_mapperYZ);
    m_imageSliceYZ->GetProperty()->SetColorWindow(4000);
    m_imageSliceYZ->GetProperty()->SetColorLevel(1000);
    m_imageSliceYZ->GetProperty()->SetInterpolationTypeToNearest();
    m_rendererYZ->AddViewProp(m_imageSliceYZ);

    m_vtkRenderWindowYZ = ui->m_coronal2DView->renderWindow();

    m_vtkRenderWindowYZ->AddRenderer(m_rendererYZ);
    ui->m_coronal2DView->interactor()->SetRenderWindow(m_vtkRenderWindowYZ);

    m_vtkRenderWindowYZ->SetDesiredUpdateRate(30.0);   // 交互帧率优先

    m_vtkRenderWindowYZ->Render();

    return 1;
}

bool MainWindow::MHD_To_DICOM_CT_Stream(DataInfo *info)//const std::string& mhdPath, const std::string& outputDir)
{
    DataInfo data(info->m_inputPath, info->m_outputDir);

    return MHD_To_CT_Stream(data);
}



MainWindow::~MainWindow()
{
    delete ui;
}


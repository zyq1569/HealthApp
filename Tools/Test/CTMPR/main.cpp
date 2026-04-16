#include "mainwindow.h"

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

/*
bool SwapXY_MHD_USHORT( const std::string& inputMhdPath,  const std::string& outputMhdPath)
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
*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
  
    //if (0)// MHD--->dicom
    //{
    //    //SwapXY_MHD_USHORT("E:/Temp_down/CT-2026-04-04-11-54-12/Volume.mhd", "E:/Temp_down/CT-2026-04-04-11-54-12/YXVolume.mhd");
    //    MainWindow w0;
    //    w0.show();
    //    return a.exec();
    //}
    MainWindow w;
    w.show();
    return a.exec();

}

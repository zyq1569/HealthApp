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

////////////+
// 简单 5x7 点阵字体，只包含 X Y 和数字 0-9
/*
std::map<char, std::vector<uint8_t>> font5x7 = {
    {'X',{0b10001,0b01010,0b00100,0b01010,0b10001,0b00000,0b00000}},
    {'Y',{0b10001,0b01010,0b00100,0b00100,0b00100,0b00000,0b00000}},
    {'0',{0b01110,0b10001,0b10011,0b10101,0b11001,0b10001,0b01110}},
    {'1',{0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110}},
    {'2',{0b01110,0b10001,0b00001,0b00110,0b01000,0b10000,0b11111}},
    {'3',{0b11110,0b00001,0b00001,0b01110,0b00001,0b00001,0b11110}},
    {'4',{0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010}},
    {'5',{0b11111,0b10000,0b10000,0b11110,0b00001,0b00001,0b11110}},
    {'6',{0b01110,0b10000,0b10000,0b11110,0b10001,0b10001,0b01110}},
    {'7',{0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000}},
    {'8',{0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110}},
    {'9',{0b01110,0b10001,0b10001,0b01111,0b00001,0b00001,0b01110}}
};

// 绘制单字符到图像矩阵
void DrawChar(std::vector<uint8_t>& image, int imgW, int imgH, char c, int startX, int startY, int scale)
{
    if (font5x7.find(c) == font5x7.end()) return;
    auto bitmap = font5x7[c];
    for (int row = 0; row < 7; ++row)
    {
        for (int col = 0; col < 5; ++col)
        {
            if ((bitmap[row] >> (4 - col)) & 1)
            {
                for (int dy = 0; dy < scale; ++dy)
                    for (int dx = 0; dx < scale; ++dx)
                    {
                        int x = startX + col * scale + dx;
                        int y = startY + row * scale + dy;
                        if (x >= 0 && x < imgW&&y >= 0 && y < imgH)
                        {
                            int idx = (y*imgW + x);
                            image[idx] = 255;
                        }
                    }
            }
        }
    }
}

// 绘制字符串到图像中心
void DrawString(std::vector<uint8_t>& image, int imgW, int imgH, const std::string& text, int scale)
{
    int totalW = text.size() * 5 * scale + (text.size() - 1)*scale;
    int startX = imgW / 2 - totalW / 2;
    int startY = imgH / 2 - (7 * scale) / 2;
    for (size_t i = 0; i < text.size(); ++i)
    {
        DrawChar(image, imgW, imgH, text[i], startX + i * 6 * scale, startY, scale);
    }
}

// 封装生成体数据函数
void GenerateVolumeMPR(const std::string& prefix)
{
    const int dimX = 800;
    const int dimY = 600;
    const int dimZ = 400;

    std::vector<uint8_t> volume(dimX*dimY*dimZ, 0);

    int scale = 10; // 每个点放大10x10像素，可调大以占切片中心1/3

    // XY切片
    for (int z = 0; z < dimZ; ++z)
    {
        std::vector<uint8_t> slice(dimX*dimY, 0);
        DrawString(slice, dimX, dimY, "XY " + std::to_string(z), scale);
        std::copy(slice.begin(), slice.end(), volume.begin() + z * dimX*dimY);
    }

    // XZ切片
    for (int y = 0; y < dimY; ++y)
    {
        std::vector<uint8_t> slice(dimX*dimZ, 0);
        DrawString(slice, dimX, dimZ, "XZ " + std::to_string(y), scale);
        for (int z = 0; z < dimZ; ++z)
            for (int x = 0; x < dimX; ++x)
                volume[z*dimX*dimY + y * dimX + x] = std::max(volume[z*dimX*dimY + y * dimX + x], slice[z*dimX + x]);
    }

    // YZ切片
    for (int x = 0; x < dimX; ++x)
    {
        std::vector<uint8_t> slice(dimY*dimZ, 0);
        DrawString(slice, dimY, dimZ, "YZ " + std::to_string(x), scale);
        for (int z = 0; z < dimZ; ++z)
            for (int y = 0; y < dimY; ++y)
                volume[z*dimX*dimY + y * dimX + x] = std::max(volume[z*dimX*dimY + y * dimX + x], slice[z*dimY + y]);
    }

    // 写 raw
    std::ofstream rawFile(prefix + ".raw", std::ios::binary);
    rawFile.write(reinterpret_cast<char*>(volume.data()), volume.size());
    rawFile.close();

    // 写 mhd
    std::ofstream mhdFile(prefix + ".mhd");
    mhdFile << "ObjectType = Image\n";
    mhdFile << "NDims = 3\n";
    mhdFile << "DimSize = " << dimX << " " << dimY << " " << dimZ << "\n";
    mhdFile << "ElementType = MET_UCHAR\n";
    mhdFile << "ElementDataFile = " << prefix << ".raw\n";
    mhdFile << "ElementByteOrderMSB = False\n";
    mhdFile.close();

    std::cout << "生成完成: " << prefix << ".mhd + .raw\n";
}


*/


//////////////////////+++++++++++++++++++++++++++
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
// 根据元素类型返回对应的类型
template <typename T>
bool ReadRawLayer(std::ifstream& rawIn, int dimX, int dimY, int z, std::vector<T>& layerData)
{
    size_t sliceBytes = dimX * dimY * sizeof(T);
    layerData.resize(dimX * dimY);

    rawIn.seekg(z * sliceBytes, std::ios::beg);  // 定位到第z层
    rawIn.read(reinterpret_cast<char*>(layerData.data()), sliceBytes);

    if (rawIn.gcount() != sliceBytes)
    {
        std::cerr << "读取第 " << z << " 层数据失败。" << std::endl;
        return false;
    }

    return true;
}

// 写入一层的 .raw 数据
template <typename T>
bool WriteRawLayer(std::ofstream& rawOut, int dimX, int dimY, int z, const std::vector<T>& layerData)
{
    size_t sliceBytes = dimX * dimY * sizeof(T);
    rawOut.seekp(z * sliceBytes, std::ios::beg);  // 定位到第z层
    rawOut.write(reinterpret_cast<const char*>(layerData.data()), sliceBytes);

    return true;
}


template <typename T>
void RotateLayerZ(std::vector<T>& layerData, int dimX, int dimY)
{
    std::vector<T> rotatedLayer(dimX * dimY);
    //for (int y = 0; y < dimY; ++y)//newx
    //{
    //    for (int x = 0; x < dimX; ++x)//newy
    //    {
    //        //[y,x]         [x,y]
    //        int new_x = y;
    //        int new_MaxX = dimY;
    //        int new_y = x;
    //        int new_MaxY = dimX;
    //        rotatedLayer[new_x * new_MaxY + new_y] = layerData[(dimY - 1 - x) * dimX + y];
    //    }
    //}
    for (int i = 0; i < dimX; ++i)
    {
        for (int j = 0; j < dimY; ++j)
        {
            // 将 input[i][j] 转换到 output[j][i] 中
            //*(output + j * dimX + i) = *(input + i * dimY + j);
            rotatedLayer[j * dimX + i] = layerData[i * dimY + j];
        }
    }

    // 更新数据
    layerData = rotatedLayer;
}

// 根据元素类型自动选择对应的数据类型
using TypeMap = std::unordered_map<std::string, std::function<std::shared_ptr<void>()>>;

// 自动适配所有支持的 MET_* 类型
std::shared_ptr<void> GetDataTypeByElementType(const std::string& elementType)
{
    if (elementType == "MET_USHORT")
        return std::make_shared<std::vector<unsigned short>>();
    else if (elementType == "MET_UCHAR")
        return std::make_shared<std::vector<unsigned char>>();
    else if (elementType == "MET_INT")
        return std::make_shared<std::vector<int>>();
    else if (elementType == "MET_FLOAT")
        return std::make_shared<std::vector<float>>();
    // 可以继续添加其他 MET_* 类型
    else
    {
        std::cerr << "不支持的元素类型: " << elementType << std::endl;
        return nullptr;
    }
}

// 读取 MHD 文件
// 去除字符串两端的空白字符
std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";  // 全部为空格的情况
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// 读取 MHD 文件
bool ReadMhdFile(const std::string& mhdPath, int& dimX, int& dimY, int& dimZ,   double& spacingX, double& spacingY, double& spacingZ,
    double& originX, double& originY, double& originZ,  std::string& elementType, std::string& rawFileName, std::string& byteOrder)
{
    std::ifstream mhdIn(mhdPath);
    if (!mhdIn.is_open())
    {
        std::cerr << "无法打开 MHD 文件: " << mhdPath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(mhdIn, line))
    {
        std::istringstream iss(line);
        std::string key, token;

        // 提取key并去除空格
        std::getline(iss, key, '=');
        key = trim(key);  // 去掉 key 的前后空格

        if (key.empty()) continue;  // 跳过空行

        // 读取等号后的内容
        std::getline(iss, token);
        token = trim(token);  // 去掉 token 的前后空格

        if (key == "DimSize")
        {
            std::istringstream tokenStream(token);
            tokenStream >> dimX >> dimY >> dimZ;
        }
        else if (key == "ElementType")
        {
            elementType = token;
        }
        else if (key == "ElementSpacing")
        {
            std::istringstream tokenStream(token);
            tokenStream >> spacingX >> spacingY >> spacingZ;
        }
        else if (key == "Origin" || key == "Offset")
        {
            std::istringstream tokenStream(token);
            tokenStream >> originX >> originY >> originZ;
        }
        else if (key == "ElementDataFile")
        {
            rawFileName = token;
        }
        else if (key == "ElementByteOrderMSB")
        {
            byteOrder = token;
        }
    }
    // 2. 构造输入 .raw 文件的完整路径（与 .mhd 在同一目录）
    std::string inputRawPath;
    size_t pos = mhdPath.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        inputRawPath = mhdPath.substr(0, pos + 1) + rawFileName;
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
    rawFileName = inputRawPath;

    mhdIn.close();
    return true;
}
// 处理旋转操作
bool RotateImage(const std::string& inputMhdPath, const std::string& outputMhdPath)
{
    int dimX, dimY, dimZ;
    double spacingX, spacingY, spacingZ;
    double originX, originY, originZ;
    std::string elementType, rawFileName, byteOrder;

    // 1. 读取 MHD 文件
    if (!ReadMhdFile(inputMhdPath, dimX, dimY, dimZ, spacingX, spacingY, spacingZ,
        originX, originY, originZ, elementType, rawFileName, byteOrder))
    {
        return false;
    }

    // 2. 获取对应的数据类型
    auto dataType = GetDataTypeByElementType(elementType);
    if (!dataType)
    {
        return false;
    }

    // 3. 打开输入和输出的 RAW 文件
    std::ifstream rawIn(rawFileName, std::ios::binary);
    if (!rawIn.is_open())
    {
        std::cerr << "无法打开输入 RAW 文件: " << rawFileName << std::endl;
        return false;
    }

    std::ofstream rawOut(outputMhdPath + ".rotated.raw", std::ios::binary);
    if (!rawOut.is_open())
    {
        std::cerr << "无法打开输出 RAW 文件: " << outputMhdPath + ".rotated.raw" << std::endl;
        return false;
    }

    // 使用正确类型的读取和写入
    if (elementType == "MET_USHORT")
    {
        auto layerData = std::static_pointer_cast<std::vector<unsigned short>>(dataType);
        for (int z = 0; z < dimZ; ++z)
        {
            if (!ReadRawLayer(rawIn, dimX, dimY, z, *layerData))
                return false;

            RotateLayerZ(*layerData, dimX, dimY);

            if (!WriteRawLayer(rawOut, dimX, dimY, z, *layerData))
                return false;
        }
    }
    else if (elementType == "MET_UCHAR")
    {
        auto layerData = std::static_pointer_cast<std::vector<unsigned char>>(dataType);
        for (int z = 0; z < dimZ; ++z)
        {
            if (!ReadRawLayer(rawIn, dimX, dimY, z, *layerData))
                return false;

            RotateLayerZ(*layerData, dimX, dimY);

            if (!WriteRawLayer(rawOut, dimX, dimY, z, *layerData))
                return false;
        }
    }
    else
    {
        std::cerr << "暂时不支持该类型的旋转: " << elementType << std::endl;
        return false;
    }

    // 4. 写入新的 MHD 文件
    std::ofstream mhdOut(outputMhdPath);
    if (!mhdOut.is_open())
    {
        std::cerr << "无法创建 MHD 文件: " << outputMhdPath << std::endl;
        return false;
    }

    mhdOut << "ObjectType = Image\n"
        << "NDims = 3\n"
        << "DimSize = " << dimY << " " << dimX << " " << dimZ << "\n"  // X, Y 交换
        << "ElementType = " << elementType << "\n"
        << "ElementSpacing = " << spacingY << " " << spacingX << " " << spacingZ << "\n"
        << "Offset = " << originX << " " << originY << " " << originZ << "\n"
        << "ElementByteOrderMSB = " << byteOrder << "\n"
        << "ElementDataFile = " << rawFileName + ".rotated.raw\n"
        << "BinaryData = True\n"
        << "BinaryDataByteOrderMSB = False\n";

    mhdOut.close();
    std::cout << "转换成功！输出 MHD 文件：" << outputMhdPath << std::endl;
    return true;
}




//////////////////////////++++++++++++++++++++++++++++++++

//////////////////////////////////////////////////////////

// BMP 文件头结构体
#pragma pack(push, 1)
struct BMPHeader {
    char signature[2];         // 文件类型 "BM"
    uint32_t fileSize;         // 文件大小
    uint32_t reserved;         // 保留字段
    uint32_t dataOffset;       // 数据偏移量
};

struct DIBHeader {
    uint32_t headerSize;       // DIB 头大小
    int32_t width;             // 图像宽度
    int32_t height;            // 图像高度
    uint16_t planes;           // 颜色平面数
    uint16_t bitsPerPixel;     // 每像素位数
    uint32_t compression;      // 压缩类型
    uint32_t imageSize;        // 图像数据大小
    int32_t xResolution;       // 水平分辨率
    int32_t yResolution;       // 垂直分辨率
    uint32_t colorsUsed;       // 调色板颜色数
    uint32_t importantColors;  // 重要颜色数
};
#pragma pack(pop)

void rotateBMP(const std::string& inputFile, const std::string& outputFile) 
{
    // 打开输入 BMP 文件
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) 
    {
        throw std::runtime_error("无法打开输入文件");
    }

    // 读取 BMP 文件头
    BMPHeader bmpHeader;
    inFile.read(reinterpret_cast<char*>(&bmpHeader), sizeof(BMPHeader));

    if (bmpHeader.signature[0] != 'B' || bmpHeader.signature[1] != 'M') 
    {
        throw std::runtime_error("不是有效的 BMP 文件");
    }

    // 读取 DIB 头
    DIBHeader dibHeader;
    inFile.read(reinterpret_cast<char*>(&dibHeader), sizeof(DIBHeader));

    // 读取图像数据
    int rowSize = (dibHeader.width * dibHeader.bitsPerPixel + 31) / 32 * 4; // 行大小（字节对齐）
    int dataSize = rowSize * dibHeader.height; // 图像数据大小
    std::vector<uint8_t> imageData(dataSize);
    inFile.seekg(bmpHeader.dataOffset, std::ios::beg);
    inFile.read(reinterpret_cast<char*>(imageData.data()), dataSize);
    inFile.close();

    // 创建一个新的图像数据（旋转后的图像）
    std::vector<uint8_t> rotatedData(dataSize);
    int newRowSize = (dibHeader.height * dibHeader.bitsPerPixel + 31) / 32 * 4; // 旋转后的行大小

    // 旋转图像数据：绕最左下角点 90 度
    for (int y = 0; y < dibHeader.height; ++y) 
    {
        for (int x = 0; x < dibHeader.width; ++x) 
        {
            int srcIndex = y * rowSize + x * (dibHeader.bitsPerPixel / 8);
            int dstIndex = (dibHeader.width - x - 1) * newRowSize + y * (dibHeader.bitsPerPixel / 8);
            std::memcpy(&rotatedData[dstIndex], &imageData[srcIndex], dibHeader.bitsPerPixel / 8);
        }
    }

    // 打开输出文件
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("无法创建输出文件");
    }

    // 写入 BMP 文件头
    outFile.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(BMPHeader));

    // 更新 DIB 头的宽度和高度
    dibHeader.width = dibHeader.height;
    dibHeader.height = dibHeader.width;

    // 写入 DIB 头
    outFile.write(reinterpret_cast<const char*>(&dibHeader), sizeof(DIBHeader));

    // 写入旋转后的图像数据
    outFile.write(reinterpret_cast<const char*>(rotatedData.data()), rotatedData.size());
    outFile.close();
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
  
    //ConvertMHD_XY_To_YX("D:/CT_3D/Test_Data/ID/id_ct.mhd");
    if (0)
    {
        try 
        {
            rotateBMP("D:/CT_3D/Test_Data/ID/id.bmp", "D:/CT_3D/Test_Data/ID/output.bmp");
            std::cout << "图像旋转成功，保存为 output.bmp" << std::endl;
        }
        catch (const std::exception& e) 
        {
            std::cerr << "错误: " << e.what() << std::endl;
        }
    }
    if (0)
    {
        // 输入和输出的 MHD 文件路径
        std::string inputMhdPath = "D:/CT_3D/Test_Data/Dental/Dental.mhd";
        std::string outputMhdPath = "D:/CT_3D/Test_Data/Dental/DentalYX";
    
        // 执行图像旋转
        if (!RotateImage(inputMhdPath, outputMhdPath))
        {
            std::cerr << "图像旋转失败" << std::endl;
            return -1;
        }
    }

    //GenerateVolumeMPR("D:/CT_3D/Test_Data/ID/id_ct",true);
    MainWindow w;
    w.show();
    return a.exec();
}


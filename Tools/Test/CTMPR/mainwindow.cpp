#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qprogressdata.h"

// DCMTK
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <vtkCamera.h>

///+++
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>   // for strerror
#include <deque>

#include <QFuture>
#include <QFutureWatcher>
#include <functional>
#include <QMessageBox>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QFileInfo>

#ifdef _WIN32
#include <direct.h>   // Windows: _mkdir
#define mkdir(dir, mode) _mkdir(dir)   // Windows 不需要权限参数
#endif

#include <type_traits>

// -------------------- 非 void 版本 --------------------
template<typename R, typename Callback>
typename std::enable_if<!std::is_void<R>::value>::type
handleResult(QFutureWatcher<R>* watcher, Callback onFinished)
{
    onFinished(watcher->result());
}

// -------------------- void 版本 --------------------
template<typename R, typename Callback>
typename std::enable_if<std::is_void<R>::value>::type
handleResult(QFutureWatcher<R>* watcher, Callback onFinished)
{
    onFinished();
}

// -------------------- 主函数 --------------------
template<typename Func, typename Callback, typename... Args>
void AsyncThread(Func task, Callback onFinished, Args&&... args)
{
    using ReturnType = typename std::result_of<Func(Args...)>::type;

    QFutureWatcher<ReturnType>* watcher = new QFutureWatcher<ReturnType>();

    QObject::connect(watcher, &QFutureWatcher<ReturnType>::finished, [watcher, onFinished]()
            {
                handleResult<ReturnType>(watcher, onFinished);
                watcher->deleteLater();
            }
    );

    watcher->setFuture(QtConcurrent::run(task, std::forward<Args>(args)...));
}

//template<typename Func, typename Callback, typename... Args>
//void runAsync(Func task, Callback onFinished, Args&&... args)
//{
//    using ReturnType = typename std::result_of<
//        Func(typename std::decay<Args>::type...)
//    >::type;
//
//    QFutureWatcher<ReturnType>* watcher = new QFutureWatcher<ReturnType>();
//
//    QObject::connect(watcher, &QFutureWatcher<ReturnType>::finished,
//        [watcher, onFinished]() {
//        handleResult<ReturnType>(watcher, onFinished);
//        watcher->deleteLater();
//    });
//
//    watcher->setFuture(QtConcurrent::run(task, std::forward<Args>(args)...));
//}
//////////////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////////////////////////////////////////////////////////
// ---------- 类型映射 ----------
size_t GetElementSize(const std::string& type)
{
    if (type == "MET_UCHAR") return 1;
    if (type == "MET_CHAR") return 1;
    if (type == "MET_USHORT") return 2;
    if (type == "MET_SHORT") return 2;
    if (type == "MET_UINT") return 4;
    if (type == "MET_INT") return 4;
    if (type == "MET_FLOAT") return 4;
    if (type == "MET_DOUBLE") return 8;

    throw std::runtime_error("Unsupported ElementType");
}

// ---------- 核心转换 ----------
template<typename T>
void ConvertXY_Internal(std::ifstream& in, std::ofstream& out, int dimX, int dimY, int dimZ)
{
    const size_t sliceSize = (size_t)dimX * dimY;

    std::vector<T> inBuf(sliceSize);
    std::vector<T> outBuf(sliceSize);

    for (int z = 0; z < dimZ; ++z)
    {
        // 读取一个 XY slice（连续）
        in.read(reinterpret_cast<char*>(inBuf.data()),
            sliceSize * sizeof(T));

        if (!in)
            throw std::runtime_error("Read failed");

        // 转换为 [x][y] 连续
        for (int y = 0; y < dimY; ++y)
        {
            for (int x = 0; x < dimX; ++x)
            {
                // 原：y * dimX + x
                size_t inIdx = (size_t)y * dimX + x;

                // 目标：[x][y]，y 连续
                size_t outIdx = (size_t)x * dimY + y;

                outBuf[outIdx] = inBuf[inIdx];
            }
        }

        out.write(reinterpret_cast<char*>(outBuf.data()), sliceSize * sizeof(T));

        if (!out)
        {
            throw std::runtime_error("Write failed");
        }
            
    }
}


struct MHDInfo
{
    int dim[3]{ 0,0,0 };
    std::string elementType;
    std::string rawFile;
};

MHDInfo ParseMHD(const std::string& mhdPath)
{
    std::ifstream in(mhdPath);
    if (!in) throw std::runtime_error("open mhd failed");

    MHDInfo info;
    std::string line;

    while (std::getline(in, line))
    {
        if (line.find("DimSize") != std::string::npos)
        {
            sscanf(line.c_str(), "DimSize = %d %d %d", &info.dim[0], &info.dim[1], &info.dim[2]);
        }
        else if (line.find("ElementType") != std::string::npos)
        {
            info.elementType = line.substr(line.find("=") + 2);
        }
        else if (line.find("ElementDataFile") != std::string::npos)
        {
            info.rawFile = line.substr(line.find("=") + 2);
        }
    }

    return info;
}

bool ConvertMHD_XY_To_YX(const std::string& mhdPath)
{
    // ---------- 解析 ----------
    auto info = ParseMHD(mhdPath);

    std::string baseDir = mhdPath.substr(0, mhdPath.find_last_of("/\\") + 1);
    std::string rawPath = baseDir + info.rawFile;
    std::string outPath = rawPath + ".yx";

    std::ifstream in(rawPath, std::ios::binary);
    std::ofstream out(outPath, std::ios::binary);

    if (!in || !out)
        throw std::runtime_error("open raw failed");

    int X = info.dim[0];
    int Y = info.dim[1];
    int Z = info.dim[2];

    // ---------- 类型分发 ----------
    if (info.elementType == "MET_UCHAR")
        ConvertXY_Internal<uint8_t>(in, out, X, Y, Z);
    else if (info.elementType == "MET_CHAR")
        ConvertXY_Internal<int8_t>(in, out, X, Y, Z);
    else if (info.elementType == "MET_USHORT")
        ConvertXY_Internal<uint16_t>(in, out, X, Y, Z);
    else if (info.elementType == "MET_SHORT")
        ConvertXY_Internal<int16_t>(in, out, X, Y, Z);
    else if (info.elementType == "MET_UINT")
        ConvertXY_Internal<uint32_t>(in, out, X, Y, Z);
    else if (info.elementType == "MET_INT")
        ConvertXY_Internal<int32_t>(in, out, X, Y, Z);
    else if (info.elementType == "MET_FLOAT")
        ConvertXY_Internal<float>(in, out, X, Y, Z);
    else if (info.elementType == "MET_DOUBLE")
        ConvertXY_Internal<double>(in, out, X, Y, Z);
    else
        throw std::runtime_error("Unsupported type");

    std::cout << "Convert done: " << outPath << std::endl;
    return true;
}

////////////////////////////////////////////////////////
/////////////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////////////////////////////////////////////////////////////////////////////////////////////////
// 点阵字体（X,Y,0-9）
std::map<char, std::vector<uint8_t>> font5x7 = 
{
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

// 绘制单字符
template<typename T>
void DrawChar(std::vector<T>& image, int imgW, int imgH, char c, int startX, int startY, int scale, T val)
{
    if (font5x7.find(c) == font5x7.end()) return;

    auto bitmap = font5x7[c];
    for (int row = 0; row < 7; ++row)
        for (int col = 0; col < 5; ++col)
            if ((bitmap[row] >> (4 - col)) & 1)
                for (int dy = 0; dy < scale; ++dy)
                    for (int dx = 0; dx < scale; ++dx)
                    {
                        int x = startX + col * scale + dx;
                        int y = startY + row * scale + dy;
                        if (x >= 0 && x < imgW&&y >= 0 && y < imgH)
                            image[y*imgW + x] = val;
                    }
}

// 绘制字符串到中心
template<typename T>
void DrawString(std::vector<T>& image, int imgW, int imgH, const std::string& text, int scale, T val)
{
    int totalW = text.size() * 5 * scale + (text.size() - 1)*scale;
    int startX = imgW / 2 - totalW / 2;
    int startY = imgH / 2 - 7 * scale / 2;
    for (size_t i = 0; i < text.size(); ++i)
        DrawChar(image, imgW, imgH, text[i], startX + i * 6 * scale, startY, scale, val);
}

// 封装生成体数据函数，增加 useUShort 参数
void GenerateVolumeMPR(const std::string& prefix, bool useUShort = false)
{
    const int dimX = 800;
    const int dimY = 600;
    const int dimZ = 400;
    int scale = 10; // 字符块放大倍数

    if (useUShort)
    {
        std::vector<uint16_t> volume(dimX*dimY*dimZ, 0);

        // XY
        for (int z = 0; z < dimZ; ++z)
        {
            std::vector<uint16_t> slice(dimX*dimY, 0);
            DrawString<uint16_t>(slice, dimX, dimY, "XY " + std::to_string(z), scale, 65535);
            std::copy(slice.begin(), slice.end(), volume.begin() + z * dimX*dimY);
        }

        // XZ
        for (int y = 0; y < dimY; ++y)
        {
            std::vector<uint16_t> slice(dimX*dimZ, 0);
            //DrawString(slice, dimX, dimZ, "XZ " + std::to_string(y), scale, 65535);
            DrawString<uint16_t>(slice, dimX, dimZ, "XZ " + std::to_string(y), scale, 65535);
            for (int z = 0; z < dimZ; ++z)
                for (int x = 0; x < dimX; ++x)
                    volume[z*dimX*dimY + y * dimX + x] = std::max(volume[z*dimX*dimY + y * dimX + x], slice[z*dimX + x]);
        }

        // YZ
        for (int x = 0; x < dimX; ++x)
        {
            std::vector<uint16_t> slice(dimY*dimZ, 0);
            DrawString<uint16_t>(slice, dimY, dimZ, "YZ " + std::to_string(x), scale, 65535);
            for (int z = 0; z < dimZ; ++z)
                for (int y = 0; y < dimY; ++y)
                    volume[z*dimX*dimY + y * dimX + x] = std::max(volume[z*dimX*dimY + y * dimX + x], slice[z*dimY + y]);
        }

        // 写 raw
        std::ofstream rawFile(prefix + ".raw", std::ios::binary);
        rawFile.write(reinterpret_cast<char*>(volume.data()), volume.size() * sizeof(uint16_t));
        rawFile.close();

        // 写 mhd
        std::ofstream mhdFile(prefix + ".mhd");
        mhdFile << "ObjectType = Image\n";
        mhdFile << "NDims = 3\n";
        mhdFile << "DimSize = " << dimX << " " << dimY << " " << dimZ << "\n";
        mhdFile << "ElementType = MET_USHORT\n";
        mhdFile << "ElementDataFile = " << prefix << ".raw\n";
        mhdFile << "ElementByteOrderMSB = False\n";
        mhdFile.close();
    }
    else
    {
        std::vector<uint8_t> volume(dimX*dimY*dimZ, 0);

        // XY
        for (int z = 0; z < dimZ; ++z)
        {
            std::vector<uint8_t> slice(dimX*dimY, 0);
            DrawString<uint8_t>(slice, dimX, dimY, "XY " + std::to_string(z), scale, 255);
            std::copy(slice.begin(), slice.end(), volume.begin() + z * dimX*dimY);
        }

        // XZ
        for (int y = 0; y < dimY; ++y)
        {
            std::vector<uint8_t> slice(dimX*dimZ, 0);
            DrawString<uint8_t>(slice, dimX, dimZ, "XZ " + std::to_string(y), scale, 255);
            for (int z = 0; z < dimZ; ++z)
                for (int x = 0; x < dimX; ++x)
                    volume[z*dimX*dimY + y * dimX + x] = std::max(volume[z*dimX*dimY + y * dimX + x], slice[z*dimX + x]);
        }

        // YZ
        for (int x = 0; x < dimX; ++x)
        {
            std::vector<uint8_t> slice(dimY*dimZ, 0);
            DrawString<uint8_t>(slice, dimY, dimZ, "YZ " + std::to_string(x), scale, 255);
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
    }

    std::cout << "生成完成: " << prefix << ".mhd + .raw" << std::endl;
}
///+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <vtkCallbackCommand.h>
// 自定义InteractorStyle，只处理右键窗宽窗位
class vtkRightClickWindowLevelStyle : public vtkInteractorStyleImage
{
public:
    static vtkRightClickWindowLevelStyle* New();
    vtkTypeMacro(vtkRightClickWindowLevelStyle, vtkInteractorStyleImage);

    vtkRightClickWindowLevelStyle()
    {
        this->m_ImageSlice = nullptr;
        this->m_LeftButtonPressed = false;
        this->m_LastPos[0] = 0;
        this->m_LastPos[1] = 0;
        this->m_Label[0] = nullptr;
        this->m_Label[1] = nullptr;
    }

    void SetImageSlice(vtkImageSlice* slice)
    {
        this->m_ImageSlice = slice;
    }

    void SetOverlayLabel(QLabel* label[])
    {
        this->m_Label[0] = label[0];
        this->m_Label[1] = label[1];
        if (this->m_Label[0])
        {
            this->m_Label[0]->setVisible(true);
            // 设置固定位置在左上角
            this->m_Label[0]->move(5, 15);
        }
        if (this->m_Label[1])
        {
            this->m_Label[1]->setVisible(true);
            // 设置固定位置在左上角
            this->m_Label[1]->move(5, 3);
        }
    }

    virtual void OnLeftButtonDown() override
    {
        this->m_LeftButtonPressed = true;
        this->GetInteractor()->GetEventPosition(this->m_LastPos);
    }

    virtual void OnLeftButtonUp() override
    {
        this->m_LeftButtonPressed = false;
        vtkInteractorStyleImage::OnLeftButtonUp();
    }

    virtual void OnMouseMove() override
    {
        if (this->m_LeftButtonPressed && this->m_ImageSlice)
        {
            int currPos[2];
            this->GetInteractor()->GetEventPosition(currPos);

            int dx = currPos[0] - m_LastPos[0];
            int dy = currPos[1] - m_LastPos[1];

            double window = this->m_ImageSlice->GetProperty()->GetColorWindow();
            double level = this->m_ImageSlice->GetProperty()->GetColorLevel();

            double windowStep = 1.0;
            double levelStep = 1.0;

            window += dx * windowStep;
            level += dy * levelStep;

            if (window < 1.0) window = 1.0;

            this->m_ImageSlice->GetProperty()->SetColorWindow(window);
            this->m_ImageSlice->GetProperty()->SetColorLevel(level);

            if (m_Label[0])
            {
                QString text = QString("WW: %1\nWL: %2").arg(window, 0, 'f', 1).arg(level, 0, 'f', 1);
                m_Label[0]->setWordWrap(true);
                m_Label[0]->setText(text);
                m_Label[0]->adjustSize();  // 自动调整 QLabel 大小以适应文字
            }

            this->GetInteractor()->GetRenderWindow()->Render();

            m_LastPos[0] = currPos[0];
            m_LastPos[1] = currPos[1];
        }
        else
        {
            vtkInteractorStyleImage::OnMouseMove();
        }
    }

    // 鼠标滚轮向前滚动
    virtual void OnMouseWheelForward() override
    {

         UpdateSlice();

    }

    // 鼠标滚轮向后滚动
    virtual void OnMouseWheelBackward() override
    {

         UpdateSlice(-1);

    }
    // 更新切片的方法
    void UpdateSlice(int nc = 1)
    {
        if (m_vtkImageSliceMapper)
        {
            int max = m_vtkImageSliceMapper->GetSliceNumberMaxValue();
            int min = m_vtkImageSliceMapper->GetSliceNumberMinValue();
            int im  = m_vtkImageSliceMapper->GetSliceNumber() + nc;
            if (im < (max) && im >(min))
            {
                // 更新 QLabel 显示的切片序号
                if (m_Label[1])
                {
                    QString text = QString("Slice: %1").arg(im);
                    m_Label[1]->setText(text);
                    m_Label[1]->adjustSize();
                }
                m_vtkImageSliceMapper->SetSliceNumber(im);
                // 渲染更新
                this->GetInteractor()->GetRenderWindow()->Render();
            }

        }
    }
    void SetVtkImageSliceMapper(vtkImageSliceMapper *vtkImageSliceMapper)
    {
        m_vtkImageSliceMapper = vtkImageSliceMapper;
    }
private:
    vtkImageSlice* m_ImageSlice;
    bool m_LeftButtonPressed;
    int m_LastPos[2];
    QLabel* m_Label[2];
    int m_SliceIndex;  // 当前切片序号
    vtkImageSliceMapper *m_vtkImageSliceMapper;
};

vtkStandardNewMacro(vtkRightClickWindowLevelStyle);

// 封装函数：绑定右键窗宽窗位
inline void EnableRightClickWindowLevel(QVTKInteractor* interactor, vtkImageSlice* slice, QWidget* parentWidget, vtkImageSliceMapper *vtkImageSliceMapper)
{
    if (!interactor || !slice || !parentWidget) return;

    vtkSmartPointer<vtkRightClickWindowLevelStyle> style =  vtkSmartPointer<vtkRightClickWindowLevelStyle>::New();
    style->SetImageSlice(slice);

    // 创建固定左上角 QLabel
    QLabel* overlay[2];
    overlay[0] = new QLabel(parentWidget);
    overlay[0]->setStyleSheet( "QLabel { background-color: rgba(0,0,0,0); color: white; padding: 2px;font-family: 'Consolas'; border-radius: 3px; }" );
    overlay[0]->setAttribute(Qt::WA_TransparentForMouseEvents); // 不阻挡鼠标
    overlay[0]->setVisible(true);  // 固定显示
    overlay[0]->raise();  // 保证显示在最上层
    overlay[0]->move(10, 10); // 左上角偏移

    overlay[1] = new QLabel(parentWidget);
    overlay[1]->setStyleSheet("QLabel { background-color: rgba(0,0,0,0); color: white; padding: 2px;font-family: 'Consolas'; border-radius: 3px; }");
    overlay[1]->setAttribute(Qt::WA_TransparentForMouseEvents); // 不阻挡鼠标
    overlay[1]->setVisible(true);  // 固定显示
    overlay[1]->raise();  // 保证显示在最上层
    overlay[1]->move(10, 10); // 左上角偏移

    style->SetOverlayLabel(overlay);
    style->SetVtkImageSliceMapper(vtkImageSliceMapper);
    interactor->SetInteractorStyle(style);

}
// ++++++++++++GPT
// ======================
// 自定义 Streaming Reader
// ======================
struct SliceCacheBlock
{
    int sliceIdx = -1;       // 当前缓存对应的slice序号
    uint16_t* data = nullptr;
};
class RawSliceReader : public vtkImageAlgorithm
{
public:
    static RawSliceReader* New();
    vtkTypeMacro(RawSliceReader, vtkImageAlgorithm);
public:
    std::string m_rawPath;
    int m_Dim[3] = { 0,0,0 };// { 18739, 13714, 100 };
    double m_spacingX = 1.0, m_spacingY = 1.0, m_spacingZ = 1.0;
    std::deque<SliceCacheBlock> m_cache[3]; // 0:XY 1:XZ 2:YZ
    int m_center;
public:
    uint16_t* AllocateSliceBuffer(int xyz)
    {
        if (xyz == 0) // XY
            return new uint16_t[(size_t)m_Dim[0] * m_Dim[1]];
        else if (xyz == 1) // XZ
            return new uint16_t[(size_t)m_Dim[0] * m_Dim[2]];
        else // YZ
            return new uint16_t[(size_t)m_Dim[1] * m_Dim[2]];
    }
    void LoadSlice(int xyz, int slice, uint16_t* dst)
    {
        std::ifstream file(m_rawPath, std::ios::binary);

        size_t xyBytes = (size_t)m_Dim[0] * m_Dim[1] * 2ULL;

        if (xyz == 0) // XY
        {
            size_t offset = (size_t)slice * xyBytes;
            file.seekg(offset, std::ios::beg);
            file.read((char*)dst, xyBytes);
        }
        else if (xyz == 1) // XZ
        {
            size_t rowBytes = (size_t)m_Dim[0] * 2ULL;

            for (int z = 0; z < m_Dim[2]; z++)
            {
                size_t offset = (size_t)z * xyBytes + (size_t)slice * rowBytes;
                file.seekg(offset, std::ios::beg);
                file.read((char*)(dst + z * m_Dim[0]), rowBytes);
            }
        }
        else // YZ
        {
            std::ifstream fileYZ(m_rawPath + ".yx", std::ios::binary);

            size_t rowBytes = (size_t)m_Dim[1] * 2ULL;

            for (int z = 0; z < m_Dim[2]; z++)
            {
                size_t offset = (size_t)z * xyBytes + (size_t)slice * rowBytes;
                fileYZ.seekg(offset, std::ios::beg);
                fileYZ.read((char*)(dst + z * m_Dim[1]), rowBytes);
            }
        }
    }
    bool Cache(int xyz = 0, int startIndex = -1)// -2:Max -1:Mid  0:0
    {
        if (m_Dim[0] == 0 || m_Dim[1] == 0 || m_Dim[2] == 0)
        {
            return false;
        }

        auto& cache = m_cache[xyz];

        int dim = (xyz == 0) ? m_Dim[2] : (xyz == 1) ? m_Dim[1] :  m_Dim[0];

        int initStart = 5;
        if (xyz == 0)//z
        {
            if (startIndex <= -2)
            {
                initStart = m_Dim[2] - 1;
            }
            else if (startIndex == -1)
            {
                initStart = m_Dim[2] / 2;
            }
            else
            {
                if (startIndex >= 0)
                {
                    initStart = startIndex;
                    if (startIndex > m_Dim[2] - 1)
                    {
                        initStart = m_Dim[2] - 1;
                    }                  
                }              
            }
        }
        else if (xyz == 1)//y
        {
            if (startIndex <= -2)
            {
                initStart = m_Dim[1] - 1;
            }
            else if (startIndex == -1)
            {
                initStart = m_Dim[1] / 2;
            }
            else
            {
                if (startIndex >= 0)
                {
                    initStart = startIndex;
                    if (startIndex > m_Dim[1] - 1)
                    {
                        initStart = m_Dim[1] - 1;
                    }
                }
            }
        }
        else //z
        {
            if (startIndex <= -2)
            {
                initStart = m_Dim[0] - 1;
            }
            else if (startIndex == -1)
            {
                initStart = m_Dim[0] / 2;
            }
            else
            {
                if (startIndex >= 0)
                {
                    initStart = startIndex;
                    if (startIndex > m_Dim[0] - 1)
                    {
                        initStart = m_Dim[0] - 1;
                    }
                }
            }
        }

        // ========= 1. 初始化（第一次）=========
        m_center = initStart;
        if (cache.empty())
        {
            int start = m_center - 4;
            if (start < 0) start = 0;
            if (start + 9 > dim) start = dim - 9;
            if (start < 0) start = 0;

            for (int i = 0; i < 9; i++)
            {
                SliceCacheBlock block;
                block.sliceIdx = start + i;
                block.data = AllocateSliceBuffer(xyz);
                LoadSlice(xyz, block.sliceIdx, block.data);
                cache.push_back(block);
            }
            return true;
        }

        int front = cache.front().sliceIdx;
        int back = cache.back().sliceIdx;

        // ========= 2. 已命中（无需更新）=========
        if (m_center >= front && m_center <= back)
        {
            // 判断是否需要扩展（保证±3）
            if (m_center <= front + 2 && front > 0)
            {
                //前扩展1个
                SliceCacheBlock block;
                block.sliceIdx = front - 1;
                block.data = AllocateSliceBuffer(xyz);
                LoadSlice(xyz, block.sliceIdx, block.data);

                cache.push_front(block);

                delete[] cache.back().data;
                cache.pop_back();
            }
            else if (m_center >= back - 2 && back < dim - 1)
            {
                //向后扩展1个
                SliceCacheBlock block;
                block.sliceIdx = back + 1;
                block.data = AllocateSliceBuffer(xyz);
                LoadSlice(xyz, block.sliceIdx, block.data);

                cache.push_back(block);

                delete[] cache.front().data;
                cache.pop_front();
            }

            return true;
        }

        // ========= 3. 跳跃（非±1）=========
        for (auto& b : cache)
        {
            delete[] b.data;
        }
        cache.clear();

        int start = m_center - 4;
        if (start < 0) start = 0;
        if (start + 9 > dim) start = dim - 9;
        if (start < 0) start = 0;

        for (int i = 0; i < 9; i++)
        {
            SliceCacheBlock block;
            block.sliceIdx = start + i;
            block.data = AllocateSliceBuffer(xyz);
            LoadSlice(xyz, block.sliceIdx, block.data);
            cache.push_back(block);
        }

        return 0;
    }
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
    ~RawSliceReader()
    {
        for (int i = 0; i < 3; i++)
        {
            for (auto& b : m_cache[i])
            {
                delete[] b.data;
            }
            m_cache[i].clear();
        }
    }
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
        /////++++
        vtkInformation* outInfo = outputVector->GetInformationObject(0);
        vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

        int extent[6];
        outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent);
        output->SetExtent(extent);
        output->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

        int xMin = extent[0], xMax = extent[1];
        int yMin = extent[2], yMax = extent[3];
        int zMin = extent[4], zMax = extent[5];

        //if (0)
        {
        ///+++++
        if (xMin == 0 && xMax == m_Dim[0] - 1 && yMin == 0 && yMax == m_Dim[1] - 1 && zMin == zMax)
        {
            Cache(0, zMin);
            auto& cache = m_cache[0];
            for (auto& b : cache)
            {
                if (b.sliceIdx == zMin)
                {
                    memcpy(output->GetScalarPointer(), b.data, (size_t)m_Dim[0] * m_Dim[1] * 2ULL);
                    return 1;
                }
            }
        }
        else if (xMin == 0 && xMax == m_Dim[0] - 1 && zMin == 0 && zMax == m_Dim[2] - 1 && yMin == yMax)
        {
            Cache(1, yMin);
            auto& cache = m_cache[1];
            for (auto& b : cache)
            {
                if (b.sliceIdx == yMin)
                {
                    memcpy(output->GetScalarPointer(), b.data, (size_t)m_Dim[0] * m_Dim[2] * 2ULL);
                    return 1;
                }
            }
        }
        else
        {
            Cache(2, xMin);
            auto& cache = m_cache[2];
            for (auto& b : cache)
            {
                if (b.sliceIdx == xMin)
                {
                    memcpy(output->GetScalarPointer(), b.data, (size_t)m_Dim[1] * m_Dim[2] * 2ULL);
                    return 1;
                }
            }
        }

        ///+++
        }

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
    m_sliceReaderYZ = RawSliceReader::New();
    m_sliceReaderXZ = RawSliceReader::New();

    connect(ui->m_selectPB, SIGNAL(clicked()), SLOT(SelectRaw()));
    connect(ui->m_rawPB, SIGNAL(clicked()), SLOT(Mhd2Stream()));
    connect(ui->m_showImages, SIGNAL(clicked()), SLOT(ShowImages()));

    m_qProgressBar = new QProgressData(this);

    connect(ui->m_Up, &QPushButton::clicked, [this]
    {
        UpdateSliceNumber(1);
    });

    connect(ui->m_Down, &QPushButton::clicked, [this]
    {
        UpdateSliceNumber(-1);

    });

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

bool MainWindow::Mhd2Stream()
{
    if (ui->m_NoAll->isChecked())
    {
        return 1;
    }
    std::string OutputDir;
    std::string inputMHDPath = qPrintable(m_Mhdfilename);

    if (ui->m_DicData->isChecked())
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
    else if (ui->m_StandardData->isChecked())
    {
        QElapsedTimer ElapsedTimer;
        ElapsedTimer.start();
        size_t lastSep = inputMHDPath.find_last_of("\\/");
        if (lastSep != std::string::npos)
        {
            OutputDir = inputMHDPath.substr(0, lastSep + 1) + "StandardData";   // 临时写死，实际应解析
            EnsureDirectoryExists(OutputDir);
        }
        GenerateVolumeMPR(OutputDir, true);
        QString str = QString::number(ElapsedTimer.elapsed());
        QMessageBox::information(nullptr, "ElapsedTimer", str, QMessageBox::Yes, QMessageBox::Yes);//1384070
        //GenerateVolumeMPR("D:/CT_3D/Test_Data/ID/id_ct", true);
    }
    else if (ui->m_YxRaw->isChecked())
    {
        QElapsedTimer ElapsedTimer;
        ElapsedTimer.start();
        bool flag = ConvertMHD_XY_To_YX(inputMHDPath);
        QString str = QString::number(ElapsedTimer.elapsed());

        QMessageBox::information(nullptr, "ElapsedTimer", str, QMessageBox::Yes, QMessageBox::Yes);//1384070
        return flag;
    }

    return 1;
}

bool  MainWindow::UpdateSliceNumber(int inc)
{
    if (m_sliceReader )
    {
        if (m_sliceReader->m_Dim[0] != 0)
        {
            int y = m_mapperXZ->GetSliceNumber();
            if (y < m_sliceReader->m_Dim[1] && y >= 0)
            {
                m_mapperXZ->SetSliceNumber(y + inc);
                m_vtkRenderWindowXZ->Render();
            }

            int x = m_mapperYZ->GetSliceNumber();
            if (x < m_sliceReader->m_Dim[0] && x >= 0)
            {
                m_mapperYZ->SetSliceNumber(x + inc);
                m_vtkRenderWindowYZ->Render();
            }

            int z = m_mapperXY->GetSliceNumber();
            if (z < m_sliceReader->m_Dim[2] && z >= 0)
            {
                m_mapperXY->SetSliceNumber(z + inc); 
                m_vtkRenderWindowXY->Render();
            }
        }

    }
    return 1;
}


bool MainWindow::ShowImages()
{       
    std::string YxRawDir, inputMHDPath;
    inputMHDPath = qPrintable(m_Mhdfilename);
    size_t dotPos = inputMHDPath.rfind('.');
    if (dotPos != std::string::npos)
    {
        YxRawDir = inputMHDPath.substr(0, dotPos) + ".raw.yx";
    }
    QFileInfo fileinfo(YxRawDir.c_str());
    if (!fileinfo.exists())
    {       
        QMessageBox::warning(NULL,"警告!", "缺少YX方向的裸数据文件,退出 选 YX-RAW！");
        return false;
    }
    m_qProgressBar->show();
    ///{ 18739, 13714, 100 };
    m_sliceReaderYZ->SetFileName(qPrintable(m_Mhdfilename));
    m_sliceReaderXZ->SetFileName(qPrintable(m_Mhdfilename));
    m_sliceReader->SetFileName(qPrintable(m_Mhdfilename));

    m_sliceReaderYZ->Cache(1, -1);
    m_sliceReaderXZ->Cache(2, -1);
    m_sliceReader->Cache(0, -1);

    AsyncThread(
        // 子线程执行
        [](RawSliceReader *reader1, RawSliceReader *reader2, RawSliceReader *reader3, QProgressData* q) -> int
        {
            //qDebug() << "Worker thread:" << QThread::currentThread();
            //QThread::sleep(2);
            reader1->Cache(1, -1);
            reader2->Cache(2, -1);
            reader3->Cache(0, -1);

            return 1;
        },

        //主线程
        [&](int result) 
            {
                //qDebug() << "Back to main thread:" << QThread::currentThread();
                //
                if (result == 1)
                {
                    //qDebug() << "Result =" << result;
                    Render();
                }
            },
            m_sliceReaderYZ, m_sliceReaderXZ, m_sliceReader, m_qProgressBar
        );

    return 1;
}

bool MainWindow::Render()
{
    m_qProgressBar->hide();
    m_mapperYZ->SetInputConnection(m_sliceReaderYZ->GetOutputPort());
    m_mapperYZ->SetOrientationToX();
    //m_mapperYZ->SetSliceNumber(9369);
    m_mapperYZ->SetSliceNumber(m_sliceReaderYZ->m_Dim[0] / 2);
    m_mapperYZ->StreamingOn();
    m_mapperYZ->SliceAtFocalPointOff();
    m_mapperYZ->SliceFacesCameraOff();
    m_imageSliceYZ->SetMapper(m_mapperYZ);
    m_imageSliceYZ->GetProperty()->SetColorWindow(4000);
    m_imageSliceYZ->GetProperty()->SetColorLevel(1000);
    m_imageSliceYZ->GetProperty()->SetInterpolationTypeToNearest();
    m_rendererYZ->AddViewProp(m_imageSliceYZ);
    m_vtkRenderWindowYZ = ui->m_sagital2DView->renderWindow();
    m_vtkRenderWindowYZ->AddRenderer(m_rendererYZ);
    ui->m_sagital2DView->interactor()->SetRenderWindow(m_vtkRenderWindowYZ);
    ui->m_sagital2DView->interactor()->RemoveObservers(vtkCommand::LeftButtonPressEvent);
    ui->m_sagital2DView->interactor()->RemoveObservers(vtkCommand::MouseWheelBackwardEvent);
    ui->m_sagital2DView->interactor()->RemoveObservers(vtkCommand::MouseWheelForwardEvent);

    // 假设已有 vtkImageSlice* imageSlice
    EnableRightClickWindowLevel(ui->m_sagital2DView->interactor(), m_imageSliceYZ, ui->m_sagital2DView, m_mapperYZ);

    m_rendererYZ->GetActiveCamera()->SetPosition(1, 0, 0);
    m_rendererYZ->GetActiveCamera()->SetViewUp(0, 0, 1);
    //m_rendererYZ->GetActiveCamera()->SetViewUp(0, 1, 0);// 旋转90度
    m_rendererYZ->ResetCamera(); // 自动适配图像位置
    m_vtkRenderWindowYZ->Render();

    ///++++++++++++++++++++++++++XZ+++++++++++++++++++++++++++++++++++++++++++++++++

    m_mapperXZ->SetInputConnection(m_sliceReaderXZ->GetOutputPort());
    m_mapperXZ->SetOrientationToY();
    m_mapperXZ->SetSliceNumber(m_sliceReaderXZ->m_Dim[1] / 2);
    m_mapperXZ->StreamingOn();
    m_mapperXZ->SliceAtFocalPointOff();
    m_mapperXZ->SliceFacesCameraOff();
    m_imageSliceXZ->SetMapper(m_mapperXZ);
    m_imageSliceXZ->GetProperty()->SetColorWindow(4000);
    m_imageSliceXZ->GetProperty()->SetColorLevel(1000);
    m_imageSliceXZ->GetProperty()->SetInterpolationTypeToNearest();
    m_rendererXZ->AddViewProp(m_imageSliceXZ);
    m_vtkRenderWindowXZ = ui->m_coronal2DView->renderWindow();
    m_vtkRenderWindowXZ->AddRenderer(m_rendererXZ);
    ui->m_coronal2DView->interactor()->SetRenderWindow(m_vtkRenderWindowXZ);
    ui->m_coronal2DView->interactor()->RemoveObservers(vtkCommand::LeftButtonPressEvent);
    ui->m_coronal2DView->interactor()->RemoveObservers(vtkCommand::MouseWheelBackwardEvent);
    ui->m_coronal2DView->interactor()->RemoveObservers(vtkCommand::MouseWheelForwardEvent);
    // 假设已有 vtkImageSlice* imageSlice
    EnableRightClickWindowLevel(ui->m_coronal2DView->interactor(), m_imageSliceXZ, ui->m_coronal2DView, m_mapperXZ);
    m_rendererXZ->GetActiveCamera()->SetPosition(0, -1, 0);
    m_rendererXZ->GetActiveCamera()->SetViewUp(0, 0, 1);
    m_rendererXZ->ResetCamera(); // 自动适配图像位置
    m_vtkRenderWindowXZ->Render();
    /// TODO 

    m_mapperXY->SetInputConnection(m_sliceReader->GetOutputPort());
    m_mapperXY->SetOrientationToZ();
    m_mapperXY->SetSliceNumber(m_sliceReader->m_Dim[2] / 2);
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
    ui->m_axial2DView->interactor()->RemoveObservers(vtkCommand::LeftButtonPressEvent);
    ui->m_axial2DView->interactor()->RemoveObservers(vtkCommand::MouseWheelBackwardEvent);
    ui->m_axial2DView->interactor()->RemoveObservers(vtkCommand::MouseWheelForwardEvent);
    EnableRightClickWindowLevel(ui->m_axial2DView->interactor(), m_imageSliceXY, ui->m_axial2DView, m_mapperXY);
    m_rendererXY->GetActiveCamera()->SetPosition(0, 0, 1);
    m_rendererXY->GetActiveCamera()->SetViewUp(0, -1, 0);//这里调整和 INimage3D.exe 现在一致
    m_rendererXY->ResetCamera(); // 自动适配图像位置
    m_vtkRenderWindowXY->Render();

    return true;

}
bool MainWindow::MHD_To_DICOM_CT_Stream(DataInfo *info)//const std::string& mhdPath, const std::string& outputDir)
{
    DataInfo data(info->m_inputPath, info->m_outputDir);

    return MHD_To_CT_Stream(data);
}

MainWindow::~MainWindow()
{
    delete ui;
    /*
    vtkImageSlice *m_imageSliceXY, *m_imageSliceXZ, *m_imageSliceYZ;
    vtkRenderer *m_rendererXY, *m_rendererXZ, *m_rendererYZ;
    vtkImageSliceMapper *m_mapperXY, *m_mapperYZ, *m_mapperXZ;
    RawSliceReader *m_sliceReader, *m_sliceReaderYZ, *m_sliceReaderXZ;
    */
    if (m_imageSliceXY)
    {
        m_imageSliceXY->Delete();
    }
    if (m_imageSliceXZ)
    {
        m_imageSliceXZ->Delete();
    }
    if (m_imageSliceYZ)
    {
        m_imageSliceYZ->Delete();
    }

    if (m_rendererXY)
    {
        m_rendererXY->Delete();
    }
    if (m_rendererXZ)
    {
        m_rendererXZ->Delete();
    }
    if (m_rendererYZ)
    {
        m_rendererYZ->Delete();
    }

    if (m_mapperXY)
    {
        m_mapperXY->Delete();
    }
    if (m_mapperYZ)
    {
        m_mapperYZ->Delete();
    }
    if (m_mapperXZ)
    {
        m_mapperXZ->Delete();
    }

    if (m_sliceReader)
    {
        m_sliceReader->Delete();
    }
    if (m_sliceReaderYZ)
    {
        m_sliceReaderYZ->Delete();
    }
    if (m_sliceReaderXZ)
    {
        m_sliceReaderXZ->Delete();
    }
    return;
    //vtkRenderWindow* m_vtkRenderWindowXY, *m_vtkRenderWindowXZ, *m_vtkRenderWindowYZ;
    //m_vtkRenderWindowXY = ui->m_axial2DView->renderWindow();
    
}


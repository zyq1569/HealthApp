#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>
// DCMTK
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcuid.h>



MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->m_selectPB, SIGNAL(clicked()), SLOT(SelectRaw()));
    connect(ui->m_rawPB, SIGNAL(clicked()), SLOT(Mhd2Stream()));
}

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
    std::string OutputDir;
    std::string inputMHDPath = qPrintable(m_Mhdfilename);
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


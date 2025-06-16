#include "qtvtkrenderwindows.h"
#include "ui_qtvtkrenderwindows.h"

#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkImageReader2.h>
#include <vtkMetaImageWriter.h>
#include <vtkMetaImageReader.h>
#include <fstream>
#include <iostream>

#include <Qdir>
#include <QMessageBox>
#include <QSettings>

#include <QElapsedTimer>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <vtkStringArray.h>
//using namespace QtConcurrent;

class DataIno
{
public:
    DataIno(QStringList list, int w = 512, int h = 512)
    {
        m_width = w;
        m_height = h;
        m_array = vtkStringArray::New();
        for (int i = 0; i < list.size(); i++)
        {
            m_array->InsertNextValue(qPrintable(list.at(i)));
        }
        m_reader = vtkSmartPointer<vtkImageReader2>::New();
    }
    vtkSmartPointer<vtkImageReader2 >  m_reader;
    vtkStringArray *m_array;
    int m_width, m_height;
};

int vtkReadFiles(DataIno info)
{
    vtkSmartPointer<vtkImageReader2> reader = info.m_reader;
    reader->SetFileNames(info.m_array);
    reader->SetFileDimensionality(2);  // 2D 图像
    reader->SetDataExtent(0, info.m_width - 1, 0, info.m_height - 1, 0, 0);  // 2D 读取
    reader->SetDataScalarTypeToUnsignedShort();  // 无符号整型
    reader->SetNumberOfScalarComponents(1);
    reader->SetFileLowerLeft(true);  // 数据从左下角开始
    // 设置像素间距
    reader->SetDataSpacing(0.025, 0.025, 0.025); // X, Y, Z 间距匹配 Scale_x/y/z
    reader->Update();
    return 1;
}

QtVTKRenderWindows::QtVTKRenderWindows(QWidget *parent) : QMainWindow(parent), ui(new Ui::QtVTKRenderWindows)
{
    ui->setupUi(this);

    connect(ui->m_pbRaw2Mhd, SIGNAL(clicked()), this, SLOT(raw2mhd()));

    //QString dir = QCoreApplication::applicationDirPath();
    //ui->m_dcmDIR->setText(dir);
}

QtVTKRenderWindows::~QtVTKRenderWindows()
{
    delete ui;
}

void QtVTKRenderWindows::raw2vtkmhd()
{
    QString dir = m_fileDir;
    if (!QDir(dir).exists())
    {
        return;
    }

    QString filenameExtra = m_extraName;
    int width = m_width;	int height = m_hight;
    bool bzipdata = (m_bzip > 0);
    // 逐个读取 0～200 的 RAW 文件
    QString raw, savefname = "/" + m_saveFileName;
    if (bzipdata)
    {
        raw = dir + savefname + "_temp.raw";
    }
    else
    {
        raw = dir + savefname + ".raw";
    }

    std::string rawFilename = qPrintable(raw);// `.raw` 文件
    QString mhd;
    if (bzipdata)
    {
        mhd = dir + savefname + "_temp.mhd";
    }
    else
    {
        mhd = dir + savefname + ".mhd";
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++int argc, char *argv[]
    QSettings settings("Raw2Mhd.ini", QSettings::IniFormat);
    settings.clear();
    settings.setValue("start/progress", 0);
    settings.setValue("start/argv", m_argvstr);
    settings.sync();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++
    std::string mhdFilename = qPrintable(mhd);// `.mhd` 文件
    std::ofstream rawFile(rawFilename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!rawFile.is_open())
    {
        return;
    }

    int formateN = (m_numberStyle.length() > 0) ? (m_numberStyle.toInt()) : (0);
    int index_s = m_start;
    int index_e = m_end + 1;
    int depth = index_e - index_s;
    int start_p = (index_s > 0) ? (index_s) : (1);
    int step = 0;
    for (int i = index_s; i < index_e; i++, step++)
    {
        QString index = dir + "\\" + m_filesName;//
        if (m_numberStyle.length() > 0)
        {
            index += QString("%1").arg(i, formateN, 10, QLatin1Char('0'));
            index += filenameExtra;
        }
        else
        {
            index += QString("%1").arg(i);
            index += filenameExtra;
        }

        std::string filename = qPrintable(index);
        QFileInfo file(index);
        if (!file.isFile())
        {
            QString info = index + "读取的数据文件不存在";//;//tr("文件不存在")
            QMessageBox::warning(NULL, "No file", info, QMessageBox::Yes, QMessageBox::Yes);
            rawFile.close();
            QFile::remove(rawFilename.c_str());
            return;
        }
        vtkSmartPointer<vtkImageReader2> reader = vtkSmartPointer<vtkImageReader2>::New();
        reader->SetFileName(filename.c_str());
        reader->SetFileDimensionality(2);  // 2D 图像
        reader->SetDataExtent(0, width - 1, 0, height - 1, 0, 0);  // 2D 读取
        reader->SetDataScalarTypeToUnsignedShort();  // 16-bit 无符号整型
        reader->SetNumberOfScalarComponents(1);
        reader->SetFileLowerLeft(true);  // 数据从左下角开始
        // 设置像素间距
        reader->SetDataSpacing(0.025, 0.025, 0.025); // X, Y, Z 间距匹配 Scale_x/y/z
        reader->Update();

        vtkSmartPointer<vtkImageData> imageData = reader->GetOutput();
        int* dims = imageData->GetDimensions();
        int dataSize = dims[0] * dims[1] * sizeof(unsigned short);
        rawFile.write(static_cast<char*>(imageData->GetScalarPointer()), dataSize);

        //++++++++++++++++++++++++++++
        if (step > 49)
        {
            double d = 0.95*(index_s++) / (depth);
            settings.setValue("start/progress", d);
            settings.sync();
            step = 0;
        }
        //+++++++++++++++++++++++++++++++
    }
    rawFile.close();


    // 生成 MHD 头文件（只执行一次）
    std::ofstream mhdFile(mhdFilename);
    if (!mhdFile.is_open())
    {
        return;
    }
    mhdFile << "ObjectType = Image\n";
    mhdFile << "NDims = 3\n";
    mhdFile << "DimSize = " << width << " " << height << " " << depth << "\n";
    mhdFile << "ElementSpacing = " << 0.025 << " " << 0.025 << " " << 0.025 << "\n";
    mhdFile << "ElementType = MET_USHORT\n";
    mhdFile << "ElementDataFile = " << rawFilename << "\n";
    mhdFile.close();

    //++++++++++++++++++++++++++++
    settings.setValue("start/progress", 96);
    settings.sync();
    //+++++++++++++++++++++++++++++++

    if (bzipdata)
    {
        //++++++++++++++++++++++++++++
        settings.setValue("start/progress", 98);
        settings.setValue("start/zipdata", 1);
        settings.sync();
        //+++++++++++++++++++++++++++++++
        vtkSmartPointer<vtkMetaImageReader> metaReader = vtkSmartPointer<vtkMetaImageReader>::New();
        metaReader->SetFileName(mhdFilename.c_str());
        metaReader->Update();

        mhd = dir + savefname + ".mhd";
        std::string mhdFilename_new = qPrintable(mhd);
        vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
        writer->SetFileName(mhdFilename_new.c_str());
        //--可以修改后缀名称——---20250410-
        //mhd = dir + savefname + ".raw";
        //std::string mhdFilename_new = qPrintable(mhd);
        //writer->SetRAWFileName(mhdFilename_new.c_str());
        //----------------------------------------------
        writer->SetInputData(metaReader->GetOutput());
        writer->Write();

        //--delete
        QFile::remove(mhdFilename.c_str());
        QFile::remove(rawFilename.c_str());
        //++++++++++++++++++++++++++++
        settings.setValue("start/zipdata", 100);
        settings.sync();
        //+++++++++++++++++++++++++++++++
    }

    //++++++++++++++++++++++++++++
    settings.setValue("start/progress", 100);
    settings.sync();
    //+++++++++++++++++++++++++++++++

}

void QtVTKRenderWindows::raw2mhd()
{
    QString dir = ui->m_dcmDIR->toPlainText();
    if (!QDir(dir).exists())
    {
        return;
    }

    QString w = ui->m_width->toPlainText();
    QString h = ui->m_height->toPlainText();
    QString filenameExtra;
    if (ui->m_extraCheck->isChecked())
    {
        filenameExtra = ui->m_extra->toPlainText();
    }
    int width = w.toInt();
    int height = h.toInt();

    bool bzipdata = ui->m_ckzipdata->isChecked();
    // 逐个读取 0～200 的 RAW 文件
    QString raw, savefname = "/VTK_Data";
    if (ui->m_saveFilename->isChecked())
    {
        savefname = "/" + ui->m_tsavefname->toPlainText();
    }
    if (bzipdata)
    {
        raw = dir + savefname + "_temp.raw";
    }
    else
    {
        raw = dir + savefname + ".raw";
    }

    std::string rawFilename = qPrintable(raw);// `.raw` 文件
    QString mhd;
    if (bzipdata)
    {
        mhd = dir + savefname + "_temp.mhd";
    }
    else
    {
        mhd = dir + savefname + ".mhd";
    }

    std::string mhdFilename = qPrintable(mhd);// `.mhd` 文件
    std::ofstream rawFile(rawFilename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!rawFile.is_open())
    {
        return;
    }

    QStringList filesList;
    QString fn = ui->m_filename->toPlainText();
    QString fa = ui->m_fmn->toPlainText();
    int formateN = fa.toInt();
    int index_s = ui->m_start->toPlainText().toInt();
    int index_e = ui->m_end->toPlainText().toInt() + 1;
    int depth = index_e - index_s;
    for (int i = index_s; i < index_e; i++)
    {
        QString index = dir + "\\" + fn;//
        if (ui->m_formate->isChecked())
        {
            index += QString("%1").arg(i, formateN, 10, QLatin1Char('0'));
            index += filenameExtra;
        }
        else
        {
            index += QString("%1").arg(i);
            index += filenameExtra;
        }
        filesList.push_back(index);

    }
    //++++++++++++++++++++++
    QElapsedTimer ElapsedTimer;
    ElapsedTimer.start();

    int count = filesList.size();
    QStringList list1, list2, list3;
    for (int i = 0; i < count / 3; i++)
    {
        list1.push_back(filesList[i]);
    }
    for (int i = count / 3; i < 2 * count / 3; i++)
    {
        list2.push_back(filesList[i]);
    }
    for (int i = 2 * count / 3; i < count; i++)
    {
        list3.push_back(filesList[i]);
    }
    DataIno info1(list1), info2(list2), info3(list3);

    QList < DataIno > listDatas = { info1 ,info2 ,info3 };
    auto listFuture = QtConcurrent::mapped(listDatas, vtkReadFiles);
    listFuture.waitForFinished();

    vtkSmartPointer<vtkImageData> imageData = info1.m_reader->GetOutput();
    int* dims = imageData->GetDimensions();
    int dataSize = dims[0] * dims[1] * sizeof(unsigned short)*list1.size();
    rawFile.write(static_cast<char*>(imageData->GetScalarPointer()), dataSize);

    imageData = info2.m_reader->GetOutput();
    dataSize = dims[0] * dims[1] * sizeof(unsigned short)*list2.size();
    rawFile.write(static_cast<char*>(imageData->GetScalarPointer()), dataSize);

    imageData = info3.m_reader->GetOutput();
    dataSize = dims[0] * dims[1] * sizeof(unsigned short)*list3.size();
    rawFile.write(static_cast<char*>(imageData->GetScalarPointer()), dataSize);
    rawFile.close();
    
    QString str = QString::number(ElapsedTimer.elapsed());
    QMessageBox::information(nullptr, "ElapsedTimer", str, QMessageBox::Yes, QMessageBox::Yes);
    // ===========生成 MHD 头文件（只执行一次）
    std::ofstream mhdFile(mhdFilename);
    if (!mhdFile.is_open())
    {
        return;
    }
    mhdFile << "ObjectType = Image\n";
    mhdFile << "NDims = 3\n";
    mhdFile << "DimSize = " << width << " " << height << " " << depth << "\n";
    mhdFile << "ElementSpacing = " << 0.025 << " " << 0.025 << " " << 0.025 << "\n";
    mhdFile << "ElementType = MET_USHORT\n";
    mhdFile << "ElementDataFile = " << rawFilename << "\n";
    mhdFile.close();


    if (bzipdata)
    {
        vtkSmartPointer<vtkMetaImageReader> metaReader = vtkSmartPointer<vtkMetaImageReader>::New();
        metaReader->SetFileName(mhdFilename.c_str());
        metaReader->Update();

        mhd = dir + savefname + ".mhd";
        std::string mhdFilename_new = qPrintable(mhd);
        vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
        writer->SetFileName(mhdFilename_new.c_str());
        //--可以修改后缀名称——---20250410-
        //mhd = dir + savefname + ".raw";
        //std::string mhdFilename_new = qPrintable(mhd);
        //writer->SetRAWFileName(mhdFilename_new.c_str());
        //----------------------------------------------
        writer->SetInputData(metaReader->GetOutput());
        writer->Write();

        //--delete
        QFile::remove(mhdFilename.c_str());
        QFile::remove(rawFilename.c_str());
    }
}
#include "qtvtkrenderwindows.h"
#include "ui_qtvtkrenderwindows.h"

#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkImageReader2.h>
#include <fstream>
#include <iostream>

#include<Qdir>

QtVTKRenderWindows::QtVTKRenderWindows(QWidget *parent) : QMainWindow(parent), ui(new Ui::QtVTKRenderWindows)
{
    ui->setupUi(this);

	connect(ui->m_pbRaw2Mhd, SIGNAL(clicked()), this, SLOT(raw2mhd()));

	QString dir = QCoreApplication::applicationDirPath();
	ui->m_dcmDIR->setText(dir);

}

QtVTKRenderWindows::~QtVTKRenderWindows()
{
    delete ui;
}

void QtVTKRenderWindows::raw2mhd()
{
    QString dir    = ui->m_dcmDIR->toPlainText();
    QString w      = ui->m_width->toPlainText();
    QString h      = ui->m_height->toPlainText();
    QString number = ui->m_number->toPlainText();
    QString filenameExtra;
    if (ui->m_extraCheck->isChecked())
    {
        filenameExtra = ui->m_extra->toPlainText();
    }
    int width  = w.toInt();
    int height = h.toInt();
    int depth  = number.toInt();  // 0~200 共 201 张
    // 逐个读取 0～200 的 RAW 文件
	QString raw, savefname= "/VTK_Data";
	if (ui->m_saveFilename->isChecked())
	{
		savefname = "/"+ui->m_tsavefname->toPlainText();
	}
	raw = dir + savefname + ".raw";

    std::string rawFilename = qPrintable(raw);// `.raw` 文件
    QString mhd             = dir + savefname +".mhd";
    std::string mhdFilename = qPrintable(mhd);// `.mhd` 文件
    std::ofstream rawFile(rawFilename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!rawFile.is_open())
    {
        return ;
    }
    QString fn = ui->m_filename->toPlainText();
    QString fa = ui->m_fmn->toPlainText();
    int formateN = fa.toInt();
    int index_s  = ui->m_start->toPlainText().toInt();
    int index_e  = index_s + depth;

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

        std::string filename = qPrintable(index);
        QFileInfo file(index);
        if (!file.isFile())
        {
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

    }
	rawFile.close();


    // 生成 MHD 头文件（只执行一次）
    std::ofstream mhdFile(mhdFilename);
    if (!mhdFile.is_open())
    {
        return ;
    }
    mhdFile << "ObjectType = Image\n";
    mhdFile << "NDims = 3\n";
    mhdFile << "DimSize = " << width << " " << height << " " << depth << "\n";
    mhdFile << "ElementSpacing = " << 0.025 << " " << 0.025 << " " << 0.025 << "\n";
    mhdFile << "ElementType = MET_USHORT\n";
    mhdFile << "ElementDataFile = " << rawFilename << "\n";
    mhdFile.close();
}


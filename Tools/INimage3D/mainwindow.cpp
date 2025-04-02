#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qfourpaneviewer.h"
#include "q3dviewer.h"
#include "qprogressData.h"
#include <vtkMetaImageReader.h>
//#include <vtkXMLImageDataWriter.h>
#include <vtkXMLImageDataReader.h>

#include <QFileDialog>
#include <QTabBar>
#include <QToolBar>
#include <QProgressDialog>
#pragma execution_character_set("utf-8")
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);
    setStatusBar(nullptr);
    setMenuBar(nullptr);

    m_image4Plane         = nullptr;
    m_MetaReader          = nullptr;
    m_XMLImageDataReader  = nullptr;
    m_closeMetaFile       = false;
                          
    m_vtkImageData        = nullptr;
    m_vtkAlgorithmOutput  = nullptr;

    m_index3D = m_index4P = -1;

    m_checkStart3D = m_checkStart4Plane = m_check3Dcolor = m_checkDefaultWL = m_colorGradient = false;
    m_DefaultLevel = m_DefaultWindow = 0;

    showMaximized();
    this->setWindowIcon(QIcon(":images/icons/pyramid.png"));

    m_workspace = new QTabWidget(this);

    this->setCentralWidget(m_workspace);
    m_workspace->setStyleSheet("background-color:rgb(48,48,48)}");//m_workspace->setStyleSheet("border-top-color:rgb(48,48,48,0)}");

    m_workspace->findChildren<QTabBar*>().at(0)->hide();

    m_mainToolbar = new QToolBar(this);
    this->addToolBar(Qt::TopToolBarArea, m_mainToolbar);
    m_mainToolbar->setFloatable(false);
    m_mainToolbar->setMovable(false);
    m_mainToolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    //
    m_openAction = new QAction("文件(&O)", this);
    m_openAction->setShortcut(QKeySequence("Ctrl+O"));
    m_mainToolbar->addAction(m_openAction);
    connect(m_openAction, &QAction::triggered, [this]
    {
        initMetaFile();
    });

    m_checkStart3D = m_checkStart4Plane = m_check3Dcolor = m_checkDefaultWL = false;
    m_configForm = new ConfigForm(this);
    m_configForm->InitConfig();

    m_editor = new QAction("调参", this);
    m_mainToolbar->addAction(m_editor);
    connect(m_editor, &QAction::triggered, [this]
    {
        if (m_image4Plane)
        {
            ((QFourpaneviewer*)m_image4Plane)->ShowEditorsWidget();
        }

    });

    m_saveImage = new QAction("导图", this);
    m_mainToolbar->addAction(m_saveImage);
    connect(m_saveImage, &QAction::triggered, [this]
    {
        if (m_image4Plane)
        {
            ((QFourpaneviewer*)m_image4Plane)->SavePaneImage();
        }
    });

    m_cfigQA = new QAction("配置", this);
    m_mainToolbar->addAction(m_cfigQA);
    connect(m_cfigQA, &QAction::triggered, [this]
    {
        m_configForm->show();
    });

    m_qProgressBar = new QProgressData(this);// (m_workspace);

    m_editor->setEnabled(false);
    m_saveImage->setEnabled(false);
}

void MainWindow::starViewer()
{
    initMetaFile();
    m_commdFilePath = "";
}

void MainWindow::initMetaFile()
{
    if (m_vtkImageData)
    {
        m_vtkImageData = nullptr;
        m_vtkAlgorithmOutput = nullptr;
        if (m_MetaReader)
        {
            m_MetaReader->Delete();
            m_MetaReader = nullptr;
        }
        if (m_XMLImageDataReader)
        {
            m_XMLImageDataReader->Delete();
            m_XMLImageDataReader = nullptr;
        }

        if (m_image4Plane)
        {
            //	//delete m_workspace->widget(m_index4P);// ->deleteLater();
            delete m_image4Plane;
            m_image4Plane = nullptr;
            m_workspace->removeTab(m_index4P);
            m_index4P = -1;
            m_editor->setEnabled(false);
            m_saveImage->setEnabled(false);
        }

        m_openAction->setText("文件(&O)");
        m_openAction->setShortcut(QKeySequence("Ctrl+O"));
        return;
    }

    QString Mhdfilename;
    if (m_commdFilePath.length() < 1)
    {
        Mhdfilename = QFileDialog::getOpenFileName(this, "mhd|vti file", QString(), "*.mhd;*.vti");
    }
    else
    {
        Mhdfilename = m_commdFilePath;
    }

    //QString Mhdfilename = QFileDialog::getOpenFileName(this, "mhd file", QString(), "*.mhd");
    QFileInfo finfo(Mhdfilename);
    if (!finfo.exists())
    {
        return;
    }
    // ------------------show UI-----------------------------------------
    m_image4Plane = new QFourpaneviewer(this);
    m_index4P = m_workspace->addTab(m_image4Plane, "image4Plane");
    m_workspace->setCurrentIndex(m_index4P);

    //----------ProgressBar--start---------------------------------
    m_qProgressBar->show();
    std::string filename = qPrintable(Mhdfilename);
    if (Mhdfilename.contains(".mhd"))
    {
        m_MetaReader = vtkMetaImageReader::New();
        m_MetaReader->SetFileName(filename.c_str());
        m_MetaReader->Update();
        m_vtkImageData = m_MetaReader->GetOutput();
        m_vtkAlgorithmOutput = m_MetaReader->GetOutputPort();
    }
    else
    {
        m_XMLImageDataReader = vtkXMLImageDataReader::New();
        m_XMLImageDataReader->SetFileName(filename.c_str());  // 输入的压缩 VTI 文件名
        m_XMLImageDataReader->Update();
        m_vtkImageData = m_XMLImageDataReader->GetOutput();
        m_vtkAlgorithmOutput = m_XMLImageDataReader->GetOutputPort();
    }

    m_openAction->setText("关闭(&C)");
    m_openAction->setShortcut(QKeySequence("Ctrl+C"));

    //-------------ProgressBar-----end---------------------------------------
    showImage4Plane();
    m_qProgressBar->hide();
    m_editor->setEnabled(true);
    m_saveImage->setEnabled(true);
}

void MainWindow::setEnabledQAction()
{
    m_show3D->setEnabled(true);
    m_show4Plane->setEnabled(true);
}
MainWindow::~MainWindow()
{
    if (m_workspace)
    {
        delete m_workspace;
    }

    if (m_MetaReader)
    {
        m_MetaReader->Delete();
        m_MetaReader = nullptr;
    }
    if (m_XMLImageDataReader)
    {
        m_XMLImageDataReader->Delete();
        m_XMLImageDataReader = nullptr;
    }

    delete m_configForm;
    delete ui;
}


void MainWindow::showImage3D()
{

}

void MainWindow::showImage4Plane()
{
    if (!m_vtkImageData)
    {
        return;
    }
    ((QFourpaneviewer*)m_image4Plane)->Show3DPlane();
}
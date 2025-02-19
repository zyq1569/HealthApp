#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qfourpaneviewer.h"
#include "q3dviewer.h"
#include <vtkMetaImageReader.h>

#include <QFileDialog>
#include <QTabBar>
#include <QToolBar>
#pragma execution_character_set("utf-8")
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	this->setAttribute(Qt::WA_DeleteOnClose);
	setStatusBar(nullptr);
	setMenuBar(nullptr);

	m_image3D = nullptr;
	m_image4Plane = nullptr;
	m_MetaReader = nullptr;

	m_index3D = m_index4P = -1;

	showMaximized();
	this->setWindowIcon(QIcon(":images/icons/pyramid.png"));
	

	//QPalette pal;
	//pal.setColor(QPalette::Window, QColor(48, 48, 48));
	//this->setPalette(pal);
	//this->setStyleSheet("background-color:rgb(48,48,48)}");
	//this->setStyleSheet("background-color:rgb(30,30,30)}");

	m_workspace = new QTabWidget(this);

	this->setCentralWidget(m_workspace);
	m_workspace->setStyleSheet("background-color:rgb(48,48,48)}");//m_workspace->setStyleSheet("border-top-color:rgb(48,48,48,0)}");

	m_workspace->findChildren<QTabBar*>().at(0)->hide();


	//m_fileMenu = menuBar()->addMenu(tr("菜单(&F)"));
	////m_fileMenu = menuBar()->addMenu(tr("Files(&F)"));
	//m_openAction = new QAction(this);
	//m_openAction->setText(tr("文件(&O)"));
	////connect(m_openAction, &QAction::triggered, [this] { showImage3D(); });//Open an existing DICOM folder
	//connect(m_openAction, &QAction::triggered, [this]
	//{
	//	QString Mhdfilename = QFileDialog::getOpenFileName(this, "Mhd", QString(), "*.mhd");
	//	QFileInfo finfo(Mhdfilename);
	//	if (!finfo.exists())
	//	{
	//		return;
	//	}
	//	std::string filename = qPrintable(Mhdfilename);
	//	m_MetaReader = vtkMetaImageReader::New();
	//	m_MetaReader->SetFileName(filename.c_str());
	//	m_MetaReader->Update();
	//}
	//);
	//m_fileMenu->addAction(m_openAction);
	//
	//m_show3D = new QAction(this);
	//m_show3D->setText(tr("三维(&D)"));
	//connect(m_show3D, &QAction::triggered, [this] { showImage3D(); });//Open an existing DICOM folder
	//m_fileMenu->addAction(m_show3D);
	//
	//m_show4Plane = new QAction(this);
	//m_show4Plane->setText(tr("三面(&P)"));
	//connect(m_show4Plane, &QAction::triggered, [this] { showImage4Plane(); });//Open an existing DICOM folder
	//m_fileMenu->addAction(m_show4Plane);
	//
	//QAction *qaClose = new QAction(this);
	//qaClose->setText(tr("关闭(&P)"));
	//connect(qaClose, &QAction::triggered, [this] 
	//{ 
	//	delete m_image4Plane;
	//	m_image4Plane = nullptr;
	//	delete m_image3D;
	//	m_image3D = nullptr;
	//	m_workspace->removeTab(0);
	//	m_workspace->removeTab(1);
	//	m_MetaReader->Delete();
	//	m_MetaReader = nullptr;
	//
	//});//Open an existing DICOM folder
	//m_fileMenu->addAction(qaClose);

	//QPalette menupal = m_fileMenu->palette();
	//menupal.setColor(QPalette::WindowText, QColor(255, 255, 255));// Qt::black);
	//m_fileMenu->setPalette(menupal);
	//showImage3D();

	m_mainToolbar = new QToolBar(this);
	this->addToolBar(Qt::TopToolBarArea, m_mainToolbar);
	//m_mainToolbar->layout()->setSpacing(10);
	m_mainToolbar->setFloatable(false);
	m_mainToolbar->setMovable(false);
	m_mainToolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
	//
	QAction *actionFile = new QAction("文件(&O)", this);
	actionFile->setShortcut(QKeySequence("Ctrl+O"));
	m_mainToolbar->addAction(actionFile);
	connect(actionFile, &QAction::triggered, [this] 
	{ 
		QString Mhdfilename = QFileDialog::getOpenFileName(this, "Mhd", QString(), "*.mhd");
		QFileInfo finfo(Mhdfilename);
		if (!finfo.exists())
		{
			return;
		}
		std::string filename = qPrintable(Mhdfilename);
		m_MetaReader = vtkMetaImageReader::New();
		m_MetaReader->SetFileName(filename.c_str());
		m_MetaReader->Update();

		m_show3D->setEnabled(true);
		m_show4Plane->setEnabled(true);
	});

	m_show3D = new QAction("三维", this);
	m_mainToolbar->addAction(m_show3D);
	connect(m_show3D, &QAction::triggered, [this]
	{
		showImage3D();
	});
	m_show3D->setEnabled(false);

	m_show4Plane = new QAction("切面", this);
	m_mainToolbar->addAction(m_show4Plane);
	connect(m_show4Plane, &QAction::triggered, [this]
	{
		showImage4Plane();
	});
	m_show4Plane->setEnabled(false);

	//m_mainToolbar->insertSeparator(actionFile);
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

    delete ui;
}


void MainWindow::showImage3D()
{
	if (!m_MetaReader)
	{
		return;
	}
	if (!m_image3D)//m_index3D,
	{
		m_image3D = new Q3dviewer(this, m_MetaReader);
		m_index3D = m_workspace->addTab(m_image3D, "image3D");
		m_workspace->setCurrentIndex(m_index3D);
	}
	else if(m_index3D>=0)
	{
		m_workspace->setCurrentIndex(m_index3D);
	}

}

void MainWindow::showImage4Plane()
{
	if (!m_MetaReader)
	{
		return;
	}
	if (!m_image4Plane)
	{
		m_image4Plane = new QFourpaneviewer(this, m_MetaReader);
		m_index4P = m_workspace->addTab(m_image4Plane, "image4Plane");
		m_workspace->setCurrentIndex(m_index4P);
	}
	else if (m_index4P >= 0)
	{
		m_workspace->setCurrentIndex(m_index4P);
	}
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qfourpaneviewer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	this->setAttribute(Qt::WA_DeleteOnClose);
	setStatusBar(nullptr);
	//setMenuBar(nullptr);

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

	connect(ui->m_open, &QAction::triggered, [this] { showImage3D(); });//Open an existing DICOM folder
	

	//m_fileMenu = menuBar()->addMenu(tr("ÎÄ¼ş(&F)"));
	//m_fileMenu = menuBar()->addMenu(tr("Files(&F)"));
	//m_openAction = new QAction(this);
	//m_openAction->setText(tr("&Open Files..."));
	//m_fileMenu->addAction(m_openAction);



	//QPalette menupal = m_fileMenu->palette();
	//menupal.setColor(QPalette::WindowText, QColor(255, 255, 255));// Qt::black);
	//m_fileMenu->setPalette(menupal);
	//showImage3D();

	//m_mainToolbar = new QToolBar(this);
	//this->addToolBar(Qt::TopToolBarArea, m_mainToolbar);
	//m_mainToolbar->setIconSize(QSize(30, 30));
	////m_mainToolbar->layout()->setSpacing(10);
	//m_mainToolbar->setFloatable(false);
	//m_mainToolbar->setMovable(false);
	//m_mainToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	//
	//QAction *actionFile = new QAction(QIcon(":/images/folderopen.png"), "Open Files", this);
	//m_mainToolbar->addAction(actionFile);
	////connect(actionFile, &QAction::triggered, [this] { m_extensionHandler->request(6); });//Open an existing DICOM folder
	//m_mainToolbar->insertSeparator(actionFile);
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
	m_image3D = new QFourpaneviewer(this);
	m_workspace->addTab(m_image3D, "3D");
}

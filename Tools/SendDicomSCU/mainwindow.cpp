#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QThreadPool>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)//, /*scanner(patientdata),*/ sender(patientdata)
{
    ui->setupUi(this);

    QThreadPool::globalInstance()->setMaxThreadCount(10);///???

    //    QString Dir = QDir::currentPath();
    //    QString iniDir = Dir+"/config";
    //    if (!isDirExist( iniDir))
    //    {
    //        CreatDir(iniDir);
    //    }
    //    QString configfilename = iniDir+"/dicomSCU.ini";
    //#if defined(Q_OS_LINUX)
    //    configfilename = iniDir+"/dicomSCU_linux.ini";
    //#endif

    //    QSettings configini(configfilename,QSettings::IniFormat);

    connect(&m_sender, &DicomSender::finishscandicomfile, this, &MainWindow::updatePatientList);



    m_pMOdel = new QStandardItemModel(ui->tableView);

    m_pMOdel->setColumnCount(8);
    m_pMOdel->setHeaderData(0,Qt::Horizontal,QString(""));
    //m_pMOdel->setHeaderData(1,Qt::Horizontal,QString("姓名"));
    //m_pMOdel->setHeaderData(2,Qt::Horizontal,QString("PatientID"));
    //m_pMOdel->setHeaderData(3,Qt::Horizontal,QString("日期"));
    //m_pMOdel->setHeaderData(4,Qt::Horizontal,QString("描述"));
    //m_pMOdel->setHeaderData(5,Qt::Horizontal,QString("路径"));
    m_pMOdel->setHeaderData(1,Qt::Horizontal,QString("Name"));
    m_pMOdel->setHeaderData(2,Qt::Horizontal,QString("PatientID"));
    m_pMOdel->setHeaderData(3,Qt::Horizontal,QString("Date"));
    m_pMOdel->setHeaderData(4,Qt::Horizontal,QString("Dec"));
    m_pMOdel->setHeaderData(5,Qt::Horizontal,QString("SOPUID"));
    m_pMOdel->setHeaderData(6,Qt::Horizontal,QString("Path"));
    m_pMOdel->setHeaderData(7,Qt::Horizontal,QString("TransferSyntaxUID"));
    ui->tableView->setModel(m_pMOdel);
    ui->tableView->setColumnWidth(0,1);
    ui->tableView->setColumnWidth(1,90);
    ui->tableView->setColumnWidth(6,300);
    //ui->tableView->resizeColumnToContents(5);
    //ui->tableView->resizeColumnToContents(1);
    //ui->tableView->resizeColumnToContents(2);
    //ui->tableView->resizeColumnsToContents();

}

MainWindow::~MainWindow()
{
    //    QString Dir = QDir::currentPath();
    //    QString iniDir = Dir+"/config";
    //    if (!isDirExist( iniDir))
    //    {
    //        CreatDir(iniDir);
    //    }
    //    QString configfilename = iniDir+"/dicomSCU.ini";
    //#if defined(Q_OS_LINUX)
    //    configfilename = iniDir+"/dicomSCU_linux.ini";
    //#endif

    //    QSettings configini(configfilename,QSettings::IniFormat);

    //    unsigned int size = destinations.size();
    //    for (unsigned int i = 0; i < size; i++ )
    //    {
    //        std::stringstream stream;
    //        stream << destinations[i].name << ","
    //            << destinations[i].destinationHost << ","
    //            << destinations[i].destinationPort << ","
    //            << destinations[i].destinationAETitle << ","
    //            << destinations[i].ourAETitle;
    //        //QString sec ="/Destinations"
    //       // configini.setValue("/Destinations", "pacs");
    //        //configini.setValue("/",);
    //    }
    //    configini.setValue("/dir", "");

    delete ui;
}

void MainWindow::on_pbUpdate_clicked()
{
    QString dir = ui->cbDcmDir->toPlainText();

    startScan(dir);


}


void MainWindow::updatePatientList()
{
    ////show row col
    ////ui->tableView;
    int rows = m_sender.m_listpatient.size();
    for (int i=0; i<rows; i++)
    {
        Patient pt = m_sender.m_listpatient[i];
        QString name = pt.patientname.c_str();//^
        name.replace("^","");
        m_pMOdel->setItem(i,1,new QStandardItem(name.toStdString().c_str()/*pt.patientname.c_str()*/));
        m_pMOdel->setItem(i,2,new QStandardItem(pt.patientid.c_str()));
        Study st = pt.studydatas[0];

        m_pMOdel->setItem(i,3,new QStandardItem(st.studydate.c_str()));
        QString str = st.studydesc.c_str();

        if (!st.filespath.empty())
        {
            if (str.length() > 0)     str += "|";
            else                      str += "Images:";

            int size = st.filespath.size();
            str += QString::number(size);
        }
        m_pMOdel->setItem(i,4,new QStandardItem(str));
        m_pMOdel->setItem(i,5,new QStandardItem(st.sopclassuid.c_str()));
        m_pMOdel->setItem(i,6,new QStandardItem(st.dir.c_str()));
        m_pMOdel->setItem(i,7,new QStandardItem(st.transfersyntax.c_str()));
        QStandardItem *item = new QStandardItem();
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        m_pMOdel->setItem(i,0,item);
    }
    ui->tableView->setColumnWidth(0,1);
    ui->tableView->setColumnWidth(1,90);
    ui->tableView->setColumnWidth(5,150);
    ui->tableView->setColumnWidth(6,700);
}


void MainWindow::stopSend()
{
    //sender.Cancel();
}

void MainWindow::stopScan()
{
    //sender.Cancel();
}

void MainWindow::startSend(int destination, int threads)
{

}


void MainWindow::loadDestinationList()
{

}

void MainWindow::loadGlobalDestinationList()
{

}

void MainWindow::saveDestinationList()
{

}

void MainWindow::startScan(QString &path)
{
    m_sender.ScanPatient(path);

}

void MainWindow::on_pBSend_clicked()
{

    QString ip,port,ae;
    ip   = ui->cb_IP->toPlainText();
    port = ui->cb_Port->toPlainText();
    ae   = ui->cb_Aetitle->toPlainText();

    m_sender.m_destination.destinationHost    = ip.toStdString().c_str();
    m_sender.m_destination.destinationPort    = port.toInt();
    m_sender.m_destination.destinationAETitle = ae.toStdString().c_str();
    m_sender.m_destination.ourAETitle         = ui->cb_LocalAetitle->toPlainText().toStdString().c_str();

    Patient pt;
    std::vector<Patient> listpatient;
    int rows =  ui->tableView->model()->rowCount();
    for (int i=0; i<rows; i++)
    {
        if (m_pMOdel->item(i,0)->checkState() == Qt::CheckState::Checked)
        {
            //QMessageBox::information(this,tr("check"),QString::number(i) + "succeed!",QMessageBox::Ok);
            pt = m_sender.m_listpatient[i];
            listpatient.push_back(pt);
        }
    }
    m_sender.SendPatiens(listpatient);
    //foreach(Patient pt, listpatient)
    //{
    //    foreach(Study st, pt.studydatas)
    //    {

    //    }
    //}

}


void MainWindow::on_pBDir_clicked()
{
    QString dlg;
    dlg = ui->cbDcmDir->toPlainText();
    //第三个参数 如果是"./" 代表当前应用的目录. QString()空为上次打开的目录
    QString  path = QFileDialog::getExistingDirectory(this,"select dicom dir...",dlg/*QString()*/,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    //QString path(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = ShowDirsOnly);
    //ui->cbDcmDir->setCurrentText(path);
    ui->cbDcmDir->setText(path);
    ui->cbDcmDir->update();

}


void MainWindow::on_pBEcho_clicked()
{
    bool ok = m_sender.Echo();
    if (ok)
    {
        QMessageBox::information(this,tr("Echo"),"succeed!",QMessageBox::Ok);
    }
    else
    {
        QMessageBox::information(this,tr("Echo"),"fail!",QMessageBox::Ok);
    }
}














///-------------------------------------------------------------------------------------------
//1.2.840.10008.1.2=Implicit VR Little Endian: Default Transfer Syntax for DICOM
//1.2.840.10008.1.2.1=Explicit VR Little Endian
//1.2.840.10008.1.2.2=Explicit VR Big Endian
//1.2.840.10008.1.2.4.57=JPEG Lossless, Non-Hierarchical(Process 14)
//1.2.840.10008.1.2.4.70=JPEG Lossless, Non-Hierarchical,First-Order Prediction (Process 14[Selection Value 1])
//1.2.840.10008.1.2.4.50=JPEG Baseline (Process 1)
//1.2.840.10008.1.2.4.51=JPEG Extended (Process 2 & 4)
//1.2.840.10008.1.2.4.56=JPEG Full Progression, Non-Hierarchical (Process 11 & 13)
//1.2.840.10008.1.2.4.70=JPEG Lossless, Non-Hierarchical,First-Order Prediction
//1.2.840.10008.1.2.4.80=JPEG-LS Lossless Image Compression
//1.2.840.10008.1.2.4.81=JPEG-LS Lossy (Near-Lossless)Image Compression
//1.2.840.10008.1.2.4.90=JPEG 2000 Image Compression(Lossless Only)
//1.2.840.10008.1.2.4.91=JPEG 2000 Image Compression
//1.2.840.113619.5.2=(Unheard)


/*
 *
Computed Radiography Image Storage
1.2.840.10008.5.1.4.1.1.1
Computed Radiography Image IOD

Digital X-Ray Image Storage - For Presentation
1.2.840.10008.5.1.4.1.1.1.1
Digital X-Ray Image IOD


Digital X-Ray Image Storage - For Processing
1.2.840.10008.5.1.4.1.1.1.1.1
Digital X-Ray Image IOD


Digital Mammography X-Ray Image Storage - For Presentation
1.2.840.10008.5.1.4.1.1.1.2
Digital Mammography X-Ray Image IOD

(see Section B.5.1.2)
Digital Mammography X-Ray Image Storage - For Processing
1.2.840.10008.5.1.4.1.1.1.2.1
Digital Mammography X-Ray Image IOD

Digital Intra-Oral X-Ray Image Storage - For Presentation
1.2.840.10008.5.1.4.1.1.1.3
Digital Intra-Oral X-Ray Image IOD

(see Section B.5.1.2)
Digital Intra-Oral X-Ray Image Storage - For Processing
1.2.840.10008.5.1.4.1.1.1.3.1
Digital Intra-Oral X-Ray Image IOD

(see Section B.5.1.2)
CT Image Storage
1.2.840.10008.5.1.4.1.1.2
Computed Tomography Image IOD

Enhanced CT Image Storage

1.2.840.10008.5.1.4.1.1.2.1

Enhanced CT Image IOD

(see Section B.5.1.7)

Legacy Converted Enhanced CT Image Storage

1.2.840.10008.5.1.4.1.1.2.2

Legacy Converted Enhanced CT Image IOD

(see Section B.5.1.7)

Ultrasound Multi-frame Image Storage

1.2.840.10008.5.1.4.1.1.3.1

Ultrasound Multi-frame Image IOD

MR Image Storage

1.2.840.10008.5.1.4.1.1.4

Magnetic Resonance Image IOD

Enhanced MR Image Storage

1.2.840.10008.5.1.4.1.1.4.1

Enhanced MR Image IOD

(see Section B.5.1.6)

MR Spectroscopy Storage

1.2.840.10008.5.1.4.1.1.4.2

MR Spectroscopy IOD

Enhanced MR Color Image Storage

1.2.840.10008.5.1.4.1.1.4.3

Enhanced MR Color Image IOD

Legacy Converted Enhanced MR Image Storage

1.2.840.10008.5.1.4.1.1.4.4

Legacy Converted Enhanced MR Image IOD

(see Section B.5.1.6)

Ultrasound Image Storage

1.2.840.10008.5.1.4.1.1.6.1

Ultrasound Image IOD

Enhanced US Volume Storage

1.2.840.10008.5.1.4.1.1.6.2

Enhanced US Volume IOD

Secondary Capture Image Storage

1.2.840.10008.5.1.4.1.1.7

Secondary Capture Image IOD

Multi-frame Single Bit Secondary Capture Image Storage

1.2.840.10008.5.1.4.1.1.7.1

Multi-frame Single Bit Secondary Capture Image IOD

Multi-frame Grayscale Byte Secondary Capture Image Storage

1.2.840.10008.5.1.4.1.1.7.2

Multi-frame Grayscale Byte Secondary Capture Image IOD

Multi-frame Grayscale Word Secondary Capture Image Storage

1.2.840.10008.5.1.4.1.1.7.3

Multi-frame Grayscale Word Secondary Capture Image IOD

Multi-frame True Color Secondary Capture Image Storage

1.2.840.10008.5.1.4.1.1.7.4

Multi-frame True Color Secondary Capture Image IOD

12-lead ECG Waveform Storage

1.2.840.10008.5.1.4.1.1.9.1.1

12-Lead Electrocardiogram IOD

General ECG Waveform Storage

1.2.840.10008.5.1.4.1.1.9.1.2

General Electrocardiogram IOD

Ambulatory ECG Waveform Storage

1.2.840.10008.5.1.4.1.1.9.1.3

Ambulatory Electrocardiogram IOD

Hemodynamic Waveform Storage

1.2.840.10008.5.1.4.1.1.9.2.1

Hemodynamic IOD

Cardiac Electrophysiology Waveform Storage

1.2.840.10008.5.1.4.1.1.9.3.1

Basic Cardiac Electrophysiology IOD

Basic Voice Audio Waveform Storage

1.2.840.10008.5.1.4.1.1.9.4.1

Basic Voice Audio IOD

General Audio Waveform Storage

1.2.840.10008.5.1.4.1.1.9.4.2

General Audio Waveform IOD

Arterial Pulse Waveform Storage

1.2.840.10008.5.1.4.1.1.9.5.1

Arterial Pulse Waveform IOD

Respiratory Waveform Storage

1.2.840.10008.5.1.4.1.1.9.6.1

Respiratory Waveform IOD

Grayscale Softcopy Presentation State Storage

1.2.840.10008.5.1.4.1.1.11.1

Grayscale Softcopy Presentation State IOD

Color Softcopy Presentation State Storage

1.2.840.10008.5.1.4.1.1.11.2

Color Softcopy Presentation State IOD

Pseudo-Color Softcopy Presentation State Storage

1.2.840.10008.5.1.4.1.1.11.3

Pseudo-color Softcopy Presentation State IOD

Blending Softcopy Presentation State Storage

1.2.840.10008.5.1.4.1.1.11.4

Blending Softcopy Presentation State IOD

XA/XRF Grayscale Softcopy Presentation State Storage

1.2.840.10008.5.1.4.1.1.11.5

XA/XRF Grayscale Softcopy Presentation State IOD

X-Ray Angiographic Image Storage

1.2.840.10008.5.1.4.1.1.12.1

X-Ray Angiographic Image IOD

Enhanced XA Image Storage

1.2.840.10008.5.1.4.1.1.12.1.1

Enhanced X-Ray Angiographic Image IOD

X-Ray Radiofluoroscopic Image Storage

1.2.840.10008.5.1.4.1.1.12.2

X-Ray RF Image IOD

Enhanced XRF Image Storage

1.2.840.10008.5.1.4.1.1.12.2.1

Enhanced X-Ray RF Image IOD

X-Ray 3D Angiographic Image Storage

1.2.840.10008.5.1.4.1.1.13.1.1

X-Ray 3D Angiographic Image IOD

X-Ray 3D Craniofacial Image Storage

1.2.840.10008.5.1.4.1.1.13.1.2

X-Ray 3D Craniofacial Image IOD

Breast Tomosynthesis Image Storage

1.2.840.10008.5.1.4.1.1.13.1.3

Breast Tomosynthesis Image IOD

Intravascular Optical Coherence Tomography Image Storage - For Presentation

1.2.840.10008.5.1.4.1.1.14.1

Intravascular OCT IOD

( 3)

Intravascular Optical Coherence Tomography Image Storage - For Processing

1.2.840.10008.5.1.4.1.1.14.2

Intravascular OCT IOD

( 3)

Nuclear Medicine Image Storage

1.2.840.10008.5.1.4.1.1.20

Nuclear Medicine Image IOD

Raw Data Storage

1.2.840.10008.5.1.4.1.1.66

Raw Data IOD

Spatial Registration Storage

1.2.840.10008.5.1.4.1.1.66.1

Spatial Registration IOD

Spatial Fiducials Storage

1.2.840.10008.5.1.4.1.1.66.2

Spatial Fiducials IOD

Deformable Spatial Registration Storage

1.2.840.10008.5.1.4.1.1.66.3

Deformable Spatial Registration IOD

Segmentation Storage

1.2.840.10008.5.1.4.1.1.66.4

Segmentation IOD

Surface Segmentation Storage

1.2.840.10008.5.1.4.1.1.66.5

Surface Segmentation IOD

Real World Value Mapping Storage

1.2.840.10008.5.1.4.1.1.67

Real World Value Mapping IOD

Surface Scan Mesh Storage

1.2.840.10008.5.1.4.1.1.68.1

Surface Scan Mesh IOD

Surface Scan Point Cloud Storage

1.2.840.10008.5.1.4.1.1.68.2

Surface Scan Point Cloud IOD

VL Endoscopic Image Storage

1.2.840.10008.5.1.4.1.1.77.1.1

VL Endoscopic Image IOD

Video Endoscopic Image Storage

1.2.840.10008.5.1.4.1.1.77.1.1.1

Video Endoscopic Image IOD

VL Microscopic Image Storage

1.2.840.10008.5.1.4.1.1.77.1.2

VL Microscopic Image IOD

Video Microscopic Image Storage

1.2.840.10008.5.1.4.1.1.77.1.2.1

Video Microscopic Image IOD

VL Slide-Coordinates Microscopic Image Storage

1.2.840.10008.5.1.4.1.1.77.1.3

VL Slide-coordinates Microscopic Image IOD

VL Photographic Image Storage

1.2.840.10008.5.1.4.1.1.77.1.4

VL Photographic Image IOD

Video Photographic Image Storage

1.2.840.10008.5.1.4.1.1.77.1.4.1

Video Photographic Image IOD

Ophthalmic Photography 8 Bit Image Storage

1.2.840.10008.5.1.4.1.1.77.1.5.1

Ophthalmic Photography 8 Bit Image IOD

Ophthalmic Photography 16 Bit Image Storage

1.2.840.10008.5.1.4.1.1.77.1.5.2

Ophthalmic Photography 16 Bit Image IOD

Stereometric Relationship Storage

1.2.840.10008.5.1.4.1.1.77.1.5.3

Stereometric Relationship IOD

Ophthalmic Tomography Image Storage

1.2.840.10008.5.1.4.1.1.77.1.5.4

Ophthalmic Tomography Image IOD

VL Whole Slide Microscopy Image Storage

1.2.840.10008.5.1.4.1.1.77.1.6

VL Whole Slide Microscopy Image IOD

Lensometry Measurements Storage

1.2.840.10008.5.1.4.1.1.78.1

Lensometry Measurements IOD

Autorefraction Measurements Storage

1.2.840.10008.5.1.4.1.1.78.2

Autorefraction Measurements IOD

Keratometry Measurements Storage

1.2.840.10008.5.1.4.1.1.78.3

Keratometry Measurements IOD

Subjective Refraction Measurements Storage

1.2.840.10008.5.1.4.1.1.78.4

Subjective Refraction Measurements IOD

Visual Acuity Measurements Storage

1.2.840.10008.5.1.4.1.1.78.5

Visual Acuity Measurements IOD

Spectacle Prescription Report Storage

1.2.840.10008.5.1.4.1.1.78.6

Spectacle Prescription Report IOD

Ophthalmic Axial Measurements Storage

1.2.840.10008.5.1.4.1.1.78.7

Ophthalmic Axial Measurements IOD

Intraocular Lens Calculations Storage

1.2.840.10008.5.1.4.1.1.78.8

Intraocular Lens Calculations IOD

Macular Grid Thickness and Volume Report

1.2.840.10008.5.1.4.1.1.79.1

Macular Grid Thickness and Volume Report IOD

Ophthalmic Visual Field Static Perimetry Measurements Storage

1.2.840.10008.5.1.4.1.1.80.1

Ophthalmic Visual Field Static Perimetry Measurements IOD

Ophthalmic Thickness Map Storage

1.2.840.10008.5.1.4.1.1.81.1

Ophthalmic Thickness Map IOD

Corneal Topography Map Storage

1.2.840.10008.5.1.4.1.1.82.1

Corneal Topography Map IOD

Basic Text SR

1.2.840.10008.5.1.4.1.1.88.11

Basic Text SR IOD

Enhanced SR

1.2.840.10008.5.1.4.1.1.88.22

Enhanced SR IOD

Comprehensive SR

1.2.840.10008.5.1.4.1.1.88.33

Comprehensive SR IOD

Comprehensive 3D SR

1.2.840.10008.5.1.4.1.1.88.34

Comprehensive 3D SR IOD

Procedure Log

1.2.840.10008.5.1.4.1.1.88.40

Procedure Log IOD

Mammography CAD SR

1.2.840.10008.5.1.4.1.1.88.50

Mammography CAD SR IOD

Key Object Selection

1.2.840.10008.5.1.4.1.1.88.59

Key Object Selection Document IOD

Chest CAD SR

1.2.840.10008.5.1.4.1.1.88.65

Chest CAD SR IOD

X-Ray Radiation Dose SR

1.2.840.10008.5.1.4.1.1.88.67

X-Ray Radiation Dose SR IOD

Colon CAD SR

1.2.840.10008.5.1.4.1.1.88.69

Colon CAD SR IOD

Implantation Plan SR Document Storage

1.2.840.10008.5.1.4.1.1.88.70

Implantation Plan SR Document IOD

Encapsulated PDF Storage

1.2.840.10008.5.1.4.1.1.104.1

Encapsulated PDF IOD

Encapsulated CDA Storage

1.2.840.10008.5.1.4.1.1.104.2

Encapsulated CDA IOD

Positron Emission Tomography Image Storage

1.2.840.10008.5.1.4.1.1.128

Positron Emission Tomography Image IOD

Enhanced PET Image Storage

1.2.840.10008.5.1.4.1.1.130

Enhanced PET Image IOD

( 6)

Legacy Converted Enhanced PET Image Storage

1.2.840.10008.5.1.4.1.1.128.1

Legacy Converted Enhanced PET Image IOD

Basic Structured Display Storage

1.2.840.10008.5.1.4.1.1.131

Basic Structured Display IOD

RT Image Storage

1.2.840.10008.5.1.4.1.1.481.1

RT Image IOD

RT Dose Storage

1.2.840.10008.5.1.4.1.1.481.2

RT Dose IOD

RT Structure Set Storage

1.2.840.10008.5.1.4.1.1.481.3

RT Structure Set IOD

RT Beams Treatment Record Storage

1.2.840.10008.5.1.4.1.1.481.4

RT Beams Treatment Record IOD

RT Plan Storage

1.2.840.10008.5.1.4.1.1.481.5

RT Plan IOD

RT Brachy Treatment Record Storage

1.2.840.10008.5.1.4.1.1.481.6

RT Brachy Treatment Record IOD

RT Treatment Summary Record Storage

1.2.840.10008.5.1.4.1.1.481.7

RT Treatment Summary Record IOD

RT Ion Plan Storage

1.2.840.10008.5.1.4.1.1.481.8

RT Ion Plan IOD

RT Ion Beams Treatment Record Storage

1.2.840.10008.5.1.4.1.1.481.9

RT Ion Beams Treatment Record IOD

RT Beams Delivery Instruction Storage

1.2.840.10008.5.1.4.34.7

RT Beams Delivery Instruction IOD

Generic Implant Template Storage

1.2.840.10008.5.1.4.43.1

Generic Implant Template IOD

Implant Assembly Template Storage

1.2.840.10008.5.1.4.44.1

Implant Assembly Template IOD

Implant Template Group Storage

1.2.840.10008.5.1.4.45.1

Implant Template Group IOD
 *
 */




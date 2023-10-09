#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sender.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QStandardItemModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public:
    DicomSender m_sender;
    QStandardItemModel *m_pMOdel;
    dcmtk::log4cplus::Logger m_log;
    int m_sendTotal;

public:
    void stopSend();
    void stopScan();
    void startSend(int destination, int threads);

    void loadDestinationList();
    void loadGlobalDestinationList();
    void saveDestinationList();
    void startScan(QString &path);

    void registerCodecs();
    void registercleanup();

public slots:
    void updatePatientList();
    void updateSendDcm(int sendFiles);

private slots:
    void on_pbUpdate_clicked();

    void on_pBSend_clicked();

    void on_pBDir_clicked();

    void on_pBEcho_clicked();

    void on_pBOpen2K_clicked();

    void on_pBOpen2KC_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H


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

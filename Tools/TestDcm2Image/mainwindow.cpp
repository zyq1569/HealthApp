#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>

#include <QImage>
#include <QImageWriter>
// We use dcmtk for scaling dicom images

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmjpeg/djdecode.h"    /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/djencode.h"    /* for dcmjpeg decoders */
#include "dcmtk/dcmjpls/djdecode.h"
#include "dcmtk/dcmjpls/djencode.h"
#include "dcmtk/dcmdata/dcrleerg.h"
#include "dcmtk/dcmdata/dcrledrg.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmimgle/digsdfn.h"      /* for DiGSDFunction */
#include "dcmtk/dcmimgle/diciefn.h"      /* for DiCIELABFunction */
#include "fmjpeg2k/djdecode.h"
#include "fmjpeg2k/djencode.h"
#include "logging.h"

#ifndef UTFSET
#define UTFSET
//#pragma execution_character_set("utf-8")
#endif
#include "dcmtk/dcmimage/diregist.h"
///
//#ifdef  DEBUG
#include <QDebug>
//#include <qlogging.h>
//#endif //  DEBUG

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_TransferSyntax = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}
//查询密码是:2402027462490031
//沒有了個體的思想 人便失去了存在的價值 ⋯
//盲從與跟風 僅僅是表象
//每個人 包括妳我他⋯ 皆毫無存在的尊嚴 ⋯ 恰恰是當今社會利益價值觀中 最被消亡的
//未压缩：
//Implicit VR Little Endian:
//Default Transfer Syntax for DICOM               1.2.840.10008.1.2
//Explicit VR Little Endian                       1.2.840.10008.1.2.1
//Deflated Explicit VR Little Endian              1.2.840.10008.1.2.1.99
//Explicit VR Big Endian                          1.2.840.10008.1.2.2

//无损压缩：
//JPEG Lossless :
//Default  Transfer Syntax for Lossless JPEG ImageCompression               1.2.840.10008.1.2.4.70
//JPEG 2000 Image Compression(Lossless Only)                                1.2.840.10008.1.2.4.90
//RLE Lossless                                   1.2.840.10008.1.2.5
//有损压缩：
//Default Transfer Syntax for Lossy JPEG 12 Bit Image Compression           1.2.840.10008.1.2.4.51
//LOSSY(8 bit)                                 1.2.840.10008.1.2.4.50
//动态：
//MPEG2 image Compression                     1.2.840.10008.1.2.4.100

//TRANSFER SYNTAXES
//dcmdjpeg supports the following transfer syntaxes for input (dcmfile-in):
//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//DeflatedExplicitVRLittleEndianTransferSyntax   1.2.840.10008.1.2.1.99 (*)
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2
//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70
//(*) if compiled with zlib support enabled
//dcmdjpeg supports the following transfer syntaxes for output (dcmfile-out):
//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2


//dcmdjpeg supports the following transfer syntaxes for input (dcmfile-in):

//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//DeflatedExplicitVRLittleEndianTransferSyntax   1.2.840.10008.1.2.1.99 (*)
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2
//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70
//(*) if compiled with zlib support enabled

//dcmdjpeg supports the following transfer syntaxes for output (dcmfile-out):

//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2


//dcmcjpeg supports the following transfer syntaxes for input (dcmfile-in):

//LittleEndianImplicitTransferSyntax             1.2.840.10008.1.2
//LittleEndianExplicitTransferSyntax             1.2.840.10008.1.2.1
//DeflatedExplicitVRLittleEndianTransferSyntax   1.2.840.10008.1.2.1.99 (*)
//BigEndianExplicitTransferSyntax                1.2.840.10008.1.2.2
//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70
//(*) if compiled with zlib support enabled

//dcmcjpeg supports the following transfer syntaxes for output (dcmfile-out):

//JPEGProcess1TransferSyntax                     1.2.840.10008.1.2.4.50
//JPEGProcess2_4TransferSyntax                   1.2.840.10008.1.2.4.51
//JPEGProcess6_8TransferSyntax                   1.2.840.10008.1.2.4.53
//JPEGProcess10_12TransferSyntax                 1.2.840.10008.1.2.4.55
//JPEGProcess14TransferSyntax                    1.2.840.10008.1.2.4.57
//JPEGProcess14SV1TransferSyntax                 1.2.840.10008.1.2.4.70


/*
青石板，石板青，青石板上钉银钉。千颗万颗数不清
青石板，石板青，红尘漂泊忆零星！
青石板，石板青，天国故乡泪盈盈！
青石板，石板青，满天银星盼你醒！
青石板，石板青，大法开传神路行！
*/
///
//未压缩：
//Transfer Syntax UID Transfer Syntax name
//Implicit VR Endian: Default Transfer Syntax for DICOM(默认，使用广)          1.2.840.10008.1.2
// Explicit VR Little Endian                                                   1.2.840.10008.1.2.1
//Deflated Explicit VR Little Endian                                           1.2.840.10008.1.2.1.99
// Explicit VR Big Endian                                                      1.2.840.10008.1.2.2
//压缩---JPEG和JPEG2000（有损和无损）
//有损：值中带Lossy
//Transfer Syntax UID Transfer Syntax name
//1.2.840.10008.1.2.4.50 JPEG Baseline (Process 1):
//Default Transfer Syntax for Lossy JPEG 8-bit Image Compression
//1.2.840.10008.1.2.4.51 JPEG Baseline (Processes 2 & 4):
//Default Transfer Syntax for Lossy JPEG 12-bit Image Compression
//(Process 4 only)
//1.2.840.10008.1.2.4.52 JPEG Extended (Processes 3 & 5)
//1.2.840.10008.1.2.4.53 JPEG Spectral Selection, Nonhierarchical (Processes 6 & 8)
//1.2.840.10008.1.2.4.54 JPEG Spectral Selection, Nonhierarchical (Processes 7 & 9)
//1.2.840.10008.1.2.4.55 JPEG Full Progression, Nonhierarchical (Processes 10 & 12)
//1.2.840.10008.1.2.4.56 JPEG Full Progression, Nonhierarchical (Processes 11 & 13)
//1.2.840.10008.1.2.4.57 JPEG Lossless, Nonhierarchical (Processes 14)
//1.2.840.10008.1.2.4.58 JPEG Lossless, Nonhierarchical (Processes 15)
//1.2.840.10008.1.2.4.59 JPEG Extended, Hierarchical (Processes 16 & 18)
//1.2.840.10008.1.2.4.60 JPEG Extended, Hierarchical (Processes 17 & 19)
//1.2.840.10008.1.2.4.61 JPEG Spectral Selection, Hierarchical (Processes 20 & 22)
//1.2.840.10008.1.2.4.62 JPEG Spectral Selection, Hierarchical (Processes 21 & 23)
//1.2.840.10008.1.2.4.63 JPEG Full Progression, Hierarchical (Processes 24 & 26)
//1.2.840.10008.1.2.4.64 JPEG Full Progression, Hierarchical (Processes 25 & 27)
//1.2.840.10008.1.2.4.65 JPEG Lossless, Nonhierarchical (Process 28)
//1.2.840.10008.1.2.4.66 JPEG Lossless, Nonhierarchical (Process 29)
//1.2.840.10008.1.2.4.70 JPEG Lossless, Nonhierarchical, First- Order Prediction
//(Processes 14 [Selection Value 1]):
//Default Transfer Syntax for Lossless JPEG Image Compression
//1.2.840.10008.1.2.4.80 JPEG-LS Lossless Image Compression
//DICOM Transfer Syntaxes
//1.2.840.10008.1.2.4.81 JPEG-LS Lossy (Near- Lossless) Image Compression
//1.2.840.10008.1.2.4.90 JPEG 2000 Image Compression (Lossless Only)
//1.2.840.10008.1.2.4.91 JPEG 2000 Image Compression
//1.2.840.10008.1.2.4.92 JPEG 2000 Part 2 Multicomponent Image Compression (Lossless Only)
//1.2.840.10008.1.2.4.93 JPEG 2000 Part 2 Multicomponent Image Compression
//Transfer Syntax UID Transfer Syntax name
//1.2.840.10008.1.2.4.94 JPIP Referenced
//1.2.840.10008.1.2.4.95 JPIP Referenced Deflate
//1.2.840.10008.1.2.5 RLE Lossless
//1.2.840.10008.1.2.6.1 RFC 2557 MIME Encapsulation
//Transfer Syntax UID Transfer Syntax name
//1.2.840.10008.1.2.4.100 MPEG2 Main Profile Main Level
//1.2.840.10008.1.2.4.102 MPEG-4 AVC/H.264 High Profile / Level 4.1
//1.2.840.10008.1.2.4.103 MPEG-4 AVC/H.264 BD-compatible High Profile / Level 4.1
///dcmdjpeg.cc dcmcjpeg.cc
///
/// https://dicomlibrary.com/dicom/transfer-syntax/
const QString PreviewNotAvailableText(QObject::tr("Preview image not available"));
DcmDataset decompressImage( const DcmDataset *olddataset, E_TransferSyntax opt_oxfer = EXS_LittleEndianImplicit, QString derror = "");
QImage makeEmptyThumbnailWithCustomText(const QString &text, int resolution);
QPixmap convertToQPixmap(DicomImage *dicomImage);
QImage dcm2QImage(DicomImage *dicomImage, int resolution = 256);

QImage makeEmptyThumbnailWithCustomText(const QString &text, int resolution = 96)
{
    QImage thumbnail;

    thumbnail = QImage(resolution, resolution, QImage::Format_RGB32);
    thumbnail.fill(Qt::black);

    QPainter painter(&thumbnail);
    painter.setPen(Qt::white);
    painter.drawText(0, 0, resolution, resolution, Qt::AlignCenter | Qt::TextWordWrap, text);

    return thumbnail;
}

DcmDataset decompressImage( const DcmDataset *olddataset, E_TransferSyntax opt_oxfer , QString derror)
{
    DcmFileFormat fileformat;
    // JPEG parameters
    OFCondition error = EC_Normal;

    DcmDataset dataset(*olddataset);

    DcmXfer opt_oxferSyn(opt_oxfer);
    DcmXfer original_xfer(dataset.getOriginalXfer());

    error = dataset.chooseRepresentation(opt_oxfer, NULL);
    if (error.bad())
    {
        //ERROR_LOG(QString( error.text()) + " decompressing file: " + opt_ifname);
        derror = error.text();
        if (error == EJ_UnsupportedColorConversion)
        {
            //ERROR_LOG( "Try --conv-never to disable color space conversion");
            derror +=  "Try --conv-never to disable color space conversion";
        }
        else if (error == EC_CannotChangeRepresentation)
        {
            //ERROR_LOG( QString("Input transfer syntax ") +  original_xfer.getXferName() + "not supported");
            derror +=  QString("Input transfer syntax ") +  original_xfer.getXferName() + "not supported";
        }
        return dataset;
    }

    if (!dataset.canWriteXfer(opt_oxfer))
    {
        // ERROR_LOG(QString ("no conversion to transfer syntax") + opt_oxferSyn.getXferName() + "possible");
        // deregister global decompression codecs
        QMessageBox::warning(NULL,"warning!","conversion to transfer syntax fail!" + QString(opt_oxferSyn.getXferName()));
        return dataset;
    }

    // deregister global decompression codecs
    return  dataset;
}

QPixmap convertToQPixmap(DicomImage *dicomImage, QPixmap &thumbnail)
{
    Q_ASSERT(dicomImage);

    // The following code creates a PGM or PPM image in memory and we load this buffer directly into the QImage
    // Based on the code of http://forum.dcmtk.org/viewtopic.php?t=120&highlight=qpixmap
    int bytesPerComponent;
    QString imageFormat;
    QString imageHeader;
    if (dicomImage->isMonochrome())
    {
        imageHeader = "P5";
        bytesPerComponent = 1;
        imageFormat = "PGM";
    }
    else
    {
        imageHeader = "P6";
        bytesPerComponent = 3;
        imageFormat = "PPM";
    }

    // Create PGM/PPM header
    const int width = (int)(dicomImage->getWidth());
    const int height = (int)(dicomImage->getHeight());
    imageHeader += QString("\n%1 %2\n255\n").arg(width).arg(height);

	//


    // QPixmap in which we will load the data buffer
    // Create output buffer for DicomImage class
    const int offset = imageHeader.size();
    const unsigned int length = (width * height) * bytesPerComponent + offset;
    Uint8 *buffer = new Uint8[length];
    if (buffer != NULL)
    {
        // Copy PGM/PPM header to buffer
		const Uint8* src = (const Uint8*)imageHeader.toLatin1().data();
        OFBitmanipTemplate<Uint8>::copyMem((const Uint8*)imageHeader.toLatin1().data(), buffer, offset);
        if (dicomImage->getOutputData((void*)(buffer + offset), length, 8))
        {
			const char* c = (char*)buffer;
			for (unsigned int i = 0; i < length; i++)
			{
				//std::cout << buffer[i] << "_";
                //qDebug() << buffer[i] << "_";
			}
		        

            if (!thumbnail.loadFromData((const unsigned char*)buffer, length, imageFormat.toLatin1()))
            {
                 DEBUG_LOG("Buffer upload to thumbnail failed :(");
            }
            //thumbnail.save("E:/test.png");
        }
        // Delete temporary pixel buffer
        delete[] buffer;
    }
    else
    {
        //DEBUG_LOG("Insufficient memory to create thumbnail buffer!");
    }

    return thumbnail;
}

QImage dcm2QImage(DicomImage *dicomImage, int resolution )
{
    bool ok = false;
    QImage thumbnail;
    if (dicomImage == NULL)
    {
        ok = false;
        //DEBUG_LOG("Insufficient memory to load DICOM image when making thumbnail or null pointer");
    }
    else if (dicomImage->getStatus() == EIS_Normal)
    {
        QImage image;
        //dicomImage->hideAllOverlays();
        //dicomImage->setMinMaxWindow(1);
		//dicomImage->setWindow(-2,1);
        // We scale the image
        DicomImage *scaledImage;
        // We climb the biggest corner
        unsigned long width, height;
        width = dicomImage->getWidth();
        height = dicomImage->getHeight();
        if ( width < height)
        {
            width = 0;
            height = resolution;
        }
        else
        {
            width = resolution;
            height = 0;
        }
        scaledImage = dicomImage->createScaledImage(width, height, 1, 1);
        if (scaledImage == NULL)
        {
            ok = false;
            //DEBUG_LOG("Scaled image returned as null");
        }
        else if (scaledImage->getStatus() == EIS_Normal)
        {
            QPixmap pixmap;
            convertToQPixmap(scaledImage, pixmap);
            if (pixmap.isNull())
            {
                //DEBUG_LOG("Could not convert DicomImage to QImage. A Preview not available thumbnail is created.");
                ok = false;
            }
            else
            {
                // The smallest side will be of "resolution" size.
                pixmap = pixmap.scaled(resolution,resolution, Qt::AspectRatioMode::KeepAspectRatioByExpanding, Qt::TransformationMode::SmoothTransformation);
                // By cropping the longer side, a squared image is made.
                int width = pixmap.width();
                int height = pixmap.height();
                if (width > height) // heigth == resolution
                {
					//pixmap = pixmap.copy((width - resolution) / 2, 0, height, height);
					thumbnail = pixmap.copy((width-resolution) / 2, 0, height, height).toImage();
                }
                else if (height > width) // width == resolution
                {
					//pixmap.copy(0, (height - resolution) / 2, width, width);
					thumbnail = pixmap.copy(0, (height-resolution) / 2, width, width).toImage();
                }
                else
                {
                    // A perfect square, nothing to do
					thumbnail = pixmap.toImage();
                }
                //thumbnail = pixmap.toImage();
                ok = true;
            }
            delete scaledImage;
        }
        else
        {
            ok = false;
        }
    }
    else
    {
        ok = false;
        //DEBUG_LOG(QString("Error loading the DicomImage. Error: %1 ").arg(DicomImage::getString(dicomImage->getStatus())));
    }

    //If we were unable to generate the thumbnail, we create a blank one
    if (!ok)
    {
        thumbnail = makeEmptyThumbnailWithCustomText(PreviewNotAvailableText);
    }

    return thumbnail;
}

void MainWindow::registerCodecs()
{
    // register global JPEG decompression codecs
    DJDecoderRegistration::registerCodecs();

    // register global JPEG compression codecs
    DJEncoderRegistration::registerCodecs();

    // register JPEG-LS decompression codecs
    DJLSDecoderRegistration::registerCodecs();

    //        // register JPEG-LS compression codecs
    DJLSEncoderRegistration::registerCodecs();

    // register RLE compression codec
    DcmRLEEncoderRegistration::registerCodecs();

    // register RLE decompression codec
    DcmRLEDecoderRegistration::registerCodecs();

    // jpeg2k
    FMJPEG2KEncoderRegistration::registerCodecs();
    FMJPEG2KDecoderRegistration::registerCodecs();
}

void MainWindow::registerCleanup()
{
    // deregister JPEG codecs Decoder Encoder
    DJDecoderRegistration::cleanup();
    DJEncoderRegistration::cleanup();

    // deregister JPEG-LS codecs
    DJLSDecoderRegistration::cleanup();
    DJLSEncoderRegistration::cleanup();

    // deregister RLE codecs
    DcmRLEDecoderRegistration::cleanup();
    DcmRLEEncoderRegistration::cleanup();

    // jpeg2k
    FMJPEG2KEncoderRegistration::cleanup();
    FMJPEG2KDecoderRegistration::cleanup();
}
// Qt
//#include <QStringList>
//#include <QTextCodec>
//#include <qglobal.h>
void MainWindow::on_dcm2Image_clicked()
{
	m_dcmpath = ui->dcmpath->text();
	ui->pngpath->setText(m_dcmpath + "dcmtk.png");
    QFileInfo info(m_dcmpath);
    if (info.isFile())
    {
        QString dcmfilename = m_dcmpath;
        QString pngfilename = m_dcmpath+".png";
		// ALL this is necessary to, among other things, be able to create thumbnails,
		// dicomdirs, etc. of compressed dicoms and treat them correctly with dcmtk
		// this is temporarily here, in the long run I will go to a setup class
		// register the JPEG and RLE decompressor codecs
        ///
        DicomImage *dicomImage = NULL;
        DcmFileFormat fileformat;
		OFString filename = qPrintable(dcmfilename);// dcmfilename.toLocal8Bit().constData();
		OFCondition error = fileformat.loadFile(filename/*, opt_ixfer, EGL_noChange, DCM_MaxReadLength, opt_readMode*/);
        if (error.bad())
        {
            QMessageBox::warning(this,"warning!"," loadFile dcm fail!");
            return;
        }
		E_TransferSyntax org_ixfer = fileformat.getDataset()->getOriginalXfer();
		DcmXfer original_xfer(org_ixfer);
		if (org_ixfer == EXS_JPEG2000LosslessOnly || org_ixfer == EXS_JPEG2000 ||
			org_ixfer == EXS_JPEG2000MulticomponentLosslessOnly || org_ixfer == EXS_JPEG2000Multicomponent)
		{
			// jpeg2k
			FMJPEG2KDecoderRegistration::registerCodecs();
			dicomImage = new DicomImage(filename.c_str()/*fileformat.getDataset(), org_ixfer, CIF_UsePartialAccessToPixelData, 0, 1*/);
			// jpeg2k
			FMJPEG2KDecoderRegistration::cleanup();
		}
		else
		{
			DJDecoderRegistration::registerCodecs();
			DcmRLEDecoderRegistration::registerCodecs();
			dicomImage = new DicomImage(filename.c_str());
			DJDecoderRegistration::cleanup();
			DcmRLEDecoderRegistration::cleanup();
		}
        ///------------------------------------------------------------------------
		if (dicomImage)
		{
			OFString value;
			fileformat.getDataset()->findAndGetOFString(DCM_SeriesDescription, value);
			QString vl = value.c_str();//QString t = vl.toLower();			
			dicomImage->hideAllOverlays();
			if (vl.length() > 1 && vl.toLower().contains("report"))
			{
				dicomImage->setWindow(-2, 1);
			}
			else
			{
				dicomImage->setMinMaxWindow(1);
			}
		}
		else
		{
			QMessageBox::warning(this, "warning!", " loadFile dcm fail!");
			return;
		}
        QImage image = dcm2QImage(dicomImage,512);
        image.save(pngfilename+"dcmtk.png");
        ui->lbshowimage->clear();
        ui->lbshowimage->setPixmap(QPixmap(pngfilename+"dcmtk.png"));
    }else
    {
        QMessageBox::warning(this,"warning!","the dcm not exit!");
    }

}

void MainWindow::on_pBdcmpath_clicked()
{
    m_dcmpath = QFileDialog::getOpenFileName(this,tr("open a file."),"",tr("dcm file(*.dcm);All files(*.*)"/*,nullptr,QFileDialog::DontUseNativeDialog*/));
    if (m_dcmpath.isEmpty())
    {
        QMessageBox::warning(this,"warning!","Failed to open the dcm");
    }
    else
    {
        ui->dcmpath->setText(m_dcmpath);
        ui->pngpath->setText(m_dcmpath+"dcmtk.png");
    }
}

void MainWindow::on_decoder_clicked()
{
    m_TransferSyntax = ui->cbmTransferSyntax->currentIndex();
    QFileInfo info(m_dcmpath);
    if (info.isFile())
    {
        QString dcmfilename = m_dcmpath;
        DcmFileFormat fileformat;
        fileformat.loadFile(dcmfilename.toLatin1().data());
        DcmXfer original_xfer(fileformat.getDataset()->getOriginalXfer());
        if (!original_xfer.isEncapsulated())
        {
            QMessageBox::warning(this,"warning!","is not JPEG-compressed DICOM file!");
            return;
        }
        registerCodecs();
        QString newfilepath = dcmfilename+"_d_LI.dcm";
        E_TransferSyntax opt_ixfer = EXS_LittleEndianImplicit;
        switch(m_TransferSyntax)
        {
        case 1:
            opt_ixfer = EXS_LittleEndianImplicit;
            break;
        case 2:
            opt_ixfer = EXS_BigEndianImplicit;
            newfilepath = dcmfilename+"_d_BI.dcm";
            break;
        case 3:
            opt_ixfer = EXS_LittleEndianExplicit;
            newfilepath = dcmfilename+"_d_LE.dcm";
            break;
        default:
            break;
        }
        DcmDataset newdataset(decompressImage(fileformat.getDataset(),opt_ixfer));

        registerCleanup();
        if (newdataset.isEmpty())
        {
            QMessageBox::warning(this,"warning!","decoderDcm fail!");
        }
        else
        {
            E_EncodingType opt_oenctype = EET_ExplicitLength;
            E_GrpLenEncoding opt_oglenc = EGL_recalcGL;
            E_PaddingEncoding opt_opadenc = EPD_noChange;
            OFCmdUnsignedInt opt_filepad = 0;
            OFCmdUnsignedInt opt_itempad = 0;
            OFCondition error = EC_Normal;

            newdataset.saveFile( newfilepath.toLatin1().data(), opt_ixfer, opt_oenctype, opt_oglenc, opt_opadenc,
                                 OFstatic_cast(Uint32, opt_filepad),  OFstatic_cast(Uint32, opt_itempad));
            if (error != EC_Normal)
            {
                QMessageBox::warning(this,"warning!","decoderDcm save fail!" + newfilepath);
                return ;
            }
            QMessageBox::information(this,"ok!","decoderDcm ok!");
        }

    }else
    {
        QMessageBox::warning(this,"warning!","the dcm not exit!");
    }
}

void MainWindow::on_pBshowimage_clicked()
{
    QString pngfile = ui->pngpath->text();
    QFileInfo info(pngfile);
    if (info.isFile())
    {
        ui->lbshowimage->clear();
        ui->lbshowimage->setPixmap(QPixmap(pngfile));
    }
    else
    {
        QMessageBox::warning(this,"warning!","file not exit!");
    }
}

void MainWindow::on_cbmTransferSyntax_currentIndexChanged(int index)
{
    m_TransferSyntax = ui->cbmTransferSyntax->currentIndex();
}

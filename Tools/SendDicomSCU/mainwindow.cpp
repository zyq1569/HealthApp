#include "mainwindow.h"
#include "ui_mainwindow.h"

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
#include "fmjpeg2k/djdecode.h"
#include "fmjpeg2k/djencode.h"

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
    connect(&m_sender, &DicomSender::finishsenddicomfile, this, &MainWindow::updateSendDcm);


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

    ui->pBSendDcm->setOrientation(Qt::Horizontal);
    ui->pBSendDcm->setMinimum(0);

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

void MainWindow::registercleanup()
{
    // deregister JPEG codecs
    DJDecoderRegistration::cleanup();
    DJEncoderRegistration::cleanup();

    // deregister JPEG-LS codecs
    DJLSDecoderRegistration::cleanup();
    DJLSEncoderRegistration::cleanup();

    // deregister RLE codecs
    DcmRLEDecoderRegistration::cleanup();
    DcmRLEEncoderRegistration::cleanup();
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
    m_sendTotal = listpatient.size();
    if (m_sendTotal < 1)
        return;

    ui->pBSendDcm->setMaximum(m_sendTotal);
    ui->pBSendDcm->setValue(1);
    m_sender.SendPatiens(listpatient);

}

void MainWindow::updateSendDcm(int sendFiles)
{
    ui->pBSendDcm->setValue(sendFiles);
    ui->pBSendDcm->setFormat(("now:")+QString(sendFiles)+"/"+QString(m_sendTotal));
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
    m_sender.m_destination.ourAETitle = ui->cb_LocalAetitle->toPlainText().toStdString();
    m_sender.m_destination.destinationAETitle = ui->cb_Aetitle->toPlainText().toStdString();
    m_sender.m_destination.destinationHost = ui->cb_IP->toPlainText().toStdString();
    m_sender.m_destination.destinationPort = ui->cb_Port->toPlainText().toUInt();
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


void MainWindow::on_pBOpen2K_clicked()
{
    QString derror;
    if (!dcmDataDict.isDictionaryLoaded())
    {
        derror = "no data dictionary loaded, check environment variable:";
        QMessageBox::warning(NULL, "warning!", derror);
        return;
    }

    QString filename;
    filename = QFileDialog::getOpenFileName(this,tr("select dcm file!"),QString(),tr("Dcm Files (*.dcm *.DCM *.*" ));
    QFileInfo fileInfo(filename);
    if(!fileInfo.exists())
    {
        QMessageBox::information(this,tr("Dcm"),"no exists!",QMessageBox::Ok);
        return;
    }

    // load file
    DcmFileFormat dcmff;
    dcmff.loadFile(filename.toStdString().c_str());

    // do some precheck of the transfer syntax
    DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());

    if (fileTransfer.isEncapsulated())
    {
        OFCondition error = EC_Normal;
        //OFBool opt_forceSingleFragmentPerFrame = OFFalse;
        // register global decompression codecs
        registerCodecs();

        QString info = "Explicit VR Little Endian";
        E_TransferSyntax opt_oxfer = EXS_LittleEndianExplicit;//EXS_LittleEndianImplicit;  //opt_oxfer = EXS_BigEndianImplicit;  //opt_oxfer = EXS_LittleEndianExplicit;
        DcmXfer opt_oxferSyn(opt_oxfer);
        DcmXfer original_xfer(fileTransfer);

        error = dcmff.chooseRepresentation(opt_oxfer, NULL);
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
            // deregister global decompression codecs
            registercleanup();
            QMessageBox::warning(NULL, "warning!", derror);

        }
        else
        {
            if (!dcmff.canWriteXfer(opt_oxfer))
            {
                // ERROR_LOG(QString ("no conversion to transfer syntax") + opt_oxferSyn.getXferName() + "possible");
                // deregister global decompression codecs
                registercleanup();
                QMessageBox::warning(NULL,"warning!","conversion to transfer syntax fail!" + QString(opt_oxferSyn.getXferName()));
                return;
            }

            QString newfilename = fileInfo.path() + "/"+ fileInfo.fileName() +"_decode.dcm";
            dcmff.saveFile(newfilename.toStdString().c_str(),EXS_LittleEndianExplicit);
            // deregister global decompression codecs
            registercleanup();
            QMessageBox::information(NULL, "OK!", "Decoder ok!-- dcm /" + info);
        }

    }
    else
    {
        QMessageBox::information(this,tr("Dcm"),"encapsulated dcm!",QMessageBox::Ok);
    }

}


//https://github.com/moyumoyu/dcmtk-openjpeg
void MainWindow::on_pBOpen2KC_clicked()
{
    QString derror;
    if (!dcmDataDict.isDictionaryLoaded())
    {
        derror = "no data dictionary loaded, check environment variable:";
        QMessageBox::warning(NULL, "warning!", derror);
        return;
    }

    QString filename;
    filename = QFileDialog::getOpenFileName(this,tr("select dcm file!"),QString(),tr("Dcm Files (*.dcm *.DCM *.*" ));
    QFileInfo fileInfo(filename);
    if(!fileInfo.exists())
    {
        QMessageBox::information(this,tr("Dcm"),"no exists!",QMessageBox::Ok);
        return;
    }

    // load file
    DcmFileFormat dcmff;
    dcmff.loadFile(filename.toStdString().c_str());

    // do some precheck of the transfer syntax
    DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());

    if (!fileTransfer.isEncapsulated())
    {
        OFCondition error = EC_Normal;
#ifdef ON_THE_FLY_COMPRESSION

        registerCodecs();

#endif

        QString info;
        E_TransferSyntax opt_oxfer;
        opt_oxfer = EXS_JPEG2000LosslessOnly;//transfersyntaxesEXS_JPEGProcess14SV1;//EXS_JPEGProcess14SV1;//EXS_LittleEndianImplicit;  //opt_oxfer = EXS_BigEndianImplicit;  //opt_oxfer = EXS_LittleEndianExplicit;
        info = "JPEG 2000 Image Compression(Lossless Only)/70";
        int index = ui->cB_transfersyntaxes->currentIndex();
        if (3 == index)
        {
            opt_oxfer = EXS_JPEGProcess14SV1;
            info = "JPEG Lossless, Non-Hierarchical,First-Order Prediction/90";
        }
        else
        {
            if (index < 3)
            {
                ui->cB_transfersyntaxes->setCurrentIndex(4);
            }
        }

        DcmXfer opt_oxferSyn(opt_oxfer);
        DcmXfer original_xfer(fileTransfer);

        error = dcmff.chooseRepresentation(opt_oxfer, NULL);
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
                derror +=  QString("[Input transfer syntax: ") +  original_xfer.getXferName() + "not supported]";
            }
            // deregister global decompression codecs
#ifdef ON_THE_FLY_COMPRESSION

            registercleanup();
#endif
            QMessageBox::warning(NULL, "warning!", derror);

        }
        else
        {
            if (!dcmff.canWriteXfer(opt_oxfer))
            {
                // ERROR_LOG(QString ("no conversion to transfer syntax") + opt_oxferSyn.getXferName() + "possible");
                // deregister global decompression codecs
#ifdef ON_THE_FLY_COMPRESSION

                registercleanup();

#endif
                QMessageBox::warning(NULL,"warning!","conversion to transfer syntax fail!" + QString(opt_oxferSyn.getXferName()));
                return;
            }

            QString newfilename = fileInfo.path() + "/"+ fileInfo.fileName();
            if (3==index)
            {
                newfilename +=    "70_encode.dcm";
            }
            else
            {
                newfilename +=   "90_encode.dcm";
            }
            dcmff.saveFile(newfilename.toStdString().c_str(),opt_oxfer);
            // deregister global decompression codecs
            QMessageBox::information(NULL, "OK!", "Encoder JPEG ok!-- dcm /" + info);
        }

#ifdef ON_THE_FLY_COMPRESSION

        registercleanup();
#endif

    }
    else
    {
        QMessageBox::information(this,tr("Dcm"),"Encapsulated dcm!",QMessageBox::Ok);
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

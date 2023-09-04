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


//#include <fcntl.h>
//#include <io.h>
//#include "zlib.h"
//#define CHUNK 16384
///https://blog.csdn.net/infoworld/article/details/55101712
/* Compress from file source to file dest until EOF on source.
def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
allocated for processing, Z_STREAM_ERROR if an invalid compression
level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
version of the library linked do not match, or Z_ERRNO if there is
an error reading or writing the files. */
//int compressString(const char* in_str, size_t in_len, std::string &out_str, int level)
//{
//    if (!in_str)
//    {
//        return Z_DATA_ERROR;
//    }

//    int ret, flush;
//    unsigned have;
//    z_stream strm;
//    unsigned char out[CHUNK];

//    /* allocate deflate state */
//    strm.zalloc = Z_NULL;
//    strm.zfree = Z_NULL;
//    strm.opaque = Z_NULL;
//    ret = deflateInit(&strm, level);
//    if (ret != Z_OK)
//        return ret;

//    std::shared_ptr<z_stream> sp_strm(&strm, [](z_stream* strm)    {        (void)deflateEnd(strm);    });
//    const char* end = in_str + in_len;

//    size_t pos_index = 0;
//    size_t distance = 0;

//    /* compress until end of file */
//    do {
//        distance = end - in_str;
//        strm.avail_in = (distance >= CHUNK)?CHUNK:distance;
//        strm.next_in = (Bytef*)in_str;

//        // next pos
//        in_str += strm.avail_in;
//        flush   = (in_str == end) ? Z_FINISH : Z_NO_FLUSH;

//        do {
//            strm.avail_out = CHUNK;
//            strm.next_out = out;
//            ret = deflate(&strm, flush);
//            if(ret == Z_STREAM_ERROR)
//            {
//                break;
//            }
//            have = CHUNK - strm.avail_out;
//            out_str.append((const char*)out,have);
//        } while (strm.avail_out == 0);
//        if (strm.avail_in != 0)
//            break;

//    } while (flush != Z_FINISH);
//    if (ret != Z_STREAM_END)
//    {
//        return Z_STREAM_ERROR;
//    }

//    return Z_OK;
//}

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
    ui->pBSendDcm->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->pBSendDcm->setValue(100);
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

    m_sendTotal = 0;
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
            foreach (Study s, pt.studydatas)
            {
                m_sendTotal += s.filespath.size();
            }

            listpatient.push_back(pt);
        }
    }

    if (m_sendTotal < 1)
    {
        QMessageBox::information(this,tr("No Dicom files"),"No Dicom files!",QMessageBox::Ok);
        return;
    }

    if (ui->cBAnonymous->isChecked() && QMessageBox::Yes == QMessageBox::question(NULL, "question", "Anonymous  Dicom file?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
    {
        m_sender.SetUpateDcmFileAnonymous(true);
    }
    else
    {
        ui->cBAnonymous->setChecked(false);
        m_sender.SetUpateDcmFileAnonymous(false);
    }

    ui->pBSendDcm->reset();
    ui->pBSendDcm->setMaximum(m_sendTotal);
    ui->pBSendDcm->setValue(1);
    m_sender.SendPatiens(listpatient);

}

void MainWindow::updateSendDcm(int sendFiles)
{
    QString info =  QString::number (sendFiles) +"|" + QString::number (m_sendTotal) + "_( %1% )";
    if (sendFiles == -1)
    {
        QMessageBox::information(this,tr("Send Dcm"), "Some dcm files send fail! pls check!",QMessageBox::Ok);
        ui->cBAnonymous->setChecked(false);
        return;
    }
    if (sendFiles == 0)
    {
        ui->cBAnonymous->setChecked(false);
        ui->pBSendDcm->setFormat(info.arg(QString::number(100,'f',1)));
        QMessageBox::information(this,tr("Send Dcm"), "All dcm files send succeed!",QMessageBox::Ok);
    }
    else
    {
        ui->pBSendDcm->setMinimum(0);
        ui->pBSendDcm->setValue(sendFiles);
        double dProgress = (sendFiles *100)/m_sendTotal;
        ui->pBSendDcm->setFormat(info.arg(QString::number(dProgress,'f',1)));
    }
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

std::vector<std::string> split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    int size = str.size();
    for(int i = 0; i <size; i++)
    {
        pos = str.find(pattern, i);
        if (pos < size)
        {
            std::string s = str.substr(i,pos-i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
const char BASE64[64] = {'A','B', 'C','D','E', 'F',   'G','H', 'I','J','K', 'L',   'M','N', 'O','P','Q', 'R',  'S','T','U', 'V','W','X',    'Y','Z',
                         'a','b', 'c','d','e', 'f',   'g','h', 'i','j','k', 'l',   'm','n', 'o','p','q', 'r',  's','t','u', 'v','w','x',    'y','z',
                         '0','1', '2','3','4', '5',   '6','7', '8','9','+', '/'      };

std::string Base64F(long long input, std::string &out)
{
    //long long a = 200911030261;
    int base;
    std::string s;
    do
    {
        base = input % 64;
        s    = BASE64[base]  + s;
        input   /= 64;
    }while (input);
    out = s;
    return out;
}

#include "Units.h"
//https://github.com/moyumoyu/dcmtk-openjpeg
void MainWindow::on_pBOpen2KC_clicked()
{
//    OFString uuid = "1.2.826.0.1.3680043.9.7604.091103151322.178053.2009110302611.2.840.113619.2.55.3.604688119.699.1256270056.701.31.2.840.113619.2.55.3.604688119.699.1256270056.756.1";

//    OFList<OFString>  list = SplitUUID(uuid, ".");
//    OFString struid;
//    for (OFListIterator(OFString) id = list.begin(); id != list.end(); id++)
//    {
//        OFString number = *id;
//        OFString out;
//        UIDBase64(atoll(number.c_str()), out);
//        out += ".";
//        struid += out;
////        qDebug("%s", number.c_str());
//    }
//    //OFString ini_filename = ini_dir + "/" + struid + ".ini";
//    qDebug("%s", struid.c_str());

//    //std::string  uid = "1.2.826.0.1.3680043.9.7604.091103151322.178053.2009110302611.2.840.113619.2.55.3.604688119.699.1256270056.701.31.2.840.113619.2.55.3.604688119.699.1256270056.756.1";
//    std::string pattern = ".";
//    std::vector<std::string>  listq = split(uuid.c_str(), pattern);
//    std::string suid;
//    foreach( std::string s, listq)
//    {
//        QString number = s.c_str();
//        std::string out;
//        Base64F(number.toLongLong(), out);
//        out += ".";
//        suid += out;
////        qDebug("%s", s.c_str());
//    }
//    qDebug("%s", suid.c_str());

//    return;

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

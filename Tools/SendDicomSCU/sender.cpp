#include "sender.h"

// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif


#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmnet/scu.h"

#include "dcmtk/dcmjpeg/djdecode.h"    /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/djencode.h"    /* for dcmjpeg decoders */
#include "dcmtk/dcmjpls/djdecode.h"
#include "dcmtk/dcmjpls/djencode.h"
#include "dcmtk/dcmdata/dcrleerg.h"
#include "dcmtk/dcmdata/dcrledrg.h"
#include "fmjpeg2k/djdecode.h"
#include "fmjpeg2k/djencode.h"

#include <QThreadPool>
#include <QDir>
#include <QMetaType>

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif

/////#########################################################################################

/////--------------------------Taskthread-----------------------------------------------------
void Taskthread::registerCodecs()
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

void Taskthread::registercleanup()
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
/*static*/ bool Taskthread::updateStringAttributeValue(DcmItem *dataset, const DcmTagKey &key, std::string &value)
{
    DcmStack stack;
    DcmTag tag(key);

    OFCondition cond = EC_Normal;
    cond = dataset->search(key, stack, ESM_fromHere, OFFalse);
    if (cond != EC_Normal)
    {
        //        OFLOG_ERROR(storescuLogger, "updateStringAttributeValue: cannot find: " << tag.getTagName()
        //            << " " << key << ": " << cond.text());
        return OFFalse;
    }

    DcmElement *elem = OFstatic_cast(DcmElement *, stack.top());

    DcmVR vr(elem->ident());
    if (elem->getLength() > vr.getMaxValueLength())
    {
        //        OFLOG_ERROR(storescuLogger, "updateStringAttributeValue: INTERNAL ERROR: " << tag.getTagName()
        //            << " " << key << ": value too large (max " << vr.getMaxValueLength()
        //            << ") for " << vr.getVRName() << " value: " << value);
        return OFFalse;
    }

    cond = elem->putOFStringArray(value.c_str());
    if (cond != EC_Normal)
    {
        //        OFLOG_ERROR(storescuLogger, "updateStringAttributeValue: cannot put string in attribute: " << tag.getTagName()
        //            << " " << key << ": " << cond.text());
        return OFFalse;
    }

    return OFTrue;
}
Taskthread::Taskthread(QObject *parent)
{
    setAutoDelete(false);
    m_type = 0;//dicomDataJob
}

Taskthread::~Taskthread()
{

}

void Taskthread::run()
{
    if (m_type == 0)
    {
        dicomDataJob();
    }
    else if (m_type == 1)
    {
        dicomSendJob();
    }

}

void Taskthread::setJob(int type)
{
    if (type >= 0 && type <=3)
        m_type = type;
}

//////------------------scan dicom files------------------------
void Taskthread::scanDir(QString dir, std::vector<Patient> listpat)
{
    m_scanDir = dir;
    //    listpatient.clear();
    m_listpatient = listpat;
}

void Taskthread::dicomDataJob()
{
    QStringList filters;
    filters<<"*.DCM"<<"*.dcm";
    QStringList list = DirFilename(m_scanDir,filters);
    DcmFileFormat dcm;
    OFCondition cond;
    Study std;
    Patient pd;
    foreach(QString it, list)
    {
        std::string path = it.toStdString();
        cond = dcm.loadFile(path.c_str());
        if (cond.good())
        {
            OFString studyuid;
            OFString sopuid, sopclassuid, transfersyntax,patid,patname;

            if (dcm.getDataset()->findAndGetOFString(DCM_StudyInstanceUID, studyuid).bad())
                return;

            if (dcm.getDataset()->findAndGetOFString(DCM_SOPInstanceUID, sopuid).bad())
                return;

            if (dcm.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopclassuid).bad())
                return;

            if (dcm.getDataset()->findAndGetOFString(DCM_PatientID, patid).bad())
                return;

            if (dcm.getDataset()->findAndGetOFString(DCM_PatientName, patname).bad())
                return;

            if (m_listpatient.empty())
            {
                //std::string studyuid,studydate,studydesc,dir,sopclassuid,transfersyntax;
                Study std;
                std.studyuid = studyuid.c_str();
                std.sopclassuid = sopclassuid.c_str();

                OFString studydate,studydesc,dir,transfersyntax,originalXfer;

                std.dir = QFileInfo(path.c_str()).absolutePath().toStdString().c_str() ;
                if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).good())
                    std.studydate = studydate.c_str();

                if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).good())
                    std.studydesc = studydesc.c_str();

                if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).good())
                    std.transfersyntax = transfersyntax.c_str();

                std.filespath.push_back(path);

                DcmXfer filexfer(dcm.getDataset()->getOriginalXfer());
                std.originalXfer = filexfer.getXferID();


                Patient patient;
                patient.patientid = patid.c_str();
                patient.patientname = patname.c_str();
                patient.studydatas.push_back(std);
                m_listpatient.push_back(patient);

            }
            else
            {
                //find
                bool bnewpatid = true;
                for(std::vector<Patient>::iterator pt = m_listpatient.begin(); pt != m_listpatient.end(); ++pt)
                {
                    bool flg = false;
                    if (pt->patientid == patid.c_str())// find patientid
                    {
                        bnewpatid = false;

                        for(std::vector<Study>::iterator std = pt->studydatas.begin(); std != pt->studydatas.end(); ++std)
                        {
                            if (std->studyuid == studyuid.c_str())//find  studyuid
                            {
                                std::vector<std::string>::iterator it = std::find(std->filespath.begin(),std->filespath.end(),path);
                                if (it == std->filespath.end())
                                {
                                    std->filespath.push_back(path);
                                }
                                //else  {                                //exit path;                            }
                                flg = true;
                                break;
                            }
                        }
                        if (flg)//find patientid: find studyuid
                        {
                            break;
                        }
                        else   // find patientid: no studyuid
                        {
                            Study std;
                            std.studyuid = studyuid.c_str();
                            std.sopclassuid = sopclassuid.c_str();

                            OFString studydate, studydesc, dir, transfersyntax, sopclassuid, originalXfer;
                            if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).good())
                                std.studydate = studydate.c_str();

                            if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).good())
                                std.studydesc = studydesc.c_str();

                            if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).good())
                                std.transfersyntax = transfersyntax.c_str();

                            DcmXfer filexfer(dcm.getDataset()->getOriginalXfer());
                            std.originalXfer = filexfer.getXferID();

                            std.filespath.push_back(it.toStdString());
                            pt->studydatas.push_back(std);

                            break;
                        }
                    }
                }
                if (bnewpatid)
                {
                    Study std;
                    std.studyuid = studyuid.c_str();
                    std.sopclassuid = sopclassuid.c_str();

                    OFString studydate,studydesc,dir,transfersyntax;
                    if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).good())
                        std.studydate = studydate.c_str();

                    if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).good())
                        std.studydesc = studydesc.c_str();

                    if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).good())
                        std.transfersyntax = transfersyntax.c_str();

                    DcmXfer filexfer(dcm.getDataset()->getOriginalXfer());
                    std.originalXfer = filexfer.getXferID();

                    std.filespath.push_back(it.toStdString());
                    std.dir = QFileInfo(path.c_str()).absolutePath().toStdString().c_str() ;

                    Patient patient;
                    patient.patientid = patid.c_str();
                    patient.patientname = patname.c_str();
                    patient.studydatas.push_back(std);
                    m_listpatient.push_back(patient);
                }
            }
        }
    }
    emit finishScanDir(m_listpatient);
}



/////-------------send dicom--------------------------------------
void Taskthread::dicomSendJob()
{
    int n = m_listpatient.size();
    m_sendFiles = 0;
    if (n < 1)
    {
        emit  finishSendDcm(m_sendFiles);
        return;
    }
    foreach (Patient pt, m_listpatient)
    {
        foreach(Study st, pt.studydatas)
        {

            if (sendStudy(st) == 1)
            {
                emit  finishSendDcm(-1);
                return;
            }
            Sleep(10);
        }

    }

    emit  finishSendDcm(m_sendFiles);
}

////----
bool Taskthread::isCanceled()
{

    return false;
}


int Taskthread::sendStudy(Study &studys)
{
    DcmSCU scu;

    if (isCanceled() || m_dest.ourAETitle.length() < 1 || m_dest.destinationHost.length() <1
            || m_dest.destinationAETitle.length() < 1)
        return 1;

    scu.setVerbosePCMode(true);
    scu.setAETitle(m_dest.ourAETitle.c_str());
    scu.setPeerHostName(m_dest.destinationHost.c_str());
    scu.setPeerPort(m_dest.destinationPort);
    scu.setPeerAETitle(m_dest.destinationAETitle.c_str());
    scu.setACSETimeout(60);
    scu.setDIMSETimeout(120);
    scu.setDatasetConversionMode(true);

    OFList<OFString> defaulttransfersyntax,dcmfiles;
    defaulttransfersyntax.push_back(UID_LittleEndianImplicitTransferSyntax);//
    defaulttransfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);
    defaulttransfersyntax.push_back(UID_JPEGLSLosslessTransferSyntax);
    defaulttransfersyntax.push_back(UID_JPEGProcess14SV1TransferSyntax);//
    defaulttransfersyntax.push_back(UID_JPEG2000LosslessOnlyTransferSyntax);

    foreach(std::string dcmf, studys.filespath)
    {
        dcmfiles.push_back(dcmf.c_str());
    }

    OFCondition cond = scu.addPresentationContext(studys.sopclassuid.c_str(), defaulttransfersyntax);//EC_Normal
    if (cond != EC_Normal)
        return 1;


    if(scu.initNetwork().bad())
        return 1;

    if(scu.negotiateAssociation().bad())
        return 1;


    int isend = 0;
    for(OFIterator<OFString> it = dcmfiles.begin(); it!= dcmfiles.end(); it++)
    {
        if(isCanceled())
        {
            break;
        }

        Uint16 status;

        // load file
        DcmFileFormat dcmff;   //QString filename = it->c_str();
        dcmff.loadFile(it->c_str());

        // do some precheck of the transfer syntax
        DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());
        OFString sopclassuid;
        dcmff.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopclassuid);

        if (scu.findPresentationContextID(sopclassuid, UID_JPEGProcess14SV1TransferSyntax) != 0)//UID_JPEGProcess14SV1TransferSyntax
        {
            dcmff.loadAllDataIntoMemory();

            if(dcmff.getDataset())
                dcmff.getDataset()->chooseRepresentation(EXS_JPEGLSLossless, NULL);

            fileTransfer = dcmff.getDataset()->getCurrentXfer();
        }

        // out found.. change to
        T_ASC_PresentationContextID pid;
        pid = scu.findAnyPresentationContextID(sopclassuid, fileTransfer.getXferID());

        registerCodecs();
        cond = scu.sendSTORERequest(pid, "", dcmff.getDataset(), status);
        registercleanup();

        if (cond.good() && (status == 0 || (status & 0xf000) == 0xb000))
        {
            //ok!
        }
        else if(cond == DUL_PEERABORTEDASSOCIATION)
        {
            return 1;
        }
        //else            // some error? keep going
        //{
        //    itr++;
        //}

        isend++;
        m_sendFiles ++;
        if (isend > 50)
        {
            Sleep(1);
            isend = 0;
            emit  finishSendDcm(m_sendFiles);
        }
    }

    scu.releaseAssociation();
    return 0;
}

///DicomSender call ---> Taskthread::sendDcm
void Taskthread::sendDcm(DestinationEntry dest, std::vector<Patient> listpat)
{
    m_dest = dest;

    m_listpatient = listpat;

}

/////###############################################################################################
/////---------------------class-----DicomSender-----------------------------------------------------
DicomSender::DicomSender()
{
    qRegisterMetaType<std::vector<Patient>>("std::vector<Patient>");
    connect(this, &DicomSender::scandicomfile, &m_taskScanDicom, &Taskthread::scanDir);
    connect(&m_taskScanDicom, &Taskthread::finishScanDir, this, &DicomSender::finishlistpatient);


    qRegisterMetaType<DestinationEntry>("DestinationEntry");
    connect(this, &DicomSender::senddicomfile, &m_taskSendDicom, &Taskthread::sendDcm);
    connect(&m_taskSendDicom, &Taskthread::finishSendDcm, this, &DicomSender::finishSendDcm);

}

bool DicomSender::IsCanceled()
{
    return false;
}

bool DicomSender::Echo()
{
    DcmSCU scu;

    scu.setVerbosePCMode(true);
    scu.setAETitle(m_destination.ourAETitle.c_str());
    scu.setPeerHostName(m_destination.destinationHost.c_str());
    scu.setPeerPort(m_destination.destinationPort);
    scu.setPeerAETitle(m_destination.destinationAETitle.c_str());
    scu.setACSETimeout(30);
    scu.setDIMSETimeout(60);
    scu.setDatasetConversionMode(true);

    OFList<OFString> transfersyntax;
    transfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);
    transfersyntax.push_back(UID_LittleEndianImplicitTransferSyntax);
    scu.addPresentationContext(UID_VerificationSOPClass, transfersyntax);

    OFCondition cond;
    cond = scu.initNetwork();
    if(cond.bad())
        return false;

    cond = scu.negotiateAssociation();
    if(cond.bad())
        return false;

    cond = scu.sendECHORequest(0);

    scu.releaseAssociation();

    if(cond == EC_Normal)
    {
        return true;
    }

    return false;
}

////-------------------------scan dicomfiles
/// mainwindow call ---> DicomSender::ScanPatient
void DicomSender::ScanPatient(QString dir)
{
    emit scandicomfile(dir,m_listpatient);

    m_taskScanDicom.setJob(0);
    QThreadPool::globalInstance()->start(&m_taskScanDicom);
}
///call ---> mainwindow call
void DicomSender::finishlistpatient(std::vector<Patient> listpat)
{

    m_listpatient = listpat;

    emit finishscandicomfile();
}


///---------------------------send dicomfiles
/// mainwindow call --->DicomSender::SendPatiens  --->call  thread
bool DicomSender::SendPatiens(std::vector<Patient> listpat)
{
    emit senddicomfile(m_destination, listpat);

    m_taskSendDicom.setJob(1);
    QThreadPool::globalInstance()->start(&m_taskSendDicom);

    return true;
}
///call ---> mainwindow call
void DicomSender::finishSendDcm(int sendFiles)
{

    emit finishsenddicomfile(sendFiles);

}

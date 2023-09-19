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

bool Taskthread::g_static_check = false;
OFString  Taskthread::g_Pname = "Anonymous";
OFString  Taskthread::g_InsUID = "1.2.826.0.1.3680043.9.7604.";

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

bool Taskthread::updateStringAttributeValue(DcmItem *dataset, const DcmTagKey &key, OFString &value)
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

    cond = elem->putOFStringArray(value);
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

    initAbstractSyntax();

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
            OFString  transfersyntax,patid,patname;

            if (dcm.getDataset()->findAndGetOFString(DCM_StudyInstanceUID, studyuid).bad())
                return;

            if (dcm.getDataset()->findAndGetOFString(DCM_PatientID, patid).bad())
                return;

            if (dcm.getDataset()->findAndGetOFString(DCM_PatientName, patname).bad())
                return;

            if (m_listpatient.empty())
            {
                Study std;
                std.studyuid = studyuid.c_str();

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
                //emit  finishSendDcm(-1);
                //return;
            }
            Sleep(10);
        }

    }

    emit  finishSendDcm(0);
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

    defaulttransfersyntax.push_back(UID_LittleEndianImplicitTransferSyntax);
    defaulttransfersyntax.push_back(UID_BigEndianExplicitTransferSyntax);
    defaulttransfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);
    defaulttransfersyntax.push_back(UID_JPEGProcess14SV1TransferSyntax);
    defaulttransfersyntax.push_back(UID_JPEGLSLosslessTransferSyntax);
    defaulttransfersyntax.push_back(UID_JPEG2000LosslessOnlyTransferSyntax);


    OFList<OFString> abstractSyntaxlist;
    OFCondition cond;
    foreach(OFString abstractSyntax, m_abstractSyntaxlist)
    {
        scu.addPresentationContext(abstractSyntax, defaulttransfersyntax);
    }

    if(scu.initNetwork().bad())
        return 1;

    if(scu.negotiateAssociation().bad())
        return 1;


    int isend = 0;
    OFString newname,newuid;
    if (Taskthread::g_Pname.length() > 1)
        newname = Taskthread::g_Pname;
    if (Taskthread::g_InsUID.length() > 1)
        newuid = Taskthread::g_InsUID;

    //for(OFIterator<OFString> it = dcmfiles.begin(); it!= dcmfiles.end(); it++)
    foreach(std::string dcmf, studys.filespath)
    {
//        if(isCanceled())
//        {
//            break;
//        }
        Uint16 status;
        // load file
        DcmFileFormat dcmff;   //QString filename = it->c_str();
        dcmff.loadFile(dcmf.c_str());

        // do some precheck of the transfer syntax
        DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());
        OFString sopclassuid;
        dcmff.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopclassuid);
        //if (scu.findPresentationContextID(sopclassuid, UID_JPEGLSLosslessTransferSyntax) != 0)//UID_JPEGProcess14SV1TransferSyntax
        //{
        //    dcmff.loadAllDataIntoMemory();
        //    if(dcmff.getDataset())
        //        dcmff.getDataset()->chooseRepresentation(EXS_JPEGLSLossless, NULL);
        //    fileTransfer = dcmff.getDataset()->getCurrentXfer();
        //}
        //qDebug("Send files numbers: %s", dcmf.c_str());
        // out found.. change to
        T_ASC_PresentationContextID pid;
        pid = scu.findAnyPresentationContextID(sopclassuid, fileTransfer.getXferID());

        registerCodecs();
        if (Taskthread::g_static_check)
        {
            OFString ImpUID,Studyuid;
            if (dcmff.getMetaInfo()->findAndGetOFString(DCM_ImplementationClassUID, ImpUID).bad())
            {
                ImpUID = "1.2.826.0.1.3680043.9.7604.";
            }
            dcmff.getDataset()->findAndGetOFString(DCM_StudyInstanceUID, Studyuid);

            updateStringAttributeValue(dcmff.getDataset(),DCM_PatientName, newname);

            newuid         += Studyuid.substr(ImpUID.length()+3,Studyuid.length());
            updateStringAttributeValue(dcmff.getDataset(),DCM_StudyInstanceUID, newuid);

            OFString MedicalName = "DCMTK_OmlyTEST";
            updateStringAttributeValue(dcmff.getDataset(),DCM_InstitutionName, MedicalName);

            cond = scu.sendSTORERequest(pid, NULL, dcmff.getDataset(), status);
        }
        else
        {
            cond = scu.sendSTORERequest(pid, dcmf.c_str(), NULL, status);
        }
        registercleanup();

        if (cond.good() && (status == 0 || (status & 0xf000) == 0xb000))
        {
            //ok!
        }
        else if(cond == DUL_PEERABORTEDASSOCIATION)
        {
            //return 1;
        }

        isend++;
        m_sendFiles ++;
        if (isend > 20)
        {
            Sleep(1);
            isend = 0;
            emit  finishSendDcm(m_sendFiles);
        }
        //qDebug("%4d  %s", ++n, dcmf.c_str());
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
void DicomSender::SetUpateDcmFileAnonymous(bool key )
{
    Taskthread::g_static_check = key;
}


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

void Taskthread::initAbstractSyntax()
{
    m_abstractSyntaxlist.push_back(UID_RETIRED_StoredPrintStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_HardcopyGrayscaleImageStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_HardcopyColorImageStorage);
    m_abstractSyntaxlist.push_back(UID_ComputedRadiographyImageStorage);
    m_abstractSyntaxlist.push_back(UID_DigitalXRayImageStorageForPresentation);
    m_abstractSyntaxlist.push_back(UID_DigitalXRayImageStorageForProcessing);
    m_abstractSyntaxlist.push_back(UID_DigitalMammographyXRayImageStorageForPresentation);
    m_abstractSyntaxlist.push_back(UID_DigitalMammographyXRayImageStorageForProcessing);
    m_abstractSyntaxlist.push_back(UID_DigitalIntraOralXRayImageStorageForPresentation);
    m_abstractSyntaxlist.push_back(UID_DigitalIntraOralXRayImageStorageForProcessing);
    m_abstractSyntaxlist.push_back(UID_CTImageStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedCTImageStorage);
    m_abstractSyntaxlist.push_back(UID_LegacyConvertedEnhancedCTImageStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundMultiframeImageStorage);
    m_abstractSyntaxlist.push_back(UID_UltrasoundMultiframeImageStorage);
    m_abstractSyntaxlist.push_back(UID_MRImageStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedMRImageStorage);
    m_abstractSyntaxlist.push_back(UID_MRSpectroscopyStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedMRColorImageStorage);
    m_abstractSyntaxlist.push_back(UID_LegacyConvertedEnhancedMRImageStorage);


    m_abstractSyntaxlist.push_back(UID_RETIRED_NuclearMedicineImageStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
    m_abstractSyntaxlist.push_back(UID_UltrasoundImageStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedUSVolumeStorage);
    m_abstractSyntaxlist.push_back(UID_SecondaryCaptureImageStorage);
    m_abstractSyntaxlist.push_back(UID_MultiframeSingleBitSecondaryCaptureImageStorage);
    m_abstractSyntaxlist.push_back(UID_MultiframeGrayscaleByteSecondaryCaptureImageStorage);
    m_abstractSyntaxlist.push_back(UID_MultiframeGrayscaleWordSecondaryCaptureImageStorage);
    m_abstractSyntaxlist.push_back(UID_MultiframeTrueColorSecondaryCaptureImageStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_StandaloneCurveStorage);
    m_abstractSyntaxlist.push_back(UID_TwelveLeadECGWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_GeneralECGWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_AmbulatoryECGWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_HemodynamicWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_CardiacElectrophysiologyWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_BasicVoiceAudioWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_GeneralAudioWaveformStorage);

    m_abstractSyntaxlist.push_back(UID_ArterialPulseWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_RespiratoryWaveformStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_StandaloneModalityLUTStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_StandaloneVOILUTStorage);
    m_abstractSyntaxlist.push_back(UID_GrayscaleSoftcopyPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_ColorSoftcopyPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_PseudoColorSoftcopyPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_BlendingSoftcopyPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_XAXRFGrayscaleSoftcopyPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_GrayscalePlanarMPRVolumetricPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_CompositingPlanarMPRVolumetricPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_AdvancedBlendingPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_VolumeRenderingVolumetricPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_SegmentedVolumeRenderingVolumetricPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_MultipleVolumeRenderingVolumetricPresentationStateStorage);
    m_abstractSyntaxlist.push_back(UID_XRayAngiographicImageStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedXAImageStorage);

    m_abstractSyntaxlist.push_back(UID_XRayRadiofluoroscopicImageStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedXRFImageStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_XRayAngiographicBiPlaneImageStorage);
    m_abstractSyntaxlist.push_back(UID_XRay3DAngiographicImageStorage);
    m_abstractSyntaxlist.push_back(UID_XRay3DCraniofacialImageStorage);
    m_abstractSyntaxlist.push_back(UID_BreastTomosynthesisImageStorage);
    m_abstractSyntaxlist.push_back(UID_BreastProjectionXRayImageStorageForPresentation);
    m_abstractSyntaxlist.push_back(UID_BreastProjectionXRayImageStorageForProcessing);
    m_abstractSyntaxlist.push_back(UID_IntravascularOpticalCoherenceTomographyImageStorageForPresentation);
    m_abstractSyntaxlist.push_back(UID_IntravascularOpticalCoherenceTomographyImageStorageForProcessing);
    m_abstractSyntaxlist.push_back(UID_NuclearMedicineImageStorage);
    m_abstractSyntaxlist.push_back(UID_ParametricMapStorage);
    m_abstractSyntaxlist.push_back(UID_RawDataStorage);
    m_abstractSyntaxlist.push_back(UID_SpatialRegistrationStorage);
    m_abstractSyntaxlist.push_back(UID_SpatialFiducialsStorage);
    m_abstractSyntaxlist.push_back(UID_DeformableSpatialRegistrationStorage);
    m_abstractSyntaxlist.push_back(UID_SegmentationStorage);

    m_abstractSyntaxlist.push_back(UID_SurfaceSegmentationStorage);
    m_abstractSyntaxlist.push_back(UID_TractographyResultsStorage);
    m_abstractSyntaxlist.push_back(UID_RealWorldValueMappingStorage);
    m_abstractSyntaxlist.push_back(UID_SurfaceScanMeshStorage);
    m_abstractSyntaxlist.push_back(UID_SurfaceScanPointCloudStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_VLImageStorage);
    m_abstractSyntaxlist.push_back(UID_VLEndoscopicImageStorage);
    m_abstractSyntaxlist.push_back(UID_VideoEndoscopicImageStorage);
    m_abstractSyntaxlist.push_back(UID_VLMicroscopicImageStorage);
    m_abstractSyntaxlist.push_back(UID_VideoMicroscopicImageStorage);
    m_abstractSyntaxlist.push_back(UID_VLSlideCoordinatesMicroscopicImageStorage);
    m_abstractSyntaxlist.push_back(UID_VLPhotographicImageStorage);
    m_abstractSyntaxlist.push_back(UID_VideoPhotographicImageStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicPhotography8BitImageStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicPhotography16BitImageStorage);
    m_abstractSyntaxlist.push_back(UID_StereometricRelationshipStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicTomographyImageStorage);

    m_abstractSyntaxlist.push_back(UID_WideFieldOphthalmicPhotographyStereographicProjectionImageStorage);
    m_abstractSyntaxlist.push_back(UID_WideFieldOphthalmicPhotography3DCoordinatesImageStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicOpticalCoherenceTomographyEnFaceImageStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicOpticalCoherenceTomographyBscanVolumeAnalysisStorage);
    m_abstractSyntaxlist.push_back(UID_VLWholeSlideMicroscopyImageStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_VLMultiFrameImageStorage);
    m_abstractSyntaxlist.push_back(UID_LensometryMeasurementsStorage);
    m_abstractSyntaxlist.push_back(UID_AutorefractionMeasurementsStorage);
    m_abstractSyntaxlist.push_back(UID_KeratometryMeasurementsStorage);
    m_abstractSyntaxlist.push_back(UID_SubjectiveRefractionMeasurementsStorage);
    m_abstractSyntaxlist.push_back(UID_VisualAcuityMeasurementsStorage);
    m_abstractSyntaxlist.push_back(UID_SpectaclePrescriptionReportStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicAxialMeasurementsStorage);
    m_abstractSyntaxlist.push_back(UID_IntraocularLensCalculationsStorage);
    m_abstractSyntaxlist.push_back(UID_MacularGridThicknessAndVolumeReportStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicVisualFieldStaticPerimetryMeasurementsStorage);
    m_abstractSyntaxlist.push_back(UID_OphthalmicThicknessMapStorage);
    m_abstractSyntaxlist.push_back(UID_CornealTopographyMapStorage);
    m_abstractSyntaxlist.push_back(UID_BasicTextSRStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedSRStorage);
    m_abstractSyntaxlist.push_back(UID_ComprehensiveSRStorage);
    m_abstractSyntaxlist.push_back(UID_Comprehensive3DSRStorage);
    m_abstractSyntaxlist.push_back(UID_ExtensibleSRStorage);
    m_abstractSyntaxlist.push_back(UID_ProcedureLogStorage);
    m_abstractSyntaxlist.push_back(UID_MammographyCADSRStorage);
    m_abstractSyntaxlist.push_back(UID_KeyObjectSelectionDocumentStorage);
    m_abstractSyntaxlist.push_back(UID_ChestCADSRStorage);
    m_abstractSyntaxlist.push_back(UID_XRayRadiationDoseSRStorage);
    m_abstractSyntaxlist.push_back(UID_RadiopharmaceuticalRadiationDoseSRStorage);
    m_abstractSyntaxlist.push_back(UID_ColonCADSRStorage);
    m_abstractSyntaxlist.push_back(UID_ImplantationPlanSRDocumentStorage);
    m_abstractSyntaxlist.push_back(UID_AcquisitionContextSRStorage);
    m_abstractSyntaxlist.push_back(UID_SimplifiedAdultEchoSRStorage);
    m_abstractSyntaxlist.push_back(UID_PatientRadiationDoseSRStorage);
    m_abstractSyntaxlist.push_back(UID_ContentAssessmentResultsStorage);
    m_abstractSyntaxlist.push_back(UID_EncapsulatedPDFStorage);
    m_abstractSyntaxlist.push_back(UID_EncapsulatedCDAStorage);
    m_abstractSyntaxlist.push_back(UID_PositronEmissionTomographyImageStorage);
    m_abstractSyntaxlist.push_back(UID_LegacyConvertedEnhancedPETImageStorage);
    m_abstractSyntaxlist.push_back(UID_RETIRED_StandalonePETCurveStorage);
    m_abstractSyntaxlist.push_back(UID_EnhancedPETImageStorage);
    m_abstractSyntaxlist.push_back(UID_BasicStructuredDisplayStorage);
    m_abstractSyntaxlist.push_back(UID_CTDefinedProcedureProtocolStorage);
    m_abstractSyntaxlist.push_back(UID_CTPerformedProcedureProtocolStorage);
    m_abstractSyntaxlist.push_back(UID_ProtocolApprovalStorage);
    m_abstractSyntaxlist.push_back(UID_RTImageStorage);
    m_abstractSyntaxlist.push_back(UID_RTDoseStorage);
    m_abstractSyntaxlist.push_back(UID_RTStructureSetStorage);
    m_abstractSyntaxlist.push_back(UID_RTBeamsTreatmentRecordStorage);
    m_abstractSyntaxlist.push_back(UID_RTPlanStorage);
    m_abstractSyntaxlist.push_back(UID_RTBrachyTreatmentRecordStorage);
    m_abstractSyntaxlist.push_back(UID_RTTreatmentSummaryRecordStorage);
    m_abstractSyntaxlist.push_back(UID_RTIonPlanStorage);
    m_abstractSyntaxlist.push_back(UID_RTIonBeamsTreatmentRecordStorage);
    m_abstractSyntaxlist.push_back(UID_RTBeamsDeliveryInstructionStorage);
    m_abstractSyntaxlist.push_back(UID_RTBrachyApplicationSetupDeliveryInstructionStorage);
    m_abstractSyntaxlist.push_back(UID_HangingProtocolStorage);
    m_abstractSyntaxlist.push_back(UID_GenericImplantTemplateStorage);
    m_abstractSyntaxlist.push_back(UID_ImplantAssemblyTemplateStorage);
    m_abstractSyntaxlist.push_back(UID_ImplantTemplateGroupStorage);

//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
//    m_abstractSyntaxlist.push_back(UID_RETIRED_UltrasoundImageStorage);
}

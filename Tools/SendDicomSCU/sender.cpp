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


#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif

DicomSender::DicomSender()
{

}

bool DicomSender::IsCanceled()
{
    return false;
}

int DicomSender::SendStudy(Study &studys)
{

    return 1;
}

void  DicomSender::AddStudy(QString dir)
{

}

void DicomSender::UpdatePatientdatas(DcmDataset *data)
{

}

void DicomSender::ScanPatient(QString dir)
{
    QStringList filters;
    filters<<"DCM"<<"dcm";
    QStringList list = DirFilename(dir,filters);
    DcmFileFormat dcm;
    OFCondition cond;
    Study std;
    Patient pd;
    //std::vector<Patient> listpatient;
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

            if (listpatient.empty())
            {
                //std::string studyuid,studydate,studydesc,dir,sopclassuid,transfersyntax;
                Study std;
                std.studyuid = studyuid.c_str();
                std.sopclassuid = sopclassuid.c_str();

                OFString studydate,studydesc,dir,transfersyntax;
                if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).bad())
                    std.studydate = studydate.c_str();

                if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).bad())
                    std.studydesc = studydesc.c_str();

                if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).bad())
                    std.transfersyntax = transfersyntax.c_str();

                std.filespath.push_back(path);

                Patient patient;
                patient.patientid = patid.c_str();
                patient.patientname = patname.c_str();
                patient.studydatas.push_back(std);
                listpatient.push_back(patient);

            }
            else
            {
                //find
                bool bpatid = false;
                foreach(Patient pt, listpatient)
                {
                    bool flg = false;
                    foreach(Study std, pt.studydatas)
                    {
                        if (std.studyuid == studyuid.c_str())
                        {
                            std.filespath.push_back(path);
                            flg = true;
                            break;
                        }
                    }
                    if (flg)
                    {
                        break;
                    }
                    else
                    {
                        if (pt.patientid == patid.c_str())
                        {
                            Study std;
                            std.studyuid = studyuid.c_str();
                            std.sopclassuid = sopclassuid.c_str();

                            OFString studydate,studydesc,dir,transfersyntax;
                            if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).bad())
                                std.studydate = studydate.c_str();

                            if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).bad())
                                std.studydesc = studydesc.c_str();

                            if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).bad())
                                std.transfersyntax = transfersyntax.c_str();

                            std.filespath.push_back(path);
                            pt.studydatas.push_back(std);

                            bpatid = true;
                            break;
                        }
                    }
                }
                if (bpatid)
                {
                    Study std;
                    std.studyuid = studyuid.c_str();
                    std.sopclassuid = sopclassuid.c_str();

                    OFString studydate,studydesc,dir,transfersyntax;
                    if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).bad())
                        std.studydate = studydate.c_str();

                    if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).bad())
                        std.studydesc = studydesc.c_str();

                    if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).bad())
                        std.transfersyntax = transfersyntax.c_str();

                    std.filespath.push_back(path);

                    Patient patient;
                    patient.patientid = patid.c_str();
                    patient.patientname = patname.c_str();
                    patient.studydatas.push_back(std);
                    listpatient.push_back(patient);
                }
            }

//            DcmXfer filexfer(dcm.getDataset()->getOriginalXfer());
//            transfersyntax = filexfer.getXferID();
        }
    }

}


int DicomSender::SendDcmFiles(Study &studys)
{
    DcmSCU scu;

    if (IsCanceled() || destination.ourAETitle.length() < 1 || destination.destinationHost.length() <1 || destination.destinationAETitle.length() < 1)
        return 1;

    scu.setVerbosePCMode(true);
    scu.setAETitle(destination.ourAETitle.c_str());
    scu.setPeerHostName(destination.destinationHost.c_str());
    scu.setPeerPort(destination.destinationPort);
    scu.setPeerAETitle(destination.destinationAETitle.c_str());
    scu.setACSETimeout(60);
    scu.setDIMSETimeout(120);
    scu.setDatasetConversionMode(true);

    OFList<OFString> defaulttransfersyntax,dcmfiles;
    defaulttransfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);//

    foreach(std::string dcmf, studys.filespath)
    {
        dcmfiles.push_back(dcmf.c_str());
    }

    scu.addPresentationContext(studys.sopclassuid.c_str(), defaulttransfersyntax);
    OFCondition cond;

    if(scu.initNetwork().bad())
        return 1;

    if(scu.negotiateAssociation().bad())
        return 1;


    for(OFIterator<OFString> it = dcmfiles.begin(); it!= dcmfiles.end(); it++)
    {
        if(IsCanceled())
        {
            break;
        }

        Uint16 status;

        // load file
        DcmFileFormat dcmff;
        dcmff.loadFile(it->c_str());

        // do some precheck of the transfer syntax
        DcmXfer fileTransfer(dcmff.getDataset()->getOriginalXfer());
        OFString sopclassuid;
        dcmff.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopclassuid);

        if (scu.findPresentationContextID(sopclassuid, UID_JPEGLSLosslessTransferSyntax) != 0)
        {
            dcmff.loadAllDataIntoMemory();

            if(dcmff.getDataset())
                dcmff.getDataset()->chooseRepresentation(EXS_JPEGLSLossless, NULL);

            fileTransfer = dcmff.getDataset()->getCurrentXfer();
        }

        // out found.. change to
        T_ASC_PresentationContextID pid = scu.findAnyPresentationContextID(sopclassuid, fileTransfer.getXferID());

        cond = scu.sendSTORERequest(pid, "", dcmff.getDataset(), status);
        if (cond.good() && (status == 0 || (status & 0xf000) == 0xb000))
        {
            //ok!
        }
        else if(cond == DUL_PEERABORTEDASSOCIATION)
        {
            return 1;
        }
        //        else            // some error? keep going
        //        {
        //            itr++;
        //        }
    }

    scu.releaseAssociation();
    return 0;
}

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
#include <QThreadPool>
#include <QDir>
#include <QMetaType>

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif


/////--------------------------Taskthread-----------------------------------------------------
Taskthread::Taskthread(QObject *parent)
{
    setAutoDelete(false);
}

Taskthread::~Taskthread()
{

}

void Taskthread::run()
{
    QStringList filters;
    filters<<"*.DCM"<<"*.dcm";
    QStringList list = DirFilename(scanDir,filters);
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

            if (listpatient.empty())
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
                listpatient.push_back(patient);

            }
            else
            {
                //find
                bool bnewpatid = true;
                for(std::vector<Patient>::iterator pt = listpatient.begin(); pt != listpatient.end(); ++pt)
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
                            if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).bad())
                                std.studydate = studydate.c_str();

                            if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).bad())
                                std.studydesc = studydesc.c_str();

                            if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).bad())
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
                    if (dcm.getDataset()->findAndGetOFString(DCM_StudyDate, studydate).bad())
                        std.studydate = studydate.c_str();

                    if (dcm.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc).bad())
                        std.studydesc = studydesc.c_str();

                    if (dcm.getDataset()->findAndGetOFString(DCM_TransferSyntaxUID, transfersyntax).bad())
                        std.transfersyntax = transfersyntax.c_str();

                    DcmXfer filexfer(dcm.getDataset()->getOriginalXfer());
                    std.originalXfer = filexfer.getXferID();

                    std.filespath.push_back(it.toStdString());
                    std.dir = QFileInfo(path.c_str()).absolutePath().toStdString().c_str() ;

                    Patient patient;
                    patient.patientid = patid.c_str();
                    patient.patientname = patname.c_str();
                    patient.studydatas.push_back(std);
                    listpatient.push_back(patient);
                }
            }
        }
    }
    emit finish(listpatient);
}


void Taskthread::scandir(QString dir, std::vector<Patient> listpat)
{
    scanDir = dir;
    //    listpatient.clear();
    listpatient = listpat;
}

/////-------------------------------------------------------------------------------
DicomSender::DicomSender()
{
    qRegisterMetaType<std::vector<Patient>>("std::vector<Patient>");
    connect(this, &DicomSender::scandicomfile, &m_taskSdicom, &Taskthread::scandir);
    connect(&m_taskSdicom, &Taskthread::finish, this, &DicomSender::finishlistpatient);
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
    emit scandicomfile(dir,m_listpatient);

    QThreadPool::globalInstance()->start(&m_taskSdicom);
}

void DicomSender::finishlistpatient(std::vector<Patient> listpat)
{

    m_listpatient = listpat;

    emit finishscandicomfile();
}


int DicomSender::SendDcmFiles(Study &studys)
{
    DcmSCU scu;

    if (IsCanceled() || m_destination.ourAETitle.length() < 1 || m_destination.destinationHost.length() <1 || m_destination.destinationAETitle.length() < 1)
        return 1;

    scu.setVerbosePCMode(true);
    scu.setAETitle(m_destination.ourAETitle.c_str());
    scu.setPeerHostName(m_destination.destinationHost.c_str());
    scu.setPeerPort(m_destination.destinationPort);
    scu.setPeerAETitle(m_destination.destinationAETitle.c_str());
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

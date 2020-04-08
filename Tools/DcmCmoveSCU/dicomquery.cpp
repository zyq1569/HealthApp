
#include "dicomquery.h"

#include <boost/thread.hpp>

// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/scu.h"

// check DCMTK functionality
#if !defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
//#error "DCMTK and this program must be compiled with DCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS"
#endif

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif


DICOMQueryScanner::DICOMQueryScanner(PatientData &patientdata)
    : patientdata(patientdata)
{
    cancelEvent = doneEvent = false;
}

DICOMQueryScanner::~DICOMQueryScanner()
{
}

bool DICOMQueryScanner::ScanPatientName(std::string name, DestinationEntry &destination)
{
    class MyDcmSCU : public DcmSCU
    {
    public:
        MyDcmSCU(PatientData &patientdata, DICOMQueryScanner &scanner) : patientdata(patientdata), scanner(scanner) {}
        PatientData &patientdata;
        DICOMQueryScanner &scanner;
        OFCondition handleFINDResponse(const T_ASC_PresentationContextID presID,
            QRResponse *response,
            OFBool &waitForNextResponse)
        {
            OFCondition ret = DcmSCU::handleFINDResponse(presID, response, waitForNextResponse);

            if (ret.good() && response->m_dataset != NULL)
            {
                OFString patientname, patientid, birthday;
                OFString studyuid, modality, studydesc, studydate;
                response->m_dataset->findAndGetOFString(DCM_StudyInstanceUID, studyuid);
                response->m_dataset->findAndGetOFString(DCM_PatientID, patientid);
                response->m_dataset->findAndGetOFString(DCM_PatientName, patientname);
                response->m_dataset->findAndGetOFString(DCM_StudyDescription, studydesc);
                response->m_dataset->findAndGetOFString(DCM_StudyDate, studydate);

                patientdata.AddStudy(studyuid.c_str(), patientid.c_str(), patientname.c_str(), studydesc.c_str(), studydate.c_str());
            }

            if (scanner.IsCanceled())
                waitForNextResponse = false;

            return ret;
        }
    };

    if (IsCanceled())
        return true;

    MyDcmSCU scu(patientdata, *this);

    scu.setVerbosePCMode(true);
    scu.setAETitle(destination.ourAETitle.c_str());
    scu.setPeerHostName(destination.destinationHost.c_str());
    scu.setPeerPort(destination.destinationPort);
    scu.setPeerAETitle(destination.destinationAETitle.c_str());
    scu.setACSETimeout(30);
    scu.setDIMSETimeout(60);
    scu.setDatasetConversionMode(true);

    OFList<OFString> defaulttransfersyntax;
    defaulttransfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);

    scu.addPresentationContext(UID_FINDStudyRootQueryRetrieveInformationModel, defaulttransfersyntax);

    OFCondition cond;

    if (scu.initNetwork().bad())
        return false;

    if (scu.negotiateAssociation().bad())
        return false;

    T_ASC_PresentationContextID pid = scu.findAnyPresentationContextID(UID_FINDStudyRootQueryRetrieveInformationModel, UID_LittleEndianExplicitTransferSyntax);

    DcmDataset query;
    query.putAndInsertString(DCM_QueryRetrieveLevel, "STUDY");
    query.putAndInsertString(DCM_StudyInstanceUID, "");
    query.putAndInsertString(DCM_PatientName, name.c_str());
    query.putAndInsertString(DCM_PatientID, "");
    query.putAndInsertString(DCM_StudyDate, "");
    query.putAndInsertString(DCM_StudyDescription, "");
    query.putAndInsertSint16(DCM_NumberOfStudyRelatedInstances, 0);
    scu.sendFINDRequest(pid, &query, NULL);

    scu.releaseAssociation();

    return true;
}

void DICOMQueryScanner::DoQueryAsync(DestinationEntry &destination)
{
    SetDone(false);
    ClearCancel();

    m_destination = destination;
    boost::thread t(DICOMQueryScanner::DoQueryThread, this);
    t.detach();
}

void DICOMQueryScanner::DoQueryThread(void *obj)
{
    DICOMQueryScanner *me = (DICOMQueryScanner *) obj;
    if(me)
    {
        me->DoQuery(me->m_destination);
        me->SetDone(true);
    }

}

void DICOMQueryScanner::DoQuery(DestinationEntry &destination)
{
    OFLog::configure(OFLogger::OFF_LOG_LEVEL);

    // catch any access errors
    try
    {
        ScanPatientName("a", destination);
        ScanPatientName("e", destination);
        ScanPatientName("i", destination);
        ScanPatientName("o", destination);
        ScanPatientName("u", destination);
    }
    catch(...)
    {

    }

}


bool DICOMQueryScanner::Echo(DestinationEntry destination)
{
    DcmSCU scu;

    scu.setVerbosePCMode(true);
    scu.setAETitle(destination.ourAETitle.c_str());
    scu.setPeerHostName(destination.destinationHost.c_str());
    scu.setPeerPort(destination.destinationPort);
    scu.setPeerAETitle(destination.destinationAETitle.c_str());
    scu.setACSETimeout(30);
    scu.setDIMSETimeout(60);
    scu.setDatasetConversionMode(true);

    OFList<OFString> transfersyntax;
    transfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);
    transfersyntax.push_back(UID_LittleEndianImplicitTransferSyntax);
    scu.addPresentationContext(UID_VerificationSOPClass, transfersyntax);

    OFCondition cond;
    cond = scu.initNetwork();
    if (cond.bad())
        return false;

    cond = scu.negotiateAssociation();
    if (cond.bad())
        return false;

    cond = scu.sendECHORequest(0);

    scu.releaseAssociation();

    if (cond == EC_Normal)
    {
        return true;
    }

    return false;
}

void DICOMQueryScanner::Cancel()
{
    boost::mutex::scoped_lock lk(mutex);
    cancelEvent = true;
}

void DICOMQueryScanner::ClearCancel()
{
    boost::mutex::scoped_lock lk(mutex);
    cancelEvent = false;
}

bool DICOMQueryScanner::IsDone()
{
    boost::mutex::scoped_lock lk(mutex);
    return doneEvent;
}

bool DICOMQueryScanner::IsCanceled()
{
    boost::mutex::scoped_lock lk(mutex);
    return cancelEvent;
}

void DICOMQueryScanner::SetDone(bool state)
{
    boost::mutex::scoped_lock lk(mutex);
    doneEvent = state;
}

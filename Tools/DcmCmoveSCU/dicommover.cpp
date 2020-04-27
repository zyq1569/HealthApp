
#include "dicommover.h"
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
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmnet/scu.h"

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif

DICOMMover::DICOMMover(PatientData &patientdata)
    : patientdata(patientdata)
{
    cancelEvent = doneEvent = false;
}

DICOMMover::~DICOMMover()
{
}

void DICOMMover::DoMoveAsync(DestinationEntry destination, std::string moveae, int threads)
{
    SetDone(false);
    ClearCancel();

    this->m_destination = destination;
    this->m_moveae = moveae;
    this->m_threads = threads;

    // start the thread, let the sender manage (e.g. cancel), so we don't need to track anymore
    boost::thread t(DICOMMover::DoMoveThread, this);
    t.detach();
}

void DICOMMover::DoMoveThread(void *obj)
{
    DICOMMover *me = (DICOMMover *) obj;
    if (me)
    {
        me->DoMove(me->m_destination, me->m_moveae, me->m_threads);
        me->SetDone(true);
    }
}

void DICOMMover::DoMove(DestinationEntry destination, std::string moveae, int threads)
{
    OFLog::configure(OFLogger::OFF_LOG_LEVEL);

    // get a list of studies
    studies.clear();
    service.reset();
    patientdata.GetStudies(boost::bind(&DICOMMover::fillstudies, this, _1));
    for (std::vector<std::string>::iterator it = studies.begin() ; it != studies.end(); ++it)
    {
        // post them in the work
        service.post(boost::bind(&DICOMMover::MoveStudy, this, *it));
    }

    // run 5 threads
    boost::thread_group threadgroup;
    for (int i = 0; i < threads; i++)
    {
        threadgroup.create_thread(boost::bind(&boost::asio::io_service::run, &service));
    }

    // wait for everything to finish, Cancel() calls service stop and stops farther work from processing
    threadgroup.join_all();
}

int DICOMMover::fillstudies(Study &study)
{
    if (study.checked)
        studies.push_back(study.studyuid);
    return 0;
}


int DICOMMover::MoveStudy(std::string studyuid)
{
    class MyDcmSCU : public DcmSCU
    {
    public:
        MyDcmSCU(DICOMMover &sender) : sender(sender) {}
        bool newtransfer;
    protected:
        virtual OFCondition handleMOVEResponse(const T_ASC_PresentationContextID presID,
            RetrieveResponse *response,
            OFBool &waitForNextResponse)
        {
            DcmSCU::handleMOVEResponse(presID, response, waitForNextResponse);

            if (sender.IsCanceled())
                waitForNextResponse = false;

            return EC_Normal;
        }

        DICOMMover &sender;
    };

    if (IsCanceled())
        return true;

    MyDcmSCU scu(*this);

    scu.setVerbosePCMode(true);
    scu.setAETitle(m_destination.ourAETitle.c_str());
    scu.setPeerHostName(m_destination.destinationHost.c_str());
    scu.setPeerPort(m_destination.destinationPort);
    scu.setPeerAETitle(m_destination.destinationAETitle.c_str());
    scu.setACSETimeout(30);
    scu.setDIMSETimeout(60);
    scu.setDatasetConversionMode(true);
    OFList<OFString> defaulttransfersyntax;
    defaulttransfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);

    scu.addPresentationContext(UID_MOVEStudyRootQueryRetrieveInformationModel, defaulttransfersyntax);

    OFCondition cond;

    if(scu.initNetwork().bad())
        return 1;

    if(scu.negotiateAssociation().bad())
        return 1;

    T_ASC_PresentationContextID pid = scu.findAnyPresentationContextID(UID_MOVEStudyRootQueryRetrieveInformationModel, UID_LittleEndianImplicitTransferSyntax);

    DcmDataset query;
    query.putAndInsertString(DCM_QueryRetrieveLevel, "STUDY");
    query.putAndInsertString(DCM_StudyInstanceUID, studyuid.c_str());

    OFList<RetrieveResponse*> responses;
    scu.sendMOVERequest(pid, m_moveae.c_str(), &query, &responses);
    scu.releaseAssociation();

    if (!responses.empty())
    {
        if (responses.back()->m_numberOfRemainingSubops == 0 && responses.back()->m_numberOfFailedSubops == 0)
            patientdata.SetStudyCheck(studyuid, false);
    }

    // free it
    OFIterator<RetrieveResponse*> itr = responses.begin();
    while (itr != responses.end())
    {
        delete (*itr);
        itr++;
    }

    return 0;
}


void DICOMMover::Cancel()
{
    boost::mutex::scoped_lock lk(mutex);
    cancelEvent = true;
    service.stop();
}

void DICOMMover::ClearCancel()
{
    boost::mutex::scoped_lock lk(mutex);
    cancelEvent = false;
}

bool DICOMMover::IsDone()
{
    boost::mutex::scoped_lock lk(mutex);
    return doneEvent;
}

bool DICOMMover::IsCanceled()
{
    boost::mutex::scoped_lock lk(mutex);
    return cancelEvent;
}

void DICOMMover::SetDone(bool state)
{
    boost::mutex::scoped_lock lk(mutex);
    doneEvent = state;
}

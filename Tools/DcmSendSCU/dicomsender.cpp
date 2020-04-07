
#include "dicomsender.h"
#include "alphanum.hpp"
#include <set>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include "patientdata.h"
#include "destinationentry.h"
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include "destinationentry.h"
#include "ndcappender.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>


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

class DICOMSenderImpl
{

public:
	DICOMSenderImpl(PatientData &patientdata);
	~DICOMSenderImpl(void);

	void DoSendAsync(DestinationEntry destination, int threads);
	void DoSend(DestinationEntry destination, int threads);

	static bool Echo(DestinationEntry destination);

	void Cancel();
	bool IsDone();
protected:
	static void DoSendThread(void *obj);
	PatientData &patientdata;

	void SendStudy(boost::filesystem::path path);

	typedef std::map<std::string, std::set<std::string> > mapset;
	typedef std::map<std::string, boost::filesystem::path, doj::alphanum_less<std::string> > naturalpathmap;
	int SendABatch(const mapset &sopclassuidtransfersyntax, naturalpathmap &instances);

	void ScanDir(boost::filesystem::path path, naturalpathmap &instances, mapset &sopclassuidtransfersyntax, std::string &study_uid);
	void ScanFile(boost::filesystem::path path, naturalpathmap &instances, mapset &sopclassuidtransfersyntax, std::string &study_uid);

	bool IsCanceled();
	void ClearCancel();
	void SetDone(bool state);

	// threading data
	boost::mutex mutex;
	bool cancelEvent, doneEvent;
	DestinationEntry m_destination;
	int m_threads;

	int fillstudies(Study &study);
	void consumer();
	
	boost::mutex queue_mutex;
	std::vector<boost::filesystem::path> queue;        // list of directories that we will be sending
};

DICOMSenderImpl::DICOMSenderImpl(PatientData &patientdata)
	: patientdata(patientdata)
{			
	cancelEvent = doneEvent = false;
}

DICOMSenderImpl::~DICOMSenderImpl()
{
}

void DICOMSenderImpl::DoSendAsync(DestinationEntry destination, int threads)
{
	SetDone(false);
	ClearCancel();
	
	this->m_destination = destination;	
	this->m_threads = threads;

	// start the thread, let the sender manage (e.g. cancel), so we don't need to track anymore
	boost::thread t(DICOMSenderImpl::DoSendThread, this);
	t.detach();
}


void DICOMSenderImpl::DoSendThread(void *obj)
{
	DICOMSenderImpl *me = (DICOMSenderImpl *) obj;

	dcmtk::log4cplus::SharedAppenderPtr stringlogger(new NDCAsFilenameAppender(boost::filesystem::temp_directory_path()));
	dcmtk::log4cplus::Logger rootLogger = dcmtk::log4cplus::Logger::getRoot();
	rootLogger.removeAllAppenders();
	rootLogger.addAppender(stringlogger);

	if (me)
	{		
		me->DoSend(me->m_destination, me->m_threads);
		me->SetDone(true);
	}
}

void DICOMSenderImpl::DoSend(DestinationEntry destination, int threads)
{
	OFLog::configure(OFLogger::OFF_LOG_LEVEL);
	
	// get a list of files	
	patientdata.GetCheckedStudies(boost::bind(&DICOMSenderImpl::fillstudies, this, _1));
	
	// run 5 threads
	boost::thread_group threadgroup;
	for (int i = 0; i < threads; i++)
		threadgroup.create_thread(boost::bind(&DICOMSenderImpl::consumer, this));

	// wait for everything to finish, Cancel() calls service stop and stops farther work from processing
	threadgroup.join_all();
}

int DICOMSenderImpl::fillstudies(Study &study)
{
	queue.push_back(study.path);
	return 0;
}

void DICOMSenderImpl::consumer()
{
	boost::random::mt19937 rng(std::time(0));

	while (!IsCanceled())
	{
		try
		{
			boost::filesystem::path value;
		
			{
				boost::mutex::scoped_lock lk(queue_mutex);
				if (queue.size() >= 1)
				{
					boost::random::uniform_int_distribution<> dist(0, queue.size() - 1);
					int i = dist(rng);
					value = queue[i];
					queue.erase(queue.begin() + i);
				}
				else
				{
					return;
				}
			}

		
			SendStudy(value);
		}
		catch (...)
		{

		}
	}
}

void DICOMSenderImpl::SendStudy(boost::filesystem::path path)
{
	if (IsCanceled())
		return;
	// each instance of this function is a thread, don't write to class members!
	int retry = 0;
	int unsentcountbefore = 0;
	int unsentcountafter = 0;
	mapset sopclassuidtransfersyntax;
	naturalpathmap instances;	// sopid, filename, this ensures we send out instances in sopid order	
	std::string study_uid;

	// scan the directory for all instances in the study
	ScanDir(path, instances, sopclassuidtransfersyntax, study_uid);

	if (IsCanceled())
		return;

	int total = instances.size();

	do
	{
		// get number of unsent images
		unsentcountbefore = instances.size();

		// batch send
		if (unsentcountbefore > 0)
			SendABatch(sopclassuidtransfersyntax, instances);

		unsentcountafter = instances.size();
		
		// only do a sleep if there's more to send, we didn't send anything out, and we still want to retry
		if (unsentcountafter > 0 && unsentcountbefore == unsentcountafter && retry < 10000)
		{
			retry++;			

			// sleep loop with cancel check, 1 minutes
			int sleeploop = 5 * 60 * 1;
			while (sleeploop > 0)
			{
#ifdef _WIN32
				Sleep(200);
#else
                usleep(200000);
#endif
                sleeploop--;
				if (IsCanceled())
					break;
			}
		}
		else		// otherwise, the next loop is not a retry
		{
			retry = 0;
		}
	}
	while (!IsCanceled() && unsentcountafter > 0 && retry < 10000);

	if (unsentcountafter == 0 && total > 0)
	{
		patientdata.SetStudyCheck(study_uid, false);
	}
}

int DICOMSenderImpl::SendABatch(const mapset &sopclassuidtransfersyntax, naturalpathmap &instances)
{		
	DcmSCU scu;

	if (IsCanceled())
		return 1;

	scu.setVerbosePCMode(true);
	scu.setAETitle(m_destination.ourAETitle.c_str());
	scu.setPeerHostName(m_destination.destinationHost.c_str());
	scu.setPeerPort(m_destination.destinationPort);
	scu.setPeerAETitle(m_destination.destinationAETitle.c_str());
	scu.setACSETimeout(60);
	scu.setDIMSETimeout(120);
	scu.setDatasetConversionMode(true);

	OFList<OFString> defaulttransfersyntax;
	defaulttransfersyntax.push_back(UID_LittleEndianExplicitTransferSyntax);

	mapset sopclassuidtransfersyntax2 = sopclassuidtransfersyntax;
	// for every class..
	for (mapset::iterator it = sopclassuidtransfersyntax2.begin(); it != sopclassuidtransfersyntax2.end(); it++)
	{
		// let's do our own optimization, propose JPEG-LS
		it->second.insert(UID_JPEGLSLosslessTransferSyntax);

		// make list of what's in the file, and propose it first.  default proposed as a seperate context
		OFList<OFString> transfersyntax;
		for(std::set<std::string>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			if(*it2 != UID_LittleEndianExplicitTransferSyntax)
				transfersyntax.push_back(it2->c_str());
		}

		if(transfersyntax.size() > 0)
			scu.addPresentationContext(it->first.c_str(), transfersyntax);

		// propose the default UID_LittleEndianExplicitTransferSyntax
		scu.addPresentationContext(it->first.c_str(), defaulttransfersyntax);
	}
	
	OFCondition cond;
	
	if(scu.initNetwork().bad())
		return 1;

	if(scu.negotiateAssociation().bad())
		return 1;

	naturalpathmap::iterator itr = instances.begin();
	while(itr != instances.end())
	{
		if(IsCanceled())
		{			
			break;
		}

		Uint16 status;		

		// load file
		DcmFileFormat dcmff;
		dcmff.loadFile(itr->second.string().c_str());

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
			instances.erase(itr++);
		else if(cond == DUL_PEERABORTEDASSOCIATION)
			return 1;
		else			// some error? keep going
		{		
			itr++;
		}		
	}

	scu.releaseAssociation();	
	return 0;
}

void DICOMSenderImpl::ScanDir(boost::filesystem::path path, naturalpathmap &instances, mapset &sopclassuidtransfersyntax, std::string &study_uid)
{
	boost::filesystem::path someDir(path);
	boost::filesystem::directory_iterator end_iter;

	if (boost::filesystem::exists(someDir) && boost::filesystem::is_directory(someDir))
	{
		for (boost::filesystem::directory_iterator dir_iter(someDir); dir_iter != end_iter; dir_iter++)
		{
			if (IsCanceled())
			{
				break;
			}

			if (boost::filesystem::is_regular_file(dir_iter->status()))
			{
				ScanFile(*dir_iter, instances, sopclassuidtransfersyntax, study_uid);
			}
			else if (boost::filesystem::is_directory(dir_iter->status()))
			{
				// descent recursively
				ScanDir(*dir_iter, instances, sopclassuidtransfersyntax, study_uid);
			}
		}
	}
}

void DICOMSenderImpl::ScanFile(boost::filesystem::path path, naturalpathmap &instances, mapset &sopclassuidtransfersyntax, std::string &study_uid)
{
	DcmFileFormat dfile;
	OFCondition cond = dfile.loadFile(path.string().c_str());
	if (cond.good())
	{		
		OFString studyuid;		
		OFString sopuid, sopclassuid, transfersyntax;
		
		if (dfile.getDataset()->findAndGetOFString(DCM_StudyInstanceUID, studyuid).bad()) 
			return;
		if (dfile.getDataset()->findAndGetOFString(DCM_SOPInstanceUID, sopuid).bad())
			return;
		if (dfile.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopclassuid).bad())
			return;

		DcmXfer filexfer(dfile.getDataset()->getOriginalXfer());
		transfersyntax = filexfer.getXferID();
	
		// add file to send
		instances.insert(std::pair<std::string, boost::filesystem::path>(sopuid.c_str(), path));

		// remember the class and transfersyntax
		sopclassuidtransfersyntax[sopclassuid.c_str()].insert(transfersyntax.c_str());

		study_uid = studyuid.c_str();
	}

}

bool DICOMSenderImpl::Echo(DestinationEntry destination)
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

void DICOMSenderImpl::Cancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = true;
}

void DICOMSenderImpl::ClearCancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = false;
}

bool DICOMSenderImpl::IsDone()
{
	boost::mutex::scoped_lock lk(mutex);
	return doneEvent;
}

bool DICOMSenderImpl::IsCanceled()
{
	boost::mutex::scoped_lock lk(mutex);
	return cancelEvent;
}

void DICOMSenderImpl::SetDone(bool state)
{
	boost::mutex::scoped_lock lk(mutex);
	doneEvent = state;
}


DICOMSender::DICOMSender(PatientData &patientdata)
{
	impl = new DICOMSenderImpl(patientdata);
}

DICOMSender::~DICOMSender(void)
{
	delete impl;
}

void DICOMSender::DoSendAsync(DestinationEntry destination, int threads)
{
	impl->DoSendAsync(destination, threads);
}

void DICOMSender::DoSend(DestinationEntry destination, int threads)
{
	impl->DoSend(destination, threads);
}

bool DICOMSender::Echo(DestinationEntry destination)
{
	return DICOMSenderImpl::Echo(destination);
}

void DICOMSender::Cancel()
{
	impl->Cancel();
}

bool DICOMSender::IsDone()
{
	return impl->IsDone();
}
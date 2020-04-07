
#include "dicomscanner.h"

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

// check DCMTK functionality
#if !defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
//#error "DCMTK and this program must be compiled with DCMTK_WIDE_CHAR_FILE_IO_FUNCTIONS"
#endif

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif


DICOMFileScanner::DICOMFileScanner(PatientData &patientdata)
	: patientdata(patientdata)
{
	cancelEvent = doneEvent = false;
	m_scanPath = "";
	m_depth = 1;
}

DICOMFileScanner::~DICOMFileScanner()
{
}

bool DICOMFileScanner::ScanFile(const boost::filesystem::path &basepath, const boost::filesystem::path &path)
{
	DcmFileFormat dfile;
    OFCondition cond = dfile.loadFile(path.string().c_str());
	if (cond.good())
	{
		OFString patientname, patientid, birthday;
		OFString studyuid, modality, studydesc, studydate;
		OFString seriesuid, seriesdesc;
		OFString sopuid, sopclassuid, transfersyntax;

		dfile.getDataset()->convertToUTF8();

		dfile.getDataset()->findAndGetOFString(DCM_PatientName, patientname);
		if (patientname.size() == 0)
			return false;

		dfile.getDataset()->findAndGetOFString(DCM_PatientID, patientid);
		dfile.getDataset()->findAndGetOFString(DCM_PatientBirthDate, birthday);
		dfile.getDataset()->findAndGetOFString(DCM_StudyInstanceUID, studyuid);
		dfile.getDataset()->findAndGetOFString(DCM_Modality, modality);
		dfile.getDataset()->findAndGetOFString(DCM_StudyDescription, studydesc);
		dfile.getDataset()->findAndGetOFString(DCM_StudyDate, studydate);		
		if (DcmDate::checkStringValue(studydate, "1").bad())
			studydate = "";

		patientdata.AddStudy(studyuid.c_str(), patientid.c_str(), patientname.c_str(), studydesc.c_str(), studydate.c_str(), basepath);
		return true;
	}

	return false;
}

void DICOMFileScanner::DoScanAsync(const boost::filesystem::path &path, int depth)
{
	SetDone(false);
	ClearCancel();

	m_scanPath = path;
	m_depth = depth;
	boost::thread t(DICOMFileScanner::DoScanThread, this);
	t.detach();
}

void DICOMFileScanner::DoScanThread(void *obj)
{
	DICOMFileScanner *me = (DICOMFileScanner *)obj;
	if (me)
	{		
		me->DoScan(me->m_scanPath, me->m_depth);
		me->SetDone(true);	
	}
}

void DICOMFileScanner::DoScan(const boost::filesystem::path &path, int depth)
{
	OFLog::configure(OFLogger::OFF_LOG_LEVEL);

	// catch any access errors
	try
	{
		ScanStructureDir(path, depth);
	}
	catch (...)
	{

	}

}

void DICOMFileScanner::ScanStructureDir(const boost::filesystem::path &path, int depth)
{
	boost::filesystem::directory_iterator end_iter;

	if (IsCanceled())
		return;

	// if we are at the study depth the user told us about
	if (depth <= 0)
	{
		// no need to scan if the path already exists in the database
		if (!patientdata.PathExists(path))
		{
			// start finding a study to add
			ScanDir(path, path);
		}
		return;
	}

	if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
	{
		for (boost::filesystem::directory_iterator dir_iter(path); dir_iter != end_iter; dir_iter++)
		{
			if (IsCanceled())
			{
				break;
			}

			if (boost::filesystem::is_directory(dir_iter->status()))
			{
				ScanStructureDir(*dir_iter, depth - 1);
			}
		}
	}
}

bool DICOMFileScanner::ScanDir(const boost::filesystem::path &basepath, const boost::filesystem::path &path)
{
	boost::filesystem::directory_iterator end_iter;

	if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path))
	{
		for (boost::filesystem::directory_iterator dir_iter(path); dir_iter != end_iter; dir_iter++)
		{
			if (IsCanceled())
			{
				break;
			}

			if (boost::filesystem::is_regular_file(dir_iter->status()))
			{
				// scan, and if found a study, exit, since we assume all files in the directory are the same study
				if (ScanFile(basepath, *dir_iter))
					return true;
			}
			else if (boost::filesystem::is_directory(dir_iter->status()))
			{
				// descent recursively
				if (ScanDir(basepath, *dir_iter))
					return true;
			}
		}
	}

	return false;
}


void DICOMFileScanner::Cancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = true;
}

void DICOMFileScanner::ClearCancel()
{
	boost::mutex::scoped_lock lk(mutex);
	cancelEvent = false;
}

bool DICOMFileScanner::IsDone()
{
	boost::mutex::scoped_lock lk(mutex);
	return doneEvent;
}

bool DICOMFileScanner::IsCanceled()
{
	boost::mutex::scoped_lock lk(mutex);
	return cancelEvent;
}

void DICOMFileScanner::SetDone(bool state)
{
	boost::mutex::scoped_lock lk(mutex);
	doneEvent = state;
}

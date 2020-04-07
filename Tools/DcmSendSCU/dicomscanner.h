#ifndef _DICOMSCANNER_
#define _DICOMSCANNER_

#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include "patientdata.h"


class DICOMFileScanner
{
public:
	DICOMFileScanner(PatientData &patientdata);
	~DICOMFileScanner(void);
	
	void DoScanAsync(const boost::filesystem::path &path, int depth);
	void DoScan(const boost::filesystem::path &path, int depth);

	void Cancel();
	bool IsDone();

	PatientData &patientdata;
protected:
	static void DoScanThread(void *obj);
	bool ScanFile(const boost::filesystem::path &basepath, const boost::filesystem::path &path);
	bool ScanDir(const boost::filesystem::path &basepath, const boost::filesystem::path &path);
	void ScanStructureDir(const boost::filesystem::path &path, int depth);

	bool IsCanceled();
	void ClearCancel();
	void SetDone(bool state);

	// threading data
	boost::mutex mutex;
	bool cancelEvent, doneEvent;	
	boost::filesystem::path m_scanPath;
	int m_depth;
};
#endif
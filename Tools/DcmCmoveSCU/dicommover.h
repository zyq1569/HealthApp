#ifndef _DICOMMOVER_
#define _DICOMMOVER_

#include "alphanum.hpp"
#include <set>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio/io_service.hpp>
#include "patientdata.h"
#include "destinationentry.h"


class DICOMMover
{
public:
	DICOMMover(PatientData &patientdata);
	~DICOMMover(void);

	void DoMoveAsync(DestinationEntry destination, std::string moveae, int threads);
	void DoMove(DestinationEntry destination, std::string moveae, int threads);
	/*
	std::string ReadLog();
	void WriteLog(const char *msg);
	void WriteLog(std::string &msg);
	*/
	void Cancel();
	bool IsDone();
protected:
	static void DoMoveThread(void *obj);
	PatientData &patientdata;

	int MoveStudy(std::string studyuid);

	bool IsCanceled();
	void ClearCancel();
	void SetDone(bool state);

	// threading data
	boost::mutex mutex;
	bool cancelEvent, doneEvent;
	DestinationEntry m_destination;
	std::string m_moveae;
	int m_threads;

	boost::asio::io_service service;
	
	int fillstudies(Study &study);
	std::vector<std::string> studies;	
};

#endif
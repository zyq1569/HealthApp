#ifndef _ENGINE_
#define _ENGINE_

#include <vector>
#include "destinationentry.h"
#include "dicomscanner.h"
#include "dicomsender.h"
#include "patientdata.h"

class engine
{
public:
	engine();
	~engine();

	std::vector<DestinationEntry> destinations, globalDestinations;
	void LoadDestinationList();
	void LoadGlobalDestinationList();
	void SaveDestinationList();
	void StartScan(const boost::filesystem::path &path, int depth);
	void StopScan();
	void StartSend(int destination, int threads);
	void StopSend();

	DICOMFileScanner scanner;
	DICOMSender sender;
	PatientData patientdata;
};

#endif
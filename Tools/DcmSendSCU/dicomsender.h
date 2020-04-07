#ifndef _DICOMSENDER_
#define _DICOMSENDER_

#include "patientdata.h"
#include "destinationentry.h"

class DICOMSenderImpl;
class DICOMSender
{

public:
	DICOMSender(PatientData &patientdata);
	~DICOMSender(void);

	void DoSendAsync(DestinationEntry destination, int threads);
	void DoSend(DestinationEntry destination, int threads);

	static bool Echo(DestinationEntry destination);

	void Cancel();
	bool IsDone();
protected:
	DICOMSenderImpl *impl;
};


#endif
#ifndef _DICOMQUERY_
#define _DICOMQUERY_

#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include "patientdata.h"
#include "destinationentry.h"

class DICOMQueryScanner
{
public:
    DICOMQueryScanner(PatientData &patientdata);
    ~DICOMQueryScanner(void);
    
    void DoQueryAsync(DestinationEntry &destination);
    void DoQuery(DestinationEntry &destination);

    static bool Echo(DestinationEntry destination);

    void Cancel();
    bool IsDone();

    PatientData &patientdata;
protected:
    static void DoQueryThread(void *obj);    
    bool ScanPatientName(std::string name, DestinationEntry &destination);

    bool IsCanceled();
    void ClearCancel();
    void SetDone(bool state);

    // threading data
    boost::mutex mutex;
    bool cancelEvent, doneEvent;    
    
    DestinationEntry m_destination;    
};
#endif
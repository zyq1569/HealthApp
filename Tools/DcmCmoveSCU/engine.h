#ifndef _ENGINE_
#define _ENGINE_

#include <vector>
#include "sqlite3.h"
#include "destinationentry.h"
#include "dicomquery.h"
#include "dicommover.h"
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
    void StartQuery(int destination);
    void StopQuery();
    void StartMove(int destination, std::string moveae, int threads);
    void StopMove();

    DICOMQueryScanner query;
    DICOMMover mover;
    PatientData patientdata;
};

#endif
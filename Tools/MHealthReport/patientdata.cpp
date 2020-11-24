#include "patientdata.h"

PatientData * PatientData::s_instance = NULL;


PatientData *PatientData::getInstance()
{
    {
        if(NULL == s_instance)
        {
            s_instance = new PatientData();
        }

        return s_instance;
    }
}


PatientData::PatientData()
{

}


bool PatientData::dataCnn()
{

    return true;
}


bool PatientData::insertData(PatientStudyData data)
{

    return true;
}


int PatientData::selectData(QString regId, QString patientID)
{
    int count = 0;



    return count;
}


bool PatientData::updateData(PatientStudyData data)
{
    bool success = true;

    return success;
}


bool PatientData::deleteData(QString regId, QString patientID)
{
    bool success = true;

    return success;
}


PatientStudyData PatientData::getDataPatient() const
{
    return pStudyData;
}




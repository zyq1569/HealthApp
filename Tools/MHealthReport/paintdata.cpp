#include "paintdata.h"

PaintData * PaintData::s_instance = NULL;


PaintData *PaintData::getInstance()
{
    {
        if(NULL == s_instance)
        {
            s_instance = new PaintData();
        }

        return s_instance;
    }
}


PaintData::PaintData()
{

}


bool PaintData::dataCnn()
{

    return true;
}


bool PaintData::insertData(PatientStudyData data)
{

    return true;
}


int PaintData::selectData(QString regId, QString patientID)
{
    int count = 0;



    return count;
}


bool PaintData::updateData(PatientStudyData data)
{
    bool success = true;

    return success;
}


bool PaintData::deleteData(QString regId, QString patientID)
{
    bool success = true;

    return success;
}


PatientStudyData PaintData::getDataPaint() const
{
    return pStudyData;
}




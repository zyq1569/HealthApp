#include "paintdata.h"

PaintData * PaintData::instance = NULL;


PaintData *PaintData::getInstance()
{
    {
        if(NULL == instance)
        {
            instance = new PaintData();
        }

        return instance;
    }
}


PaintData::PaintData()
{

}


bool PaintData::dataCnn()
{

    return true;
}


bool PaintData::insertData(DataPaint data)
{

    return true;
}


int PaintData::selectData(QString regId, QString patientID)
{
    int count = 0;



    return count;
}


bool PaintData::updateData(DataPaint data)
{
    bool success = true;

    return success;
}


bool PaintData::deleteData(QString regId, QString patientID)
{
    bool success = true;

    return success;
}


DataPaint PaintData::getDataPaint() const
{
    return dataPaint;
}




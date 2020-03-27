#pragma once
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/fileap.h"

DCMTK_OFSTD_EXPORT struct OFHashValue
{

#ifdef _WIN32
    INT16 first;
    INT16 second;
#else
    int16_t first;
    int16_t second;
#endif

    OFHashValue()
    {
        first = second = 0;
    }
};



/*
OFListIterator(OFFilename) iter = filenameList.begin();
OFListIterator(OFFilename) last = filenameList.end();
while (iter != last)
{
inputFiles.push_back(OFSTRING_GUARD((*iter).getCharPointer()));
++iter;
}
*/
DCMTK_OFSTD_EXPORT struct DicomFileInfo
{
    OFString patientName;
    OFString patientSex;
    OFString patientAge;
    OFString patientBirthDate;
    OFString patientBirthTime;
    OFString patientId;

    OFString modality;
    OFString manufacturer;
    OFString institutionName;

    OFString studyId;
    OFString studyUID;
    OFString studyDescription;
    OFString studyDate;
    OFString studyTime;

    OFString seriesUID;
    OFString seriesDescription;
    OFString seriesNumber;

    OFString imageSOPInstanceUID;
    OFString instanceNumber;
};
DCMTK_OFSTD_EXPORT struct ImageInfo
{
    //(0008,0018) UI 1.2.840.113619.2.55.3.604688119.699.1256270047.398.2   # 1, 52 SOP Instance UID
    OFString imageSOPInstanceUID;
    // (0020, 0013) IS 2                                                 # 1, 2 Instance Number;
    int instanceNumber;
};
DCMTK_OFSTD_EXPORT struct SeriesInfo
{
    OFList<ImageInfo> imagesInfoList;
    OFString seriesUID;
    OFString seriesDescription;
    //Series Number
    int seriesNumber;
};
DCMTK_OFSTD_EXPORT struct StudyInfo
{
    OFString patientName;
    OFString patientId;
    OFString modality;
    OFString studyId;
    OFString studyUID;
    OFString studyDate;
    OFString studyDescription;
    OFList<SeriesInfo> seriesInfoList;
};
//!根据字符计算两个Hash数值
DCMTK_OFSTD_EXPORT OFHashValue CreateHashValue(const char * buffer, unsigned int length);
DCMTK_OFSTD_EXPORT OFString GetStudyHashDir(OFString studyuid);
DCMTK_OFSTD_EXPORT unsigned long studyuid_hash(const char *str);

DCMTK_OFSTD_EXPORT OFString longToString(unsigned long i);

//验证目录是否存在
DCMTK_OFSTD_EXPORT bool DirectoryExists(const OFString Dir);

//在全路径文件名中提取文件路径
DCMTK_OFSTD_EXPORT OFString ExtractFileDir(const OFString FileName);

//创建一个新目录
DCMTK_OFSTD_EXPORT bool ForceDirectories(const OFString Dir);

DCMTK_OFSTD_EXPORT OFBool CreatDir(OFString dir);

DCMTK_OFSTD_EXPORT OFString GetCurrWorkingDir();

DCMTK_OFSTD_EXPORT OFString AdjustDir(const OFString dir);

DCMTK_OFSTD_EXPORT OFString ToSearchDateTimeFormate(OFString datetime, OFString &StartDateTime, OFString &EndDateTime);

DCMTK_OFSTD_EXPORT OFString ToDateTimeFormate(OFString datetime, OFString &date, OFString &time);

DCMTK_OFSTD_EXPORT OFString ToDateFormate(OFString date);

DCMTK_OFSTD_EXPORT OFString DbDateTimeToDateTimeFormate(OFString datetime, OFString &date, OFString &time);

DCMTK_OFSTD_EXPORT void SearchDirFile(const OFString Dir, const OFString FileExt, OFList<OFString> &datas, const bool Not = false, const int Count = 200);

DCMTK_OFSTD_EXPORT void SearchDirectory(const OFString Dir, OFList<OFString> &datas);

#ifndef _WIN32
DCMTK_OFSTD_EXPORT uint64_t CreateGUID();
#else
DCMTK_OFSTD_EXPORT UINT64 CreateGUID();
#endif

DCMTK_OFSTD_EXPORT OFString StringGUID();

DCMTK_OFSTD_EXPORT OFString GetCurrentDir();

DCMTK_OFSTD_EXPORT OFString FormatePatienName(OFString name);

DCMTK_OFSTD_EXPORT OFString GetFromFile(OFString filename);

DCMTK_OFSTD_EXPORT OFBool SaveStudy2JsonFile(StudyInfo study, OFString filename);
DCMTK_OFSTD_EXPORT OFBool SaveString2File(OFString str, OFString filename);

DCMTK_OFSTD_EXPORT OFBool ReadStudyInfo(OFString filename, OFString dir, OFList<OFString> &data);
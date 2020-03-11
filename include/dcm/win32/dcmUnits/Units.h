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
//___________________________________________________________________________________________________________________________________
//struct OFHashValue
//{
//    INT16 first;
//    INT16 second;
//    OFHashValue()
//    {
//        first = second = 0;
//    }
//};
//!根据字符计算两个Hash数值
//OFHashValue CreateHashValue(const char * buffer, unsigned int length)
//{
//    //2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
//    //101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
//    //181 191 193 197 199
//    static const int M1 = 71;
//    static const int M2 = 79;
//
//    static const int D1 = 197;
//    static const int D2 = 199;
//    int value = 0;
//    OFHashValue hashvalue;
//    for (unsigned int i = 0; i < length; i++)
//    {
//        value = value * M1 + buffer[i];
//    }
//    value %= D1;
//    if (value < 0)
//    {
//        value = value + D1;
//    }
//    hashvalue.first = value;
//
//    value = 0;
//    for (unsigned int i = 0; i < length; i++)
//    {
//        value = value * M2 + buffer[i];
//    }
//    value %= D2;
//    if (value < 0)
//    {
//        value = value + D2;
//    }
//    hashvalue.second = value;
//    return hashvalue;
//}
//unsigned long studyuid_hash(const char *str)
//{
//    unsigned long hash = 32767;
//    unsigned long pid = 65537;
//    int c;
//    while (c = *str++)
//    {
//        hash = hash + c *pid; /* hash * 33 + c */
//    }
//    return hash;
//}
//
//static OFString longToString(unsigned long i)
//{
//    char numbuf[10240];
//    sprintf(numbuf, "%d", i);
//    return numbuf;
//}
//
////验证目录是否存在。
//bool DirectoryExists(const OFString Dir)
//{
//    if (Dir == "")
//        return false;
//    int Code = GetFileAttributes(Dir.c_str());
//    return (Code != -1) && ((FILE_ATTRIBUTE_DIRECTORY & Code) != 0);
//}
////在全路径文件名中提取文件路径。
//OFString ExtractFileDir(const OFString FileName)
//{
//    OFString tempstr;
//    int pos = FileName.find_last_of('/');
//    if (pos == -1)
//    {
//        return "";
//    }
//    tempstr = FileName.substr(0, pos);
//    return tempstr;
//    OFString str = OFStandard::getDirNameFromPath(tempstr, FileName);
//    return OFStandard::getDirNameFromPath(tempstr, FileName);
//}
////创建一个新目录
//bool ForceDirectories(const OFString Dir)
//{
//    OFString path = ExtractFileDir(Dir);
//    if (!OFStandard::dirExists(path))
//    {
//        int len = path.size();
//        if (len > 1)
//        {
//            char  temp = path[path.size() - 1];
//            if (path[path.size() - 1] != ':')
//                ForceDirectories(path);
//        }
//    }
//    if (DirectoryExists(Dir) != true)
//    {
//        //if (::CreateDirectory(Dir.c_str(), NULL) == 0)
//#ifdef HAVE_WINDOWS_H
//        if (_mkdir(Dir.c_str()) == -1)
//#else
//        if (mkdir(Dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
//#endif
//        {
//            OFString msg;
//            msg = "CreateDirectory() failed with error %s,(%s)" + GetLastError() + Dir;
//            OFLOG_ERROR(storescpLogger, msg);
//            return false;
//        }
//    }
//    return true;
//}
//
//OFBool CreatDir(OFString dir)
//{
//    return ForceDirectories(dir);
//    if (!OFStandard::dirExists(dir))
//    {
//#ifdef HAVE_WINDOWS_H
//        if (_mkdir(dir.c_str()) == -1)
//#else
//        if (mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
//#endif
//        {
//            OFLOG_ERROR(storescpLogger, "mkdir :" + dir + "  .error!");
//            return OFFalse;
//        }
//        else
//        {
//            return OFTrue;
//        }
//    }
//    return OFTrue;
//}
//OFString GetCurrWorkingDir()
//{
//    OFString strPath;
//#ifdef HAVE_WINDOWS_H
//    TCHAR szFull[_MAX_PATH];
//    //TCHAR szDrive[_MAX_DRIVE];
//    //TCHAR szDir[_MAX_DIR];
//    ::GetModuleFileName(NULL, szFull, sizeof(szFull) / sizeof(TCHAR));
//    strPath = OFString(szFull);
//#else
//    //to do add!
//#endif
//    return strPath;
//}

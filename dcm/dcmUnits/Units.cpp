#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
//-------統一使用dcmtklog方式
//#include "dcmtk/oflog/fileap.h"
#include "dcmtk/oflog/oflog.h"
//--------------------
#include <iostream>
#include <string>
#include <vector>
#include <map>

//--------------------
//***************$$$$$$$$$*************************
#ifdef HAVE_WINDOWS_H
#include <direct.h>      /* for _mkdir() */
#include <objbase.h>   //CoCreateGuid()
#endif

#include "dcmtk/oflog/fileap.h"
#include "cJSON.h"
#include "Units.h"
#include "dcmtk/ofstd/ofdatime.h"

#ifndef HAVE_WINDOWS_H
#include <uuid/uuid.h>
#endif
 // _WIN32

//#ifndef _WIN32
//typedef uint64_t UINT64
//#endif // _WIN32

OFString GetStudyHashDir(OFString studyuid)
{
    OFString dir;
    OFHashValue vl = CreateHashValue(studyuid.c_str(), studyuid.length());
    dir = "/" + longToString(vl.first) + "/" + longToString(vl.second);
    return dir;
}
//!根据字符计算两个Hash数值  to do user uint64
OFHashValue CreateHashValue(const char * buffer, unsigned int length)
{
//2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
//101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
//181 191 193 197 199
    static const int M1 = 71;
    static const int M2 = 79;

    static const int D1 = 197;
    static const int D2 = 199;
    int value = 0;
    OFHashValue hashvalue;
    for (unsigned int i = 0; i < length; i++)
    {
        value = value * M1 + buffer[i];
    }
    value %= D1;
    if (value < 0)
    {
        value = value + D1;
    }
    hashvalue.first = value;

    value = 0;
    for (unsigned int i = 0; i < length; i++)
    {
        value = value * M2 + buffer[i];
    }
    value %= D2;
    if (value < 0)
    {
        value = value + D2;
    }
    hashvalue.second = value;
    return hashvalue;
}

unsigned long studyuid_hash(const char *str)
{
    unsigned long hash = 32767;
    unsigned long pid = 65537;
    int c;
    while (c = *str++)
    {
        hash = hash + c *pid; /* hash * 33 + c */
    }
    return hash;
}

OFString longToString(unsigned long i)
{
    char numbuf[10240];
    sprintf(numbuf, "%d", i);
    return numbuf;
}

//验证目录是否存在。
bool DirectoryExists(const OFString Dir)
{
    if (Dir == "")
        return false;
    int Code = -3;
#ifdef _WIN32
    Code = GetFileAttributes(Dir.c_str());
    //int Code = GetFileAttributes(Dir.c_str());
    return (Code != -1) && ((FILE_ATTRIBUTE_DIRECTORY & Code) != 0);
#else
    struct stat statbuff;
    Code = stat(Dir.c_str(), &statbuff);
    if(Code == 0)
        return true;
    else
        return false;
#endif // _WIN32
}

//在全路径文件名中提取文件路径。
OFString ExtractFileDir(const OFString FileName)
{
    OFString tempstr;
    int pos = FileName.find_last_of('/');
    if (pos == -1)
    {
        return "";
    }
    tempstr = FileName.substr(0, pos);
    return tempstr;
    OFString str = OFStandard::getDirNameFromPath(tempstr, FileName);
    return OFStandard::getDirNameFromPath(tempstr, FileName);
}

//创建一个新目录
bool ForceDirectories(const OFString Dir)
{
    OFString path = ExtractFileDir(Dir);
    if (!OFStandard::dirExists(path))
    {
        int len = path.size();
        if (len > 1)
        {
            char  temp = path[path.size() - 1];
            if (path[path.size() - 1] != ':')
                ForceDirectories(path);
        }
    }
    if (DirectoryExists(Dir) != true)
    {
        //if (::CreateDirectory(Dir.c_str(), NULL) == 0)
#ifdef HAVE_WINDOWS_H
        if (_mkdir(Dir.c_str()) == -1)
        {
            OFString msg;
            msg = "CreateDirectory() failed with error %s,(%s)" + GetLastError() + Dir;
            //OFLOG_ERROR(storescpLogger, msg);
            return false;
        }
#else
        if (mkdir(Dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
        {
            OFString msg;
            msg = "CreateDirectory() failed with error %s,(%s)" + Dir;
            //OFLOG_ERROR(storescpLogger, msg);
            return false;
        }
#endif

    }
    return true;
}

OFBool CreatDir(OFString dir)
{
    return ForceDirectories(dir);
    if (!OFStandard::dirExists(dir))
    {
#ifdef HAVE_WINDOWS_H
        if (_mkdir(dir.c_str()) == -1)
#else
        if (mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
#endif
        {
            //OFLOG_ERROR(storescpLogger, "mkdir :" + dir + "  .error!");
            return OFFalse;
        }
        else
        {
            return OFTrue;
        }
    }
    return OFTrue;
}

OFString GetCurrWorkingDir()
{
    OFString strPath;
#ifdef HAVE_WINDOWS_H
    TCHAR szFull[_MAX_PATH];
    //TCHAR szDrive[_MAX_DRIVE];
    //TCHAR szDir[_MAX_DIR];
    ::GetModuleFileName(NULL, szFull, sizeof(szFull) / sizeof(TCHAR));
    strPath = OFString(szFull);
#else
    //to do add!
#endif
    return strPath;
}

//!根据字符计算两个Hash数值
OFString AdjustDir(const OFString dir)
{
#ifdef HAVE_WINDOWS_H
    OFString path = dir;
    if (path == "")
        return "";
    if (path[path.length() - 1] != '\\'&& path[path.length() - 1] != '/')
        path += "\\";
    return path;
#else
    //to do add!

#endif
}
//-----------------------------------------------------------------------------------
//查找Dir目录下的扩展名为FileExt的内容，包含所有子级目录，如果FileExt为空表示查找所有文件
void SearchDirFile(const OFString Dir, const OFString FileExt, OFList<OFString> &datas, const bool Not, const int Count)
{
    if (datas.size() >= Count)
    {
        return;
    }
#ifdef HAVE_WINDOWS_H
    OFString dir = AdjustDir(Dir);
    if (dir == "")
        return;
    OFString pach = dir + "*.*";
    WIN32_FIND_DATA sr;
    HANDLE h;
    if ((h = FindFirstFile(pach.c_str(), &sr)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            OFString name = sr.cFileName;
            if ((sr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
                if (name != "." && name != "..")
                {
                    SearchDirFile(dir + name, FileExt, datas);
                }
            }
            else
            {
                OFString temp;
                if (FileExt == "")
                    datas.push_back(dir + name);
                else if (Not == false && OFStandard::toUpper(temp, FileExt) == OFStandard::toUpper(temp, name))
                    datas.push_back(dir + name);
                else if (Not == true && OFStandard::toUpper(temp, FileExt) != OFStandard::toUpper(temp, name))
                    datas.push_back(dir + name);
            }
        }
        while (FindNextFile(h, &sr) != 0);
        FindClose(h);
    }
#else
    //to do add!

#endif
}
OFString ToDateFormate(OFString date)
{
    OFString str(date);
    if (str.length() < 1)
    {
        str.clear();
        return str;
    }
    int pos = str.find('-');
    while (pos > -1)
    {
        int len = str.length();
        OFString temp = str.substr(0, pos);
        temp = temp + str.substr(pos + 1, len - pos);
        str = temp;
        pos = str.find('-');
    }
    return str;
}


OFString DbDateTimeToDateTimeFormate(OFString datetime, OFString &date, OFString &time)
{
    OFString str(datetime);
    if (str.length() < 1)
    {
        str.clear();
        return str;
    }
    int pos = str.find('-');
    while (pos > -1)
    {
        int len = str.length();
        OFString temp = str.substr(0, pos);
        temp = temp + str.substr(pos + 1, len - pos);
        str = temp;
        pos = str.find('-');
    }
    pos = str.find(' ');
    while (pos > 0)
    {
        int len = str.length();
        OFString temp = str.substr(0, pos);
        temp = temp + str.substr(pos + 1, len - pos);
        str = temp;
        pos = str.find(' ');
    }

    date = str.substr(0, 8);
    if ((str.length() - 8) > 5)
    {
        time = str.substr(8, str.length() - 8);
    }
    else
    {
        time.clear();
    }
    return str;
}
OFString ToDateTimeFormate(OFString datetime, OFString &date, OFString &time)
{
    OFString str(datetime);
    if (str.length() < 1)
    {
        str.clear();
        return str;
    }
    int pos = str.find('-');
    while (pos > -1)
    {
        int len = str.length();
        OFString temp = str.substr(0, pos);
        temp = temp + str.substr(pos + 1, len - pos);
        str = temp;
        pos = str.find('-');
    }
    pos = str.find(' ');
    while (pos > 0)
    {
        int len = str.length();
        OFString temp = str.substr(0, pos);
        temp = temp + str.substr(pos + 1, len - pos);
        str = temp;
        pos = str.find(' ');
    }
    pos = str.find(':');
    while (pos > 0)
    {
        int len = str.length();
        OFString temp = str.substr(0, pos);
        temp = temp + str.substr(pos + 1, len - pos);
        str = temp;
        pos = str.find(':');
    }
    date = str.substr(0, 8);
    if ((str.length() - 8) > 5)
    {
        time = str.substr(8, str.length() - 8);
        //if (time.length() > 10)
        //{
        //    time = time.substr(0, 10);
        //}
    }
    else
    {
        time.clear();
    }

    return str;
}

OFString ToSearchDateTimeFormate(OFString datetime, OFString &StartDateTime, OFString &EndDateTime)
{
    OFString str(datetime);
    if (str.length() < 1)
    {
        str.clear();
        return str;
    }
    int pos = str.find('-');
    if (pos < 1)
    {
        StartDateTime = str;
        EndDateTime.clear();
        return str;
    }
    int len = str.length();
    OFString temp = str.substr(0, pos);
    StartDateTime = temp;
    temp = str.substr(pos + 1, len - pos);
    EndDateTime = temp;

    return (StartDateTime + EndDateTime);
}

void SearchDirectory(const OFString Dir, OFList<OFString> &datas)
{
    OFString dir;
#ifdef HAVE_WINDOWS_H
    dir = AdjustDir(Dir);
    if (dir == "")
        return;
    OFString pach = dir + "*.*";
    WIN32_FIND_DATA sr;
    HANDLE h;
    if ((h = FindFirstFile(pach.c_str(), &sr)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            OFString name = sr.cFileName;
            if ((sr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
                if (name != "." && name != "..")
                {
                    SearchDirectory(dir + name, datas);
                    datas.push_back(dir + name);
                }
            }
        }
        while (FindNextFile(h, &sr) != 0);
        FindClose(h);
    }
#else
    //to do add!
#endif
}
OFString StringGUID()
{
    char uuid[64];
    sprintf(uuid, "%llu", CreateGUID());
    OFString s = uuid;
    return s;
}
#ifdef _WIN32
UINT64 CreateGUID()
#else
uint64_t CreateGUID()
#endif // _WIN32
{
#ifdef _WIN32
    UINT32 uid[2];
#else
    uint32_t uid[2];
#endif // _WIN32
    // get the current time (needed for subdirectory name)
    OFDateTime dateTime;
    dateTime.setCurrentDateTime();

    // create a name for the new subdirectory.
    char timestamp[32];
    sprintf(timestamp, "%04u%02u%02u%02u%02u%02u%03u",
            dateTime.getDate().getYear(), dateTime.getDate().getMonth(),
            dateTime.getDate().getDay(), dateTime.getTime().getHour(),
            dateTime.getTime().getMinute(), dateTime.getTime().getIntSecond(),
            dateTime.getTime().getMilliSecond());
    //2019 0424 0951 14 708
    //2005 0000 0000 00 000
    #ifdef _WIN32
    UINT64 delt_y,delt_M,delt_d,delt_h,delt_m,delt_s,delt_ma,delt_mi;
    #else
    uint64_t delt_y,delt_M,delt_d,delt_h,delt_m,delt_s,delt_ma,delt_mi;
    #endif // _WIN32
     delt_y = (dateTime.getDate().getYear() - 2015) * 365;
     delt_M = dateTime.getDate().getMonth() * 30;
     delt_d = dateTime.getDate().getDay() * 24 * 60 * 60 * 1000;
     delt_h = dateTime.getTime().getHour() * 60 * 60 * 1000;
     delt_m = dateTime.getTime().getMinute() * 60 * 1000;
     delt_s = dateTime.getTime().getIntSecond() * 1000;
     delt_ma = delt_y + delt_M;
     delt_mi = delt_d + delt_h + delt_m + delt_s + dateTime.getTime().getMilliSecond();
    uid[1] = delt_ma << 11;
    uid[1] += delt_mi;
#ifdef HAVE_WINDOWS_H
    GUID guid;
    HRESULT result = NULL;
    do
    {
        result = CoCreateGuid(&guid);
        uid[0] = guid.Data1;
    }
    while (result != S_OK);

    return *((UINT64*)uid);
#else
    //to do add! install uuid-dev
    //uuid_generate(reinterpret_cast<unsigned char *>(&guid));
    //uid[0] = guid.Data1;
    return *((uint64_t*)uid);
#endif
    //return *((UINT64*)uid);
}

OFString GetCurrentDir()
{
    static OFString CurrentDir = "";
    if (CurrentDir != "")
    {
        return CurrentDir;
    }
    OFString tempstr, path;
#if HAVE_WINDOWS_H
    char szFullPath[MAX_PATH];
    ZeroMemory(szFullPath, MAX_PATH);
    ::GetModuleFileName(NULL, szFullPath, MAX_PATH);
    path = szFullPath;
#endif
    CurrentDir = OFStandard::getDirNameFromPath(tempstr, path);
    return CurrentDir;
}

OFString FormatePatienName(OFString name)
{
    OFString str(name);
    int pos = name.find('^');
    while (pos > -1)
    {
        int len = str.length();
        OFString temp = str.substr(0, pos);
        temp = temp + str.substr(pos + 1, len - pos);
        str = temp;
        pos = str.find('^');
    }
    return str;
}
OFString GetFromFile(OFString filename)
{
    OFString value;
    if (OFStandard::fileExists(filename))
    {
        using namespace std;
        char buffer[256];
        fstream out;
        out.open(filename.c_str(), ios::in);
        //cout<<"com.txt"<<" 的内容如下:"<<endl;
        while (!out.eof())
        {
            out.getline(buffer, 256, '\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
            value += OFString(buffer);
        }
        out.close();
    }
    return value;
}

OFBool SaveStudy2JsonFile(StudyInfo studyinfo, OFString filename)
{
    //cJSON *study = cJSON_CreateObject();
    ////studyinfo
    //cJSON_AddStringToObject(study, "patientName", studyinfo.patientName.c_str());
    //cJSON_AddStringToObject(study, "patientId", studyinfo.patientId.c_str());
    //cJSON_AddStringToObject(study, "studyDate", studyinfo.studyDate.c_str());
    //cJSON_AddStringToObject(study, "modality", studyinfo.modality.c_str());
    //cJSON_AddStringToObject(study, "studyDescription", studyinfo.studyDescription.c_str());
    //cJSON_AddNumberToObject(study, "numImages", studyinfo.seriesInfoList.size());
    //cJSON_AddStringToObject(study, "studyId", studyinfo.studyId.c_str());
    //cJSON_AddStringToObject(study, "studyuid", studyinfo.studyUID.c_str());
    ////seriesinfo

    //cJSON *seriesListItem = cJSON_CreateArray();
    //const char *seriesList = "seriesList";
    //cJSON_AddItemToObject(study, seriesList, seriesListItem);
    //OFListIterator(SeriesInfo) iter = studyinfo.seriesInfoList.begin();
    //OFListIterator(SeriesInfo) last = studyinfo.seriesInfoList.end();
    //while (iter != last)
    //{
    //    cJSON *series = cJSON_CreateObject();
    //    cJSON_AddItemToObject(seriesListItem, seriesList, series);
    //    cJSON_AddStringToObject(series, "seriesUid", (*iter).seriesUID.c_str());
    //    cJSON_AddStringToObject(series, "seriesDescription", (*iter).seriesDescription.c_str());
    //    cJSON_AddStringToObject(series, "seriesNumber", longToString((*iter).seriesNumber).c_str());

    //    OFListIterator(ImageInfo) ibegin = (*iter).imagesInfoList.begin();
    //    OFListIterator(ImageInfo) lend = (*iter).imagesInfoList.end();
    //    cJSON *images = cJSON_CreateArray();
    //    cJSON_AddItemToObject(series, "instanceList", images);
    //    while (ibegin != lend)
    //    {
    //        cJSON *imageid = cJSON_CreateObject();
    //        cJSON_AddStringToObject(imageid, "imageId", (*ibegin).imageSOPInstanceUID.c_str());
    //        cJSON_AddItemToObject(images, "instanceList", imageid);
    //        ibegin++;
    //    }
    //    ++iter;
    //}
    //char *p = cJSON_Print(study);
    //SaveString2File(p, filename);
    //cJSON_Delete(study);

    return OFTrue;
}
OFBool SaveString2File(OFString str, OFString filename)
{
    if (str.length() > 0)
    {
        //using namespace std;
        //ofstream savedcmfile;
        //char *pfilename = (char *)filename.c_str();
        //savedcmfile.open(filename.c_str(), ios::out | ios::app); //ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建
        //savedcmfile << str.c_str();
        //savedcmfile.close();//关闭文件
        OFFile inifile;
        inifile.fopen(filename, "w");
        inifile.fputs(str.c_str());
        inifile.fclose();
    }
    return OFTrue;
}

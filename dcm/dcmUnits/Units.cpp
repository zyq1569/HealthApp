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
#include "CJsonObject.hpp"

#ifndef HAVE_WINDOWS_H
#include <uuid/uuid.h>
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID, UUID;
#endif
// _WIN32

//#ifndef _WIN32
//typedef uint64_t UINT64
//#endif // _WIN32

OFString GetStudyHashDir(DicomFileInfo studyuid)
{
    OFString dir;
    OFHashValue vl = CreateHashValue(studyuid);
    dir = "/" + vl.first + "/" + vl.second;
    return dir;
}

OFString GetStudyDateDir(DicomFileInfo dcminfo)
{
    OFString dir;
    OFString studyDate = dcminfo.studyDate;
    OFString studyTime = dcminfo.studyTime;
    //OFHashValue vl = CreateHashValue(studyuid);
    dir = studyDate.substr(0, 4) + "/" + studyDate.substr(4, 2) + "/" + studyDate.substr(6, 2) + "/" + studyTime.substr(0, 2) + "/" + studyTime.substr(2, 2);
    //dir = "/" + vl.first + "/" + vl.second;
    return dir;
}

OFString HashValue(int F, int S, OFString str)
{
    size_t len = str.length();
    size_t value = F + S;
    for (size_t i = 0; i < len; i++)
    {
        value = (value + str[i])*F + i * S;
    }

#if defined(_WIN32)

#if defined(_MSC_VER)
    // Windows:  Visual Statuio 
    char oxstr[256] = { 0 };
    sprintf(oxstr, "%04x", value);
    return oxstr;
#endif

#if defined(__GNUC__)
    // Windows平台 GCC编译器特定的代码
//    QString hex = QString("%1").arg(value, 4, 16, QLatinlChar('0')); ///// 保留四位，不足补零
//    return QString.toLatin1().data();
#endif

#else
    //---------? now linux QT
    QString hex = QString("%1").arg(value, 4, 16, QLatinlChar('0'));
    return QString.toLatin1().data();
#endif

    return longToString(value);
}

///依据常规的hash 算法方式，目前不考虑使用强加密不可逆方式
//参考 https://github.com/Tessil/hopscotch-map 类似很多，注意协议
//!根据字符计算两个Hash数值  to do user uint64
OFHashValue CreateHashValue(DicomFileInfo dcminfo)
{
    //2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
    //101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
    //181 191 193 197 199
    OFString str = dcminfo.studyUID;
    OFHashValue hashvalue;
    //71 197
    hashvalue.first = HashValue(71, 197, str);
    //79 199
    hashvalue.second = HashValue(79, 199, str);

    return hashvalue;
}

unsigned long studyuid_hash(const char *str)
{
    unsigned long hash = 32767;
    unsigned long pid = 65537;
    int c;
    while (c = *str++)
    {
        hash = hash + c * pid; /* hash * 33 + c */
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
    {
        return false;
    }
    int Code = -3;
#ifdef _WIN32
    Code = GetFileAttributes(Dir.c_str());
    //int Code = GetFileAttributes(Dir.c_str());
    return (Code != -1) && ((FILE_ATTRIBUTE_DIRECTORY & Code) != 0);
#else
    struct stat statbuff;
    Code = stat(Dir.c_str(), &statbuff);
    if (Code == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
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
            {
                ForceDirectories(path);
            }
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
    if ((strPath = getcwd(NULL, 0)) == "")
    {
        perror("getcwd error");
    }
#endif
    return strPath;
}

//!判断目录是否为空
OFString CheckDirPath(const OFString dir)
{
#ifdef HAVE_WINDOWS_H
    OFString path = dir;
    if (path == "")
    {
        return "";
    }
    if (path[path.length() - 1] != '\\'&& path[path.length() - 1] != '/')
    {
        path += "\\";
    }
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
    OFString dir = CheckDirPath(Dir);
    if (dir == "")
    {
        return;
    }
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
                OFString findExt;
                int pos = name.find_last_of('.');
                if (pos > 0)
                {
                    findExt = name.substr(pos+1);
                }
                OFString temp,up1,up2;
                up1 = OFStandard::toUpper(temp, FileExt);
                up2 = OFStandard::toUpper(temp, findExt);
                if (FileExt == "")
                {
                    datas.push_back(dir + name);
                }
                else if (Not == false && up1 == up2)
                {
                    datas.push_back(dir + name);
                }
                else if (Not == true && up1 != up2)
                {
                    datas.push_back(dir + name);
                }
            }
        } while (FindNextFile(h, &sr) != 0);
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
    dir = CheckDirPath(Dir);
    if (dir == "")
    {
        return;
    }
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
        } while (FindNextFile(h, &sr) != 0);
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
/// 2020-11-19: set len == 10
const int g_GUID_len = 10;
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

#ifdef _WIN32
    //-------------
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
    UINT64 delt_y, delt_M, delt_d, delt_h, delt_m, delt_s, delt_ma, delt_mi;
    delt_y = (dateTime.getDate().getYear() - 2015) * 365;
    delt_M = dateTime.getDate().getMonth() * 30;
    delt_d = dateTime.getDate().getDay() * 24 * 60 * 60 * 1000;
    delt_h = dateTime.getTime().getHour() * 60 * 60 * 1000;
    delt_m = dateTime.getTime().getMinute() * 60 * 1000;
    delt_s = dateTime.getTime().getIntSecond() * 1000;
    delt_ma = delt_y + delt_M;
    delt_mi = delt_d + delt_h + delt_m + delt_s + dateTime.getTime().getMilliSecond();
#else
    uint64_t delt_y, delt_M, delt_d, delt_h, delt_m, delt_s, delt_ma, delt_mi;
    time_t t;
    time(&t);
    struct tm *t1;
    t1 = localtime(&t);
    delt_y = t1->tm_year;
    delt_M = t1->tm_mon;
    delt_d = t1->tm_mday;
    delt_h = t1->tm_hour;
    delt_m = t1->tm_min;
    delt_s = t1->tm_sec;
    delt_ma = delt_y + delt_M;
    delt_mi = delt_d + delt_h + delt_m + delt_s + t1->tm_gmtoff;
    //    struct timeval t_start; //microseconds
    //    gettimeofday(&t_start, NULL);
    //    printf("Start time: %ld us", t_start.tv_usec);
#endif // _WIN32

    uid[1] = delt_ma << g_GUID_len;
    uid[1] += delt_mi;
    //----------
#ifdef HAVE_WINDOWS_H
    GUID guid;
    HRESULT result = NULL;
    do
    {
        result = CoCreateGuid(&guid);
        uid[0] = guid.Data1;
    } while (result != S_OK);

    return (*((UINT64*)uid) >> 1);
#else
    //to do add! :centos yum install uuid-dev
    GUID guid;
    uuid_generate(reinterpret_cast<unsigned char *>(&guid));
    uid[0] = guid.Data1;
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
        try
        {
            OFFile inifile;
            inifile.fopen(filename, "w+");
            inifile.fputs(str.c_str());
            inifile.fclose();
        }
        catch (...)
        {
            return OFFalse;
        }

    }
    return OFTrue;
}

OFBool ReadStudyInfoJsonFile(OFString filename, OFString dir, OFList<OFString> &data)
{
    OFString strjson;
    if (OFStandard::fileExists(filename))
    {
        OFFile jsonfile;
        jsonfile.fopen(filename, "r+");
        const int maxline = 256;
        char line[maxline];
        OFList<OFString> StudyInfo;
        while (jsonfile.fgets(line, maxline) != NULL)
        {
            strjson += line;
        }
        jsonfile.fclose();
        CJSON::CJsonObject Json(strjson.c_str());

        int size = Json["seriesList"].GetArraySize();
        for (int i = 0; i < size; i++)
        {
            std::string strValue;
            if (Json["seriesList"][i].Get("seriesUid", strValue))
            {
                OFString suid = strValue.c_str();
                int asize = Json["seriesList"][i]["instanceList"].GetArraySize();
                for (int j = 0; j < asize; j++)
                {
                    if (Json["seriesList"][i]["instanceList"][j].Get("imageId", strValue))
                    {
                        OFString imageid = strValue.c_str();
                        data.push_back(dir + "/" + suid + "/" + imageid + ".dcm");
                    }
                }

            }
        }
        return OFTrue;
    }
    /*
    else
    {
        CJSON::CJsonObject Json;
        int numImages = 1;
        Json.Add("patientName", dcminfo.StudyPatientName.c_str());
        Json.Add("patientId", dcminfo.StudyPatientId.c_str());
        Json.Add("modality", dcminfo.StudyModality.c_str());
        Json.Add("studyDescription", dcminfo.studydescription.c_str());
        Json.Add("numImages", numImages);
        Json.Add("studyId", dcminfo.StudyID.c_str());
        Json.Add("studyuid", dcminfo.StudyInstanceUID.c_str());
        //add 2021-0106
        Json.Add("studyDate", dcminfo.StudyDateTime.c_str());
        Json.Add("patientsex", dcminfo.StudySex.c_str());
        Json.Add("patientage", dcminfo.StudyAge.c_str());
        Json.Add("patientbirth", dcminfo.PatientBirth.c_str());
        Json.Add("manufacturer", dcminfo.StudyManufacturer.c_str());
        Json.Add("institutionname", dcminfo.StudyInstitutionName.c_str());

        Json.AddEmptySubArray("seriesList");
        CJSON::CJsonObject Series;
        Series.Add("seriesUid", dcminfo.Seriesuid.c_str());
        Series.Add("seriesDescription", dcminfo.Seriesdescription.c_str());
        Series.Add("seriesNumber", dcminfo.Seriesnumber.c_str());

        Series.AddEmptySubArray("instanceList");
        CJSON::CJsonObject Images;
        Images.Add("imageId", dcminfo.Sopinstanceuid.c_str());
        Series["instanceList"].Add(Images);
        Json["seriesList"].Add(Series);

        strjson = Json.ToJsonString().c_str();
        SaveString2File(strjson, filename);
        return OFTrue;
    }
    */

    return OFTrue;
}

OFBool ReadStudyInfo(OFString filename, OFString dir, OFList<OFString> &data)
{
    OFString value;
    if (OFStandard::fileExists(filename))
    {
        using namespace std;
        int max = 1024;
        char buffer[1024];
        fstream out;
        out.open(filename.c_str(), ios::in);
        out.getline(buffer, max, '\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
        value = OFString(buffer);
        do
        {
            if (value == "[SERIES]")
            {
                out.getline(buffer, max, '\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
                value = OFString(buffer);
                int pos = value.find_last_of('|');
                OFString seruid = value.substr(pos + 1, value.length());
                out.getline(buffer, max, '\n');
                out.getline(buffer, max, '\n');
                value = OFString(buffer);
                if (out.eof())
                {
                    pos = value.find('|');
                    OFString imageuid = value.substr(pos + 1, value.length());
                    value = dir + "/";
                    value += seruid;
                    value += "/";
                    value += imageuid + ".dcm";
                    data.push_back(value);
                    break;
                }
                while (value != "[SERIES]")
                {
                    pos = value.find('|');
                    OFString imageuid = value.substr(pos + 1, value.length());
                    value = dir + "/";
                    value += seruid;
                    value += "/";
                    value += imageuid + ".dcm";
                    data.push_back(value);
                    out.getline(buffer, max, '\n');
                    value = OFString(buffer);
                    if (out.eof())
                    {
                        pos = value.find('|');
                        imageuid = value.substr(pos + 1, value.length());
                        value = dir + "/";
                        value += seruid;
                        value += "/";
                        value += imageuid + ".dcm";
                        data.push_back(value);
                        break;
                    }
                }
            }
            else
            {
                out.getline(buffer, max, '\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
                value = OFString(buffer);
            }
        } while (!out.eof());
        out.close();
    }
    return OFTrue;
}


OFList<OFString> SplitUUID(OFString str, OFString pattern)
{
    std::string::size_type pos;
    OFList<OFString> result;
    str += pattern;
    int size = str.size();
    for (int i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if (pos < size)
        {
            OFString s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

static const char BASE64[64] = { 'A','B', 'C','D','E', 'F',   'G','H', 'I','J','K', 'L',   'M','N', 'O','P','Q', 'R',  'S','T','U', 'V','W','X',    'Y','Z',
                                 'a','b', 'c','d','e', 'f',   'g','h', 'i','j','k', 'l',   'm','n', 'o','p','q', 'r',  's','t','u', 'v','w','x',    'y','z',
                                 '0','1', '2','3','4', '5',   '6','7', '8','9','+', '/' };

OFString UIDBase64(long long input, OFString &out)
{
    int base;
    OFString s;
    do
    {
        base = input % 64;
        s = BASE64[base] + s;
        input /= 64;
    } while (input);
    out = s;
    return out;
}

struct  SqlDbdataInfo
{
    OFString  IpAddress, SqlName, SqlUserName, SqlPWD;
    int Sqltype;
    SqlDbdataInfo()
    {
        IpAddress = SqlName = SqlUserName = SqlPWD = "";
        Sqltype = 0;
    }
};
static SqlDbdataInfo g_SqlDbdataInfo;
void SetSqlDbInfo(OFString  IpAddress, OFString SqlName, OFString SqlUserName, OFString SqlPWD, int Sqltype)
{
    g_SqlDbdataInfo.IpAddress   = IpAddress;
    g_SqlDbdataInfo.SqlName     = SqlName;
    g_SqlDbdataInfo.SqlUserName = SqlUserName;
    g_SqlDbdataInfo.SqlPWD      = SqlPWD;
    g_SqlDbdataInfo.Sqltype     = Sqltype;

}

void GetSqlDbInfo(OFString  &IpAddress, OFString  &SqlName, OFString  &SqlUserName, OFString  &SqlPWD, int &Sqltype)
{
    IpAddress   = g_SqlDbdataInfo.IpAddress;
    SqlName     = g_SqlDbdataInfo.SqlName;
    SqlUserName = g_SqlDbdataInfo.SqlUserName;
    SqlPWD      = g_SqlDbdataInfo.SqlPWD;
    Sqltype     = g_SqlDbdataInfo.Sqltype;

}
static std::string g_appDir;
void SetAppDir(std::string dir)
{
    g_appDir = dir;
}

std::string GetAppDir()
{
    return g_appDir;
}

///dcm query db cache
static std::map<std::string, std::string> g_StudyQueryList;
void AddDcmQrCache(std::string key, std::string vl)
{
    g_StudyQueryList.insert(std::make_pair(key,vl));
}
std::string GetDcmQrCache(std::string key)
{
    std::string vl;
    std::map<std::string, std::string>::iterator it = g_StudyQueryList.find(key);
    if (it != g_StudyQueryList.end())
    {
        vl = (*it).second;
    }
    return vl;
}

///------------------Sqlite----------------database-----------------------------------------------------------------------------
int onerowresult_sqlitecallback(void *para, int col, char** pValue, char** pNmae);

int init(sqlite3* db)
{
    static bool flag = true;
    if (flag)
    {
        flag = false;
        OFString str =    " CREATE TABLE  if not exists h_order( "
                          " StudyOrderIdentity        INTEGER PRIMARY KEY, "
                          " PatientIdentity           INTEGER, "
                          " StudyID                   TEXT      NOT NULL DEFAULT ('0'), "
                          " ClinicID                  TEXT      NULL DEFAULT ('0'), "
                          " StudyUID                  TEXT      NOT NULL DEFAULT ('0'), "
                          " StudyModality             TEXT      DEFAULT (''), "
                          " StudyAge                  INTEGER   DEFAULT -1, "
                          " ScheduledDateTime         DATETIME  NULL DEFAULT ('1900-01-01 00:00:01'), "
                          " AETitle                   TEXT      NULL DEFAULT (''), "
                          " OrderDateTime             TIMESTAMP default (datetime('now', 'localtime')), "
                          " StudyDescription          TEXT      NULL DEFAULT (''), "
                          " StudyDepart               TEXT      NULL DEFAULT (''), "
                          " StudyCode                 TEXT      NULL DEFAULT ('0000'), "
                          " StudyCost                 TEXT      NULL DEFAULT ('0'), "
                          " CostType                  TEXT      NULL DEFAULT (''), "
                          " StudyType                 INTEGER   NULL DEFAULT ('0'), "
                          " StudyState                INTEGER   NULL DEFAULT '1', "
                          " StudyDateTime             DATETIME  NULL DEFAULT ('1900-01-01 00:00:01'), "
                          " InstitutionName           TEXT      NULL DEFAULT (''), "
                          " ProcedureStepStartDate    DATETIME  NULL DEFAULT ('1900-01-01 00:00:01'), "
                          " StudyModalityIdentity     INTEGER   NULL DEFAULT -1, "
                          " StudyManufacturer         INTEGER   NULL DEFAULT (''), "
                          " RegisterID                INTEGER   NULL DEFAULT -1 "
                          " )";
        CreateTableSqlite(db, str.c_str());

        str          =    "CREATE TABLE if not exists h_patient( "
                          "PatientIdentity    INTEGER PRIMARY KEY, "
                          "PatientID          TEXT    NOT NULL, "
                          "PatientName        TEXT, "
                          "PatientNameEnglish TEXT, "
                          "PatientSex         TEXT    DEFAULT ('0'), "
                          "PatientBirthday    TEXT    DEFAULT ('1900-01-01 00:00:01'), "
                          "PatientAddr        TEXT    DEFAULT (''), "
                          "PatientEmail       TEXT    DEFAULT (''), "
                          "PatientCarID       TEXT    DEFAULT (''), "
                          "PatientTelNumber   TEXT    DEFAULT (''), "
                          "PatientType        INTEGER DEFAULT ('0'), "
                          "PatientState       INTEGER DEFAULT ('0'), "
                          "PatientJob         TEXT    DEFAULT 'JOB', "
                          "PatientNation      TEXT    DEFAULT 'H', "
                          "PatientMarriage    TEXT    DEFAULT 'unknow', "
                          "PatientHometown    TEXT    DEFAULT 'guangdong', "
                          "PatientHisID       TEXT    DEFAULT ('-1'), "
                          "PatientHistoryTell TEXT    DEFAULT 'NULL' "
                          ")";
        CreateTableSqlite(db, str.c_str());

        str = "CREATE TABLE if not exists h_user( "
            "id                INTEGER PRIMARY KEY, "
            "username          TEXT    NOT NULL, "
            "password          TEXT    NOT NULL"
            ")";
        CreateTableSqlite(db, str.c_str());

        str = "CREATE TABLE if not exists h_report( "
            "ReportIdentity                INTEGER     PRIMARY KEY, "
            "StudyOrderIdentity            INTEGER     NOT NULL, "
            "ReportState                   INTEGER     DEFAULT -1, "
            "ReportTemplate                TEXT        NULL DEFAULT (''), "
            "ReportCreatDate               TIMESTAMP   default (datetime('now', 'localtime')), "
            "ReportWriterID                TEXT        NULL DEFAULT (''), "
            "ReportCheckID                 TEXT        NULL DEFAULT (''), "
            "ReportCheckDate               TIMESTAMP   default (datetime('now', 'localtime')), "
            "ReportContent                 TEXT        NULL DEFAULT (''), "
            "ReportOther                   BLOB        NULL DEFAULT ('') "
            ")";
        CreateTableSqlite(db, str.c_str());
    }
    return 0;
}
sqlite3* OpenSqlite(OFString filename)
{
    sqlite3* db = NULL;
    if (filename.length() < 1)
    {
        filename = "hitsqlite.db";
    }
    if (sqlite3_open_v2(filename.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL) != SQLITE_OK)
    {
        std::ostringstream msg;
        msg << "Can't create database: " << sqlite3_errmsg(db);
        throw std::runtime_error(msg.str().c_str());
    }
    init(db);
    return db;
}
int CloseSqlite(sqlite3* db)
{
    if (db)
    {
       return sqlite3_close(db);
    }
    return -1;
}
int CreateTableSqlite(sqlite3* db, std::string sqlstr)
{
    //if (db)
    //sqlite3_exec(db, "CREATE TABLE studies (studyuid TEXT UNIQUE, patid TEXT, patname TEXT, studydesc TEXT, studydate TEXT, path TEXT, checked TEXT)", NULL, NULL, NULL);

    if (db)
    {
        return sqlite3_exec(db, sqlstr.c_str(), NULL, NULL, NULL);
    }
    return -1;
}
int InsertSqlite(sqlite3* db, std::string sqlstr, std::vector<std::string> param)
{
    //std::string sql = "INSERT INTO studies VALUES(?, ?, ?, ?, ?, ?, '1')";
    sqlite3_stmt *insert;
    sqlite3_prepare_v2(db, sqlstr.c_str(), sqlstr.length(), &insert, NULL);
    int nu = param.size();
    for (int i = 0; i < nu; i++)
    {
        std::string str = param[i].c_str();
        sqlite3_bind_text(insert, i+1, str.c_str(), str.length(), SQLITE_STATIC);
    }

    int res = sqlite3_exec_stmt(insert, NULL, NULL, NULL);
    sqlite3_finalize(insert);
    return res;

    //if (res == SQLITE_DONE || res == SQLITE_ABORT)
    //    return 1;
   // else
    //    return SQLITE_OK;

}

class SqliteDbResult
{
public:
    SqliteDbResult()
    {
        result = NULL;
    }
    ~SqliteDbResult()
    {

    }
public:
    std::vector<std::map<std::string, std::string>> *result;
private:

};
int onerowresult_sqlitecallback(void *para, int col, char** pValue, char** pNmae)
{
    if (para == NULL)
        return 0;
    SqliteDbResult *result = (SqliteDbResult*)para;
    std::map<std::string, std::string> keyvl;
    for (int i = 0; i < col; i++)
    {
        std::string key = pNmae[i];
        std::string vl = pValue[i];
        keyvl.insert(std::make_pair(key, vl));
    }
    result->result->push_back(keyvl);
    return 0;
}

int SelectSqlite(sqlite3* db, std::string sqlstr, std::vector<std::string> param, std::vector<std::map<std::string, std::string>> &result)
{
    result.clear();
    SqliteDbResult resultdb;
    resultdb.result = &result;
    //std::string sql = "SELECT studyuid, patid, patname, studydesc, studydate, path, checked FROM studies WHERE (patid = ? AND patname = ?) ORDER BY studyuid ASC";
    sqlite3_stmt *select;
    sqlite3_prepare_v2((sqlite3*)db, sqlstr.c_str(), sqlstr.length(), &select, NULL);
    int nu = param.size();
    for (int i = 0; i < nu; i++)
    {
        std::string str = param[i].c_str();
        sqlite3_bind_text(select, i + 1, str.c_str(), str.length(), SQLITE_STATIC);
    }
    result.clear();
    int res = sqlite3_exec_stmt(select, onerowresult_sqlitecallback, &resultdb, NULL);
    sqlite3_finalize(select);
    return res;

}

int UpdateSqlite(sqlite3* db, std::string sqlstr, std::vector<std::string> param)
{
    //std::string sql = "UPDATE studies SET checked = ? WHERE (studyuid = ?)";
    sqlite3_stmt *update;
    sqlite3_prepare_v2(db, sqlstr.c_str(), sqlstr.length(), &update, NULL);
    int nu = param.size();
    for (int i = 0; i < nu; i++)
    {
        std::string str = param[i].c_str();
        sqlite3_bind_text(update, i + 1, str.c_str(), str.length(), SQLITE_STATIC);
    }
    int res = sqlite3_exec_stmt(update, onerowresult_sqlitecallback, NULL, NULL);
    sqlite3_finalize(update);
    return res;

}

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

OFString GetStudyHashDir(OFString studyuid)
{
    OFString dir;
    OFHashValue vl = CreateHashValue(studyuid);
    dir = "/" + vl.first + "/" + vl.second;
    return dir;
}

OFString HashValue(int F, int S, OFString str)
{
    size_t len = str.length();
    size_t value = F + S;
    for (size_t i = 0; i < len; i++)
    {
        value = (value + str[i])*F + i*S;
    }

#if defined(_WIN32)

#if defined(_MSC_VER)
    // Windows:  Visual Statuio 
    char oxstr[256] = {0};
    sprintf(oxstr,"%04x",value);
    return oxstr;
#endif

#if defined(__GNUC__)
    // Windows平台 GCC编译器特定的代码
    QString hex = QString("%1").arg(value, 4, 16, QLatinlChar('0')); ///// 保留四位，不足补零
    return QString.toLatin1().data();
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
OFHashValue CreateHashValue(DicomFileInfo dcminfo )
{
    //2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
    //101 103 107 109 113 127 131 137 139 149 151 157 163 167 173 179
    //181 191 193 197 199
    OFString str = dcminfo.studyUID;
    OFHashValue hashvalue;
    //71 197
    hashvalue.first  = HashValue(71, 197, str);
    //79 199
    hashvalue.second = HashValue(79, 199, str);

    return hashvalue;
}

unsigned long studyuid_hash(const char *str)
{
    unsigned long hash = 32767;
    unsigned long pid  = 65537;
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
    if(Code == 0)
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
    if((strPath = getcwd(NULL, 0)) == "")
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
                OFString temp;
                if (FileExt == "")
                {
                    datas.push_back(dir + name);
                }
                else if (Not == false && OFStandard::toUpper(temp, FileExt) == OFStandard::toUpper(temp, name))
                {
                    datas.push_back(dir + name);
                }
                else if (Not == true && OFStandard::toUpper(temp, FileExt) != OFStandard::toUpper(temp, name))
                {
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
    UINT64 delt_y,delt_M,delt_d,delt_h,delt_m,delt_s,delt_ma,delt_mi;
    delt_y = (dateTime.getDate().getYear() - 2015) * 365;
    delt_M = dateTime.getDate().getMonth() * 30;
    delt_d = dateTime.getDate().getDay() * 24 * 60 * 60 * 1000;
    delt_h = dateTime.getTime().getHour() * 60 * 60 * 1000;
    delt_m = dateTime.getTime().getMinute() * 60 * 1000;
    delt_s = dateTime.getTime().getIntSecond() * 1000;
    delt_ma = delt_y + delt_M;
    delt_mi = delt_d + delt_h + delt_m + delt_s + dateTime.getTime().getMilliSecond();
#else
    uint64_t delt_y,delt_M,delt_d,delt_h,delt_m,delt_s,delt_ma,delt_mi;
    time_t t;
    time(&t);
    struct tm *t1;
    t1=localtime(&t);
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
    }
    while (result != S_OK);

    return (*((UINT64*)uid)>>1);
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
        OFFile inifile;
        inifile.fopen(filename, "w");
        inifile.fputs(str.c_str());
        inifile.fclose();
    }
    return OFTrue;
}

OFBool ReadStudyInfo(OFString filename,OFString dir, OFList<OFString> &data)
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
                    value += imageuid+".dcm";
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
        }
        while (!out.eof());
        out.close();
    }
    return OFTrue;
}
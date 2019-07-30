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
#endif

#include "dcmtk/oflog/fileap.h"
#include "Units.h"


//!根据字符计算两个Hash数值
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
    int Code = GetFileAttributes(Dir.c_str());
    return (Code != -1) && ((FILE_ATTRIBUTE_DIRECTORY & Code) != 0);
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
#else
        if (mkdir(Dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
#endif
        {
            OFString msg;
            msg = "CreateDirectory() failed with error %s,(%s)" + GetLastError() + Dir;
            //OFLOG_ERROR(storescpLogger, msg);
            return false;
        }
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
    if (path[path.length() - 1] != '\\')
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
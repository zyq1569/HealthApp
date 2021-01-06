/*
*save dcm info to db (mysql)
*
*/

#include "dcmtk/config/osconfig.h" /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CCTYPE
#include "dcmtk/ofstd/ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
END_EXTERN_C

#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/ofstd/ofstring.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmnet/dicom.h"      /* for DICOM_APPLICATION_REQUESTOR */
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmnet/dcmtrans.h"   /* for dcmSocketSend/ReceiveTimeout */
#include "dcmtk/dcmnet/dcasccfg.h"   /* for class DcmAssociationConfiguration */
#include "dcmtk/dcmnet/dcasccff.h"   /* for class DcmAssociationConfigurationFile */
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/dcostrmz.h"  /* for dcmZlibCompressionLevel */
//-------add add 201806
#include "dcmtk/oflog/fileap.h"
//#ifdef HAVE_WINDOWS_H
//#include <direct.h>      /* for _mkdir() */
//#endif
//--------------------
//add mysql driver  后期需要将数据库的处理独立模块
#include"HMariaDb.h"
//#include "dcmtk/ofstd/ofdatime.h"
//#include <objbase.h>
#include"DcmConfig.h"
//////////////
#include"cJSON.h"

#ifdef ON_THE_FLY_COMPRESSION
#include "dcmtk/dcmjpeg/djdecode.h"  /* for JPEG decoders */
#include "dcmtk/dcmjpeg/djencode.h"  /* for JPEG encoders */
#include "dcmtk/dcmjpls/djdecode.h"  /* for JPEG-LS decoders */
#include "dcmtk/dcmjpls/djencode.h"  /* for JPEG-LS encoders */
#include "dcmtk/dcmdata/dcrledrg.h"  /* for RLE decoder */
#include "dcmtk/dcmdata/dcrleerg.h"  /* for RLE encoder */
#include "dcmtk/dcmjpeg/dipijpeg.h"  /* for dcmimage JPEG plugin */
#endif

#ifdef WITH_OPENSSL
#include "dcmtk/dcmtls/tlstrans.h"
#include "dcmtk/dcmtls/tlslayer.h"
#endif

#ifdef WITH_ZLIB
#include <zlib.h>          /* for zlibVersion() */
#endif

#ifndef _WIN32
//#include <stdio.h>
#include <dirent.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <error.h>
//#include <sys/stat.h>
//#include <malloc.h>
#endif

#include "Units.h"

#include "CJsonObject.hpp"

#if defined (HAVE_WINDOWS_H) || defined(HAVE_FNMATCH_H)
#define PATTERN_MATCHING_AVAILABLE
#endif

#define OFFIS_CONSOLE_APPLICATION "saveDcmInfoDB"

#define MAX_FILE_NAME_LEN 256

static OFLogger SaveDcmInfoDbLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

static OFBool opt_recurse = OFTrue;
static const char *opt_scanPattern = "";
static HMariaDb *g_pMariaDb = NULL;
static OFString g_mySql_IP = "127.0.0.1";
static OFString g_mySql_username = "root";
static OFString g_mySql_pwd = "root";
static OFString g_mySql_dbname = "HIT";
static OFString g_ImageDir = "";



#ifndef _WIN32
typedef struct foldernode_t {
    char *path;                // point to foldername or filename path
    struct foldernode_t *next;
} foldernode;

void travel_files(char *path, const OFString FileExt, OFList<OFString> &datas, const bool Not = false, const int Count = 100)
{
    OFString UpfileExt;
    OFStandard::toUpper(UpfileExt, FileExt);//OFStandard::toUpper(FileExt);
    UpfileExt = "." + UpfileExt;
    DIR *dir;
    struct dirent *ptr;
    char foldername[MAX_FILE_NAME_LEN] = { 0 };
    char folderpath[MAX_FILE_NAME_LEN] = { 0 };

    foldernode *folderstart;
    folderstart = (foldernode*)calloc(1, sizeof(foldernode));/* ignore err case */
    folderstart->path = (char *)calloc(1, MAX_FILE_NAME_LEN + 1);
    strncpy(folderstart->path, path, MAX_FILE_NAME_LEN);
    folderstart->next = NULL;

    foldernode *folderfirst = folderstart; /* use to search */
    foldernode *folderlast = folderstart; /* use to add foldernode */
    foldernode *oldfirst = NULL;

    while (folderfirst != NULL)
    {
        //        printf("folderfirst->path: dir=%s\n", folderfirst->path);
        if ((dir = opendir(folderfirst->path)) != NULL)
        {
            while ((ptr = readdir(dir)) != NULL)
            {
                if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
                {
                    continue;
                }
                else if (ptr->d_type == DT_REG)
                { /* regular file */
                    OFString filename = ptr->d_name;
                    size_t len = filename.length();
                    OFString ext = filename.substr(len-4,len);
                    if (OFStandard::toUpper(ext) == UpfileExt)
                    {
                        OFString path = folderfirst->path;
                        datas.push_back(path+"/"+filename);
                        if (datas.size() > Count)
                        {
                            //释放当前查找目录节点&新的目录节点 空间，并返回
                            foldernode *currentDir = folderfirst;
                            if (currentDir)
                            {
                                if (currentDir->path)
                                {
                                    free(currentDir->path);
                                    currentDir->path = NULL;
                                }
                                free(currentDir);
                                currentDir = NULL;
                            }
                            if (folderfirst->next)
                            {
                                foldernode *nextDir = folderfirst->next;
                                if (nextDir->path)
                                {
                                    free(nextDir->path);
                                    nextDir->path = NULL;
                                }
                                free(nextDir);
                                nextDir = NULL;
                            }
                            closedir(dir);
                            return;
                        }
                        //                        printf("folderfirst->path : %s\n", folderfirst->path);
                        //                        printf("------********ptr->d_name : %s\n", ptr->d_name);
                    }
                    //                    else
                    //                    {
                    //                        printf("folderfirst->path : %s\n", folderfirst->path);
                    //                        printf("------#########ptr->d_name : %s\n", ptr->d_name);
                    //                    }
                }
                else if (ptr->d_type == DT_DIR)
                { /* dir */
                    bzero(foldername, sizeof(foldername));
                    bzero(folderpath, sizeof(folderpath));
                    strncpy(foldername, ptr->d_name, sizeof(foldername));
                    snprintf(folderpath, sizeof(folderpath), "%s/%s", folderfirst->path, foldername);
                    //                    printf("folderpath: %s\n", folderpath);

                    //增加新的目录节点,并释放空间
                    foldernode *foldernew;
                    foldernew = (foldernode *)calloc(1, sizeof(foldernode));
                    foldernew->path = (char *)calloc(1, MAX_FILE_NAME_LEN + 1);
                    strncpy(foldernew->path, folderpath, MAX_FILE_NAME_LEN);
                    foldernew->next = NULL;
                    folderlast->next = foldernew;
                    folderlast = foldernew;

                }
            }
        }
        else
        {
            printf("opendir[%s] error: %m\n", folderfirst->path);
            return;
        }
        //释放当前查找目录节点空间，开始查找下一个目录节点
        oldfirst = folderfirst;
        folderfirst = folderfirst->next; // change folderfirst point to next foldernode
        if (oldfirst)
        {
            if (oldfirst->path)
            {
                free(oldfirst->path);
                oldfirst->path = NULL;
            }
            free(oldfirst);
            oldfirst = NULL;
        }
        closedir(dir);
        //---
    }
}
#endif

struct  HStudyInfo
{
    OFString StudyInstanceUID, StudyPatientName, StudyPatientId, StudySex, StudyID, PatientNameEnglish;
    OFString StudyAge, PatientBirth, StudyState, StudyDateTime, StudyModality, StudyManufacturer, StudyInstitutionName;
    OFString Seriesuid, studydescription, Seriesdescription, Seriesnumber, Sopinstanceuid, instanceNumber;
};

OFBool CjsonSaveFile(HStudyInfo dcminfo, OFString filename)
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
        int numImages = 0;
        if (Json.Get("numImages", numImages))
        {
            numImages++;
        }
        Json.Replace("numImages", numImages);
        int size = Json["seriesList"].GetArraySize();
        for (int i = 0; i < size; i++)
        {
            std::string strValue;
            if (Json["seriesList"][i].Get("seriesUid", strValue))
            {
                OFString suid = strValue.c_str();
                if (suid == dcminfo.Seriesuid)
                {
                    int asize = Json["seriesList"][i]["instanceList"].GetArraySize();
                    for (int j = 0; j < asize; j++)
                    {
                        if (Json["seriesList"][i]["instanceList"][j].Get("imageId", strValue))
                        {
                            OFString imageid = strValue.c_str();
                            if (imageid == dcminfo.Sopinstanceuid)
                            {
                                return OFTrue;
                            }
                        }
                    }
                    CJSON::CJsonObject Images;
                    Images.Add("imageId", dcminfo.Sopinstanceuid.c_str());
                    Json["seriesList"][i]["instanceList"].Add(Images);
                    strjson = Json.ToJsonString().c_str();
                    SaveString2File(strjson, filename);
                    return OFTrue;
                }
            }
        }
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

    return OFTrue;
}
OFBool SaveDcmInfoJsonFile(HStudyInfo dcminfo, OFString filename)
{
    return CjsonSaveFile(dcminfo, filename);
}

OFBool SaveDcmInfoFile(HStudyInfo dcminfo, OFString filename)
{
    OFFile inifile;
    if (OFStandard::fileExists(filename))
    {
        bool f = inifile.fopen(filename, "r+");
        const int maxline = 256;
        char line[maxline];
        OFList<OFString> StudyInfo;
        //OFString fileinfo;
        OFString temp = dcminfo.Seriesnumber + "|" + dcminfo.Seriesdescription + "|" + dcminfo.Seriesuid;
        OFString imageinfo = dcminfo.instanceNumber + "|" + dcminfo.Sopinstanceuid;
        OFBool flag = OFFalse;
        //offile_off_t pos=0;
        while (inifile.fgets(line, maxline) != NULL)
        {
            OFString str = line;
            if (flag)
            {
                if (str.find(imageinfo) == 0)
                {
                    inifile.fclose();//该image信息 已经添加
                    return OFTrue;
                }
            }
            StudyInfo.push_back(str);
            if (!flag)
            {
                if (str.find("[SERIES]") == 0)
                {
                    inifile.fgets(line, maxline);
                    str = line;
                    StudyInfo.push_back(str);
                    if (str.find(temp) == 0)//same SERIES dcmfile
                    {
                        inifile.fgets(line, maxline);//"[image]"
                        str = line;
                        StudyInfo.push_back(str);
                        str = imageinfo + "\n";
                        StudyInfo.push_back(str);
                        flag = OFTrue;
                    }
                }
            }
        }
        if (!flag)//直接在后面追加即可
        {
            OFString str = "\n[SERIES]\n";
            str += dcminfo.Seriesnumber + "|" + dcminfo.Seriesdescription + "|" + dcminfo.Seriesuid + "\n";
            str += "[image]\n";
            str += dcminfo.instanceNumber + "|" + dcminfo.Sopinstanceuid;
            inifile.fputs(str.c_str());
            //StudyInfo.push_back(str);
        }
        else
        {
            OFListIterator(OFString) if_iter = StudyInfo.begin();
            OFListIterator(OFString) if_last = StudyInfo.end();
            OFString temp;
            while (if_iter != if_last)
            {
                temp += *if_iter;//inifile.fputs((*if_iter).c_str());
                if_iter++;
            }
            inifile.rewind();//可以优化直接在插入pos 覆盖后面信息
            inifile.fputs(temp.c_str());
        }
        inifile.fclose();
    }
    else
    {
        inifile.fopen(filename, "w");
        inifile.fputs("[STUDY]\n");
        OFString str = "studyuid=" + dcminfo.StudyInstanceUID + "\n";
        str += "patientid=" + dcminfo.StudyPatientId + "\n";
        str += "patientname=" + dcminfo.StudyPatientName + "\n";
        str += "patientsex=" + dcminfo.StudySex + "\n";

        str += "studyid=" + dcminfo.StudyID + "\n";
        str += "patientage=" + dcminfo.StudyAge + "\n";
        str += "patientbirth=" + dcminfo.PatientBirth + "\n";
        str += "studydatetime=" + dcminfo.StudyDateTime + "\n";
        str += "modality=" + dcminfo.StudyModality + "\n";
        str += "manufacturer=" + dcminfo.StudyManufacturer + "\n";

        str += "institutionname=" + dcminfo.StudyInstitutionName + "\n";
        str += "studydescription=" + dcminfo.studydescription + "\n";
        inifile.fputs(str.c_str());

        inifile.fputs("[SERIES]\n");
        str = dcminfo.Seriesnumber + "|";
        str += dcminfo.Seriesdescription + "|" + dcminfo.Seriesuid + "\n";
        str += "[image]\n";
        str += dcminfo.instanceNumber + "|" + dcminfo.Sopinstanceuid;
        inifile.fputs(str.c_str());
        inifile.fclose();
    }
    return OFTrue;
}

OFBool SaveDcmInfo2Db(OFString filename, DcmConfigFile *configfile)
{
    HStudyInfo StudyInfo;
    OFFile inifile;
    if (OFStandard::fileExists(filename))
    {
        inifile.fopen(filename, "r");
        //OFString str = "studyuid=" + currentStudyInstanceUID;
        const int maxline = 256;
        char line[maxline];
        OFList<OFString> value_list;
        while (inifile.fgets(line, maxline) != NULL)
        {
            OFString str = line;
            value_list.push_back(str);
        }
        inifile.fclose();
        OFListIterator(OFString) if_iter = value_list.begin();
        OFListIterator(OFString) if_last = value_list.end();
        while (if_iter != if_last)
        {
            OFString str = *if_iter;
            OFString studyuid = "studyuid=";
            OFString PatientId = "patientid=";
            OFString PatientName = "patientname=";
            OFString PatientSex = "patientsex=";
            OFString StudyID = "studyid=";
            OFString PatientAge = "patientage=";
            OFString PatientBirth = "patientbirth=";
            OFString StudyDateTime = "studydatetime=";
            OFString StudyModality = "modality=";
            OFString StudyManufacturer = "manufacturer=";
            OFString StudyInstitutionName = "institutionname=";
            OFString Studydescription = "studydescription=";
            OFString Seriesuid = "seriesuid=";
            OFString Seriesdescription = "seriesdescription=";
            OFString Seriesnumber = "seriesnumber=";
            OFString Sopinstanceuid = "sopinstanceuid=";
            OFString InstanceNumber = "instanceNumber=";
            //int pos = str.find(studyuid);
            if (str.find(studyuid) == 0)
            {
                int sublen = studyuid.length();
                StudyInfo.StudyInstanceUID = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(PatientId) == 0)
            {
                int sublen = PatientId.length();
                StudyInfo.StudyPatientId = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(PatientName) == 0)
            {
                int sublen = PatientName.length();
                OFString temp = str.substr(sublen, str.length() - sublen - 1);
                StudyInfo.StudyPatientName = FormatePatienName(temp);
            }
            if (str.find(PatientSex) == 0)
            {
                int sublen = PatientSex.length();
                StudyInfo.StudySex = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(StudyID) == 0)
            {
                int sublen = StudyID.length();
                StudyInfo.StudyID = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(PatientAge) == 0)
            {
                int sublen = PatientAge.length();
                StudyInfo.StudyAge = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(PatientBirth) == 0)
            {
                int sublen = PatientBirth.length();
                StudyInfo.PatientBirth = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(StudyDateTime) == 0)
            {
                int sublen = StudyDateTime.length();
                StudyInfo.StudyDateTime = str.substr(sublen, str.length() - sublen - 1);
            }//StudyModality, StudyManufacturer, StudyInstitutionName
            else if (str.find(StudyModality) == 0)
            {
                int sublen = StudyModality.length();
                StudyInfo.StudyModality = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(StudyManufacturer) == 0)
            {
                int sublen = StudyManufacturer.length();
                StudyInfo.StudyManufacturer = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(StudyInstitutionName) == 0)
            {
                int sublen = StudyInstitutionName.length();
                StudyInfo.StudyInstitutionName = str.substr(sublen, str.length() - sublen - 1);
            }//Seriesuid, Seriesdescription, Seriesnumber, Sopinstanceuid;
            else if (str.find(Seriesuid) == 0)
            {
                int sublen = Seriesuid.length();
                StudyInfo.Seriesuid = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(Seriesdescription) == 0)
            {
                int sublen = Seriesdescription.length();
                StudyInfo.Seriesdescription = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(Seriesnumber) == 0)
            {
                int sublen = Seriesnumber.length();
                StudyInfo.Seriesnumber = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(Sopinstanceuid) == 0)
            {
                int sublen = Sopinstanceuid.length();
                StudyInfo.Sopinstanceuid = str.substr(sublen, str.length() - sublen - 1);
            }//Studydescription
            else if (str.find(Studydescription) == 0)
            {
                int sublen = Studydescription.length();
                StudyInfo.studydescription = str.substr(sublen, str.length() - sublen - 1);
            }
            else if (str.find(InstanceNumber) == 0)
            {
                int sublen = InstanceNumber.length();
                StudyInfo.instanceNumber = str.substr(sublen, str.length() - sublen - 1);
            }
            if_iter++;
        }
        //save to db
        //OFCondition l_error = EC_Normal;
        OFString strsql, querysql;
        try
        {
            if (StudyInfo.StudyInstanceUID.length() < 1)
            {
                OFLOG_ERROR(SaveDcmInfoDbLogger, "NO StudyInstanceUID filename:" + filename);
                return OFFalse;
            }
            OFString pathname = g_ImageDir + GetStudyHashDir(StudyInfo.StudyInstanceUID) + "/" + StudyInfo.StudyInstanceUID + "/" + StudyInfo.StudyInstanceUID;
            OFString studyinifile = pathname + ".ini";
            OFString studyjsonfile = pathname + ".json";
            if (!OFStandard::fileExists(studyinifile))
            {
                if (g_pMariaDb == NULL)
                {
#ifdef _UNICODE
                    g_pMariaDb = new HMariaDb(W2S(strIP.GetBuffer()).c_str(), W2S(strUser.GetBuffer()).c_str(), \
                        W2S(strPwd.GetBuffer()).c_str(), W2S(strDadaName.GetBuffer()).c_str());///*"127.0.0.1"*/"root", "root", "HIT");
#else
                    g_pMariaDb = new HMariaDb(g_mySql_IP.c_str(), g_mySql_username.c_str(), \
                        g_mySql_pwd.c_str(), g_mySql_dbname.c_str());///*"127.0.0.1"*/"root", "root", "HIT");

#endif
                }
                querysql = "select * from h_patient where PatientID = '" + StudyInfo.StudyPatientId + "'";
                g_pMariaDb->query(querysql.c_str());
                ResultSet * rs = g_pMariaDb->QueryResult();
                OFString PatientIdentity;
                if (rs == NULL)
                {
                    //int rows = rs->countRows();
                    char uuid[64];
                    sprintf(uuid, "%llu", (CreateGUID() >> 1));
                    PatientIdentity = uuid;
                    querysql = "insert into h_patient (PatientIdentity,PatientID,PatientName,PatientNameEnglish,\
                                                           PatientSex,PatientBirthday) value(";
                    querysql += PatientIdentity;
                    querysql += ",'";
                    querysql += StudyInfo.StudyPatientId;
                    querysql += "','";
                    querysql += StudyInfo.StudyPatientName;
                    querysql += "','";
                    querysql += StudyInfo.PatientNameEnglish;
                    querysql += "','";
                    if (StudyInfo.StudySex == "")
                    {
                        querysql += "O";
                    }
                    else
                    {
                        querysql += StudyInfo.StudySex;
                    }
                    querysql += "','";
                    querysql += StudyInfo.PatientBirth;
                    querysql += "');";
                    g_pMariaDb->execute(querysql.c_str());
                }
                else
                {
                    std::vector<std::string> row;
                    //std::string sdata;
                    while (rs->fetch(row))
                    {
                        PatientIdentity = row[0].c_str();
                    }
                }

                ///-----------------old h_study table--------------------------------------------------
                //to do : delete code  abandon H_study/
                /*
                querysql = "select * from H_study where StudyUID = '" + StudyInfo.StudyInstanceUID + "'";
                g_pMariaDb->query(querysql.c_str());
                rs = g_pMariaDb->QueryResult();
                if (rs == NULL)
                {
                char uuid[64];
                sprintf(uuid, "%llu", (CreateGUID() >> 1));
                OFString StudyIdentity = uuid;
                strsql = "insert into H_study (StudyIdentity,StudyID,StudyUID,PatientIdentity,";
                strsql += " StudyDateTime,StudyModality,InstitutionName,StudyManufacturer,StudyState,StudyDcmPatientName,StudyDescription) value(";
                strsql += StudyIdentity;
                strsql += ",'";
                strsql += StudyInfo.StudyID;
                strsql += "','";
                strsql += StudyInfo.StudyInstanceUID;
                strsql += "',";
                strsql += PatientIdentity;
                strsql += ",'";
                if (StudyInfo.StudyDateTime.empty())
                {
                StudyInfo.StudyDateTime = "1970-01-01 00:00:01.000000";
                }
                strsql += StudyInfo.StudyDateTime;
                strsql += "','";
                strsql += StudyInfo.StudyModality;
                strsql += "','";
                strsql += StudyInfo.StudyInstitutionName;
                strsql += "','";
                strsql += StudyInfo.StudyManufacturer;
                strsql += "','dcm','";
                strsql += StudyInfo.StudyPatientName;
                strsql += "','";//StudyDescription
                strsql += StudyInfo.studydescription;
                strsql += "');";
                g_pMariaDb->execute(strsql.c_str());
                }
                //OFStandard::deleteFile(filename);
                OFLOG_INFO(SaveDcmInfoDbLogger, "SaveDcmInfo2Db filename:" + filename);
                */

                //-------------------------------------------------------------------------------
                //更新预约表检查状态 // modify 2020-11-19
                //querysql = "update H_order set StudyState = 3 where StudyUID = '" + StudyInfo.StudyInstanceUID + "';";
                //g_pMariaDb->execute(querysql.c_str());
                //OFLOG_INFO(SaveDcmInfoDbLogger, "update table H_order:" + StudyInfo.StudyInstanceUID);
                //-------------------------------------------------------------------------------
                ///---------------UP  old h_study table------------------------------------------


                ///------------------ - 2020 - 11 - 19 - add-------------------------- -
                querysql = "select * from H_order where StudyUID = '" + StudyInfo.StudyInstanceUID + "'";
                g_pMariaDb->query(querysql.c_str());
                rs = g_pMariaDb->QueryResult();
                if (rs == NULL)
                {
                    char uuid[64];
                    sprintf(uuid, "%llu", (CreateGUID()>>1));
                    OFString StudyIdentity = uuid;
                    strsql = "insert into H_order (StudyOrderIdentity,StudyID,StudyUID,PatientIdentity,StudyDateTime,";
                    strsql += "StudyModality,InstitutionName,StudyManufacturer,StudyState,StudyDescription) value(";
                    strsql += StudyIdentity;
                    strsql += ",'";
                    strsql += StudyInfo.StudyID;
                    strsql += "','";
                    strsql += StudyInfo.StudyInstanceUID;
                    strsql += "',";
                    strsql += PatientIdentity;
                    strsql += ",'";
                    if (StudyInfo.StudyDateTime.empty())
                    {
                        StudyInfo.StudyDateTime = "1800-01-01 00:00:01.000000";
                    }
                    strsql += StudyInfo.StudyDateTime;
                    strsql += "','";
                    strsql += StudyInfo.StudyModality;
                    strsql += "','";
                    strsql += StudyInfo.StudyInstitutionName;
                    strsql += "','";
                    strsql += StudyInfo.StudyManufacturer;
                    strsql += "','3','";///检查状态：-1.标记删除 1.预约 2.等待检查 3.已检查 4.诊断 5.报告审核
                    //strsql += StudyInfo.StudyPatientName;
                    //strsql += "','";//StudyDescription
                    strsql += StudyInfo.studydescription;
                    strsql += "');";
                    g_pMariaDb->execute(strsql.c_str());
                }
                ///-------------------2020-11-19-add--------------------------- 

            }
            if (SaveDcmInfoFile(StudyInfo, studyinifile) && CjsonSaveFile(StudyInfo, studyjsonfile))
            {
                OFStandard::deleteFile(filename);
            }
        }
        catch (...)
        {
            //OFLOG_ERR(storescuLogger, "---------argv[]:" + tempstr + " ----------------------");
            OFLOG_ERROR(SaveDcmInfoDbLogger, "SaveDcmInfo2Db filename:" + filename);
            OFLOG_ERROR(SaveDcmInfoDbLogger, "DB sql querysql:" + querysql);
            OFLOG_ERROR(SaveDcmInfoDbLogger, "DB sql strsql:" + strsql);
            return OFFalse;
        }
    }
    return OFTrue;
}

int main(int argc, char *argv[])
{
    //uint64 uid = CreateGUID();
    static DcmConfigFile dcmconfig;
    OFString Task_Dir, Log_Dir, Error_Dir;
    //OFString ini_dir, ini_error_dir;

    OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION, "DICOM file Info 2 DB", rcsid);
    OFString tempstr, path = argv[0];
    OFString currentAppPath = OFStandard::getDirNameFromPath(tempstr, path);
    Log_Dir = currentAppPath + "/log";
    int pos = 0;
    pos = path.find_last_of('\\');
    if (pos < 1)
    {
        OFString message = " start app by commline:";
        app.printMessage(message.c_str());
        path = GetCurrWorkingDir();
        app.printMessage("GetCurrWorkingDir filePath:");
        app.printMessage(path.c_str());
    }

    if (argc > 1)
    {
        OFString dir = argv[1];
        //OFList<OFString> list_file_ini_test;
        //travel_files((char*)dir.c_str(),"ini",list_file_ini_test);
        if (OFStandard::dirExists(dir))
        {
            Task_Dir = dir + "/Task/1";
            Error_Dir = dir + "/Task/error";
            //Log_Dir    = dir + "/log";
            g_ImageDir = dir + "/Images";
            //int pos = dir.find_last_of("/");
            //if (pos > -1)
            //{
            //    Log_Dir = dir.substr(0, pos) + "/log";
            //}
            //else
            //{
            //    int pos = dir.find_last_of("\\");
            //    if (pos > -1)
            //    {
            //        Log_Dir = dir.substr(0, pos) + "/log";
            //    }
            //}
        }
        if (argc > 5)
        {
            g_mySql_IP = argv[2];
            g_mySql_dbname = argv[3];
            g_mySql_username = argv[4];
            g_mySql_pwd = argv[5];
        }
    }
    else
    {
        if (dcmconfig.init((currentAppPath + "/config/DcmServerConfig.cfg").c_str()))
        {
            OFString dir = dcmconfig.getStoreDir()->front();
            Task_Dir = dir + "/Task/1";
            Error_Dir = dir + "/Task/error";
            //Log_Dir    = dir + "/log";
            g_ImageDir = dir + "/Images";
            //int pos = dir.find_last_of("/");
            //if (pos > -1)
            //{
            //    Log_Dir = dir.substr(0, pos) + "/log";
            //}
            //else
            //{
            //    int pos = dir.find_last_of("\\");
            //    if (pos > -1)
            //    {
            //        Log_Dir = dir.substr(0, pos) + "/log";
            //    }
            //}
            //OFString strIP("127.0.0.1"), strUser("root"), strPwd("root"), strDadaName("HIT");
            g_mySql_IP = dcmconfig.getSqlServer();
            g_mySql_username = dcmconfig.getSqlusername();
            g_mySql_pwd = dcmconfig.getSqlpwd();
            g_mySql_dbname = dcmconfig.getSqldbname();
        }
        else
        {
            Task_Dir = currentAppPath + "/DCM_SAVE/Task/1";// +currentStudyInstanceUID + ".ini";
            Error_Dir = currentAppPath + "/DCM_SAVE/Task/error";
            g_ImageDir = currentAppPath + "/DCM_SAVE/Images";
            Log_Dir = currentAppPath + "/log";
        }
    }

    app.printMessage("Log_Dir:");
    app.printMessage(Log_Dir.c_str());
    if (!OFStandard::dirExists(Log_Dir))
    {
        CreatDir(Log_Dir);
    }

    OFString logfilename = Log_Dir + "/SaveDcmInfoDb.log";//"/home/zyq/code/C++/DicomScuApp/DicomSCU/bin/Debug/dcmtk_storescu";

    const char *pattern = "%D{%Y-%m-%d %H:%M:%S.%q} %i %T %5p: %M %m%n";//https://support.dcmtk.org/docs/classdcmtk_1_1log4cplus_1_1PatternLayout.html
    OFunique_ptr<dcmtk::log4cplus::Layout> layout(new dcmtk::log4cplus::PatternLayout(pattern));
    dcmtk::log4cplus::SharedAppenderPtr logfile(new dcmtk::log4cplus::FileAppender(logfilename, STD_NAMESPACE ios::app));
    dcmtk::log4cplus::Logger log = dcmtk::log4cplus::Logger::getRoot();

    logfile->setLayout(OFmove(layout));
    //log.removeAllAppenders();
    log.addAppender(logfile);
    if (argc > 6)//qt set (argc[10])
    {
        log.setLogLevel(atoi(argv[6]));
    }
    else
    {
        log.setLogLevel(30000);
    }
    tempstr = "";
    for (int i = 0; i < argc; i++)
    {
        tempstr += argv[i];
        tempstr += " ";
    }
    OFLOG_INFO(SaveDcmInfoDbLogger, "---------argv[]:" + tempstr + " ----------------------");
    OFLOG_INFO(SaveDcmInfoDbLogger, "---------currentAppPath:" + currentAppPath + " ----------------------");

    if (!OFStandard::dirExists(Error_Dir))
    {
        OFStandard::createDirectory(Error_Dir, currentAppPath + "/DCM_SAVE/Task/");//CreatDir(log_dir);
    }
    OFList<OFString> list_file_ini;

    while (true)
    {
        list_file_ini.clear();
#ifndef _WIN32
        travel_files((char*)Task_Dir.c_str(),"ini",list_file_ini);
#else
        SearchDirFile(Task_Dir, "ini", list_file_ini);
#endif
        //Sleep(1);
        if (list_file_ini.size() > 0)
        {
            OFListIterator(OFString) iter = list_file_ini.begin();
            OFListIterator(OFString) enditer = list_file_ini.end();
            while (iter != enditer)
            {
                if (!SaveDcmInfo2Db(*iter, &dcmconfig))
                {
                    OFString filename;
                    OFStandard::getFilenameFromPath(filename, *iter);
                    if (OFStandard::copyFile(*iter, Task_Dir + "/" + filename), OFTrue)
                    {
                        OFStandard::deleteFile(*iter);
                    }
                }
                ++iter;
            }
        }
    }
    if (g_pMariaDb != NULL)
    {
        delete g_pMariaDb;
        g_pMariaDb = NULL;
    }
    return  1;
}

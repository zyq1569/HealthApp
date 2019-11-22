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


#include "Units.h"


#if defined (HAVE_WINDOWS_H) || defined(HAVE_FNMATCH_H)
#define PATTERN_MATCHING_AVAILABLE
#endif

#define OFFIS_CONSOLE_APPLICATION "storescu"

static OFLogger SaveDcmInfoDbLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

/* default application titles */
#define APPLICATIONTITLE     "STORESCU"
#define PEERAPPLICATIONTITLE "ANY-SCP"

static OFBool opt_showPresentationContexts = OFFalse;
static OFBool opt_abortAssociation = OFFalse;
static OFCmdUnsignedInt opt_maxReceivePDULength = ASC_DEFAULTMAXPDU;
static OFCmdUnsignedInt opt_maxSendPDULength = 0;

static E_TransferSyntax opt_networkTransferSyntax = EXS_Unknown;
static E_FileReadMode opt_readMode = ERM_autoDetect;

static OFBool opt_scanDir = OFTrue;
static OFBool opt_recurse = OFTrue;
static const char *opt_scanPattern = "";

static OFBool opt_haltOnUnsuccessfulStore = OFFalse;
static OFBool unsuccessfulStoreEncountered = OFFalse;
static int lastStatusCode = STATUS_Success;

static OFBool opt_proposeOnlyRequiredPresentationContexts = OFFalse; //修改为True
static OFBool opt_combineProposedTransferSyntaxes = OFFalse;

static OFCmdUnsignedInt opt_repeatCount = 1;
static OFCmdUnsignedInt opt_inventPatientCount = 25;
static OFCmdUnsignedInt opt_inventStudyCount = 50;
static OFCmdUnsignedInt opt_inventSeriesCount = 100;
static OFBool opt_inventSOPInstanceInformation = OFFalse;
static OFBool opt_correctUIDPadding = OFFalse;
static OFString patientNamePrefix("OFFIS^TEST_PN_");   // PatientName is PN (maximum 16 chars)
static OFString patientIDPrefix("PID_"); // PatientID is LO (maximum 64 chars)
static OFString studyIDPrefix("SID_");   // StudyID is SH (maximum 16 chars)
static OFString accessionNumberPrefix;   // AccessionNumber is SH (maximum 16 chars)
static OFBool opt_secureConnection = OFFalse; /* default: no secure connection */
static const char *opt_configFile = NULL;
static const char *opt_profileName = NULL;
T_DIMSE_BlockingMode opt_blockMode = DIMSE_BLOCKING;
int opt_dimse_timeout = 0;
int opt_acse_timeout = 30;
OFCmdSignedInt opt_socket_timeout = 60;

static HMariaDb *g_pMariaDb = NULL;
static OFString g_ImageDir = "";

#ifdef WITH_ZLIB
static OFCmdUnsignedInt opt_compressionLevel = 0;
#endif

#ifdef WITH_OPENSSL
static int         opt_keyFileFormat = SSL_FILETYPE_PEM;
static OFBool      opt_doAuthenticate = OFFalse;
static const char *opt_privateKeyFile = NULL;
static const char *opt_certificateFile = NULL;
static const char *opt_passwd = NULL;
#if OPENSSL_VERSION_NUMBER >= 0x0090700fL
static OFString    opt_ciphersuites(TLS1_TXT_RSA_WITH_AES_128_SHA ":" SSL3_TXT_RSA_DES_192_CBC3_SHA);
#else
static OFString    opt_ciphersuites(SSL3_TXT_RSA_DES_192_CBC3_SHA);
#endif
static const char *opt_readSeedFile = NULL;
static const char *opt_writeSeedFile = NULL;
static DcmCertificateVerification opt_certVerification = DCV_requireCertificate;
static const char *opt_dhparam = NULL;
#endif

// User Identity Negotiation
static T_ASC_UserIdentityNegotiationMode opt_identMode = ASC_USER_IDENTITY_NONE;
static OFString opt_user;
static OFString opt_password;
static OFString opt_identFile;
static OFBool opt_identResponse = OFFalse;

static OFCondition  addStoragePresentationContexts(T_ASC_Parameters *params, OFList<OFString> &sopClasses);

static OFCondition  cstore(T_ASC_Association *assoc, const OFString &fname);

static OFBool  findSOPClassAndInstanceInFile(const char *fname, char *sopClass, char *sopInstance);

static OFCondition  configureUserIdentityRequest(T_ASC_Parameters *params);

static OFCondition  checkUserIdentityResponse(T_ASC_Parameters *params);

/* helper macro for converting stream output to a string */
#define CONVERT_TO_STRING(output, string) \
    optStream.str(""); \
    optStream.clear(); \
    optStream << output << OFStringStream_ends; \
    OFSTRINGSTREAM_GETOFSTRING(optStream, string)

#define SHORTCOL 4
#define LONGCOL 19

//-------------------------------------------------------------------------------------
//OFBool CreatDir(OFString dir)
//{
//    if (!OFStandard::dirExists(dir))
//    {
//#if HAVE_WINDOWS_H
//        if (_mkdir(dir.c_str()) == -1)
//#else
//        if( mkdir( dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ) == -1 )
//#endif
//        {
//            OFLOG_ERROR(SaveDcmInfoDbLogger, "mkdir :" + dir + "  .error!");
//            return OFFalse;
//        }
//        else
//        {
//            return OFTrue;
//        }
//    }
//    return OFTrue;
//}

//OFString GetCurrentDir()
//{
//    static OFString CurrentDir = "";
//    if (CurrentDir != "")
//    {
//        return CurrentDir;
//    }
//    OFString tempstr, path;
//#if HAVE_WINDOWS_H
//    char szFullPath[MAX_PATH];
//    ZeroMemory(szFullPath, MAX_PATH);
//    ::GetModuleFileName(NULL, szFullPath, MAX_PATH);
//    path = szFullPath;
//#endif
//    CurrentDir = OFStandard::getDirNameFromPath(tempstr, path);
//    return CurrentDir;
//}

uint searchDirectoryRecursivelyAndRecord(const OFFilename &directory,
    OFList<OFFilename> &fileList,
    const OFFilename &pattern = "",
    const OFFilename &dirPrefix = "",
    const OFBool recurse = OFTrue)
{
    const size_t initialSize = fileList.size();
    OFFilename dirName, pathName, tmpString;
    OFStandard::combineDirAndFilename(dirName, dirPrefix, directory);
#ifdef HAVE_WINDOWS_H
    /* check whether given directory exists */
    if (OFStandard::dirExists(dirName))
    {
#if defined(WIDE_CHAR_FILE_IO_FUNCTIONS) && defined(_WIN32)
        /* check whether to use the wide-char version of the API function */
        if (dirName.usesWideChars())
        {
            HANDLE handle;
            WIN32_FIND_DATAW data;
            /* check whether file pattern is given */
            if (!pattern.isEmpty())
            {
                /* first, search for matching files on this directory level */
                handle = FindFirstFileW(combineDirAndFilename(tmpString, dirName, pattern, OFTrue /*allowEmptyDirName*/).getWideCharPointer(), &data);
                if (handle != INVALID_HANDLE_VALUE)
                {
                    do {
                        /* avoid leading "." */
                        if (wcscmp(dirName.getWideCharPointer(), L".") == 0)
                            pathName.set(data.cFileName, OFTrue /*convert*/);
                        else
                            combineDirAndFilename(pathName, directory, data.cFileName, OFTrue /*allowEmptyDirName*/);
                        /* ignore directories and the like */
                        if (fileExists(combineDirAndFilename(tmpString, dirPrefix, pathName, OFTrue /*allowEmptyDirName*/)))
                            fileList.push_back(pathName);
                    } while (FindNextFileW(handle, &data));
                    FindClose(handle);
                }
            }
            /* then search for _any_ file/directory entry */
            handle = FindFirstFileW(combineDirAndFilename(tmpString, dirName, L"*.*", OFTrue /*allowEmptyDirName*/).getWideCharPointer(), &data);
            if (handle != INVALID_HANDLE_VALUE)
            {
                do {
                    /* filter out current and parent directory */
                    if ((wcscmp(data.cFileName, L".") != 0) && (wcscmp(data.cFileName, L"..") != 0))
                    {
                        /* avoid leading "." */
                        if (wcscmp(dirName.getWideCharPointer(), L".") == 0)
                            pathName.set(data.cFileName, OFTrue /*convert*/);
                        else
                            combineDirAndFilename(pathName, directory, data.cFileName, OFTrue /*allowEmptyDirName*/);
                        if (dirExists(combineDirAndFilename(tmpString, dirPrefix, pathName, OFTrue /*allowEmptyDirName*/)))
                        {
                            /* recursively search sub directories */
                            if (recurse)
                                searchDirectoryRecursively(pathName, fileList, pattern, dirPrefix, recurse);
                        }
                        else if (pattern.isEmpty())
                        {
                            /* add filename to the list (if no pattern is given) */
                            fileList.push_back(pathName);
                        }
                    }
                } while (FindNextFileW(handle, &data));
                FindClose(handle);
            }
        } else
#endif
            /* otherwise, use the conventional 8-bit characters version */
        {
            HANDLE handle;
            WIN32_FIND_DATAA data;
            /* check whether file pattern is given */
            if (!pattern.isEmpty())
            {
                /* first, search for matching files on this directory level */
                handle = FindFirstFileA(OFStandard::combineDirAndFilename(tmpString, dirName, pattern, OFTrue /*allowEmptyDirName*/).getCharPointer(), &data);
                if (handle != INVALID_HANDLE_VALUE)
                {
                    do {
                        /* avoid leading "." */
                        if (strcmp(dirName.getCharPointer(), ".") == 0)
                            pathName.set(data.cFileName);
                        else
                            OFStandard::combineDirAndFilename(pathName, directory, data.cFileName, OFTrue /*allowEmptyDirName*/);
                        /* ignore directories and the like */
                        if (OFStandard::fileExists(OFStandard::combineDirAndFilename(tmpString, dirPrefix, pathName, OFTrue /*allowEmptyDirName*/)))
                            fileList.push_back(pathName);
                    } while (FindNextFileA(handle, &data));
                    FindClose(handle);
                }
            }
            /* then search for _any_ file/directory entry */
            handle = FindFirstFileA(OFStandard::combineDirAndFilename(tmpString, dirName, "*.*", OFTrue /*allowEmptyDirName*/).getCharPointer(), &data);
            if (handle != INVALID_HANDLE_VALUE)
            {
                do
                {
                    /* filter out current and parent directory */
                    if ((strcmp(data.cFileName, ".") != 0) && (strcmp(data.cFileName, "..") != 0))
                    {
                        /* avoid leading "." */
                        if (strcmp(dirName.getCharPointer(), ".") == 0)
                            pathName.set(data.cFileName);
                        else
                            OFStandard::combineDirAndFilename(pathName, directory, data.cFileName, OFTrue /*allowEmptyDirName*/);
                        if (OFStandard::dirExists(OFStandard::combineDirAndFilename(tmpString, dirPrefix, pathName, OFTrue /*allowEmptyDirName*/)))
                        {
                            /* recursively search sub directories */
                            if (recurse)
                                searchDirectoryRecursivelyAndRecord(pathName, fileList, pattern, dirPrefix, recurse);
                        }
                        else if (pattern.isEmpty())
                        {
                            /* add filename to the list (if no pattern is given) */
                            fileList.push_back(pathName);
                        }
                    }
                } while (FindNextFileA(handle, &data));
                FindClose(handle);
            }
        }
    }
#else
    /* try to open the directory */
    DIR *dirPtr = opendir(dirName.getCharPointer());
    if (dirPtr != NULL)
    {
        struct dirent *entry = NULL;
#ifdef HAVE_READDIR_R
        dirent d = {};
        while (!readdir_r(dirPtr, &d, &entry) && entry)
#else
        while ((entry = readdir(dirPtr)) != NULL)
#endif
        {
            /* filter out current (".") and parent directory ("..") */
            if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
            {
                /* avoid leading "." */
                if (strcmp(dirName.getCharPointer(), ".") == 0)
                    pathName = entry->d_name;
                else
                    combineDirAndFilename(pathName, directory, entry->d_name, OFTrue /*allowEmptyDirName*/);
                if (dirExists(combineDirAndFilename(tmpString, dirPrefix, pathName, OFTrue /*allowEmptyDirName*/)))
                {
                    /* recursively search sub directories */
                    if (recurse)
                        searchDirectoryRecursively(pathName, fileList, pattern, dirPrefix, recurse);
                }
                else
                {
#ifdef HAVE_FNMATCH_H
                    /* check whether filename matches pattern */
                    if ((pattern.isEmpty()) || (fnmatch(pattern.getCharPointer(), entry->d_name, FNM_PATHNAME) == 0))
#else
                    /* no pattern matching, sorry :-/ */
#endif
                    fileList.push_back(pathName);
                }
            }
        }
        closedir(dirPtr);
    }
#endif
    /* return number of added files */
    return fileList.size() - initialSize;
}
void writesendfile(OFString filename, OFList<OFString> inputFiles);
//_beginthread( BounceProc, 0, &ThreadNr );  
void searchdicomfile(OFString inputdir)
{
    OFList<OFString> inputFiles;
    OFList<OFFilename> filenameList;
    /* iterate over all input filenames/directories */
    //for (int i = 3; i <= paramCount; i++)
    {
        //cmd.getParam(i, paramString);
        /* search directory recursively (if required) */
        if (OFStandard::dirExists(inputdir))
        {
            //if (opt_scanDir)
            searchDirectoryRecursivelyAndRecord(inputdir, filenameList, opt_scanPattern, "" /*dirPrefix*/, opt_recurse);
            //else
            //   OFLOG_WARN(storescuLogger, "ignoring directory because option --scan-directories is not set: " << inputdir);
        }
        //else
        //    inputFiles.push_back(paramString);
    }



    /* call the real function */
    //const size_t result = searchDirectoryRecursively(directory, filenameList, pattern, dirPrefix, recurse);
    /* copy all list entries to reference parameter */
    OFListIterator(OFFilename) iter = filenameList.begin();
    OFListIterator(OFFilename) last = filenameList.end();
    while (iter != last)
    {
        inputFiles.push_back(OFSTRING_GUARD((*iter).getCharPointer()));
        ++iter;
    }
    uint size = inputFiles.size();
    OFString file_send_name = GetCurrentDir() + "\\index.txt";
    writesendfile(file_send_name, inputFiles);
    /* check whether there are any input files at all */
}
void writesendfile(OFString filename, OFList<OFString> inputFiles)
{
    using namespace std;
    ofstream savedcmfile;
    char *pfilename = (char *)filename.c_str();
    savedcmfile.open(filename.c_str(), ios::out | ios::app); //ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建
    OFListIterator(OFString) iter = inputFiles.begin();
    OFListIterator(OFString) last = inputFiles.end();
    while (iter != last)
    {
        savedcmfile << iter->c_str() << "\n";
        ++iter;
    }
    savedcmfile.close();//关闭文件
}

void readSendFile(OFList<OFString> &inputFiles)
{
    using namespace std;
    char buffer[256];
    fstream out;
    OFString filename = GetCurrentDir() + "\\index.txt";
    out.open(filename.c_str(), ios::in);
    //cout<<"com.txt"<<" 的内容如下:"<<endl;
    while (!out.eof())
    {
        out.getline(buffer, 256, '\n');//getline(char *,int,char) 表示该行字符达到256个或遇到换行就结束
        /*cout<<buffer<<endl;*/
        OFString file_path = OFString(buffer);
        if (file_path.length() > 3)
        {
            inputFiles.push_back(OFString(buffer));
        }
    }
    out.close();
}

void getSendFile(OFString dicomDir, OFList<OFString> &inputFiles)
{
    OFString filename = GetCurrentDir() + "\\index.txt";
    if (OFStandard::fileExists(filename))
    {
        readSendFile(inputFiles);
    }
    else
    {
        searchdicomfile(dicomDir);
        readSendFile(inputFiles);
    }

}

struct SendParm
{
    int argc;
    char **argv;

};
OFBool thread_end = OFFalse;
long long int dicom_total_count = 0;
long long int dicom_send_count = 0;

//            OFString Studydescription = "studydescription=";

struct  HStudyInfo
{
    OFString StudyInstanceUID, StudyPatientName, StudyPatientId, StudySex, StudyID, PatientNameEnglish;
    OFString StudyAge, PatientBirth, StudyState, StudyDateTime, StudyModality, StudyManufacturer, StudyInstitutionName;
    OFString Seriesuid, studydescription, Seriesdescription, Seriesnumber, Sopinstanceuid, instanceNumber;
};

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
                //static OFBool binster = OFTrue;
                //if (binster)
                //{
                //    StudyInfo.push_back("\n");
                //    binster = OFFalse;
                //}
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
                        str = imageinfo+"\n";
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
        str += "patientbirthdata=" + dcminfo.PatientBirth + "\n";
        str += "studydatetime=" + dcminfo.StudyDateTime + "\n";
        str += "modality=" + dcminfo.StudyModality + "\n";
        str += "manufacturer=" + dcminfo.StudyManufacturer + "\n";

        str += "institutionname=" + dcminfo.StudyInstitutionName + "\n";
        str += "studydescription=" + dcminfo.Seriesdescription + "\n";
        str += "manufacturer=" + dcminfo.StudyManufacturer + "\n";
        str += "studydescription=" + dcminfo.studydescription + "\n";
        inifile.fputs(str.c_str());

        inifile.fputs("[SERIES]\n");
        str = dcminfo.Seriesnumber + "|";
        str += dcminfo.Seriesdescription + "|" + dcminfo.Seriesuid+"\n";
        str += "[image]\n";
        str += dcminfo.instanceNumber + "|" + dcminfo.Sopinstanceuid;
        inifile.fputs(str.c_str());
        inifile.fclose();
    }

    return OFTrue;
}
OFBool SaveDcmInfo2Db(OFString filename, DcmConfigFile *configfile)
{
    //UINT64 uuid = CreateGUID();
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
        try
        {
            if (StudyInfo.StudyInstanceUID.length() < 1)
            {
                OFLOG_ERROR(SaveDcmInfoDbLogger, "NO StudyInstanceUID filename:" + filename);
                return OFFalse;
            }
            OFString studyinifile = g_ImageDir + GetStudyHashDir(StudyInfo.StudyInstanceUID) + "\\" + StudyInfo.StudyInstanceUID + "\\" + StudyInfo.StudyInstanceUID + ".ini";
            if (!OFStandard::fileExists(studyinifile))
            {
                if (g_pMariaDb == NULL)
                {
                    OFString strIP("127.0.0.1"), strUser("root"), strPwd("root"), strDadaName("HIT");
                    //if (configfile != NULL)
                    //{
                    //    strIP = configfile->getSqlServer();
                    //    strUser = configfile->getSqlusername();
                    //    strPwd = configfile->getSqlpass();
                    //    strDadaName = configfile->getSqldbname();
                    //}
#ifdef _UNICODE
                    g_pMariaDb = new HMariaDb(W2S(strIP.GetBuffer()).c_str(), W2S(strUser.GetBuffer()).c_str(), \
                        W2S(strPwd.GetBuffer()).c_str(), W2S(strDadaName.GetBuffer()).c_str());///*"127.0.0.1"*/"root", "root", "HIT");
#else
                    g_pMariaDb = new HMariaDb(strIP.c_str(), strUser.c_str(), \
                        strPwd.c_str(), strDadaName.c_str());///*"127.0.0.1"*/"root", "root", "HIT");

#endif
                }
                OFString querysql = "select * from h_patient where PatientID = '" + StudyInfo.StudyPatientId + "'";
                g_pMariaDb->query(querysql.c_str());
                ResultSet * rs = g_pMariaDb->QueryResult();
                OFString PatientIdentity;
                if (rs == NULL)
                {
                    //int rows = rs->countRows();
                    char uuid[64];
                    sprintf(uuid, "%llu", CreateGUID());
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
                    querysql += StudyInfo.StudySex;
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
                        //for (size_t i = 0; i < row.size(); i++)
                        //{
                        //sdata += row[i] + " | ";
                        PatientIdentity = row[0].c_str();
                        // }
                        //sdata += "\r\n";
                    }
                }

                querysql = "select * from H_study where StudyUID = '" + StudyInfo.StudyInstanceUID + "'";
                g_pMariaDb->query(querysql.c_str());
                rs = g_pMariaDb->QueryResult();
                if (rs == NULL)
                {
                    char uuid[64];
                    sprintf(uuid, "%llu", CreateGUID());
                    OFString StudyIdentity = uuid;
                    OFString strsql = "insert into H_study (StudyIdentity,StudyID,StudyUID,PatientIdentity,";
                    strsql += " StudyDateTime,StudyModality,InstitutionName,StudyManufacturer,StudyState) value(";
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
                    strsql += "','dcm');";
                    g_pMariaDb->execute(strsql.c_str());
                }
                //OFStandard::deleteFile(filename);
                OFLOG_INFO(SaveDcmInfoDbLogger, "SaveDcmInfo2Db filename:" + filename);

                //-------------------------------------------------------------------------------
                //更新预约表检查状态
                querysql = "update H_order set StudyState = 3 where StudyUID = '" + StudyInfo.StudyInstanceUID + "';";
                g_pMariaDb->execute(querysql.c_str());
                OFLOG_INFO(SaveDcmInfoDbLogger, "update table H_order:" + StudyInfo.StudyInstanceUID);
                //-------------------------------------------------------------------------------
            }
            if (SaveDcmInfoFile(StudyInfo, studyinifile))
            {
                OFStandard::deleteFile(filename);
            }
        }
        catch (...)
        {
            //OFLOG_ERR(storescuLogger, "---------argv[]:" + tempstr + " ----------------------");
            OFLOG_ERROR(SaveDcmInfoDbLogger, "SaveDcmInfo2Db filename:" + filename);
            return OFFalse;
        }
    }
    return OFTrue;
}
int main(int argc, char *argv[])
{
    OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION, "DICOM file Info 2 DB", rcsid);
    OFString tempstr, path = argv[0];
    int pos = 0;
#ifdef HAVE_WINDOWS_H
    pos = path.find_last_of('\\');
#else
    //to do add!
#endif
    if (pos < 1)
    {
#ifdef HAVE_WINDOWS_H
        OFString message = " start app by commline:";
        app.printMessage(message.c_str());
        path = GetCurrWorkingDir();
        app.printMessage("GetCurrWorkingDir filePath:");
        app.printMessage(path.c_str());
#else
        //to do add!
#endif
    }
    OFString currentAppPath = OFStandard::getDirNameFromPath(tempstr, path);
    OFString log_dir = currentAppPath/*OFStandard::getDirNameFromPath(tempstr, path)*/ + "/log";
    app.printMessage("log_dir:");
    app.printMessage(log_dir.c_str());
    if (!OFStandard::dirExists(log_dir))
    {
        CreatDir(log_dir);
    }

    OFString logfilename = log_dir + "/SaveDcmInfoDb.log";//"/home/zyq/code/C++/DicomScuApp/DicomSCU/bin/Debug/dcmtk_storescu";

    const char *pattern = "%D{%Y-%m-%d %H:%M:%S.%q} %i %T %5p: %M %m%n";//https://support.dcmtk.org/docs/classdcmtk_1_1log4cplus_1_1PatternLayout.html
    OFunique_ptr<dcmtk::log4cplus::Layout> layout(new dcmtk::log4cplus::PatternLayout(pattern));
    dcmtk::log4cplus::SharedAppenderPtr logfile(new dcmtk::log4cplus::FileAppender(logfilename, STD_NAMESPACE ios::app));
    dcmtk::log4cplus::Logger log = dcmtk::log4cplus::Logger::getRoot();

    logfile->setLayout(OFmove(layout));
    //log.removeAllAppenders();
    log.addAppender(logfile);
    tempstr = "";
    for (int i = 0; i < argc; i++)
    {
        tempstr += argv[i];
        tempstr += " ";
    }
    OFLOG_INFO(SaveDcmInfoDbLogger, "---------argv[]:" + tempstr + " ----------------------");

    OFString ini_dir, ini_error_dir;
    static DcmConfigFile dcmconfig;
    if (dcmconfig.init((currentAppPath + "/config/DcmServerConfig.cfg").c_str()))
    {
        OFString dir = dcmconfig.getStoreDir()->front();
        ini_dir = dir + "/Task/1";
        ini_error_dir = dir + "/Task/error";
        g_ImageDir = dir;
    }
    else
    {
        ini_dir = currentAppPath + "/DCM_SAVE/Task/1";// +currentStudyInstanceUID + ".ini";
        ini_error_dir = currentAppPath + "/DCM_SAVE/Task/error";
        g_ImageDir = currentAppPath + "/DCM_SAVE/Images";
    }

    if (!OFStandard::dirExists(ini_error_dir))
    {
        OFStandard::createDirectory(ini_error_dir, currentAppPath + "/DCM_SAVE/Task/");//CreatDir(log_dir);
    }
    OFList<OFString> list_file_ini;

    while (true)
    {
        list_file_ini.clear();
        SearchDirFile(ini_dir, "ini", list_file_ini);
        Sleep(2);
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
                    if (OFStandard::copyFile(*iter, ini_error_dir + "/" + filename), OFTrue)
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
}
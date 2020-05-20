/*
 *
 *  Copyright (C) 1993-2018, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmqrdb
 *
 *  Author:  Andrew Hewett
 *
 *  Purpose: Image Server Central Test Node (ctn) Main Program
 *
 */


//--vld ???????
//#include <SDKDDKVer.h>
//#include <stdio.h>
//#include <tchar.h>
//#include <vld.h>
////--vld ???????
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#define INCLUDE_CSTDARG
#define INCLUDE_CTIME
#define INCLUDE_LIBC
#include "dcmtk/ofstd/ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
END_EXTERN_C

#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmnet/dicom.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/dcasccff.h"
#include "dcmtk/dcmqrdb/dcmqropt.h"
#include "dcmtk/dcmqrdb/dcmqrcnf.h"
#include "dcmtk/dcmqrdb/dcmqrsrv.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmdata/dcostrmz.h"    /* for dcmZlibCompressionLevel */
#include "dcmtk/ofstd/ofgrp.h"
#include "dcmtk/ofstd/ofpwd.h"

#ifdef WITH_SQL_DATABASE
#include "dcmtk/dcmqrdbx/dcmqrdbq.h"
#else
#include "dcmtk/dcmqrdb/dcmqrdbi.h"
#endif

#ifdef WITH_ZLIB
#include <zlib.h>                      /* for zlibVersion() */
#endif
#ifdef DCMTK_ENABLE_CHARSET_CONVERSION
#include "dcmtk/ofstd/ofchrenc.h"      /* for OFCharacterEncoding */
#endif

#ifndef OFFIS_CONSOLE_APPLICATION
#define OFFIS_CONSOLE_APPLICATION "dcmqrscp"
#endif


//-------add add 201806
#include "dcmtk/oflog/fileap.h"
#ifdef HAVE_WINDOWS_H
#include <direct.h>      /* for _mkdir() */
#endif

#include "Units.h"
//#include <vld.h>
//--------------------

static OFLogger dcmqrscpLogger = OFLog::getLogger("dcmtk.dcmqr.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

#define APPLICATIONTITLE "DCMQRSCP"

const char *opt_configFileName = DEFAULT_CONFIGURATION_DIR "DcmServerConfig.cfg";
OFBool      opt_checkFindIdentifier = OFFalse;
OFBool      opt_checkMoveIdentifier = OFFalse;
OFCmdUnsignedInt opt_port = 0;
OFCmdUnsignedInt g_query_port = 1668;

#define SHORTCOL 4
#define LONGCOL 22

OFBool             opt_forkedChild = OFFalse;
#if defined(HAVE_FORK) || defined(_WIN32)
OFBool             opt_forkMode = OFTrue/*OFFalse*/;//�޸�ΪĬ�϶���̴���
#endif

OFString g_imagedir;
DcmQueryRetrieveConfig g_config;
DcmQueryRetrieveOptions g_options;
DcmAssociationConfiguration g_asccfg;
DcmQueryRetrieveSCP *g_scp;


static void mangleAssociationProfileKey(OFString& key)
{
    for (size_t ui = 0; ui < key.size();)
    {
        if (!isspace(key[ui]))
        {
            key[ui] = toupper(key[ui]);
            ++ui;
        }
        else key.erase(ui, 1);
    }
}

#ifdef _WIN32
//
//����̨��������
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
//--------------------
DWORD WINAPI threadFunc(LPVOID threadNum);

int call_net_qrscp(int startcode);
int start_newqrscp(int startcode);

int g_argc;
char **g_argv;
CRITICAL_SECTION g_Lock;
enum ThreadState { Init = -1, Listen, Run, Wait, Suspend };
int g_thread_nums;
struct ThreadInfo
{
    HANDLE hand;
    OFString threadID;
    int  state;//-1,0,1//init run Suspend
};
#define MAX_THREAD_NU 500
ThreadInfo g_thread_hand[MAX_THREAD_NU];

OFBool g_bSuspend = OFTrue;// OFFalse;
HANDLE g_event = NULL;
//---------use Hthread ---
unsigned __stdcall MainQuerScpThread(void *argv)
{
    int thread_num = 0;
    while (TRUE)
    {
        thread_num = 0;
        bool bNoListenThread = true;
        bool bStartNewThread = true;
        for (int i = 0; i < MAX_THREAD_NU; i++)
        {
            if (NULL != g_thread_hand[i].hand)
            {
                if (Wait == g_thread_hand[i].state)
                {
                    OFLOG_INFO(dcmqrscpLogger, "qrscp:ThreadID(" + g_thread_hand[i].threadID + ") Suspend!");
                    if (g_bSuspend)
                    {
                        ::SuspendThread(g_thread_hand[i].hand);
                    }
                    g_thread_hand[i].state = Suspend;
                }
                if (bNoListenThread && Listen == g_thread_hand[i].state)
                {
                    bNoListenThread = false;
                }
                thread_num++;
            }
        }
        if (thread_num == MAX_THREAD_NU - 1)
        {
            g_thread_nums = MAX_THREAD_NU;
        }
        if (bNoListenThread)
        {
            for (int i = 0; i < MAX_THREAD_NU; i++)
            {
                if (NULL != g_thread_hand[i].hand && Suspend == g_thread_hand[i].state)
                {
                    if (g_bSuspend)
                    {
                        ::ResumeThread(g_thread_hand[i].hand);
                    }
                    g_thread_hand[i].state = Listen;
                    OFLOG_INFO(dcmqrscpLogger, "qrscp:ThreadID(" + g_thread_hand[i].threadID + ") Listen!");
                    bStartNewThread = false;
                    break;
                }
            }
            if (bStartNewThread)
            {
                start_newqrscp(0);
            }
        }
        Sleep(1);
    }
    return 0;
}

class HQuerScp : public DcmQueryCallBack
{
public:
    HQuerScp()
    {

    }
    ~HQuerScp()
    {
        for (int i = 0; i < MAX_THREAD_NU; i++)
        {
            if (NULL != g_thread_hand[i].hand)
            {
                CloseHandle(g_thread_hand[i].hand);
                g_thread_hand[i].hand = NULL;
            }
        }
        OFLOG_FATAL(dcmqrscpLogger, "CloseHandle All g_thread_hand.");
    }
    int CallBack(void* statecode)
    {
        OFString ThreadID = longToString(::GetCurrentThreadId());
        for (int i = 0; i < MAX_THREAD_NU; i++)
        {
            if (NULL != g_thread_hand[i].hand && ThreadID == g_thread_hand[i].threadID)
            {
                g_thread_hand[i].state = Run;
                OFLOG_INFO(dcmqrscpLogger, "qrscp:ThreadID(" + ThreadID + ") run!");
                break;
            }
        }
        return 1;
    }
};
static HQuerScp g_QueryScp;
#endif
// qrscp way:
//OFCondition DcmQueryRetrieveSCP::waitForAssociation(T_ASC_Network * theNet)
//OFCondition DcmQueryRetrieveSCP::handleAssociation(T_ASC_Association * assoc, OFBool correctUIDPadding)
//OFCondition DcmQueryRetrieveSCP::dispatch(T_ASC_Association *assoc, OFBool correctUIDPadding)
//DcmQueryRetrieveSCP::moveSCP
//cond = DIMSE_moveProvider(assoc, presID, request,
//moveCallback, &context, options_.blockMode_, options_.dimse_timeout_);
//DcmQueryRetrieveMoveContext::callbackHandler(
//    DcmQueryRetrieveMoveContext::startMoveRequest(
int RunQRSCP(int argc, char *argv[])
{
#ifdef _WIN32
    g_argc = argc;
    g_argv = argv;
#endif
    //##################---------------------------------------------------------------------------------
    const char *pattern = "%D{%Y-%m-%d %H:%M:%S.%q} %i %T %5p: %M %m%n";//https://support.dcmtk.org/docs/classdcmtk_1_1log4cplus_1_1PatternLayout.html
    OFString temps, path = argv[0];
    int pos = 0;

    OFString currentAppPath = OFStandard::getDirNameFromPath(temps, path);
    OFString log_dir = currentAppPath + "/log";
    //opt_configFileName
    OFString confilepath = currentAppPath + "/config/DcmServerConfig.cfg";
    opt_configFileName = confilepath.c_str();

    if (!OFStandard::dirExists(log_dir))
    {
        CreatDir(log_dir);
    }

    OFString logfilename = log_dir + "/DcmQueryRetrieveSCP.log";//"/home/zyq/code/C++/DicomScuApp/DicomSCU/bin/Debug/dcmtk_storescu";

    OFunique_ptr<dcmtk::log4cplus::Layout> layout(new dcmtk::log4cplus::PatternLayout(pattern));
    dcmtk::log4cplus::SharedAppenderPtr logfile(new dcmtk::log4cplus::FileAppender(logfilename, STD_NAMESPACE ios::app));
    dcmtk::log4cplus::Logger log = dcmtk::log4cplus::Logger::getRoot();

    logfile->setLayout(OFmove(layout));
    log.removeAllAppenders();
    log.addAppender(logfile);

    temps = "";
    for (int i = 0; i < argc; i++)
    {
        temps += argv[i];
        temps += " ";
    }
    OFLOG_INFO(dcmqrscpLogger, "---------argv[]:" + temps + " ----------------------");
    OFLOG_INFO(dcmqrscpLogger, "-----$$------DcmNet dcmstoreqrscp start run!---------$$------------");
    //###################------------------------------------------------------------------
    OFCondition cond = EC_Normal;
    //OFCmdUnsignedInt overridePort = 0;
    OFCmdUnsignedInt overrideMaxPDU = 0;

    OFStandard::initializeNetwork();
    /* evaluate command line */
    prepareCmdLineArgs(argc, argv, OFFIS_CONSOLE_APPLICATION);

    /* print resource identifier */
    OFLOG_DEBUG(dcmqrscpLogger, rcsid << OFendl);

    /* read config file */
    if (access(opt_configFileName, R_OK) < 0)
    {
        OFLOG_FATAL(dcmqrscpLogger, "cannot access " << opt_configFileName << ": "
            << OFStandard::getLastSystemErrorCode().message());
        //return 10;
    }

    int imagedirsize = 1;
    //OFString imagedir;
    OFString clientAE;
    int clientPort;
    //DcmQueryRetrieveConfig config;
    g_options.maxAssociations_ = 30;
    opt_port = 1668;
    if (!g_config.init(opt_configFileName))
    {
        OFLOG_FATAL(dcmqrscpLogger, "bad config file: " << opt_configFileName);
    }
    else
    {
        OFString filepath = opt_configFileName;
        OFLOG_INFO(dcmqrscpLogger, "----opt_configFileName:" + filepath);
        g_options.maxAssociations_ = g_config.getMaxAssociations();
        opt_port = g_config.getNetworkTCPPort();
    }
    if (argc > 4) // 0:exe dir  1: scp port 2:Image Dir 3 :client AE 4:client port
    {
        g_query_port = atoi(argv[1]);
        opt_port = g_query_port;
        g_imagedir = argv[2];
        clientAE = argv[3];
        clientPort = atoi(argv[4]);
    }

    if (opt_port == 0)
    {
        opt_port = g_query_port; /* not set, use default */
        OFString msg = "----default QR NetworkTCPPort:" + longToString(opt_port);
        OFLOG_INFO(dcmqrscpLogger, msg);
    }
    else
    {
        OFString msg = "----QR NetworkTCPPort:" + longToString(opt_port);
        OFLOG_INFO(dcmqrscpLogger, msg);
    }

    g_options.maxPDU_ = g_config.getMaxPDUSize();
    if (g_options.maxPDU_ == 0)
        g_options.maxPDU_ = ASC_DEFAULTMAXPDU; /* not set, use default */
    if (g_options.maxPDU_ < ASC_MINIMUMPDUSIZE || g_options.maxPDU_ > ASC_MAXIMUMPDUSIZE)
    {
        OFLOG_FATAL(dcmqrscpLogger, "invalid MaxPDUSize in config file");
        return 0;
    }
    if (overrideMaxPDU > 0)
        g_options.maxPDU_ = overrideMaxPDU;

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
        OFLOG_ERROR(dcmqrscpLogger, "no data dictionary loaded, check environment variable: "
            << DCM_DICT_ENVIRONMENT_VARIABLE);
    }

#ifdef HAVE_FORK
    //if (cmd.findOption("--single-process"))
    //    options.singleProcess_ = OFTrue;
    //if (cmd.findOption("--fork"))//Ĭ������linux Ϊ fork
    g_options.singleProcess_ = OFFalse;//OFTrue;//OFFalse;
#endif

#ifdef _WIN32
    OFString temp_str;
    cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, g_options.acse_timeout_, &g_options.net_);
    if (cond.bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "cannot initialize network: " << DimseCondition::dump(temp_str, cond));
        return 0;
    }

    /* drop root privileges now and revert to the calling user id (if we are running as setuid root) */
    if (OFStandard::dropPrivileges().bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "setuid() failed, maximum number of processes/threads for uid already running.");
        return 0;
    }
    //----------------------------------------
    for (int i = 0; i < MAX_THREAD_NU; i++)
    {
        g_thread_hand[i].hand = NULL;
        g_thread_hand[i].state = Init;
    }
    InitializeCriticalSection(&g_Lock);
    start_newqrscp(1);
    unsigned uThreadID;
    HANDLE hand = (HANDLE)_beginthreadex(NULL,// security
        0,// stack size
        MainQuerScpThread,
        NULL,           // arg list
        CREATE_SUSPENDED,  // so we can later call ResumeThread()
        &uThreadID);
    ResumeThread(hand);
    HANDLE vent = CreateEvent(NULL, FALSE, FALSE, NULL);
    while (WaitForSingleObject(vent, INFINITE) != WAIT_OBJECT_0)
    {
        //stop here
    }
    CloseHandle(hand);
    OFStandard::shutdownNetwork();
#else
    OFString temp_str;
    cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, g_options.acse_timeout_, &g_options.net_);
    if (cond.bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "cannot initialize network: " << DimseCondition::dump(temp_str, cond));
        return 0;
    }

    /* drop root privileges now and revert to the calling user id (if we are running as setuid root) */
    if (OFStandard::dropPrivileges().bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "setuid() failed, maximum number of processes/threads for uid already running.");
        return 0;
    }
    // use linear index database (index.dat)
    DcmQueryRetrieveIndexDatabaseHandleFactory factory(&g_config);
    DcmQueryRetrieveSCP scp(g_config, g_options, factory, g_asccfg);
    scp.setDatabaseFlags(opt_checkFindIdentifier, opt_checkMoveIdentifier);
    if (g_imagedir.length() > 0 && argc > 5)
    {
        scp.SetDcmDirSize(1);
        scp.SetDcmDir(0, g_imagedir);
        QueryClientInfo qc;
        qc.AEtitle = argv[3];
        qc.port = atoi(argv[4]);
        qc.IpAddress = argv[5];
        scp.SetQueryClientSize(1);
        scp.SetQueryClient(&qc, 0);
        if (argc > 9)
        {
            MySqlInfo sql;
            sql.IpAddress = argv[6];
            sql.SqlName = argv[7];
            sql.SqlUserName = argv[8];
            sql.SqlPWD = argv[9];
            scp.SetMysql(&sql);
        }
        if (argc > 10)
        {
            //other query client info
            int size = atoi(argv[10]);
            scp.SetQueryClientSize(size+1);
            for (int i = 0; i < size; i++)
            {
                QueryClientInfo qc;
                int delt = i * 3;
                qc.AEtitle = argv[11 + delt];
                qc.port = atoi(argv[12 + delt]);
                qc.IpAddress = argv[13 + delt];
                scp.SetQueryClient(&qc, i+1);
            }
        }
    }
    /* loop waiting for associations */
    while (cond.good())
    {
        cond = scp.waitForAssociation(g_options.net_);
        if (!g_options.singleProcess_)
            scp.cleanChildren();  /* clean up any child processes */
    }
    cond = ASC_dropNetwork(&g_options.net_);
    if (cond.bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "cannot drop network: " << DimseCondition::dump(temp_str, cond));
        return 10;
    }
    OFStandard::shutdownNetwork();
#endif
    return 0;
}

#ifdef _WIN32

unsigned __stdcall QueryThread(void *argv)
{
    DcmQueryRetrieveIndexDatabaseHandleFactory factory(&g_config);
    DcmQueryRetrieveSCP scp(g_config, g_options, factory, g_asccfg);
    scp.setDatabaseFlags(opt_checkFindIdentifier, opt_checkMoveIdentifier);
    if (g_imagedir.length() > 0 && g_argc > 5)
    {
        scp.SetDcmDirSize(1);
        scp.SetDcmDir(0, g_imagedir);
        QueryClientInfo qc;
        qc.AEtitle = g_argv[3];
        qc.port = atoi(g_argv[4]);
        qc.IpAddress = g_argv[5];
        scp.SetQueryClientSize(1);
        scp.SetQueryClient(&qc, 0);
        if (g_argc > 9)
        {
            MySqlInfo sql;
            sql.IpAddress = g_argv[6];
            sql.SqlName = g_argv[7];
            sql.SqlUserName = g_argv[8];
            sql.SqlPWD = g_argv[9];
            scp.SetMysql(&sql);
        }
        if (g_argc > 10)
        {
            //other query client info
            int size = atoi(g_argv[10]);
            scp.SetQueryClientSize(size+1);
            for (int i = 0; i < size; i++)
            {
                QueryClientInfo qc;
                int delt = i * 3;
                qc.AEtitle = g_argv[11 + delt];
                qc.port = atoi(g_argv[12 + delt]);
                qc.IpAddress = g_argv[13 + delt];
                scp.SetQueryClient(&qc, i+1);
            }
        }
    }
    OFString ThreadID = longToString(GetCurrentThreadId());
    int index = -1;
    for (int i = 0; i < MAX_THREAD_NU; i++)
    {
        if (ThreadID == g_thread_hand[i].threadID)
        {
            index = i;
            break;
        }
    }
    //SuspendThread(hand);
    while (true)
    {
        if (Listen == g_thread_hand[index].state)
        {
            OFLOG_INFO(dcmqrscpLogger, "qrscp:ThreadID(" + ThreadID + ") Listening for client!");
            scp.waitForAssociation_win32_thread(g_options.net_, &g_QueryScp);
            OFLOG_INFO(dcmqrscpLogger, "qrscp:ThreadID(" + ThreadID + ") finish!");
            g_thread_hand[index].state = Wait;
            OFLOG_INFO(dcmqrscpLogger, "qrscp:ThreadID(" + ThreadID + ") Wait!");
        }
    }
    return 0;
}

int start_newqrscp(int startcode)
{
    //_beginthreadex
    HANDLE hand = NULL;
    //DWORD   dwExitCode;
    OFString newThreadID;
    unsigned  uiThreadID = 0;
    hand = (HANDLE)_beginthreadex(NULL,// security
        0,// stack size
        QueryThread,
        NULL,           // arg list
        CREATE_SUSPENDED,  // so we can later call ResumeThread()
        &uiThreadID);

    if (hand == NULL)
    {
        //fail
        OFLOG_INFO(dcmqrscpLogger, "CreateNewThread fial! ID:" + newThreadID);
        //to do add ? fail to  help call
    }
    else
    {
        //ok g_Lock.Lock();
        //�����ٽ��� (����)
        EnterCriticalSection(&g_Lock);
        newThreadID = longToString(uiThreadID);
        bool flag = false;
        while (!flag)
        {
            for (int i = 0; i < MAX_THREAD_NU; i++)
            {
                if (NULL == g_thread_hand[i].hand)
                {
                    g_thread_hand[i].threadID = newThreadID;
                    g_thread_hand[i].hand = hand;
                    g_thread_hand[i].state = Listen;
                    OFLOG_INFO(dcmqrscpLogger, "qrscp:new ThreadID(" + newThreadID + ")start Listen!");
                    ResumeThread(hand);
                    flag = true;
                    break;
                }
            }
        }
        //����
        LeaveCriticalSection(&g_Lock);
        OFLOG_INFO(dcmqrscpLogger, "qrscp:CreateNewThread thread ID:" + newThreadID);
    }
    return 1;
}

int call_net_qrscp(int startcode)
{
    OFString ThreadID = longToString(::GetCurrentThreadId());
    for (int i = 0; i < MAX_THREAD_NU; i++)
    {
        if (NULL != g_thread_hand[i].hand && ThreadID == g_thread_hand[i].threadID)
        {
            g_thread_hand[i].state = Run;
            OFLOG_INFO(dcmqrscpLogger, "qrscp:ThreadID(" + ThreadID + ") run!");
            break;
        }
    }
    return 1;
}

#endif

//free OFCondition nextFindResponse(
// find data -->OFCondition DcmQueryRetrieveFindContext::startFindRequestFromSql(
int main(int argc, char *argv[])
{
    //OFList<OFString> data;
    //OFString dir = "F:\temp\HealthApp\bin\win32\DCM_SAVE\Images\106\164\1.2.840.113619.2.66.2158408118.16050010109105933.20000";
    //OFString uid = "/1.2.840.113619.2.66.2158408118.16050010109105933.20000.ini";
    //ReadStudyInfo(dir + uid, dir, data);
    //char *p = new char[100];
    RunQRSCP(argc, argv);
    return 0;
}


//DWORD   dwExitCode;
//WaitForSingleObject(g_thread_hand[i].hand, INFINITE);
//GetExitCodeThread(g_thread_hand[i].hand, &dwExitCode);
//if (STILL_ACTIVE != dwExitCode)
//{
//    OFLOG_INFO(dcmqrscpLogger, "free Thread hand/ThreadID(" + g_thread_hand[i].threadID + ") exited with code :" + longToString(dwExitCode));
//    CloseHandle(g_thread_hand[i].hand);
//    g_thread_hand[i].hand = NULL;
//}
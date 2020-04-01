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
//--------------------

static OFLogger dcmqrscpLogger = OFLog::getLogger("dcmtk.dcmqr.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

#define APPLICATIONTITLE "DCMQRSCP"

const char *opt_configFileName = DEFAULT_CONFIGURATION_DIR "DcmServerConfig.cfg";
OFBool      opt_checkFindIdentifier = OFFalse;
OFBool      opt_checkMoveIdentifier = OFFalse;
OFCmdUnsignedInt opt_port = 0;
OFCmdUnsignedInt g_worklist_port = 1668;

#define SHORTCOL 4
#define LONGCOL 22

OFBool             opt_forkedChild = OFFalse;
#if defined(HAVE_FORK) || defined(_WIN32)
OFBool             opt_forkMode = OFTrue/*OFFalse*/;//修改为默认多进程处理
#endif

OFString g_imagedir;
DcmQueryRetrieveConfig g_config;
DcmQueryRetrieveOptions g_options;
DcmAssociationConfiguration g_asccfg;

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
DWORD WINAPI threadFunc(LPVOID threadNum);

int call_net_qrscp(int a, int b);

int g_argc;
char **g_argv;

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
        g_worklist_port = atoi(argv[1]);
        opt_port = g_worklist_port;
        g_imagedir = argv[2];
        clientAE = argv[3];
        clientPort = atoi(argv[4]);
    }

    if (opt_port == 0)
    {
        opt_port = 1668; /* not set, use default */
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
        return 10;
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
    //if (cmd.findOption("--fork"))//默认设置linux 为 fork
    g_options.singleProcess_ = OFFalse;//OFTrue;//OFFalse;
#endif

#ifdef _WIN32
    //-------------------------------- need thread
    OFString temp_str;
    cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, g_options.acse_timeout_, &g_options.net_);
    if (cond.bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "cannot initialize network: " << DimseCondition::dump(temp_str, cond));
        return 10;
    }

    /* drop root privileges now and revert to the calling user id (if we are running as setuid root) */
    if (OFStandard::dropPrivileges().bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "setuid() failed, maximum number of processes/threads for uid already running.");
        return 10;
    }
    //----------------------------------------
    call_net_qrscp(1, 1);
    while (true)
    {
        Sleep(1);
    }
    OFStandard::shutdownNetwork();
#else 
    OFString temp_str;
    cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, g_options.acse_timeout_, &g_options.net_);
    if (cond.bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "cannot initialize network: " << DimseCondition::dump(temp_str, cond));
        return 10;
    }

    /* drop root privileges now and revert to the calling user id (if we are running as setuid root) */
    if (OFStandard::dropPrivileges().bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "setuid() failed, maximum number of processes/threads for uid already running.");
        return 10;
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
DWORD WINAPI threadFunc(LPVOID threadNum)
{
    DcmQueryRetrieveIndexDatabaseHandleFactory factory(&g_config);
    //DcmQueryRetrieveOptions options;
    //options = g_options;
    //options.maxAssociations_ = g_options.maxAssociations_;
    //options.maxPDU_ = g_options.maxPDU_;
    //ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, options.acse_timeout_, &options.net_);
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
    }
    scp.waitForAssociation_win32_thread(g_options.net_, &call_net_qrscp);
    //ASC_dropNetwork(&options.net_);
    OFLOG_INFO(dcmqrscpLogger, "threadID Exit");
    return 0;
}
int call_net_qrscp(int i, int j)
{
    DWORD threadID = 0;
    HANDLE hand = NULL;
    if ((hand = CreateThread(NULL, 0, threadFunc, NULL, 0, &threadID)) == NULL)
    {
        //fail
        OFLOG_INFO(dcmqrscpLogger, "CreateNewThread fial! ID:" + longToString(threadID));
    }
    else
    {
        //ok
        OFString id = longToString(threadID);
        OFLOG_INFO(dcmqrscpLogger, "CreateNewThread threadID:" + longToString(threadID));
    }
    CloseHandle(hand);
    return 1;
}
#endif

int main(int argc, char *argv[])
{
    //OFList<OFString> data;
    //OFString dir = "F:\temp\HealthApp\bin\win32\DCM_SAVE\Images\106\164\1.2.840.113619.2.66.2158408118.16050010109105933.20000";
    //OFString uid = "/1.2.840.113619.2.66.2158408118.16050010109105933.20000.ini";
    //ReadStudyInfo(dir + uid, dir, data);
    return RunQRSCP(argc, argv);
}

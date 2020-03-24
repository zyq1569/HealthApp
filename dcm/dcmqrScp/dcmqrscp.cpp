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
    DcmQueryRetrieveOptions options;
    DcmAssociationConfiguration asccfg;

    OFStandard::initializeNetwork();
    /* evaluate command line */
    prepareCmdLineArgs(argc, argv, OFFIS_CONSOLE_APPLICATION);

#ifdef HAVE_FORK
    OFCommandLine cmd;
    cmd.beginOptionBlock();
    if (cmd.findOption("--single-process"))
        options.singleProcess_ = OFTrue;
    if (cmd.findOption("--fork"))
        options.singleProcess_ = OFFalse;
    cmd.endOptionBlock();
#endif

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
    OFString imagedir;
    OFString clientAE;
    int clientPort;
    DcmQueryRetrieveConfig config;
    options.maxAssociations_ = 30;
    opt_port = 1668;
    if (!config.init(opt_configFileName))
    {
        OFLOG_FATAL(dcmqrscpLogger, "bad config file: " << opt_configFileName);
    }
    else
    {
        OFString filepath = opt_configFileName;
        OFLOG_INFO(dcmqrscpLogger, "----opt_configFileName:" + filepath);
        options.maxAssociations_ = config.getMaxAssociations();
        opt_port = config.getNetworkTCPPort();
    }
    if (argc > 4) // 0:exe dir  1: scp port 2:Image Dir 3 :client AE 4:client port
    {
        g_worklist_port = atoi(argv[1]);
        opt_port = g_worklist_port;
        imagedir = argv[2];
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

    options.maxPDU_ = config.getMaxPDUSize();
    if (options.maxPDU_ == 0)
        options.maxPDU_ = ASC_DEFAULTMAXPDU; /* not set, use default */
    if (options.maxPDU_ < ASC_MINIMUMPDUSIZE || options.maxPDU_ > ASC_MAXIMUMPDUSIZE)
    {
        OFLOG_FATAL(dcmqrscpLogger, "invalid MaxPDUSize in config file");
        return 10;
    }
    if (overrideMaxPDU > 0)
        options.maxPDU_ = overrideMaxPDU;

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
        OFLOG_WARN(dcmqrscpLogger, "no data dictionary loaded, check environment variable: "
            << DCM_DICT_ENVIRONMENT_VARIABLE);
    }

    OFString temp_str;
    cond = ASC_initializeNetwork(NET_ACCEPTORREQUESTOR, (int)opt_port, options.acse_timeout_, &options.net_);
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
    DcmQueryRetrieveIndexDatabaseHandleFactory factory(&config);
    DcmQueryRetrieveSCP scp(config, options, factory, asccfg);
    scp.setDatabaseFlags(opt_checkFindIdentifier, opt_checkMoveIdentifier);
    if ( imagedir.length() >0 && argc > 5)
    {
        scp.SetDcmDirSize(1);
        scp.SetDcmDir(0, imagedir);
        QueryClientInfo qc;
        qc.AEtitle = argv[3];
        qc.port = atoi(argv[4]);
        qc.IpAddress = argv[5];
        scp.SetQueryClientSize(1);
        scp.SetQueryClient(&qc,0);
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
        cond = scp.waitForAssociation(options.net_);
        if (!options.singleProcess_)
            scp.cleanChildren();  /* clean up any child processes */
    }

    cond = ASC_dropNetwork(&options.net_);
    if (cond.bad())
    {
        OFLOG_FATAL(dcmqrscpLogger, "cannot drop network: " << DimseCondition::dump(temp_str, cond));
        return 10;
    }

    OFStandard::shutdownNetwork();
    return 0;
}
int main(int argc, char *argv[])
{
    return RunQRSCP(argc, argv);
}

/*
*
*  Copyright (C) 1996-2010, OFFIS e.V.
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
*  Module:  dcmwlm
*
*  Author:  Thomas Wilkens
*
*  Purpose: Basic work list management service class provider based on a
*           set of files as data source.
*
*/

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmnet/dicom.h"
#include "dcmtk/ofstd/ofcmdln.h"
#include "dcmtk/dcmwlm/wltypdef.h"
#include "dcmtk/dcmdata/dcxfer.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "wlcefs.h"
#include "dcmtk/dcmwlm/wlds.h"
#include "dcmtk/dcmwlm/wldsfs.h"
#ifdef HAVE_WINDOWS_H
#include <direct.h>      /* for _mkdir() */
#endif
//-------add add 201806
#include "dcmtk/oflog/fileap.h"
//#include "Units.h"
#include "DcmConfig.h"
#include "Units.h"

//--------------------
#define OFFIS_CONSOLE_APPLICATION "wlmscpfs"

OFBool CreatDir_(OFString dir)
{
    if (!OFStandard::dirExists(dir))
    {
#ifdef HAVE_WINDOWS_H
        if (_mkdir(dir.c_str()) == -1)
#else
        if( mkdir( dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ) == -1 )
#endif
        {
            OFLOG_ERROR(DCM_dcmwlmLogger, "mkdir :" + dir + "  .error!");
            return OFFalse;
        }
        else
        {
            return OFTrue;
        }
    }
    return OFTrue;
}

OFString GetCurrentWorkingDir()
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

// release 1024 --fork
// debug 1024 -d -s
int main(int argc, char *argv[])
{
    //--------------------增加日志文件的方式----------------------------------------------------------------
    const char *pattern = "%D{%Y-%m-%d %H:%M:%S.%q} %i %T %5p: %M %m%n";//https://support.dcmtk.org/docs/classdcmtk_1_1log4cplus_1_1PatternLayout.html
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
        DCMWLM_INFO(message);
        path = GetCurrentWorkingDir();
#else
        //to do add!
#endif
    }
    OFString currentAppPath = OFStandard::getDirNameFromPath(tempstr, path);
    OFString log_dir = currentAppPath + "/log";
    if (!OFStandard::dirExists(log_dir))
    {
        CreatDir_(log_dir);
    }

    OFString logfilename = log_dir + "/WorklistSCP.log";//"/home/zyq/code/C++/DicomScuApp/DicomSCU/bin/Debug/dcmtk_storescu";

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
    //DCM_dcmwlmLogger.setLogLevel(3000);
    OFLOG_INFO(DCM_dcmwlmLogger, "---------argv[]:" + tempstr + " ----------------------");

    static DcmConfigFile config;
    OFString configdir = currentAppPath + "/config/DcmServerConfig.cfg";
    config.init(configdir.c_str());
    OFString TCPport;
    TCPport = argv[1];

    pos = tempstr.find("forked-child");
    if (pos > 0)
    {
        TCPport = argv[2];
        if (argc == 7)
        {
            config.setSqlServer(argv[3]);
            config.setSqldbname(argv[4]);
            config.setSqlusername(argv[5]);
            config.setSqlpwd(argv[6]);
        }
    }
    else
    {
        if (argc == 6)
        {
            config.setSqlServer(argv[2]);
            config.setSqldbname(argv[3]);
            config.setSqlusername(argv[4]);
            config.setSqlpwd(argv[5]);
        }
    }

    if (TCPport == "")
    {
        TCPport = longToString(config.getWorklistScpPort());
    }
    OFLOG_INFO(DCM_dcmwlmLogger, configdir + "---Load OK!------WorklistScpPort:" + TCPport);
    OFLOG_INFO(DCM_dcmwlmLogger, "---------DcmConfigFile info:--------------");
    OFLOG_INFO(DCM_dcmwlmLogger, "   WorklistScpPort:" + TCPport);
    OFString mysqlserver = config.getSqlServer();
    OFLOG_INFO(DCM_dcmwlmLogger, "   mysql:" + mysqlserver);
    OFString mysqldbname = config.getSqldbname();
    OFLOG_INFO(DCM_dcmwlmLogger, "   mysql dbname:" + mysqldbname);
    OFLOG_INFO(DCM_dcmwlmLogger, "-----$$------DcmNet storescp start run!---------$$------------");
    //-----------------------------------------------------------------------------------------------------
    // Initialize object which provides a connection to the data source
    WlmDataSourceFileSystem *dataSource = new WlmDataSourceFileSystem();
    dataSource->SetDcmConfig(&config);
    static char parm[3][128];
    memset(parm[0], 0, 128);
    memset(parm[1], 0, 128);
    memset(parm[2], 0, 128);
    OFString strtemp = argv[0];
    memcpy(parm[0], strtemp.c_str(), strtemp.length());
    memcpy(parm[1], TCPport.c_str(), TCPport.length());
    strtemp = "--fork";
    memcpy(parm[2], strtemp.c_str(), strtemp.length());
    // Initialize and provide service. After having terminated free memory.
    WlmConsoleEngineFileSystem *consoleEngine;
    if (argc >= 6)
    {
        int count = 3;
        char *new_argv[3];// = new char*[count]; WlmFileSystemInteractionManager
        for (int i = 0; i < count; i++)
        {
            new_argv[i] = parm[i];
        }
        consoleEngine = new WlmConsoleEngineFileSystem(count, new_argv, OFFIS_CONSOLE_APPLICATION, dataSource);
    }
    else
    {
        consoleEngine = new WlmConsoleEngineFileSystem(argc, argv, OFFIS_CONSOLE_APPLICATION, dataSource);
    }
    //int lev = DCM_dcmwlmLogger.getLogLevel();
    int result = consoleEngine->StartProvidingService();

    // Free memory
    delete consoleEngine;
    delete dataSource;

    return(result);
}

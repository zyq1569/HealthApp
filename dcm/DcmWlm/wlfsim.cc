/*
 *
 *  Copyright (C) 1996-2018, OFFIS e.V.
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
 *  Purpose: Class for managing file system interaction.
 *
 */

// ----------------------------------------------------------------------------

#include "dcmtk/config/osconfig.h"

#define INCLUDE_CLIMITS
#include "dcmtk/ofstd/ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>  // for struct DIR, opendir()
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>    // for struct DIR, opendir()
#endif
END_EXTERN_C

#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/ofstd/ofconsol.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofdate.h"
#include "dcmtk/ofstd/oftime.h"
#include "dcmtk/ofstd/oftypes.h"
#include "dcmtk/dcmnet/dicom.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcitem.h"
#include "dcmtk/dcmdata/dcvrda.h"
#include "dcmtk/dcmdata/dcvrtm.h"
#include "dcmtk/dcmdata/dcmatch.h"
#include "dcmtk/dcmwlm/wltypdef.h"
#include "dcmtk/dcmwlm/wlds.h"
#include "dcmtk/dcmdata/dctk.h"
#include <stdio.h>
#include <stdlib.h>

#include "dcmtk/dcmwlm/wlfsim.h"

//add mysql driver  后期需要将数据库的处理独立模块
#include"HMariaDb.h"
#include "Units.h"
//#include "dcmtk/ofstd/ofdatime.h"
//#include <objbase.h>
//////////////
// ----------------------------------------------------------------------------

class WlmFileSystemInteractionManager::MatchingKeys
{
    MatchingKeys(int)
    {
        keys.push_back(OFMake_pair(DCM_PatientName, OFTrue));
        keys.push_back(OFMake_pair(DCM_ResponsiblePerson, OFTrue));
        keys.push_back(OFMake_pair(DCM_ResponsiblePersonRole, OFTrue));
        keys.push_back(OFMake_pair(DCM_PatientID, OFFalse));
        keys.push_back(OFMake_pair(DCM_AccessionNumber, OFTrue));
        keys.push_back(OFMake_pair(DCM_RequestedProcedureID, OFTrue));
        keys.push_back(OFMake_pair(DCM_ReferringPhysicianName, OFTrue));
        keys.push_back(OFMake_pair(DCM_PatientSex, OFTrue));
        keys.push_back(OFMake_pair(DCM_RequestingPhysician, OFTrue));
        keys.push_back(OFMake_pair(DCM_AdmissionID, OFTrue));
        keys.push_back(OFMake_pair(DCM_RequestedProcedurePriority, OFTrue));
        keys.push_back(OFMake_pair(DCM_PatientBirthDate, OFTrue));
        keys.push_back(OFMake_pair(DCM_IssuerOfPatientID, OFTrue));
        combinedKeys.push_back(OFMake_pair(DCM_StudyDate, DCM_StudyTime));
        sequenceKeys.push_back(OFMake_pair(DCM_ScheduledProcedureStepSequence, MatchingKeys()));
        sequenceKeys.back().second.keys.push_back(OFMake_pair(DCM_ScheduledStationAETitle, OFFalse));
        sequenceKeys.back().second.keys.push_back(OFMake_pair(DCM_Modality, OFFalse));
        sequenceKeys.back().second.keys.push_back(OFMake_pair(DCM_ScheduledPerformingPhysicianName, OFTrue));
        sequenceKeys.back().second.combinedKeys.push_back(OFMake_pair(DCM_ScheduledProcedureStepStartDate, DCM_ScheduledProcedureStepStartTime));
    }

public:
    static const MatchingKeys root;
    MatchingKeys() {}
    OFVector<OFPair<DcmTagKey, OFBool> >       keys;
    OFVector<OFPair<DcmTagKey, DcmTagKey> >    combinedKeys;
    OFVector<OFPair<DcmTagKey, MatchingKeys> > sequenceKeys;
};

const WlmFileSystemInteractionManager::MatchingKeys WlmFileSystemInteractionManager::MatchingKeys::root(0);

// ----------------------------------------------------------------------------

WlmFileSystemInteractionManager::WlmFileSystemInteractionManager()
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : Constructor.
// Parameters   : none.
// Return Value : none.
: dfPath(""),
enableRejectionOfIncompleteWlFiles(OFTrue), calledApplicationEntityTitle(""),
matchingRecords(NULL), numOfMatchingRecords(0)
{
}

// ----------------------------------------------------------------------------

WlmFileSystemInteractionManager::~WlmFileSystemInteractionManager()
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : Destructor.
// Parameters   : none.
// Return Value : none.
{
}

// ----------------------------------------------------------------------------

void WlmFileSystemInteractionManager::SetEnableRejectionOfIncompleteWlFiles(OFBool value)
// Date         : May 3, 2005
// Author       : Thomas Wilkens
// Task         : Set value in member variable.
// Parameters   : value - [in] The value to set.
// Return Value : none.
{
    enableRejectionOfIncompleteWlFiles = value;
}

// ----------------------------------------------------------------------------

OFCondition WlmFileSystemInteractionManager::ConnectToFileSystem(const OFString& dfPathv)
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : Connects to the worklist file system database.
// Parameters   : dfPathv - [in] Path to worklist file system database.
// Return Value : Indicates if the connection could be established or not.
{
    // check parameter
    if (dfPathv.empty())
    {
        DCMWLM_ERROR("Invalid parameters, cannot connect to worklist file system database");
        return(WLM_EC_CannotConnectToDataSource);
    }

    // copy value
    dfPath = dfPathv;

    // check if the specified path is existent and accessible for reading
    if (!OFStandard::dirExists(dfPath) || !OFStandard::isReadable(dfPath))
        return(WLM_EC_CannotConnectToDataSource);
    else
        return(EC_Normal);
}

// ----------------------------------------------------------------------------

OFCondition WlmFileSystemInteractionManager::DisconnectFromFileSystem()
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : Disconnects from the worklist file system database.
// Parameters   : none.
// Return Value : Indicates if the connection was disconnected successfully.
{
    return(EC_Normal);
}

// ----------------------------------------------------------------------------

OFBool WlmFileSystemInteractionManager::IsCalledApplicationEntityTitleSupported(const OFString& calledApplicationEntityTitlev)
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : Checks if the given called application entity title is supported. If this is the case,
//                OFTrue will be returned, else OFFalse.
// Parameters   : calledApplicationEntityTitlev - [in] The application entity title which shall be checked
//                                                for support. Valid pointer expected.
// Return Value : OFTrue  - The called application entity title is supported.
//                OFFalse - The called application entity title is not supported or it is not given.
{
    // copy value
    calledApplicationEntityTitle = calledApplicationEntityTitlev;

    // Determine complete path to the files that make up the data source.
    OFString fullPath(dfPath);
    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != PATH_SEPARATOR)
        fullPath += PATH_SEPARATOR;
    fullPath += calledApplicationEntityTitle;

    // in case the path is not existent, we need to return OFFalse
    if (!(OFStandard::dirExists(OFString(fullPath))))
    {
        DCMWLM_WARN("dfPath:in case the path is not existent "); //add zyq20190507
        //return(OFFalse);
    }
    // if we get to here, the path is existent and we need to return OFTrue
    return(OFTrue);
}
//将时间格式字符转换为数字格式时间
OFString DateTimeFormate(OFString datetime, OFString &date, OFString &time)
{
    OFString str(datetime);
    if (str.length() < 1)
    {
        str.clear();
        return str;
    }
    int pos = str.find('-');
    while (pos > 0)
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
// ----------------------------------------------------------------------------
/******************************************************************************************
参数
- s - dfr - dfp  D : \DCMTK\dcmtk - 3.6.0\dcmwlm\data\wlistdb  104
- s 是单独进程模式或者线程模式
unsigned long WlmFileSystemInteractionManager::DetermineMatchingRecords(DcmDataset *searchMask

scp 选择 wlmscpfs 工程 debug设置 成 - s - dfr - dfp  D : \DCMTK\dcmtk - 3.6.0\dcmwlm\data\wlistdb  104
代码在dcmtk - 3.6.0\dcmwlm\libsrc\wlfsim.cc

findscu 127.0.0.1 104 wlistqry.wl -aec OFFIS
************************************************************************************************/
//void GetWorklistData(OFList<DcmDataset *> &listDataset, DcmDataset *searchMask)
void WlmFileSystemInteractionManager::GetWorklistData(OFList<DcmDataset > &listDataset, DcmDataset *searchMask, DcmConfigFile *configfile)
{
    listDataset.clear();
    //DcmDataset *dataset = NULL;
    //1.读取设备查询条件信息
    OFString PatientName, PatientID, Studyid, StartDate, StartTime, EndDate, EndTime, Modality, AETitle;
    if (searchMask->findAndGetOFString(DCM_PatientName, PatientName).good())
    {
        if (PatientName.empty())
        {
            DCMWLM_INFO("searchMask no DCM_PatientName");
        }
    }
    if (searchMask->findAndGetOFString(DCM_PatientID, PatientID).good())
    {
        if (PatientID.empty())
        {
            DCMWLM_INFO("searchMask no DCM_PatientID");
        }
    }
    if (searchMask->findAndGetOFString(DCM_StudyID, Studyid).good())
    {
        if (Studyid.empty())
        {
            DCMWLM_INFO("searchMask no DCM_StudyID");
        }
    }
    DcmElement *ScheduledProcedureStepSequence;// = searchMask->getElement(DCM_ScheduledProcedureStepSequence);
    if (searchMask->findAndGetElement(DCM_ScheduledProcedureStepSequence, ScheduledProcedureStepSequence).good() || ((DcmSequenceOfItems*)ScheduledProcedureStepSequence)->card() != 1)
    {
        DcmItem *scheduledProcedureStepSequenceItem = ((DcmSequenceOfItems*)ScheduledProcedureStepSequence)->getItem(0);
        if (scheduledProcedureStepSequenceItem->findAndGetOFString(DCM_Modality, Modality).good())
        {
            if (Modality.empty())
            {
                DCMWLM_WARN("searchMask no DCM_Modality");
            }
        }
        //AETitle
        if (scheduledProcedureStepSequenceItem->findAndGetOFString(DCM_ScheduledStationAETitle, AETitle).good())
        {
            if (AETitle.empty())
            {
                DCMWLM_INFO("searchMask no DCM_ScheduledStationAETitle");
            }
        }
        //DCM_ScheduledProcedureStepStartDate
        if (scheduledProcedureStepSequenceItem->findAndGetOFString(DCM_ScheduledProcedureStepStartDate, StartDate).good())
        {
            if (StartDate.empty())
            {
                DCMWLM_INFO("searchMask no DCM_ScheduledProcedureStepStartDate");
            }
        }
        if (scheduledProcedureStepSequenceItem->findAndGetOFString(DCM_ScheduledProcedureStepStartTime, StartTime).good())
        {
            if (StartTime.empty())
            {
                DCMWLM_INFO("searchMask no DCM_ScheduledProcedureStepStartTime");
            }
        }
        if (scheduledProcedureStepSequenceItem->findAndGetOFString(DCM_ScheduledProcedureStepEndDate, EndDate).good())
        {
            if (EndDate.empty())
            {
                DCMWLM_INFO("searchMask no DCM_ScheduledProcedureStepEndDate");
            }
        }
        if (scheduledProcedureStepSequenceItem->findAndGetOFString(DCM_ScheduledProcedureStepEndTime, EndTime).good())
        {
            if (EndTime.empty())
            {
                DCMWLM_INFO("searchMask no DCM_ScheduledProcedureStepEndTime");
            }
        }
    }
    //2.准备数据库读取数据
    static bool initDb    = false;
    static bool useSqlite = false;
    OFString strIP("127.0.0.1"), strUser("root"), strPwd("root"), strDadaName("HIT");
    int sqltype = 0;
    static sqlite3 *g_pSqlite = NULL;
    if (!initDb)
    {
        GetSqlDbInfo(strIP, strDadaName,strUser,strPwd,sqltype);
        DCMWLM_ERROR("sqlinfo: " << strIP);
        if (strIP == "0.0.0.0")
        {
            useSqlite       = true;
            OFString appdir = GetCurrentDir() + "/hitSqlite.db";
            g_pSqlite       = OpenSqlite(appdir.c_str());
        }
    }
    //use sqlite (local)
    if (useSqlite)
    {      
        if (g_pSqlite == NULL)
        {
            DCMWLM_ERROR("OpenSqlite error: " << GetCurrentDir() + "/hitSqlite.db");
            return ;
        }
        //int count;//数据库查询的记录条数
        OFString sql = "select p.PatientID, p.PatientName, p.PatientSex, p.PatientBirthday,";
        sql          = sql + " s.StudyID, s.StudyUID, s.OrderDateTime, s.ScheduledDateTime, ";
        sql          = sql + " s.StudyModality, s.AETitle, s.StudyDescription ,s.StudyCode from h_patient p, h_order s where  s.StudyState = 1 and p.PatientIdentity=s.PatientIdentity ";
        if (!Modality.empty())
        {
            sql = sql + " and s.StudyModality = '" + Modality + "'";
        }
        int pos = StartDate.find('-');
        if (pos > 7)
        {
            OFString s = StartDate.substr(0, pos);
            OFString e = StartDate.substr(pos + 1);
            if (EndDate.empty() && e != s)
            {
                EndDate = e;
            }
            StartDate = s;
        }
        DCMWLM_DEBUG("StartDate:" + StartDate);
        DCMWLM_DEBUG("EndDate:" + EndDate);
        OFString tempSql = sql;
        if (!StartDate.empty())
        {
            pos = StartDate.find('-');
            if (pos < 1)
            {
                StartDate = StartDate.substr(0, 4) + "-" + StartDate.substr(4, 2) + "-" + StartDate.substr(6, 2);
            }
            sql = sql + " and s.ScheduledDateTime>= " + StartDate;
        }
        if (!EndDate.empty())
        {
            pos = EndDate.find('-');
            if (pos < 1)
            {
                EndDate = EndDate.substr(0, 4) + "-" + EndDate.substr(4, 2) + "-" + EndDate.substr(6, 2);
            }
            sql = sql + " and s.ScheduledDateTime<=" + EndDate;
        }
        if (!StartDate.empty() && !EndDate.empty())
        {
            int ST = atoi(StartDate.c_str());
            int ET = atoi(EndDate.c_str());
            if (ST > ET)
            {
                sql = tempSql + " and s.ScheduledDateTime>=" + EndDate;
                sql = sql + " and s.ScheduledDateTime<=" + StartDate;
            }
        }
        if (!PatientName.empty())
        {
            sql = sql + " and p.PatientName = '" + PatientName + "'";
        }//PatientID Studyid
        if (!PatientID.empty())
        {
            sql = sql + " and p.PatientID = '" + PatientID + "'";
        }
        if (!Studyid.empty())
        {
            sql = sql + " and s.StudyID = '" + Studyid + "'";
        }
        sql = sql + " ;";
        std::vector<std::string> param;
        std::vector<std::map<std::string, std::string>> result;
        OFString PatientIdentity;
        int res = SelectSqlite(g_pSqlite, sql.c_str(), param, result);
        int size = result.size();
        if (size > 0)
        {
            DcmDataset dataset;
            for (int i = 0; i < size; i++)
            {
                //!result[i].at("StudyModality").empty()
                if (!result[i].at("StudyUID").empty())
                {
                    dataset.putAndInsertString(DCM_StudyInstanceUID, result[i].at("StudyUID").c_str());
                }
                if (!result[i].at("PatientName").empty())
                {
                    dataset.putAndInsertString(DCM_PatientName, result[i].at("PatientName").c_str());
                }
                else
                {
                    dataset.putAndInsertString(DCM_PatientName, "unknow");
                }
                if (!result[i].at("PatientID").empty())
                {
                    dataset.putAndInsertString(DCM_PatientID, result[i].at("PatientID").c_str());
                }

                if (!result[i].at("StudyID").empty())
                {
                    dataset.putAndInsertString(DCM_AccessionNumber, result[i].at("StudyID").c_str());
                }

                if (!result[i].at("PatientBirthday").empty())
                {
                    dataset.putAndInsertString(DCM_PatientBirthDate, result[i].at("PatientBirthday").c_str());
                }
                //PatientSex
                if (!result[i].at("PatientSex").empty())
                {
                    dataset.putAndInsertString(DCM_PatientSex, result[i].at("PatientSex").c_str());
                }
                else
                {
                    dataset.putAndInsertString(DCM_PatientSex, "O");
                }
                //dataset.putAndInsertString(DCM_RequestingPhysician, "NEIER");
                //dataset.putAndInsertString(DCM_RequestedProcedureDescription, "EXAM78");
                //dataset->putAndInsertString(DCM_PatientSex, "W");
                DcmItem *ditem = NULL;
                if (dataset.findOrCreateSequenceItem(DCM_ScheduledProcedureStepSequence, ditem).good())
                {
                    if (!result[i].at("StudyModality").empty())
                    {
                        ditem->putAndInsertString(DCM_Modality, result[i].at("StudyModality").c_str());
                    }

                    if (!result[i].at("ScheduledDateTime").empty())
                    {
                        OFString str, date, time;
                        str = DbDateTimeToDateTimeFormate(result[i].at("ScheduledDateTime").c_str(), date, time);
                        ditem->putAndInsertString(DCM_ScheduledProcedureStepStartDate, date.c_str());
                        ditem->putAndInsertString(DCM_ScheduledProcedureStepStartTime, time.c_str());
                    }

                    if (!result[i].at("StudyCode").empty())
                    {
                        ditem->putAndInsertString(DCM_ScheduledProcedureStepID, result[i].at("StudyCode").c_str());
                    }
                    else
                    {
                        ditem->putAndInsertString(DCM_ScheduledProcedureStepID, "unknow");
                    }
                    if (!result[i].at("StudyDescription").empty())
                    {
                        ditem->putAndInsertString(DCM_ScheduledProcedureStepDescription, result[i].at("StudyDescription").c_str());
                    }
                    else
                    {
                        ditem->putAndInsertString(DCM_ScheduledProcedureStepDescription, "unknow");
                    }
                }
                //dataset.putAndInsertString(DCM_RequestedProcedureID, "RP34734H328");
                //dataset.putAndInsertString(DCM_RequestedProcedurePriority, "HIGH");
                listDataset.push_back(dataset);
            }
            DCMWLM_DEBUG("data info in database/sqlite:" + sql);
            return;
        }
        else
        {
            DCMWLM_WARN("No data info in database/sqlite:" + sql);
            return;
        }
    }
    //else 
    //use MariaDb
    static HMariaDb *pMariaDb = NULL;
    if (pMariaDb == NULL)
    {
        if (configfile != NULL)
        {
            strIP       = configfile->getSqlServer();
            strUser     = configfile->getSqlusername();
            strPwd      = configfile->getSqlpwd();
            strDadaName = configfile->getSqldbname();
        }
#ifdef _UNICODE
        pMariaDb = new HMariaDb(W2S(strIP.GetBuffer()).c_str(), W2S(strUser.GetBuffer()).c_str(), \
            W2S(strPwd.GetBuffer()).c_str(), W2S(strDadaName.GetBuffer()).c_str());///*"127.0.0.1"*/"root", "root", "HIT");
#else
        pMariaDb = new HMariaDb(strIP.c_str(), strUser.c_str(), \
            strPwd.c_str(), strDadaName.c_str());///*"127.0.0.1"*/"root", "root", "HIT");
#endif
    }
    //int count;//数据库查询的记录条数
    OFString sql = "select p.PatientID, p.PatientName, p.PatientSex, p.PatientBirthday,";
    sql = sql + " s.StudyID, s.StudyUID, s.OrderDateTime, s.ScheduledDateTime, ";
    sql = sql + " s.StudyModality, s.AETitle, s.StudyDescription ,s.StudyCode from h_patient p, h_order s where  s.StudyState = 1 and p.PatientIdentity=s.PatientIdentity ";
    if (!Modality.empty())
    {
        sql = sql + " and s.StudyModality = '" + Modality + "'";
    }
    int pos = StartDate.find('-');
    if (pos > 7)
    {
        OFString s = StartDate.substr(0, pos);
        OFString e = StartDate.substr(pos + 1, StartDate.length() - pos);
        if (EndDate.empty() && e!=s)
        {
            EndDate = e;
        }
        StartDate = s;
    }
    OFString tempSql = sql;
    if (!StartDate.empty())
    {
        StartDate = ToDateFormate(StartDate);
        if (!StartTime.empty())
        {
            StartDate += StartTime;
        }
        sql = sql + " and s.ScheduledDateTime>= " + StartDate;
    }
    if (!EndDate.empty())
    {
        EndDate = ToDateFormate(EndDate);
        if (!EndTime.empty())
        {
            EndDate += EndTime;
        }
        sql = sql + " and s.ScheduledDateTime<=" + EndDate;
    }
    if (!StartDate.empty() && !EndDate.empty())
    {
        int ST = atoi(StartDate.c_str());
        int ET = atoi(EndDate.c_str());
        if (ST > ET)
        {
            sql = tempSql + " and s.ScheduledDateTime>=" + EndDate;
            sql = sql + " and s.ScheduledDateTime<=" + StartDate;
        }
    }
    if (!PatientName.empty())
    {
        sql = sql + " and p.PatientName = '" + PatientName+"'";
    }//PatientID Studyid
    if (!PatientID.empty())
    {
        sql = sql + " and p.PatientID = '" + PatientID + "'";
    }
    if (!Studyid.empty())
    {
        sql = sql + " and s.StudyID = '" + Studyid + "'";
    }
    sql = sql + " ;";
    pMariaDb->query(sql.c_str());
    ResultSet * rs = pMariaDb->QueryResult();
    if (rs == NULL)
    {
        DCMWLM_INFO("No data info in database/MariaDb:"+sql);
        return;
    }
    else
    {
        std::vector<std::string> row;
        std::string sdata;
        int row_size = rs->countRows();
        int row_index = 0;
        while (rs->fetch(row))
        {
            //OFString temp;
            //for (size_t i = 0; i < row.size(); i++)
            //{
            //    sdata += row[i] + " | ";
            //    temp = row[i].c_str();
            //}
            //sdata += "\r\n";
            //DCMWLM_INFO(sdata.c_str());
            ////////////////////////////
            std::string patienName, StudyInstanceUID, PatientID, StudyID, StudyCode;
            std::string PatientBirthDate, PatientSex, StudyModality, StepStartDate, StudyDescription;

            //DcmDataset *dataset = new DcmDataset();
            DcmDataset dataset;

            //dataset->putAndInsertString(DCM_PatientName, "Doe^John");
            //dataset->putAndInsertString(DCM_AccessionNumber, "A00001");
            //dataset->putAndInsertString(DCM_PatientID, "A000001");
            //dataset->putAndInsertString(DCM_PatientBirthDate, "19991001");
            //dataset->putAndInsertString(DCM_PatientBirthTime, "20:22:20");
            //dataset->putAndInsertString(DCM_MedicalAlerts, "ABZESS");
            //dataset->putAndInsertString(DCM_Allergies, "BARIUMSULFAT");

            if (rs->getValue(row_index, "StudyUID", StudyInstanceUID))
            {
                dataset.putAndInsertString(DCM_StudyInstanceUID, StudyInstanceUID.c_str());
            }
            else
            {
                dataset.putAndInsertString(DCM_StudyInstanceUID, "1.2.276.0.179081.1207");
            }
            if (rs->getValue(row_index, "PatientName", patienName))
            {
                dataset.putAndInsertString(DCM_PatientName, patienName.c_str());
            }
            else
            {
                dataset.putAndInsertString(DCM_PatientName, "unknow");
            }
            if (rs->getValue(row_index, "PatientID", PatientID))
            {
                dataset.putAndInsertString(DCM_PatientID, PatientID.c_str());
            }
            else
            {
                //dataset->putAndInsertString(DCM_PatientID, "0001");
            }
            if (rs->getValue(row_index, "StudyID", StudyID))
            {
                dataset.putAndInsertString(DCM_AccessionNumber, StudyID.c_str());
            }
            else
            {
                //dataset->putAndInsertString(DCM_StudyInstanceUID, "1.2.276.0.179081.1207");
            }
            if (rs->getValue(row_index, "PatientBirthday", PatientBirthDate))
            {
                dataset.putAndInsertString(DCM_PatientBirthDate, PatientBirthDate.c_str());
            }
            else
            {
                //dataset->putAndInsertString(DCM_StudyInstanceUID, "1.2.276.0.179081.1207");
            }//PatientSex
            if (rs->getValue(row_index, "PatientSex", PatientSex))
            {
                dataset.putAndInsertString(DCM_PatientSex, PatientSex.c_str());
            }
            else
            {
                dataset.putAndInsertString(DCM_PatientSex, "O");
            }
            //dataset.putAndInsertString(DCM_RequestingPhysician, "NEIER");
            //dataset.putAndInsertString(DCM_RequestedProcedureDescription, "EXAM78");
            //dataset->putAndInsertString(DCM_PatientSex, "W");
            DcmItem *ditem = NULL;
            if (dataset.findOrCreateSequenceItem(DCM_ScheduledProcedureStepSequence, ditem).good())
            {
                if (rs->getValue(row_index, "StudyModality", StudyModality))
                {
                    ditem->putAndInsertString(DCM_Modality, StudyModality.c_str());
                }
                else
                {
                    ditem->putAndInsertString(DCM_Modality, Modality.c_str());
                }
                if (rs->getValue(row_index, "ScheduledDateTime", StepStartDate))
                {
                    OFString str, date, time;
                    str = DbDateTimeToDateTimeFormate(StepStartDate.c_str(), date, time);
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepStartDate, date.c_str());
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepStartTime, time.c_str());
                }
                else
                {
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepStartDate, "20190208");
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepStartTime, "21:44:00");
                }
                if (rs->getValue(row_index, "StudyCode", StudyCode))
                {
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepID, StudyCode.c_str());
                }
                else
                {
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepID, "unknow");
                }
                if (rs->getValue(row_index, "StudyDescription", StudyDescription))
                {
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepDescription, StudyDescription.c_str());
                }
                else
                {
                    ditem->putAndInsertString(DCM_ScheduledProcedureStepDescription, "unknow");
                    //ditem->putAndInsertString(DCM_ScheduledStationName, "STN345");
                    //ditem->putAndInsertString(DCM_ScheduledProcedureStepLocation, "B55P56");
                }
            }
            //dataset.putAndInsertString(DCM_RequestedProcedureID, "RP34734H328");
            //dataset.putAndInsertString(DCM_RequestedProcedurePriority, "HIGH");
            ///////////////////////////
            listDataset.push_back(dataset);
            row_index++;
        }
        DCMWLM_INFO("sql database:" + sql);
    }
}

unsigned long WlmFileSystemInteractionManager::DetermineMatchingRecords(DcmDataset *searchMask, DcmConfigFile *configfile)
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : This function determines the records from the worklist files which match
//                the given search mask and returns the number of matching records. Also,
//                this function will store the matching records in memory in the array
//                member variable matchingRecords.
// Parameters   : searchMask - [in] The search mask.
// Return Value : Number of matching records.
{
    matchingRecords = NULL;
    numOfMatchingRecords = 0;

    OFList<DcmDataset > list_data;
    //set matchingRecords
    GetWorklistData(list_data, searchMask, configfile);//bitter 20130501
    OFListIterator(DcmDataset) if_iter = list_data.begin();
    OFListIterator(DcmDataset) if_last = list_data.end();
    while (if_iter != if_last)
    {
        DcmDataset dataset(*if_iter);
        if (dataset.isEmpty())
        {
            DCMWLM_WARN("Creat Worklist file fail! will be ignored ");
        }
        else
        {
            //if (enableRejectionOfIncompleteWlFiles)
            //    DCMWLM_INFO("Checking whether Creat Worklist file  is complete");
            //if (enableRejectionOfIncompleteWlFiles && !DatasetIsComplete(dataset))//屏蔽对数据中部分字段
            //{
            //    DCMWLM_WARN("Creat Worklist file  is incomplete, file will be ignored");
            //}
            //else
            {

                if (numOfMatchingRecords == 0)
                {
                    matchingRecords = new DcmDataset*[1];
                    matchingRecords[0] = new DcmDataset(dataset);
                }
                else
                {
                    DcmDataset **tmp = new DcmDataset*[numOfMatchingRecords + 1];
                    for (unsigned long j = 0; j < numOfMatchingRecords; j++)
                    {
                        tmp[j] = matchingRecords[j];
                    }
                    tmp[numOfMatchingRecords] = new DcmDataset(dataset);
                    delete[] matchingRecords;
                    matchingRecords = tmp;
                    //delete[] dataset;
                }
                numOfMatchingRecords++;
            }
        }
        if_iter++;
    }
    list_data.clear();
    return(numOfMatchingRecords);
}

// ----------------------------------------------------------------------------

unsigned long WlmFileSystemInteractionManager::GetNumberOfSequenceItemsForMatchingRecord(DcmTagKey sequenceTag, WlmSuperiorSequenceInfoType *superiorSequenceArray, unsigned long numOfSuperiorSequences, unsigned long idx)
// Date         : January 6, 2004
// Author       : Thomas Wilkens
// Task         : For the matching record that is identified through idx, this function returns the number
//                of items that are contained in the sequence element that is referred to by sequenceTag.
//                In case this sequence element is itself contained in a certain item of another superior
//                sequence, superiorSequenceArray contains information about where to find the correct
//                sequence element.
// Parameters   : sequenceTag            - [in] The tag of the sequence element for which the number of items
//                                              shall be determined.
//                superiorSequenceArray  - [in] Array which contains information about superior sequence elements
//                                              the given sequence element is contained in.
//                numOfSuperiorSequences - [in] The number of elements in the above array.
//                idx                    - [in] Identifies the record from which the number of sequence items
//                                              shall be determined.
// Return Value : The number of items that are contained in the sequence element that is referred to by
//                sequenceTag and that can be found in sequence items which are specified in superiorSequenceArray.
{
    OFCondition cond;
    DcmSequenceOfItems *sequenceElement = NULL, *tmp = NULL;
    unsigned long i;

    // initialize result variable
    unsigned long numOfItems = 0;

    // if the sequence in question is not contained in another sequence
    if (numOfSuperiorSequences == 0)
    {
        // simply find and get this sequence in the matching dataset (on the main level)
        cond = matchingRecords[idx]->findAndGetSequence(sequenceTag, sequenceElement, OFFalse);
    }
    else
    {
        // if it is contained in (an)other sequence(s), find it
        cond = matchingRecords[idx]->findAndGetSequence(superiorSequenceArray[0].sequenceTag, sequenceElement, OFFalse);
        for (i = 1; i < numOfSuperiorSequences && cond.good(); i++)
        {
            cond = sequenceElement->getItem(superiorSequenceArray[i - 1].currentItem)->findAndGetSequence(superiorSequenceArray[i].sequenceTag, tmp, OFFalse);
            if (cond.good())
                sequenceElement = tmp;
        }

        if (cond.good())
        {
            cond = sequenceElement->getItem(superiorSequenceArray[numOfSuperiorSequences - 1].currentItem)->findAndGetSequence(sequenceTag, tmp, OFFalse);
            if (cond.good())
                sequenceElement = tmp;
        }
    }

    // determine number of items for the sequence in question
    if (cond.good())
        numOfItems = sequenceElement->card();

    // return result
    return(numOfItems);
}

// ----------------------------------------------------------------------------

void WlmFileSystemInteractionManager::GetAttributeValueForMatchingRecord(DcmTagKey tag, WlmSuperiorSequenceInfoType *superiorSequenceArray, unsigned long numOfSuperiorSequences, unsigned long idx, char *&value)
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : This function determines an attribute value of a matching record
//                and returns this value in a newly created string to the caller.
// Parameters   : tag                    - [in] Attribute tag. Specifies which attribute's value shall be returned.
//                superiorSequenceArray  - [in] Array which contains information about superior sequence elements
//                                              the given element is contained in.
//                numOfSuperiorSequences - [in] The number of elements in the above array.
//                idx                    - [in] Identifies the record from which the attribute value shall be retrieved.
//                value                  - [out] Pointer to a newly created string that contains the requested value.
//                                               If value was not found an emtpy string will be returned.
// Return Value : none.
{
    OFCondition cond;
    DcmSequenceOfItems *sequenceElement = NULL, *tmp = NULL;
    unsigned long i;
    const char *val = NULL;
    Uint16 v;

    // if the element in question is not contained in another sequence
    if (numOfSuperiorSequences == 0)
    {
        // simply find and get this element in the matching dataset (on the main level);
        // here, we have to distinguish two cases: attribute PregnancyStatus has to be re-
        // trieved as a Uint16 value (also note for this case, that this attribute can only
        // occur on the main level, it will never be contained in a sequence), all other
        // attributes have to be retrieved as strings
        if (tag == DCM_PregnancyStatus)
        {
            cond = matchingRecords[idx]->findAndGetUint16(tag, v, 0, OFFalse);
            if (cond.good())
            {
                value = new char[20];
                sprintf(value, "%u", v);
            }
            else
            {
                value = new char[2];
                strcpy(value, "4");           // a value of "4" in attribute PregnancyStatus means "unknown" in DICOM
            }
        }
        else
        {
            cond = matchingRecords[idx]->findAndGetString(tag, val, OFFalse);
            if (cond.good() && val != NULL)
            {
                value = new char[strlen(val) + 1];
                strcpy(value, val);
            }
            else
            {
                value = new char[1];
                strcpy(value, "");
            }
        }
    }
    else
    {
        // if it is contained in (an)other sequence(s), go through all corresponding sequence items
        cond = matchingRecords[idx]->findAndGetSequence(superiorSequenceArray[0].sequenceTag, sequenceElement, OFFalse);
        for (i = 1; i < numOfSuperiorSequences && cond.good(); i++)
        {
            cond = sequenceElement->getItem(superiorSequenceArray[i - 1].currentItem)->findAndGetSequence(superiorSequenceArray[i].sequenceTag, tmp, OFFalse);
            if (cond.good())
                sequenceElement = tmp;
        }

        // now sequenceElement points to the sequence element in which the attribute
        // in question can be found; retrieve a value for this attribute (note that
        // all attributes in sequences can actually be retrieved as strings)
        if (cond.good())
        {
            cond = sequenceElement->getItem(superiorSequenceArray[numOfSuperiorSequences - 1].currentItem)->findAndGetString(tag, val, OFFalse);
            if (cond.good() && val != NULL)
            {
                value = new char[strlen(val) + 1];
                strcpy(value, val);
            }
            else
            {
                value = new char[1];
                strcpy(value, "");
            }
        }
        else
        {
            value = new char[1];
            strcpy(value, "");
        }
    }
}

// ----------------------------------------------------------------------------

void WlmFileSystemInteractionManager::ClearMatchingRecords()
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : This function frees the memory which was occupied by matchingRecords.
//                It shall be called when the matching records are no longer needed.
// Parameters   : none.
// Return Value : none.
{
    for (unsigned int i = 0; i < numOfMatchingRecords; i++)
        delete matchingRecords[i];
    delete[] matchingRecords;
    matchingRecords = NULL;
    numOfMatchingRecords = 0;
}

// ----------------------------------------------------------------------------

void WlmFileSystemInteractionManager::DetermineWorklistFiles(OFVector<OFString> &worklistFiles)
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : This function determines all worklist files in the directory specified by
//                dfPath and calledApplicationEntityTitle, and returns the complete path and
//                filename information in an array of strings.
// Parameters   : worklistFiles - [out] Set of strings, each specifying path and filename
//                                      to one worklist file.
// Return Value : none.
{
    // initialize out parameters
    worklistFiles.clear();

    // determine complete path to data source files
    // (dfPath + PATH_SEPARATOR + calledApplicationEntityTitle)
    OFString path(dfPath);
    if (!path.empty() && path[path.length() - 1] != PATH_SEPARATOR)
        path += PATH_SEPARATOR;
    path += calledApplicationEntityTitle;

    // determine worklist files in this folder
#ifdef HAVE__FINDFIRST
    OFString searchname = path + PATH_SEPARATOR + '*';
    struct _finddata_t fileData;
    int ret = 0;

    // start a loop; in each iteration another directory entry is determined
    intptr_t hFile = _findfirst((char*)searchname.c_str(), &fileData);
    while (hFile != -1L && ret == 0)
    {
        // if the current entry refers to a worklist file, do something
        if (strcmp(fileData.name, ".") != 0 && strcmp(fileData.name, "..") != 0 && IsWorklistFile(fileData.name))
        {
            // Create a string that contains path and filename of the current worklist file.
            OFString subname(path);
            subname += PATH_SEPARATOR;
            subname += fileData.name;

            // Add string to the set of strings
            worklistFiles.push_back(subname);
        }
        ret = _findnext(hFile, &fileData);
    }
    _findclose(hFile);
#else
    struct dirent *dp = NULL;

    // open directory
    DIR *dirp = opendir(path.c_str());
    if (dirp != NULL)
    {
        // start a loop; in each iteration another directory entry is determined.
#ifdef HAVE_READDIR_R
        unsigned char entryBuffer[sizeof(struct dirent) + _POSIX_PATH_MAX + 1];
#ifdef HAVE_OLD_READDIR_R
        for (dp = readdir_r(dirp, (struct dirent *)entryBuffer); dp != NULL; dp = readdir_r(dirp, (struct dirent *)entryBuffer))
#else
        for (int readResult = readdir_r(dirp, (struct dirent *)entryBuffer, &dp); readResult == 0 && dp; readResult = readdir_r(dirp, (struct dirent *)entryBuffer, &dp))
#endif
#else // HAVE_READDIR_R
        for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
#endif
        {
            // if the current entry refers to a worklist file
            if (IsWorklistFile(dp->d_name))
            {
                // create a string that contains path and filename of the current worklist file.
                OFString subname(path);
                subname += PATH_SEPARATOR;
                subname += dp->d_name;

                // add string to the set of strings
                worklistFiles.push_back(subname);
            }
        }

        // close directory
        closedir(dirp);
    }
#endif

    // in case we are running in verbose mode, dump all worklist file information
    if (DCM_dcmwlmLogger.isEnabledFor(OFLogger::INFO_LOG_LEVEL))
    {
        DCMWLM_INFO("=============================");
        DCMWLM_INFO("Worklist Database Files:");
        if (worklistFiles.empty())
            DCMWLM_INFO("<no files found>");
        else
        {
            OFVector<OFString>::const_iterator iter = worklistFiles.begin();
            while (iter != worklistFiles.end())
            {
                DCMWLM_INFO(*iter);
                ++iter;
            }
        }
        DCMWLM_INFO("=============================");
    }
}

// ----------------------------------------------------------------------------

OFBool WlmFileSystemInteractionManager::IsWorklistFile(const char *fname)
// Date         : July 11, 2002
// Author       : Thomas Wilkens
// Task         : This function returns OFTrue if the given filename refers to a worklist file,
//                i.e. has an extension of ".wl".
// Parameters   : fname - [in] The name of the file.
// Return Value : OFTrue  - The given filename refers to a worklist file.
//                OFFalse - The given filename does not refer to a worklist file.
{
    // If no filename is specified, return OFFalse
    if (fname == NULL)
        return(OFFalse);

    // look for an '.' in the filename
    char *s = (char *)strrchr(fname, '.');

    // if there was no '.' return OFFalse
    if (s == NULL)
        return(OFFalse);

    // if the extension is ".wl" return OFTrue
    if (strcmp(s, ".wl") == 0)
        return(OFTrue);

    // return OFFalse in all other cases
    return(OFFalse);
}

// ----------------------------------------------------------------------------

OFBool WlmFileSystemInteractionManager::DatasetIsComplete(DcmDataset *dataset)
// Date         : May 2, 2005
// Author       : Thomas Wilkens
// Task         : This function checks if the given dataset (which represents the information from a
//                worklist file) contains all necessary return type 1 information. According to the
//                DICOM standard part 4 annex K, the following attributes are type 1 attributes in
//                C-Find RSP messages:
//                      Attribute                             Tag      Return Key Type
//                  ScheduledProcedureStepSequence        (0040,0100)        1
//                   > ScheduledStationAETitle            (0040,0001)        1
//                   > ScheduledProcedureStepStartDate    (0040,0002)        1
//                   > ScheduledProcedureStepStartTime    (0040,0003)        1
//                   > Modality                           (0008,0060)        1
//                   > ScheduledProcedureStepDescription  (0040,0007)        1C (The ScheduledProcedureStepDescription (0040,0007) or the ScheduledProtocolCodeSequence (0040,0008) or both shall be supported by the SCP; we actually support both, so we have to check if at least one of the two attributes contains valid information.)
//                   > ScheduledProtocolCodeSequence      (0040,0008)        1C (see abobve)
//                   > > CodeValue                        (0008,0100)        1
//                   > > CodingSchemeDesignator           (0008,0102)        1
//                   > ScheduledProcedureStepID           (0040,0009)        1
//                  RequestedProcedureID                  (0040,1001)        1
//                  RequestedProcedureDescription         (0032,1060)        1C (The RequestedProcedureDescription (0032,1060) or the RequestedProcedureCodeSequence (0032,1064) or both shall be supported by the SCP; we actually support both, so we have to check if at least one of the two attributes contains valid information.)
//                  RequestedProcedureCodeSequence        (0032,1064)        1C (see abobve)
//                   > > CodeValue                        (0008,0100)        1
//                   > > CodingSchemeDesignator           (0008,0102)        1
//                  StudyInstanceUID                      (0020,000D)        1
//                  ReferencedStudySequence               (0008,1110)        2
//                   > ReferencedSOPClassUID              (0008,1150)        1C (Required if a sequence item is present)
//                   > ReferencedSOPInstanceUID           (0008,1155)        1C (Required if a sequence item is present)
//                  ReferencedPatientSequence             (0008,1120)        2
//                   > ReferencedSOPClassUID              (0008,1150)        1C (Required if a sequence item is present)
//                   > ReferencedSOPInstanceUID           (0008,1155)        1C (Required if a sequence item is present)
//                  PatientName                           (0010,0010)        1
//                  PatientID                             (0010,0020)        1
// Parameters   : dataset - [in] The dataset of the worklist file which is currently examined.
// Return Value : OFTrue in case the given dataset contains all necessary return type 1 information,
//                OFFalse otherwise.
{
    DcmElement *scheduledProcedureStepSequence = NULL;

    // intialize returnValue
    OFBool complete = OFTrue;

    DCMWLM_DEBUG("Checking whether dataset is complete ...");

    // the dataset is considered to be incomplete...
    // ...if the ScheduledProcedureStepSequence is missing or
    // ...if the ScheduledProcedureStepSequence does not have exactly one item
    if (dataset->findAndGetElement(DCM_ScheduledProcedureStepSequence, scheduledProcedureStepSequence).bad() || ((DcmSequenceOfItems*)scheduledProcedureStepSequence)->card() != 1)
    {
        DCMWLM_DEBUG("- ScheduledProcedureStepSequence " << DCM_ScheduledProcedureStepSequence << " is missing or does not have exactly one item");
        complete = OFFalse;
    }
    else
    {
        // so the ScheduledProcedureStepSequence is existent and has exactly one item;
        // get this one and only item from the ScheduledProcedureStepSequence
        DcmItem *scheduledProcedureStepSequenceItem = ((DcmSequenceOfItems*)scheduledProcedureStepSequence)->getItem(0);

        // the dataset is considered to be incomplete...
        // ...if ScheduledStationAETitle is missing or empty in the ScheduledProcedureStepSequence, or
        // ...if ScheduledProcedureStepStartDate is missing or empty in the ScheduledProcedureStepSequence, or
        // ...if ScheduledProcedureStepStartTime is missing or empty in the ScheduledProcedureStepSequence, or
        // ...if Modality is missing or empty in the ScheduledProcedureStepSequence, or
        // ...if ScheduledProcedureStepID is missing or empty in the ScheduledProcedureStepSequence, or
        // ...if RequestedProcedureID is missing or empty in the dataset, or
        // ...if StudyInstanceUID is missing or empty in the dataset, or
        // ...if PatientName is missing or empty in the dataset, or
        // ...if PatientID is missing or empty in the dataset, or
        // ...if inside the ScheduledProcedureStepSequence, no information can be retrieved from
        //    the ScheduledProcedureStepDescription attribute and (at the same time) also no
        //    complete information can be retrieved from the ScheduledProtocolCodeSequence attribute, or
        // ...if inside the dataset, no information can be retrieved from the RequestedProcedureDescription
        //    attribute and (at the same time) also no complete information can be retrieved from the
        //    RequestedProcedureCodeSequence attribute, or
        // ...if inside the dataset, the ReferencedStudySequence is either absent or it is existent and non-empty but incomplete, or
        // ...if inside the dataset, the ReferencedPatientSequence is either absent or it is existent and non-empty but incomplete
        if (AttributeIsAbsentOrEmpty(DCM_ScheduledStationAETitle, scheduledProcedureStepSequenceItem) ||
            AttributeIsAbsentOrEmpty(DCM_ScheduledProcedureStepStartDate, scheduledProcedureStepSequenceItem) ||
            AttributeIsAbsentOrEmpty(DCM_ScheduledProcedureStepStartTime, scheduledProcedureStepSequenceItem) ||
            AttributeIsAbsentOrEmpty(DCM_Modality, scheduledProcedureStepSequenceItem) ||
            AttributeIsAbsentOrEmpty(DCM_ScheduledProcedureStepID, scheduledProcedureStepSequenceItem) ||
            AttributeIsAbsentOrEmpty(DCM_RequestedProcedureID, dataset) ||
            AttributeIsAbsentOrEmpty(DCM_StudyInstanceUID, dataset) ||
            AttributeIsAbsentOrEmpty(DCM_PatientName, dataset) ||
            AttributeIsAbsentOrEmpty(DCM_PatientID, dataset) ||
            DescriptionAndCodeSequenceAttributesAreIncomplete(DCM_ScheduledProcedureStepDescription, DCM_ScheduledProtocolCodeSequence, scheduledProcedureStepSequenceItem) ||
            DescriptionAndCodeSequenceAttributesAreIncomplete(DCM_RequestedProcedureDescription, DCM_RequestedProcedureCodeSequence, dataset) ||
            ReferencedStudyOrPatientSequenceIsAbsentOrExistentButNonEmptyAndIncomplete(DCM_ReferencedStudySequence, dataset) ||
            ReferencedStudyOrPatientSequenceIsAbsentOrExistentButNonEmptyAndIncomplete(DCM_ReferencedPatientSequence, dataset))
            complete = OFFalse;
    }
    // return result
    return(complete);
}

// ----------------------------------------------------------------------------

OFBool WlmFileSystemInteractionManager::ReferencedStudyOrPatientSequenceIsAbsentOrExistentButNonEmptyAndIncomplete(DcmTagKey sequenceTagKey, DcmItem *dset)
// Date         : May 4, 2005
// Author       : Thomas Wilkens
// Task         : This function checks if the specified sequence attribute is absent or existent but non-empty
//                and incomplete in the given dataset.
// Parameters   : sequenceTagKey - [in] The sequence attribute which shall be checked.
//                dset           - [in] The dataset in which the attribute is contained.
// Return Value : OFTrue in case the sequence attribute is absent (and cannot be added to the dataset)
//                or existent but non-empty and incomplete, OFFalse otherwise.
{
    DcmElement *sequence = NULL;
    OFBool result;

    // check whether the type 2 sequence attribute is absent
    if (dset->findAndGetElement(sequenceTagKey, sequence).bad())
    {
        DCMWLM_DEBUG("- " << DcmTag(sequenceTagKey).getTagName() << " " << sequenceTagKey << " is missing");
        // try to add it to the dataset and return OFFalse if successful
        if (dset->insertEmptyElement(sequenceTagKey).good())
        {
            DCMWLM_WARN("Added missing type 2 sequence attribute " << sequenceTagKey << " to the current record");
            result = OFFalse;
        }
        else
            result = OFTrue;
    }
    else
    {
        // if the sequence attribute is existent but empty, we want to return OFFalse
        // (note that the sequence is actually type 2, so being empty is ok)
        if (((DcmSequenceOfItems*)sequence)->card() == 0)
            result = OFFalse;
        else
        {
            // if the sequence attribute is existent and non-empty, we need
            // to check every item in the sequence for completeness
            result = OFFalse;
            for (unsigned long i = 0; i < ((DcmSequenceOfItems*)sequence)->card() && !result; i++)
            {
                if (AttributeIsAbsentOrEmpty(DCM_ReferencedSOPClassUID, ((DcmSequenceOfItems*)sequence)->getItem(i)) ||
                    AttributeIsAbsentOrEmpty(DCM_ReferencedSOPInstanceUID, ((DcmSequenceOfItems*)sequence)->getItem(i)))
                    result = OFTrue;
            }
            if (result)
                DCMWLM_DEBUG("- " << DcmTag(sequenceTagKey).getTagName() << " " << sequenceTagKey << " is incomplete");
        }
    }

    // return result
    return(result);
}

// ----------------------------------------------------------------------------

OFBool WlmFileSystemInteractionManager::DescriptionAndCodeSequenceAttributesAreIncomplete(DcmTagKey descriptionTagKey, DcmTagKey codeSequenceTagKey, DcmItem *dset)
// Date         : April 18, 2017
// Author       : Michael Onken
// Task         : This method ensures that either code or description is set to a non-empty value,
//                and at the same time none of the attributes is present with a zero-length value.
//                If one of these requirements are not met, then OFTrue is returned, otherwise OFFalse.
// Parameters   : descriptionTagKey  - [in] The description attribute which shall be checked.
//                codeSequenceTagKey - [in] The codeSequence attribute which shall be checked.
//                dset               - [in] The dataset in which the attributes are contained.
// Return Value : OFFalse (i.e. no error regarding the standard) in case at least
//                one of both attributes has a non-empty, valid value, and none
//                is set to an empty value. OFTrue otherwise.
{
    // check if the code sequence attribute is complete,
    // i.e. if complete information can be retrieved from this attribute

    // if the attribute is not existent or has no items, we consider it incomplete
    DcmSequenceOfItems* seq = NULL;
    if (dset->findAndGetSequence(codeSequenceTagKey, seq).good())
    {
        if (seq->card() == 0)
        {
            DCMWLM_DEBUG("- " << DcmTag(codeSequenceTagKey).getTagName() << " " << codeSequenceTagKey << " is empty");
            return OFTrue;
        }
        else if (seq->card() > 0)
        {
            // if it is existent and has items, check every item for completeness
            for (unsigned long i = 0; i < seq->card(); i++)
            {
                if (AttributeIsAbsentOrEmpty(DCM_CodeValue, seq->getItem(i)) ||
                    AttributeIsAbsentOrEmpty(DCM_CodingSchemeDesignator, seq->getItem(i)))
                {
                    DCMWLM_DEBUG("- " << DcmTag(codeSequenceTagKey).getTagName() << " " << codeSequenceTagKey << " has incomplete or empty code entries");
                    return OFTrue;
                }
            }
        }
    }
    // check whether description is valid. If sequence does exists, the attribute
    // must exist with a valid value.
    OFString description;
    if (dset->findAndGetOFStringArray(descriptionTagKey, description).bad())
    {
        if (!seq)
        {
            DCMWLM_DEBUG("- " << DcmTag(codeSequenceTagKey).getTagName() << " " << codeSequenceTagKey
                << " and " << DcmTag(descriptionTagKey) << " " << descriptionTagKey << " are both not set");
            return OFTrue;
        }
    }
    // in any case, description must not exist with empty value in the dataset
    else if (description.empty())
    {
        DCMWLM_DEBUG("- " << DcmTag(descriptionTagKey).getTagName() << " " << descriptionTagKey << " is empty");
        return OFTrue;
    }

    return OFFalse;
}

// ----------------------------------------------------------------------------

OFBool WlmFileSystemInteractionManager::AttributeIsAbsentOrEmpty(DcmTagKey elemTagKey, DcmItem *dset)
// Date         : May 4, 2005
// Author       : Thomas Wilkens
// Task         : This function checks if the specified attribute is absent or contains an empty value in the given dataset.
// Parameters   : elemTagKey - [in] The attribute which shall be checked.
//                dset       - [in] The dataset in which the attribute is contained.
// Return Value : OFTrue in case the attribute is absent or contains an empty value, OFFalse otherwise.
{
    DcmElement *elem = NULL;

    if (dset->findAndGetElement(elemTagKey, elem).bad() || elem->getLength() == 0)
    {
        DCMWLM_DEBUG("- " << DcmTag(elemTagKey).getTagName() << " " << elemTagKey << " is missing or empty");
        return(OFTrue);
    }
    else
        return(OFFalse);
}


OFBool WlmFileSystemInteractionManager::isUniversalMatchingSequences(DcmSequenceOfItems& query,
    const MatchingKeys& matchingKeys,
    const OFBool normalize,
    const OFBool enableWildcardMatching)
{
    DcmItem* pQueryItem = OFstatic_cast(DcmItem*, query.nextInContainer(OFnullptr));
    if (pQueryItem) {
#ifdef HAVE_CXX11
        for (auto& key : matchingKeys.keys)
        {
#else
        // remove this bloated version of the code if C++11 ever becomes a requirement of DCMTK
        for (OFVector<OFPair<DcmTagKey, OFBool> >::const_iterator it = matchingKeys.keys.begin(); it != matchingKeys.keys.end(); ++it)
        {
            const OFPair<DcmTagKey, OFBool>& key = *it;
#endif
            DcmElement* query = OFnullptr;
            if (pQueryItem->findAndGetElement(key.first, query, OFFalse).good() && query && !query->isUniversalMatch(normalize, enableWildcardMatching))
                return OFFalse;
        }

#ifdef HAVE_CXX11
        for (auto& combinedKey : matchingKeys.combinedKeys)
        {
#else
        // remove this bloated version of the code if C++11 ever becomes a requirement of DCMTK
        for (OFVector<OFPair<DcmTagKey, DcmTagKey> >::const_iterator it = matchingKeys.combinedKeys.begin(); it != matchingKeys.combinedKeys.end(); ++it)
        {
            const OFPair<DcmTagKey, DcmTagKey>& combinedKey = *it;
#endif
            DcmElement* query = OFnullptr;
            if (pQueryItem->findAndGetElement(combinedKey.first, query, OFFalse).good() && query && !query->isUniversalMatch(normalize, enableWildcardMatching))
                return OFFalse;
            else if (pQueryItem->findAndGetElement(combinedKey.second, query, OFFalse).good() && query && !query->isUniversalMatch(normalize, enableWildcardMatching))
                return OFFalse;
        }

        // sequence matching
#ifdef HAVE_CXX11
        for (auto& sequenceKey : matchingKeys.sequenceKeys)
        {
#else
        // remove this bloated version of the code if C++11 ever becomes a requirement of DCMTK
        for (OFVector<OFPair<DcmTagKey, MatchingKeys> >::const_iterator it = matchingKeys.sequenceKeys.begin(); it != matchingKeys.sequenceKeys.end(); ++it)
        {
            const OFPair<DcmTagKey, MatchingKeys>& sequenceKey = *it;
#endif
            DcmElement* query = OFnullptr;
            if (pQueryItem->findAndGetElement(sequenceKey.first, query, OFFalse).good() && query && query->ident() == EVR_SQ && !isUniversalMatchingSequences(OFstatic_cast(DcmSequenceOfItems&, *query), sequenceKey.second, normalize, enableWildcardMatching))
                return OFFalse;
        }
    }
    return OFTrue;
}

OFBool WlmFileSystemInteractionManager::MatchSequences(DcmSequenceOfItems& candidate,
    DcmSequenceOfItems& query,
    const MatchingKeys& matchingKeys)
{
    for (DcmObject* pQueryItem = query.nextInContainer(OFnullptr); pQueryItem; pQueryItem = query.nextInContainer(pQueryItem))
        for (DcmObject* pCandidateItem = candidate.nextInContainer(OFnullptr); pCandidateItem; pCandidateItem = candidate.nextInContainer(pCandidateItem))
            if (DatasetMatchesSearchMask(OFstatic_cast(DcmItem*, pCandidateItem), OFstatic_cast(DcmItem*, pQueryItem), matchingKeys))
                return OFTrue;
    return OFFalse;
}

// ----------------------------------------------------------------------------

OFBool WlmFileSystemInteractionManager::DatasetMatchesSearchMask(DcmItem *dataset, DcmItem *searchMask,
    const MatchingKeys& matchingKeys)
    // Date         : July 11, 2002
    // Author       : Thomas Wilkens
    // Task         : This function returns OFTrue, if the matching key attribute values in the
    //                dataset match the matching key attribute values in the search mask.
    // Parameters   : dataset    - [in] The dataset which shall be checked.
    //                searchMask - [in] The search mask.
    // Return Value : OFTrue  - The dataset matches the search mask in the matching key attribute values.
    //                OFFalse - The dataset does not match the search mask in the matching key attribute values.
{
    assert(dataset);
    assert(searchMask);

#ifdef HAVE_CXX11
    for (auto& key : matchingKeys.keys)
    {
#else
    // remove this bloated version of the code if C++11 ever becomes a requirement of DCMTK
    for (OFVector<OFPair<DcmTagKey, OFBool> >::const_iterator it = matchingKeys.keys.begin(); it != matchingKeys.keys.end(); ++it)
    {
        const OFPair<DcmTagKey, OFBool>& key = *it;
#endif
        DcmElement* query = OFnullptr;
        if (searchMask->findAndGetElement(key.first, query, OFFalse).good() && query && !query->isUniversalMatch())
        {
            DcmElement* candidate = OFnullptr;
            if (dataset->findAndGetElement(key.first, candidate, OFFalse).bad() || !candidate || !query->matches(*candidate, key.second))
                return OFFalse;
        }
    }

#ifdef HAVE_CXX11
    for (auto& combinedKey : matchingKeys.combinedKeys)
    {
#else
    // remove this bloated version of the code if C++11 ever becomes a requirement of DCMTK
    for (OFVector<OFPair<DcmTagKey, DcmTagKey> >::const_iterator it = matchingKeys.combinedKeys.begin(); it != matchingKeys.combinedKeys.end(); ++it)
    {
        const OFPair<DcmTagKey, DcmTagKey>& combinedKey = *it;
#endif
        DcmElement* query = OFnullptr;
        if (searchMask->findAndGetElement(combinedKey.first, query, OFFalse).good() && query && !query->isUniversalMatch())
        {
            DcmElement* candidate = OFnullptr;
            if (dataset->findAndGetElement(combinedKey.first, candidate, OFFalse).bad() || !candidate)
                return OFFalse;
            DcmElement* secondQuery = OFnullptr;
            if (searchMask->findAndGetElement(combinedKey.second, secondQuery, OFFalse).good() && secondQuery)
            {
                DcmElement* secondCandidate = OFnullptr;
                if (dataset->findAndGetElement(combinedKey.second, secondCandidate, OFFalse).bad() || !secondCandidate || !query->combinationMatches(*secondQuery, *candidate, *secondCandidate))
                    return OFFalse;
            }
            else if (!query->matches(*candidate))
            {
                return OFFalse;
            }
        }
        else if (searchMask->findAndGetElement(combinedKey.second, query, OFFalse).good() && query && !query->isUniversalMatch())
        {
            DcmElement* candidate = OFnullptr;
            if (dataset->findAndGetElement(combinedKey.second, candidate, OFFalse).bad() || !candidate || !query->matches(*candidate))
                return OFFalse;
        }
    }

    // sequence matching

#ifdef HAVE_CXX11
    for (auto& sequenceKey : matchingKeys.sequenceKeys)
    {
#else
    // remove this bloated version of the code if C++11 ever becomes a requirement of DCMTK
    for (OFVector<OFPair<DcmTagKey, MatchingKeys> >::const_iterator it = matchingKeys.sequenceKeys.begin(); it != matchingKeys.sequenceKeys.end(); ++it)
    {
        const OFPair<DcmTagKey, MatchingKeys>& sequenceKey = *it;
#endif
        DcmElement* query = OFnullptr;
        if (searchMask->findAndGetElement(sequenceKey.first, query, OFFalse).good() && query && query->ident() == EVR_SQ && !isUniversalMatchingSequences(OFstatic_cast(DcmSequenceOfItems&, *query), sequenceKey.second))
        {
            DcmElement* candidate = OFnullptr;
            if (dataset->findAndGetElement(sequenceKey.first, candidate, OFFalse).bad() || !candidate || candidate->ident() != EVR_SQ || !MatchSequences(OFstatic_cast(DcmSequenceOfItems&, *candidate), OFstatic_cast(DcmSequenceOfItems&, *query), sequenceKey.second))
                return OFFalse;
        }
    }
    return OFTrue;
}

void WlmFileSystemInteractionManager::SetDcmConfig(DcmConfigFile *config)
{
    m_config = config;
}

DcmConfigFile* WlmFileSystemInteractionManager::GetDcmConfig()
{
    return m_config;
}

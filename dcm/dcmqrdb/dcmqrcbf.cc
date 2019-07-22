/*
 *
 *  Copyright (C) 1993-2017, OFFIS e.V.
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
 *  Author:  Marco Eichelberg
 *
 *  Purpose: class DcmQueryRetrieveFindContext
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmqrdb/dcmqrcbf.h"

#include "dcmtk/dcmqrdb/dcmqrcnf.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmqrdb/dcmqropt.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmqrdb/dcmqrdbs.h"
#include "dcmtk/dcmqrdb/dcmqrdbi.h"
#include "dcmtk/dcmqrdb/dcmqridx.h"


/*******************
*    Is the specified tag supported
*/

static const DB_FindAttr TbFindAttr[] =
{
    DB_FindAttr(DCM_PatientBirthDate, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_PatientSex, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_PatientName, PATIENT_LEVEL, REQUIRED_KEY),
    DB_FindAttr(DCM_PatientID, PATIENT_LEVEL, UNIQUE_KEY),
    DB_FindAttr(DCM_PatientBirthTime, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_RETIRED_OtherPatientIDs, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_OtherPatientNames, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_EthnicGroup, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_PatientComments, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_IssuerOfPatientID, PATIENT_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_StudyDate, STUDY_LEVEL, REQUIRED_KEY),
    DB_FindAttr(DCM_StudyTime, STUDY_LEVEL, REQUIRED_KEY),
    DB_FindAttr(DCM_StudyID, STUDY_LEVEL, REQUIRED_KEY),
    DB_FindAttr(DCM_AccessionNumber, STUDY_LEVEL, REQUIRED_KEY),
    DB_FindAttr(DCM_ReferringPhysicianName, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_StudyDescription, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_NameOfPhysiciansReadingStudy, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_StudyInstanceUID, STUDY_LEVEL, UNIQUE_KEY),
    DB_FindAttr(DCM_RETIRED_OtherStudyNumbers, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_AdmittingDiagnosesDescription, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_PatientAge, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_PatientSize, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_PatientWeight, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_Occupation, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_AdditionalPatientHistory, STUDY_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_SeriesNumber, SERIE_LEVEL, REQUIRED_KEY),
    DB_FindAttr(DCM_SeriesInstanceUID, SERIE_LEVEL, UNIQUE_KEY),
    DB_FindAttr(DCM_Modality, SERIE_LEVEL, OPTIONAL_KEY),
    DB_FindAttr(DCM_InstanceNumber, IMAGE_LEVEL, REQUIRED_KEY),
    DB_FindAttr(DCM_SOPInstanceUID, IMAGE_LEVEL, UNIQUE_KEY)
};

/**** The NbFindAttr variable contains the length of the TbFindAttr table
***/

static int NbFindAttr = ((sizeof(TbFindAttr)) / (sizeof(TbFindAttr[0])));
static int DB_TagSupported(DcmTagKey tag)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    {
        if (TbFindAttr[i].tag == tag)
        {
            return (OFTrue);
        }
    }

    return (OFFalse);
}

/***********************
*    Duplicate a DICOM element
*    dst space is supposed provided by the caller
*/

static void DB_DuplicateElement(DB_SmallDcmElmt *src, DB_SmallDcmElmt *dst)
{
    bzero((char*)dst, sizeof(DB_SmallDcmElmt));
    dst->XTag = src->XTag;
    dst->ValueLength = src->ValueLength;

    if (src->ValueLength == 0)
    {
        dst->PValueField = NULL;
    }
    else
    {
        dst->PValueField = (char *)malloc((int)src->ValueLength + 1);
        bzero(dst->PValueField, (size_t)(src->ValueLength + 1));
        if (dst->PValueField != NULL)
        {
            memcpy(dst->PValueField, src->PValueField,
                (size_t)src->ValueLength);
        }
        else
        {
            DCMQRDB_ERROR("DB_DuplicateElement: out of memory");
        }
    }
}

/*******************
*    Get tag level of a specified tag
*/

static OFCondition DB_GetTagLevel(DcmTagKey tag, DB_LEVEL *level)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
        if (TbFindAttr[i].tag == tag)
            break;

    if (i < NbFindAttr) {
        *level = TbFindAttr[i].level;
        return (EC_Normal);
    }
    else
        return (QR_EC_IndexDatabaseError);
}

static OFCondition DB_GetTagKeyAttr(DcmTagKey tag, DB_KEY_TYPE *keyAttr)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
        if (TbFindAttr[i].tag == tag)
            break;

    if (i < NbFindAttr) {
        *keyAttr = TbFindAttr[i].keyAttr;
        return (EC_Normal);
    }
    else
        return (QR_EC_IndexDatabaseError);
}


OFCondition DB_lock(OFBool exclusive)
{
    static int pidx;//???
    int lockmode;

    if (exclusive) {
        lockmode = LOCK_EX;     /* exclusive lock */
    }
    else {
        lockmode = LOCK_SH;     /* shared lock */
    }
    if (dcmtk_flock(pidx, lockmode) < 0) {
        dcmtk_plockerr("DB_lock");
        return QR_EC_IndexDatabaseError;
    }
    return EC_Normal;
}

/************
**      Test a Find Request List
**      Returns EC_Normal if OK, else returns QR_EC_IndexDatabaseError
*/

OFCondition testFindRequestList(
    DB_ElementList  *findRequestList,
    DB_LEVEL        queryLevel,
    DB_LEVEL        infLevel,
    DB_LEVEL        lowestLevel
    )
{
    DB_ElementList *plist;
    DB_LEVEL    XTagLevel = PATIENT_LEVEL; // DB_GetTagLevel() will set this correctly
    DB_KEY_TYPE XTagType = OPTIONAL_KEY;  // DB_GetTagKeyAttr() will set this
    int level;

    /**** Query level must be at least the infLevel
    ***/

    if (queryLevel < infLevel) {
        DCMQRDB_INFO("Level incompatible with Information Model (level " << queryLevel << ")");
        return QR_EC_IndexDatabaseError;
    }

    if (queryLevel > lowestLevel) {
        DCMQRDB_DEBUG("Level incompatible with Information Model (level " << queryLevel << ")");
        return QR_EC_IndexDatabaseError;
    }

    for (level = PATIENT_LEVEL; level <= IMAGE_LEVEL; level++) {

        /**** Manage exception due to StudyRoot Information Model :
        **** In this information model, queries may include Patient attributes
        **** but only if they are made at the study level
        ***/

        if ((level == PATIENT_LEVEL) && (infLevel == STUDY_LEVEL)) {
            /** In Study Root Information Model, accept only Patient Tags
            ** if the Query Level is the Study level
            */

            int atLeastOneKeyFound = OFFalse;
            for (plist = findRequestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != level)
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (atLeastOneKeyFound && (queryLevel != STUDY_LEVEL)) {
                DCMQRDB_DEBUG("Key found in Study Root Information Model (level " << level << ")");
                return QR_EC_IndexDatabaseError;
            }
        }

        /**** If current level is above the QueryLevel
        ***/

        else if (level < queryLevel)
        {

            /** For this level, only unique keys are allowed
            ** Parse the request list elements referring to
            ** this level.
            ** Check that only unique key attr are provided
            */

            int uniqueKeyFound = OFFalse;
            for (plist = findRequestList; plist; plist = plist->next)
            {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != level)
                    continue;
                DB_GetTagKeyAttr(plist->elem.XTag, &XTagType);
                if (XTagType != UNIQUE_KEY)
                {
                    DCMQRDB_DEBUG("Non Unique Key found (level " << level << ")");
                    return QR_EC_IndexDatabaseError;
                }
                else if (uniqueKeyFound)
                {
                    DCMQRDB_DEBUG("More than one Unique Key found (level " << level << ")");
                    return QR_EC_IndexDatabaseError;
                }
                else
                    uniqueKeyFound = OFTrue;
            }
        }

        /**** If current level is the QueryLevel
        ***/

        else if (level == queryLevel)
        {

            /** For this level, all keys are allowed
            ** Parse the request list elements referring to
            ** this level.
            ** Check that at least one key is provided
            */

            int atLeastOneKeyFound = OFFalse;
            for (plist = findRequestList; plist; plist = plist->next)
            {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != level)
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (!atLeastOneKeyFound)
            {
                DCMQRDB_DEBUG("No Key found at query level (level " << level << ")");
                return QR_EC_IndexDatabaseError;
            }
        }

        /**** If current level beyond the QueryLevel
        ***/

        else if (level > queryLevel)
        {

            /** For this level, no key is allowed
            ** Parse the request list elements referring to
            ** this level.
            ** Check that no key is provided
            */

            int atLeastOneKeyFound = OFFalse;
            for (plist = findRequestList; plist; plist = plist->next)
            {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != level)
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (atLeastOneKeyFound)
            {
                DCMQRDB_DEBUG("Key found beyond query level (level " << level << ")");
                return QR_EC_IndexDatabaseError;
            }
        }

    }
    return EC_Normal;
}
/*******************
*    Free a UID List
*/

static OFCondition DB_FreeElementList(DB_ElementList *lst)
{
    if (lst == NULL) return EC_Normal;

    OFCondition cond = DB_FreeElementList(lst->next);
    if (lst->elem.PValueField != NULL)
    {
        free((char *)lst->elem.PValueField);
    }
    delete lst;
    return (cond);
}

void DB_IdxSetRecord(IdxRecord *idx)
{
    /////////////////////////
    strcpy(idx->PatientBirthDate, "20190719");
    strcpy(idx->PatientSex, "F");
    strcpy(idx->PatientName, "ceshi_name1");
    strcpy(idx->PatientID, "201907200001");
    strcpy(idx->PatientBirthTime, "154200");

    strcpy(idx->OtherPatientIDs, "201907200001");
    strcpy(idx->OtherPatientNames, "ceshi_name1");
    strcpy(idx->PatientName, "ceshi_name1");
    strcpy(idx->StudyDate, "20190718-20190719");
    strcpy(idx->StudyTime, "154200");

    strcpy(idx->StudyID, "20190718");
    strcpy(idx->StudyDescription, "this is a test study! ");
    strcpy(idx->StudyInstanceUID, "1.2.276.0.179081.1208");
    strcpy(idx->PatientAge, "1");
    strcpy(idx->SeriesNumber, "1");
    strcpy(idx->SeriesInstanceUID, "1.2.276.0.179081.1208.1");
    strcpy(idx->ImageNumber, "1");
    strcpy(idx->SpecificCharacterSet, "ISO_IR 100");
    ///////////////////////
}
static void DB_IdxInitRecord(IdxRecord *idx, int linksOnly)
{
    if (!linksOnly)
    {
        idx->param[RECORDIDX_PatientBirthDate].XTag = DCM_PatientBirthDate;
        idx->param[RECORDIDX_PatientBirthDate].ValueLength = DA_MAX_LENGTH;
        idx->PatientBirthDate[0] = '\0';
        idx->param[RECORDIDX_PatientSex].XTag = DCM_PatientSex;
        idx->param[RECORDIDX_PatientSex].ValueLength = CS_MAX_LENGTH;
        idx->PatientSex[0] = '\0';
        idx->param[RECORDIDX_PatientName].XTag = DCM_PatientName;
        idx->param[RECORDIDX_PatientName].ValueLength = PN_MAX_LENGTH;
        idx->PatientName[0] = '\0';
        idx->param[RECORDIDX_PatientID].XTag = DCM_PatientID;
        idx->param[RECORDIDX_PatientID].ValueLength = LO_MAX_LENGTH;
        idx->PatientID[0] = '\0';
        idx->param[RECORDIDX_PatientBirthTime].XTag = DCM_PatientBirthTime;
        idx->param[RECORDIDX_PatientBirthTime].ValueLength = TM_MAX_LENGTH;
        idx->PatientBirthTime[0] = '\0';
        idx->param[RECORDIDX_OtherPatientIDs].XTag = DCM_RETIRED_OtherPatientIDs;
        idx->param[RECORDIDX_OtherPatientIDs].ValueLength = LO_MAX_LENGTH;
        idx->OtherPatientIDs[0] = '\0';
        idx->param[RECORDIDX_OtherPatientNames].XTag = DCM_OtherPatientNames;
        idx->param[RECORDIDX_OtherPatientNames].ValueLength = PN_MAX_LENGTH;
        idx->OtherPatientNames[0] = '\0';
        idx->param[RECORDIDX_EthnicGroup].XTag = DCM_EthnicGroup;
        idx->param[RECORDIDX_EthnicGroup].ValueLength = SH_MAX_LENGTH;
        idx->EthnicGroup[0] = '\0';
        idx->param[RECORDIDX_StudyDate].XTag = DCM_StudyDate;
        idx->param[RECORDIDX_StudyDate].ValueLength = DA_MAX_LENGTH;
        idx->StudyDate[0] = '\0';
        idx->param[RECORDIDX_StudyTime].XTag = DCM_StudyTime;
        idx->param[RECORDIDX_StudyTime].ValueLength = TM_MAX_LENGTH;
        idx->StudyTime[0] = '\0';
        idx->param[RECORDIDX_StudyID].XTag = DCM_StudyID;
        idx->param[RECORDIDX_StudyID].ValueLength = CS_MAX_LENGTH;
        idx->StudyID[0] = '\0';
        idx->param[RECORDIDX_StudyDescription].XTag = DCM_StudyDescription;
        idx->param[RECORDIDX_StudyDescription].ValueLength = LO_MAX_LENGTH;
        idx->StudyDescription[0] = '\0';
        idx->param[RECORDIDX_NameOfPhysiciansReadingStudy].XTag = DCM_NameOfPhysiciansReadingStudy;
        idx->param[RECORDIDX_NameOfPhysiciansReadingStudy].ValueLength = PN_MAX_LENGTH;
        idx->NameOfPhysiciansReadingStudy[0] = '\0';
        idx->param[RECORDIDX_AccessionNumber].XTag = DCM_AccessionNumber;
        idx->param[RECORDIDX_AccessionNumber].ValueLength = CS_MAX_LENGTH;
        idx->AccessionNumber[0] = '\0';
        idx->param[RECORDIDX_ReferringPhysicianName].XTag = DCM_ReferringPhysicianName;
        idx->param[RECORDIDX_ReferringPhysicianName].ValueLength = PN_MAX_LENGTH;
        idx->ReferringPhysicianName[0] = '\0';
        idx->param[RECORDIDX_ProcedureDescription].XTag = DCM_StudyDescription;
        idx->param[RECORDIDX_ProcedureDescription].ValueLength = LO_MAX_LENGTH;
        idx->ProcedureDescription[0] = '\0';
        idx->param[RECORDIDX_AttendingPhysiciansName].XTag = DCM_NameOfPhysiciansReadingStudy;
        idx->param[RECORDIDX_AttendingPhysiciansName].ValueLength = PN_MAX_LENGTH;
        idx->AttendingPhysiciansName[0] = '\0';
        idx->param[RECORDIDX_StudyInstanceUID].XTag = DCM_StudyInstanceUID;
        idx->param[RECORDIDX_StudyInstanceUID].ValueLength = UI_MAX_LENGTH;
        idx->StudyInstanceUID[0] = '\0';
        idx->param[RECORDIDX_OtherStudyNumbers].XTag = DCM_RETIRED_OtherStudyNumbers;
        idx->param[RECORDIDX_OtherStudyNumbers].ValueLength = IS_MAX_LENGTH;
        idx->OtherStudyNumbers[0] = '\0';
        idx->param[RECORDIDX_AdmittingDiagnosesDescription].XTag = DCM_AdmittingDiagnosesDescription;
        idx->param[RECORDIDX_AdmittingDiagnosesDescription].ValueLength = LO_MAX_LENGTH;
        idx->AdmittingDiagnosesDescription[0] = '\0';
        idx->param[RECORDIDX_PatientAge].XTag = DCM_PatientAge;
        idx->param[RECORDIDX_PatientAge].ValueLength = AS_MAX_LENGTH;
        idx->PatientAge[0] = '\0';
        idx->param[RECORDIDX_PatientSize].XTag = DCM_PatientSize;
        idx->param[RECORDIDX_PatientSize].ValueLength = DS_MAX_LENGTH;
        idx->PatientSize[0] = '\0';
        idx->param[RECORDIDX_PatientWeight].XTag = DCM_PatientWeight;
        idx->param[RECORDIDX_PatientWeight].ValueLength = DS_MAX_LENGTH;
        idx->PatientWeight[0] = '\0';
        idx->param[RECORDIDX_Occupation].XTag = DCM_Occupation;
        idx->param[RECORDIDX_Occupation].ValueLength = SH_MAX_LENGTH;
        idx->Occupation[0] = '\0';
        idx->param[RECORDIDX_SeriesNumber].XTag = DCM_SeriesNumber;
        idx->param[RECORDIDX_SeriesNumber].ValueLength = IS_MAX_LENGTH;
        idx->SeriesNumber[0] = '\0';
        idx->param[RECORDIDX_SeriesInstanceUID].XTag = DCM_SeriesInstanceUID;
        idx->param[RECORDIDX_SeriesInstanceUID].ValueLength = UI_MAX_LENGTH;
        idx->SeriesInstanceUID[0] = '\0';
        idx->param[RECORDIDX_Modality].XTag = DCM_Modality;
        idx->param[RECORDIDX_Modality].ValueLength = CS_MAX_LENGTH;
        idx->ImageNumber[0] = '\0';
        idx->param[RECORDIDX_ImageNumber].XTag = DCM_InstanceNumber;
        idx->param[RECORDIDX_ImageNumber].ValueLength = IS_MAX_LENGTH;
        idx->ImageNumber[0] = '\0';
        idx->param[RECORDIDX_SOPInstanceUID].XTag = DCM_SOPInstanceUID;
        idx->param[RECORDIDX_SOPInstanceUID].ValueLength = UI_MAX_LENGTH;
        idx->SOPInstanceUID[0] = '\0';
        idx->param[RECORDIDX_SeriesDate].XTag = DCM_SeriesDate;
        idx->param[RECORDIDX_SeriesDate].ValueLength = DA_MAX_LENGTH;
        idx->SeriesDate[0] = '\0';
        idx->param[RECORDIDX_SeriesTime].XTag = DCM_SeriesTime;
        idx->param[RECORDIDX_SeriesTime].ValueLength = TM_MAX_LENGTH;
        idx->SeriesTime[0] = '\0';
        idx->param[RECORDIDX_SeriesDescription].XTag = DCM_SeriesDescription;
        idx->param[RECORDIDX_SeriesDescription].ValueLength = LO_MAX_LENGTH;
        idx->SeriesDescription[0] = '\0';
        idx->param[RECORDIDX_ProtocolName].XTag = DCM_ProtocolName;
        idx->param[RECORDIDX_ProtocolName].ValueLength = LO_MAX_LENGTH;
        idx->ProtocolName[0] = '\0';
        idx->param[RECORDIDX_OperatorsName].XTag = DCM_OperatorsName;
        idx->param[RECORDIDX_OperatorsName].ValueLength = PN_MAX_LENGTH;
        idx->OperatorsName[0] = '\0';
        idx->param[RECORDIDX_PerformingPhysicianName].XTag = DCM_PerformingPhysicianName;
        idx->param[RECORDIDX_PerformingPhysicianName].ValueLength = PN_MAX_LENGTH;
        idx->PerformingPhysicianName[0] = '\0';
        idx->param[RECORDIDX_PresentationLabel].XTag = DCM_ContentLabel;
        idx->param[RECORDIDX_PresentationLabel].ValueLength = CS_LABEL_MAX_LENGTH;
        idx->PresentationLabel[0] = '\0';
        idx->param[RECORDIDX_IssuerOfPatientID].XTag = DCM_IssuerOfPatientID;
        idx->param[RECORDIDX_IssuerOfPatientID].ValueLength = LO_MAX_LENGTH;
        idx->IssuerOfPatientID[0] = '\0';
        idx->param[RECORDIDX_SpecificCharacterSet].XTag = DCM_SpecificCharacterSet;
        idx->param[RECORDIDX_SpecificCharacterSet].ValueLength = CS_MAX_LENGTH * 8;
        idx->SpecificCharacterSet[0] = '\0';

        /////////////////////////
        strcpy(idx->PatientBirthDate, "20190720");
        strcpy(idx->PatientSex, "F");
        strcpy(idx->PatientName, "ceshi_name");
        strcpy(idx->PatientID, "201907200001");
        strcpy(idx->PatientBirthTime, "154200");

        strcpy(idx->OtherPatientIDs, "201907200001");
        strcpy(idx->OtherPatientNames, "ceshi_name");
        strcpy(idx->PatientName, "ceshi_name");
        strcpy(idx->StudyDate, "20190718-20190719");
        strcpy(idx->StudyTime, "154200");

        strcpy(idx->StudyID, "20190720");
        strcpy(idx->StudyDescription, "this is a test study! ");
        strcpy(idx->StudyInstanceUID, "1.2.276.0.179081.1207");
        strcpy(idx->PatientAge, "1");
        strcpy(idx->SeriesNumber, "1");
        strcpy(idx->SeriesInstanceUID, "1.2.276.0.179081.1207.1");
        strcpy(idx->ImageNumber, "1");
        strcpy(idx->SpecificCharacterSet, "ISO_IR 100");
        ///////////////////////
    }
    idx->param[RECORDIDX_PatientBirthDate].PValueField = (char *)idx->PatientBirthDate;
    idx->param[RECORDIDX_PatientSex].PValueField = (char *)idx->PatientSex;
    idx->param[RECORDIDX_PatientName].PValueField = (char *)idx->PatientName;
    idx->param[RECORDIDX_PatientID].PValueField = (char *)idx->PatientID;
    idx->param[RECORDIDX_PatientBirthTime].PValueField = (char *)idx->PatientBirthTime;
    idx->param[RECORDIDX_OtherPatientIDs].PValueField = (char *)idx->OtherPatientIDs;
    idx->param[RECORDIDX_OtherPatientNames].PValueField = (char *)idx->OtherPatientNames;
    idx->param[RECORDIDX_EthnicGroup].PValueField = (char *)idx->EthnicGroup;
    idx->param[RECORDIDX_StudyDate].PValueField = (char *)idx->StudyDate;
    idx->param[RECORDIDX_StudyTime].PValueField = (char *)idx->StudyTime;
    idx->param[RECORDIDX_StudyID].PValueField = (char *)idx->StudyID;
    idx->param[RECORDIDX_StudyDescription].PValueField = (char *)idx->StudyDescription;
    idx->param[RECORDIDX_NameOfPhysiciansReadingStudy].PValueField = (char *)idx->NameOfPhysiciansReadingStudy;
    idx->param[RECORDIDX_AccessionNumber].PValueField = (char *)idx->AccessionNumber;
    idx->param[RECORDIDX_ReferringPhysicianName].PValueField = (char *)idx->ReferringPhysicianName;
    idx->param[RECORDIDX_ProcedureDescription].PValueField = (char *)idx->ProcedureDescription;
    idx->param[RECORDIDX_AttendingPhysiciansName].PValueField = (char *)idx->AttendingPhysiciansName;
    idx->param[RECORDIDX_StudyInstanceUID].PValueField = (char *)idx->StudyInstanceUID;
    idx->param[RECORDIDX_OtherStudyNumbers].PValueField = (char *)idx->OtherStudyNumbers;
    idx->param[RECORDIDX_AdmittingDiagnosesDescription].PValueField = (char *)idx->AdmittingDiagnosesDescription;
    idx->param[RECORDIDX_PatientAge].PValueField = (char *)idx->PatientAge;
    idx->param[RECORDIDX_PatientSize].PValueField = (char *)idx->PatientSize;
    idx->param[RECORDIDX_PatientWeight].PValueField = (char *)idx->PatientWeight;
    idx->param[RECORDIDX_Occupation].PValueField = (char *)idx->Occupation;
    idx->param[RECORDIDX_SeriesNumber].PValueField = (char *)idx->SeriesNumber;
    idx->param[RECORDIDX_SeriesInstanceUID].PValueField = (char *)idx->SeriesInstanceUID;
    idx->param[RECORDIDX_Modality].PValueField = (char *)idx->Modality;
    idx->param[RECORDIDX_ImageNumber].PValueField = (char *)idx->ImageNumber;
    idx->param[RECORDIDX_SOPInstanceUID].PValueField = (char *)idx->SOPInstanceUID;
    idx->param[RECORDIDX_SeriesDate].PValueField = (char *)idx->SeriesDate;
    idx->param[RECORDIDX_SeriesTime].PValueField = (char *)idx->SeriesTime;
    idx->param[RECORDIDX_SeriesDescription].PValueField = (char *)idx->SeriesDescription;
    idx->param[RECORDIDX_ProtocolName].PValueField = (char *)idx->ProtocolName;
    idx->param[RECORDIDX_OperatorsName].PValueField = (char *)idx->OperatorsName;
    idx->param[RECORDIDX_PerformingPhysicianName].PValueField = (char *)idx->PerformingPhysicianName;
    idx->param[RECORDIDX_PresentationLabel].PValueField = (char *)idx->PresentationLabel;
    idx->param[RECORDIDX_IssuerOfPatientID].PValueField = (char *)idx->IssuerOfPatientID;
    idx->param[RECORDIDX_SpecificCharacterSet].PValueField = (char *)idx->SpecificCharacterSet;
}
/********************
**      Start find in Database
**/
DB_QUERY_CLASS rootLevel;
DcmSpecificCharacterSet findRequestConverter;
OFString findRequestCharacterSet;
int idxCounter;
OFBool doCheckFindIdentifier;

DB_UidList *uidList;

void makeResponseList(DB_ElementList* &findResponseList,
    DB_ElementList       *findRequestList,
    IdxRecord               *idxRec
    )
{
    int i;
    DB_ElementList *pRequestList = NULL;
    DB_ElementList *plist = NULL;
    DB_ElementList *last = NULL;

    //DB_ElementList * findResponseList = NULL;

    /*** For each element in Request identifier
    **/
    for (pRequestList = findRequestList; pRequestList; pRequestList = pRequestList->next)
    {
        /*** Find Corresponding Tag in index record
        **/
        for (i = 0; i < NBPARAMETERS; i++)
        {
            if (idxRec->param[i].XTag == pRequestList->elem.XTag)
                break;
        }

        /*** If Tag not found, skip the element
        **/
        if (i >= NBPARAMETERS)
            continue;

        /*** Append index record element to response list
        **/

        plist = new DB_ElementList;
        if (plist == NULL)
        {
            DCMQRDB_ERROR("makeResponseList: out of memory");
            return;
        }

        DB_DuplicateElement(&idxRec->param[i], &plist->elem);

        if (/*phandle->*/findResponseList == NULL)
        {
            /*phandle->*/findResponseList = last = plist;
            findResponseList->first = plist;
        }
        else
        {
            last->next = plist;
            last = plist;
            findResponseList->last = plist;
        }
    }

    /** Specific Character Set stuff
    **/

    if (idxRec->param[RECORDIDX_SpecificCharacterSet].ValueLength)
    {
        plist = new DB_ElementList;
        if (plist == NULL)
        {
            DCMQRDB_ERROR("makeResponseList: out of memory");
            return;
        }

        DB_DuplicateElement(&idxRec->param[RECORDIDX_SpecificCharacterSet], &plist->elem);

        if (/*phandle->*/findResponseList == NULL)
        {
            /*phandle->*/findResponseList = last = plist;
            findResponseList->first = plist;
        }
        else
        {
            last->next = plist;
            last = plist;
            findResponseList->last = plist;
        }
    }
}


static char *DB_strdup(const char* str)
{
    if (str == NULL)
        return NULL;
    char* s = (char*)malloc(strlen(str) + 1);
    strcpy(s, str);
    return s;
}


static void DB_UIDAddFound(
    DB_UidList *uidList,
    IdxRecord               *idxRec,
    DB_LEVEL queryLevel
    )
{
    DB_UidList *plist = NULL;
    plist = (DB_UidList *)malloc(sizeof(DB_UidList));
    if (plist == NULL)
    {
        DCMQRDB_ERROR("DB_UIDAddFound: out of memory");
        return;
    }
    plist->next = /*phandle->*/uidList;
    plist->patient = NULL;
    plist->study = NULL;
    plist->serie = NULL;
    plist->image = NULL;

    if (/*(int)phandle->*/queryLevel >= PATIENT_LEVEL)
        plist->patient = DB_strdup((char *)idxRec->PatientID);
    if (/*(int)phandle->*/queryLevel >= STUDY_LEVEL)
        plist->study = DB_strdup((char *)idxRec->StudyInstanceUID);
    if (/*(int)phandle->*/queryLevel >= SERIE_LEVEL)
        plist->serie = DB_strdup((char *)idxRec->SeriesInstanceUID);
    if (/*(int)phandle->*/queryLevel >= IMAGE_LEVEL)
        plist->image = DB_strdup((char *)idxRec->SOPInstanceUID);

    /*phandle->*/uidList = plist;
}

OFCondition startFindRequestFromSql(DB_ElementList* &findRequestList,
    DB_ElementList* &findResponseList,
    DB_LEVEL &queryLevel,
    const char      *SOPClassUID,
    DcmDataset      *findRequestIdentifiers,
    DcmQueryRetrieveDatabaseStatus  *status)
{
    DB_SmallDcmElmt     elem;
    DB_ElementList      *plist = NULL;
    DB_ElementList      *last = NULL;
    int                 MatchFound = 1;
    IdxRecord           idxRec;
    DB_LEVEL            qLevel = PATIENT_LEVEL; // highest legal level for a query in the current model
    DB_LEVEL            lLevel = IMAGE_LEVEL;   // lowest legal level for a query in the current model

    OFCondition         cond = EC_Normal;
    OFBool qrLevelFound = OFFalse;

    /**** Is SOPClassUID supported ?
    ***/

    if (strcmp(SOPClassUID, UID_FINDPatientRootQueryRetrieveInformationModel) == 0)
    {
        rootLevel = PATIENT_ROOT;
    }
    else if (strcmp(SOPClassUID, UID_FINDStudyRootQueryRetrieveInformationModel) == 0)
    {
        rootLevel = STUDY_ROOT;
    }
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    else if (strcmp(SOPClassUID, UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel) == 0)
    {
        rootLevel = PATIENT_STUDY;
    }
#endif
    else
    {
        status->setStatus(STATUS_FIND_Refused_SOPClassNotSupported);
        return (QR_EC_IndexDatabaseError);
    }

    /**** Parse Identifiers in the Dicom Object
    **** Find Query Level and construct a list
    **** of query identifiers
    ***/
    OFString PatientName;
    if (findRequestIdentifiers->findAndGetOFString(DCM_PatientName, PatientName).bad())
    {
        PatientName.clear();
    }

    OFString StudyDateStart, StudyDateEnd;
    if (findRequestIdentifiers->findAndGetOFString(DCM_StudyDate, StudyDateStart).bad())
    {
        StudyDateStart.clear();
    }
    if (findRequestIdentifiers->findAndGetOFStringArray(DCM_SpecificCharacterSet, findRequestCharacterSet).bad())
    {
        findRequestCharacterSet.clear();
    }
    //DcmElement *dcmitem;
    //findRequestIdentifiers->findAndGetElement(DCM_StudyDate, dcmitem);
    //int elemCount = dcmitem->;

    if (findRequestConverter && findRequestConverter.getSourceCharacterSet() != findRequestCharacterSet)
    {
        findRequestConverter.clear();
    }
    findRequestList = NULL;

    int elemCount = OFstatic_cast(int, findRequestIdentifiers->card());
    for (int elemIndex = 0; elemIndex < elemCount; elemIndex++)
    {
        DcmElement* dcelem = findRequestIdentifiers->getElement(elemIndex);
        elem.XTag = dcelem->getTag().getXTag();
        if (elem.XTag == DCM_QueryRetrieveLevel || DB_TagSupported(elem.XTag))
        {
            elem.ValueLength = dcelem->getLength();
            if (elem.ValueLength == 0)
            {
                elem.PValueField = NULL;
            }
            else if ((elem.PValueField = OFstatic_cast(char*, malloc(OFstatic_cast(size_t, elem.ValueLength + 1)))) == NULL)
            {
                status->setStatus(STATUS_FIND_Refused_OutOfResources);
                return (QR_EC_IndexDatabaseError);
            }
            else
            {
                /* only char string type tags are supported at the moment */
                char *s = NULL;
                dcelem->getString(s);
                strcpy(elem.PValueField, s);
            }
            /** If element is the Query Level, store it in handle
            */
            if (elem.XTag == DCM_QueryRetrieveLevel && elem.PValueField)
            {
                char *pc;
                char level[50];

                strncpy(level, (char*)elem.PValueField,
                    (elem.ValueLength < 50) ? (size_t)(elem.ValueLength) : 49);

                /*** Skip this two lines if you want strict comparison
                **/

                for (pc = level; *pc; pc++)
                {
                    *pc = ((*pc >= 'a') && (*pc <= 'z')) ? 'A' - 'a' + *pc : *pc;
                }

                if (strncmp(level, PATIENT_LEVEL_STRING, strlen(PATIENT_LEVEL_STRING)) == 0)
                    queryLevel = PATIENT_LEVEL;
                else if (strncmp(level, STUDY_LEVEL_STRING, strlen(STUDY_LEVEL_STRING)) == 0)
                    queryLevel = STUDY_LEVEL;
                else if (strncmp(level, SERIE_LEVEL_STRING, strlen(SERIE_LEVEL_STRING)) == 0)
                    queryLevel = SERIE_LEVEL;
                else if (strncmp(level, IMAGE_LEVEL_STRING, strlen(IMAGE_LEVEL_STRING)) == 0)
                    queryLevel = IMAGE_LEVEL;
                else
                {
                    if (elem.PValueField)
                    {
                        free(elem.PValueField);
                    }
#ifdef DEBUG
                    DCMQRDB_DEBUG("DB_startFindRequest () : Illegal query level (" << level << ")");
#endif
                    status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                    return (QR_EC_IndexDatabaseError);
                }
                qrLevelFound = OFTrue;
            }
            else
            {
                /** Else it is a query identifier.
                ** Append it to our RequestList if it is supported
                */
                if (DB_TagSupported(elem.XTag))
                {
                    plist = new DB_ElementList;
                    if (plist == NULL)
                    {
                        status->setStatus(STATUS_FIND_Refused_OutOfResources);
                        return (QR_EC_IndexDatabaseError);
                    }
                    DB_DuplicateElement(&elem, &(plist->elem));
                    if (findRequestList == NULL)
                    {
                        findRequestList = last = plist;
                    }
                    else
                    {
                        last->next = plist;
                        last = plist;
                    }
                }
            }
            if (elem.PValueField)
            {
                free(elem.PValueField);
            }
        }
    }

    if (!qrLevelFound)
    {
        /* The Query/Retrieve Level is missing */
        status->setStatus(STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass);
        DCMQRDB_WARN("DB_startFindRequest(): missing Query/Retrieve Level");
        idxCounter = -1;
        DB_FreeElementList(findRequestList);
        findRequestList = NULL;
        return (QR_EC_IndexDatabaseError);
    }

    switch (rootLevel)
    {
    case PATIENT_ROOT:
        qLevel = PATIENT_LEVEL;
        lLevel = IMAGE_LEVEL;
        break;
    case STUDY_ROOT:
        qLevel = STUDY_LEVEL;
        lLevel = IMAGE_LEVEL;
        break;
    case PATIENT_STUDY:
        qLevel = PATIENT_LEVEL;
        lLevel = STUDY_LEVEL;
        break;
    }
    DB_lock(OFFalse);
    if (cond != EC_Normal)
    {
        /*handle_->*/idxCounter = -1;
        DB_FreeElementList(/*handle_->*/findRequestList);
        /*handle_->*/findRequestList = NULL;
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_FIND_Failed_UnableToProcess");
#endif
        status->setStatus(STATUS_FIND_Failed_UnableToProcess);
        return (cond);
    }
    if (MatchFound)
    {
        DB_IdxInitRecord(&idxRec, 0);
        static int a = 0;
        if (a%2 == 0)
        {
            a++;
            DB_IdxSetRecord(&idxRec);
        }
        //DB_UIDAddFound(uidList, &idxRec, queryLevel);
        makeResponseList(findResponseList, findRequestList, &idxRec);
        //DB_IdxSetRecord(&idxRec);
        //makeResponseList(findResponseList->last->next, findRequestList, &idxRec);

#ifdef DEBUG
        DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_Pending");
#endif
        status->setStatus(STATUS_Pending);
        return (EC_Normal);
    }
    else
    {
        return (EC_Normal);
    }
}

OFCondition nextFindResponse(DB_ElementList* &findResponseList,
    DcmDataset      **findResponseIdentifiers,
    DB_LEVEL queryLevel,
    DcmQueryRetrieveDatabaseStatus  *status,
    const DcmQueryRetrieveCharacterSetOptions& characterSetOptions)
{
    DB_ElementList      *plist = NULL;
    int                 MatchFound = OFFalse;
    IdxRecord           idxRec;
    DB_LEVEL            qLevel = PATIENT_LEVEL;
    const char          *queryLevelString = NULL;
    OFCondition         cond = EC_Normal;

    /***** Create the response (findResponseIdentifiers) using
    ***** the last find done and saved in handle findResponseList
    ****/

    *findResponseIdentifiers = new DcmDataset;
    if (*findResponseIdentifiers != NULL)
    {
        for (plist = /*handle_->*/findResponseList; plist != NULL; plist = plist->next)
        {
            DcmTag t(plist->elem.XTag);
            DcmElement *dce = DcmItem::newDicomElement(t);
            if (dce == NULL)
            {
                status->setStatus(STATUS_FIND_Refused_OutOfResources);
                return QR_EC_IndexDatabaseError;
            }
            if (plist->elem.PValueField != NULL &&
                strlen(plist->elem.PValueField) > 0)
            {
                OFCondition ec = dce->putString(plist->elem.PValueField);
                if (ec != EC_Normal)
                {
                    DCMQRDB_WARN("dbfind: DB_nextFindResponse: cannot put()");
                    status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                    return QR_EC_IndexDatabaseError;
                }
            }
            OFCondition ec = (*findResponseIdentifiers)->insert(dce, OFTrue /*replaceOld*/);
            if (ec != EC_Normal)
            {
                DCMQRDB_WARN("dbfind: DB_nextFindResponse: cannot insert()");
                status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                return QR_EC_IndexDatabaseError;
            }
        }

        /*** Append the Query level
        **/

        switch (/*handle_->*/queryLevel)
        {
        case PATIENT_LEVEL:
            queryLevelString = PATIENT_LEVEL_STRING;
            break;
        case STUDY_LEVEL:
            queryLevelString = STUDY_LEVEL_STRING;
            break;
        case SERIE_LEVEL:
            queryLevelString = SERIE_LEVEL_STRING;
            break;
        case IMAGE_LEVEL:
            queryLevelString = IMAGE_LEVEL_STRING;
            break;
        }
        DU_putStringDOElement(*findResponseIdentifiers, DCM_QueryRetrieveLevel, queryLevelString);

#ifdef DCMTK_ENABLE_CHARSET_CONVERSION
        OFString specificCharacterSet;
        if ((*findResponseIdentifiers)->findAndGetOFStringArray(DCM_SpecificCharacterSet, specificCharacterSet).bad())
            specificCharacterSet.clear();

        const OFString* destinationCharacterSet = NULL;
        const OFString* fallbackCharacterSet = NULL;

        if (characterSetOptions.flags & DcmQueryRetrieveCharacterSetOptions::Override)
        {
            destinationCharacterSet = &characterSetOptions.characterSet;
            if (
                (characterSetOptions.flags & DcmQueryRetrieveCharacterSetOptions::Fallback) &&
                characterSetOptions.characterSet != handle_->findRequestCharacterSet
                )
            {
                fallbackCharacterSet = &handle_->findRequestCharacterSet;
            }
        }
        else
        {
            destinationCharacterSet = &handle_->findRequestCharacterSet;
            if (
                (characterSetOptions.flags & DcmQueryRetrieveCharacterSetOptions::Fallback) &&
                characterSetOptions.characterSet != handle_->findRequestCharacterSet
                )
            {
                fallbackCharacterSet = &characterSetOptions.characterSet;
            }
        }

        if (isConversionNecessary(specificCharacterSet, *destinationCharacterSet))
        {
            OFCondition status = (*findResponseIdentifiers)->convertCharacterSet(
                specificCharacterSet,
                *destinationCharacterSet,
                characterSetOptions.conversionFlags,
                OFTrue);
            if (status.bad())
            {
                DCMQRDB_WARN("Converting response from character set \""
                    << characterSetName(specificCharacterSet)
                    << "\" to character set \""
                    << characterSetName(*destinationCharacterSet)
                    << "\" failed, (error message: " << status.text() << ')');
                if (fallbackCharacterSet && isConversionNecessary(specificCharacterSet, *fallbackCharacterSet))
                {
                    DCMQRDB_INFO("Trying to convert response from character set \""
                        << characterSetName(specificCharacterSet)
                        << "\" to fall-back character set \""
                        << characterSetName(*fallbackCharacterSet) << "\" instead");
                    status = (*findResponseIdentifiers)->convertCharacterSet(
                        specificCharacterSet,
                        *fallbackCharacterSet,
                        characterSetOptions.conversionFlags,
                        OFTrue);
                    if (status.bad())
                    {
                        DCMQRDB_WARN("Converting response from character set \""
                            << characterSetName(specificCharacterSet)
                            << "\" to character set \""
                            << characterSetName(*fallbackCharacterSet)
                            << "\" failed, (error message: " << status.text() << ')');
                    }
                    else
                    {
                        DCMQRDB_INFO("Successfully converted response from character set \""
                            << characterSetName(specificCharacterSet)
                            << "\" to character set \""
                            << characterSetName(*fallbackCharacterSet) << "\"");
                    }
                }
                else if (fallbackCharacterSet)
                {
                    DCMQRDB_INFO("Conversion to fall-back character set \""
                        << characterSetName(*fallbackCharacterSet)
                        << "\" is not necessary, since the original character set is compatible");
                }
            }
            else
            {
                DCMQRDB_INFO("Successfully converted response from character set \""
                    << characterSetName(specificCharacterSet)
                    << "\" to character set \""
                    << characterSetName(*destinationCharacterSet)
                    << "\"");
            }
        }
#endif

#ifdef DEBUG
        DCMQRDB_DEBUG("DB: findResponseIdentifiers:" << OFendl
            << DcmObject::PrintHelper(**findResponseIdentifiers));
#endif
    }
    else
    {
        //DB_unlock();
        return (QR_EC_IndexDatabaseError);
    }

    switch (/*handle_->*/rootLevel)
    {
    case PATIENT_ROOT:
        qLevel = PATIENT_LEVEL;
        break;
    case STUDY_ROOT:
        qLevel = STUDY_LEVEL;
        break;
    case PATIENT_STUDY:
        qLevel = PATIENT_LEVEL;
        break;
    }
    MatchFound = OFFalse;
    cond = EC_Normal;

#ifdef DEBUG
    DCMQRDB_DEBUG("DB_nextFindResponse () : STATUS_Pending");
#endif
    status->setStatus(STATUS_Pending);
    return (EC_Normal);
}
OFCondition cancelFindRequest(DcmQueryRetrieveDatabaseStatus *status)
{

    //handle_->idxCounter = -1;
    //DB_FreeElementList(handle_->findRequestList);
    //handle_->findRequestList = NULL;
    //DB_FreeElementList(handle_->findResponseList);
    //handle_->findResponseList = NULL;
    //DB_FreeUidList(handle_->uidList);
    //handle_->uidList = NULL;

    status->setStatus(STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest);

    //DB_unlock();

    return (EC_Normal);
}
void DcmQueryRetrieveFindContext::callbackHandler(
    /* in */
    OFBool cancelled, T_DIMSE_C_FindRQ *request,
    DcmDataset *requestIdentifiers, int responseCount,
    /* out */
    T_DIMSE_C_FindRSP *response,
    DcmDataset **responseIdentifiers,
    DcmDataset **stDetail)
{
    ////////////////////
    DB_ElementList *findRequestList = NULL;
    DB_ElementList *findResponseList = NULL;
    DB_LEVEL queryLevel;
    ////////////////////
    OFCondition dbcond = EC_Normal;
    DcmQueryRetrieveDatabaseStatus dbStatus(priorStatus);

    if (responseCount == 1)
    {
        /* start the database search */
        DCMQRDB_INFO("Find SCP Request Identifiers:" << OFendl << DcmObject::PrintHelper(*requestIdentifiers));
        //dbcond = dbHandle.startFindRequest(
        //    request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        dbcond = startFindRequestFromSql(findRequestList, findResponseList, queryLevel, request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        if (dbcond.bad())
        {
            DCMQRDB_ERROR("findSCP: Database: startFindRequest Failed ("
                << DU_cfindStatusString(dbStatus.status()) << "):");
        }
    }

    /* only cancel if we have pending responses */
    if (cancelled && DICOM_PENDING_STATUS(dbStatus.status()))
    {
        /*dbHandle.*/cancelFindRequest(&dbStatus);
    }

    if (DICOM_PENDING_STATUS(dbStatus.status()))
    {
        if (false)
        {
            dbcond = dbHandle.nextFindResponse(responseIdentifiers, &dbStatus, characterSetOptions);
        }
        else
        {
            nextFindResponse(findResponseList, responseIdentifiers, queryLevel, &dbStatus, characterSetOptions);
        }
        if (dbcond.bad())
        {
            DCMQRDB_ERROR("findSCP: Database: nextFindResponse Failed ("
                << DU_cfindStatusString(dbStatus.status()) << "):");
        }
    }

    if (*responseIdentifiers != NULL)
    {
        if (!DU_putStringDOElement(*responseIdentifiers, DCM_RetrieveAETitle, ourAETitle.c_str()))
        {
            DCMQRDB_ERROR("DO: adding Retrieve AE Title");
        }
    }

    /* set response status */
    response->DimseStatus = dbStatus.status();
    *stDetail = dbStatus.extractStatusDetail();

    OFString str;
    DCMQRDB_INFO("Find SCP Response " << responseCount << " [status: "
        << DU_cfindStatusString(dbStatus.status()) << "]");
    DCMQRDB_DEBUG(DIMSE_dumpMessage(str, *response, DIMSE_OUTGOING));
    if (DICOM_PENDING_STATUS(dbStatus.status()) && (*responseIdentifiers != NULL))
    {
        DCMQRDB_DEBUG("Find SCP Response Identifiers:" << OFendl << DcmObject::PrintHelper(**responseIdentifiers));
    }
    if (*stDetail)
    {
        DCMQRDB_DEBUG("  Status detail:" << OFendl << DcmObject::PrintHelper(**stDetail));
    }
}

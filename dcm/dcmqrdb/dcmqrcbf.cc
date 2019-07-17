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

        else if (level < queryLevel) {

            /** For this level, only unique keys are allowed
            ** Parse the request list elements referring to
            ** this level.
            ** Check that only unique key attr are provided
            */

            int uniqueKeyFound = OFFalse;
            for (plist = findRequestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != level)
                    continue;
                DB_GetTagKeyAttr(plist->elem.XTag, &XTagType);
                if (XTagType != UNIQUE_KEY) {
                    DCMQRDB_DEBUG("Non Unique Key found (level " << level << ")");
                    return QR_EC_IndexDatabaseError;
                }
                else if (uniqueKeyFound) {
                    DCMQRDB_DEBUG("More than one Unique Key found (level " << level << ")");
                    return QR_EC_IndexDatabaseError;
                }
                else
                    uniqueKeyFound = OFTrue;
            }
        }

        /**** If current level is the QueryLevel
        ***/

        else if (level == queryLevel) {

            /** For this level, all keys are allowed
            ** Parse the request list elements referring to
            ** this level.
            ** Check that at least one key is provided
            */

            int atLeastOneKeyFound = OFFalse;
            for (plist = findRequestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != level)
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (!atLeastOneKeyFound) {
                DCMQRDB_DEBUG("No Key found at query level (level " << level << ")");
                return QR_EC_IndexDatabaseError;
            }
        }

        /**** If current level beyond the QueryLevel
        ***/

        else if (level > queryLevel) {

            /** For this level, no key is allowed
            ** Parse the request list elements referring to
            ** this level.
            ** Check that no key is provided
            */

            int atLeastOneKeyFound = OFFalse;
            for (plist = findRequestList; plist; plist = plist->next) {
                DB_GetTagLevel(plist->elem.XTag, &XTagLevel);
                if (XTagLevel != level)
                    continue;
                atLeastOneKeyFound = OFTrue;
            }
            if (atLeastOneKeyFound) {
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
    if (lst->elem.PValueField != NULL) {
        free((char *)lst->elem.PValueField);
    }
    delete lst;
    return (cond);
}
/********************
**      Start find in Database
**/
DB_QUERY_CLASS rootLevel;
DcmSpecificCharacterSet findRequestConverter;
OFString findRequestCharacterSet;
int idxCounter;
OFBool doCheckFindIdentifier;

OFCondition startFindRequestFromSql(
                                    const char      *SOPClassUID,
                                    DcmDataset      *findRequestIdentifiers,
                                    DcmQueryRetrieveDatabaseStatus  *status)
{
    DB_ElementList *findRequestList = NULL;
    DB_LEVEL queryLevel;
    DB_SmallDcmElmt     elem;
    DB_ElementList      *plist = NULL;
    DB_ElementList      *last = NULL;
    int                 MatchFound;
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

    if (findRequestIdentifiers->findAndGetOFStringArray(DCM_SpecificCharacterSet, findRequestCharacterSet).bad())
    {
        findRequestCharacterSet.clear();
    }

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
                else if (strncmp(level, STUDY_LEVEL_STRING,  strlen(STUDY_LEVEL_STRING)) == 0)
                    queryLevel = STUDY_LEVEL;
                else if (strncmp(level, SERIE_LEVEL_STRING,  strlen(SERIE_LEVEL_STRING)) == 0)
                    queryLevel = SERIE_LEVEL;
                else if (strncmp(level, IMAGE_LEVEL_STRING,  strlen(IMAGE_LEVEL_STRING)) == 0)
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

    /**** Test the consistency of the request list
    ***/

    if (doCheckFindIdentifier)
    {
        cond = testFindRequestList(findRequestList, queryLevel, qLevel, lLevel);
        if (cond != EC_Normal)
        {
            idxCounter = -1;
            DB_FreeElementList(findRequestList);
            findRequestList = NULL;
#ifdef DEBUG
            DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass - Invalid RequestList");
#endif
            status->setStatus(STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass);
            return (cond);
        }
    }

    /**** Goto the beginning of Index File
    **** Then find the first matching image
    ***/

    DB_lock(OFFalse);

    //DB_IdxInitLoop(&(handle_->idxCounter));
    //MatchFound = OFFalse;
    //cond = EC_Normal;

    //CharsetConsideringMatcher dbmatch(*handle_);
    //while (1)
    //{

    //    /*** Exit loop if read error (or end of file)
    //    **/

    //    if (DB_IdxGetNext(&(handle_->idxCounter), &idxRec) != EC_Normal)
    //        break;

    //    /*** Exit loop if error or matching OK
    //    **/

    //    dbmatch.setRecord(idxRec);
    //    cond = hierarchicalCompare(handle_, &idxRec, qLevel, qLevel, &MatchFound, dbmatch);
    //    if (cond != EC_Normal)
    //        break;
    //    if (MatchFound)
    //        break;
    //}

    /**** If an error occurred in Matching function
    ****    return a failed status
    ***/

    if (cond != EC_Normal)
    {
        /*handle_->*/idxCounter = -1;
        DB_FreeElementList(/*handle_->*/findRequestList);
        /*handle_->*/findRequestList = NULL;
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_FIND_Failed_UnableToProcess");
#endif
        status->setStatus(STATUS_FIND_Failed_UnableToProcess);

        //DB_unlock();

        return (cond);
    }


    /**** If a matching image has been found,
    ****         add index record to UID found list
    ****    prepare Response List in handle
    ****    return status is pending
    ***/

    if (MatchFound)
    {
        //DB_UIDAddFound(handle_, &idxRec);
        //makeResponseList(handle_, &idxRec);
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_Pending");
#endif
        status->setStatus(STATUS_Pending);
        return (EC_Normal);
    }

    /**** else no matching image has been found,
    ****    free query identifiers list
    ****    status is success
    ***/

    else
    {
//        handle_->idxCounter = -1;
//        DB_FreeElementList(handle_->findRequestList);
//        handle_->findRequestList = NULL;
//#ifdef DEBUG
//        DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_Success");
//#endif
//        status->setStatus(STATUS_Success);
//
//        DB_unlock();

        return (EC_Normal);
    }

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
    OFCondition dbcond = EC_Normal;
    DcmQueryRetrieveDatabaseStatus dbStatus(priorStatus);

    if (responseCount == 1)
    {
        /* start the database search */
        DCMQRDB_INFO("Find SCP Request Identifiers:" << OFendl << DcmObject::PrintHelper(*requestIdentifiers));
        //dbcond = dbHandle.startFindRequest(
        //    request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        dbcond = startFindRequestFromSql(request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
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
        dbcond = dbHandle.nextFindResponse(responseIdentifiers, &dbStatus, characterSetOptions);
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

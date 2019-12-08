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
 *  Purpose: class DcmQueryRetrieveMoveContext
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/dcmqrdb/dcmqrcbm.h"

#include "dcmtk/dcmqrdb/dcmqrcnf.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmqrdb/dcmqropt.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmqrdb/dcmqrdbs.h"
#include "dcmtk/dcmqrdb/dcmqrdbi.h"

#include "Units.h"

BEGIN_EXTERN_C

#ifdef HAVE_FCNTL_H
#include <fcntl.h>       /* needed on Solaris for O_RDONLY */
#endif

END_EXTERN_C

static OFLogger dcmqrscpLogger = OFLog::getLogger("dcmtk.dcmqr.apps.");

static void moveSubOpProgressCallback(void * /* callbackData */,
T_DIMSE_StoreProgress *progress,
T_DIMSE_C_StoreRQ * /*req*/)
{
    //DcmQueryRetrieveMoveContext *context = OFstatic_cast(DcmQueryRetrieveMoveContext *, callbackData);
    // We can't use oflog for the pdu output, but we use a special logger for
    // generating this output. If it is set to level "INFO" we generate the
    // output, if it's set to "DEBUG" then we'll assume that there is debug output
    // generated for each PDU elsewhere.
    //OFLogger progressLogger = OFLog::getLogger("dcmtk.dcmqrdb.progress");
    //if (progressLogger.getChainedLogLevel() == OFLogger::INFO_LOG_LEVEL)
    //{
    //    switch (progress->state)
    //    {
    //    case DIMSE_StoreBegin:
    //        printf("XMIT: ");
    //        break;
    //    case DIMSE_StoreEnd:
    //        printf("\n");
    //        break;
    //    default:
    //        putchar('.');
    //        break;
    //    }
    //    fflush(stdout);
    //}
}

/////////_____________________________________________________________
//______________________________________________________________________________________
OFCondition DcmQueryRetrieveMoveContext::startMoveRequest(
    const char      *SOPClassUID,
    DcmDataset      *moveRequestIdentifiers,
    DcmQueryRetrieveDatabaseStatus  *status, OFList<OFString> *imagedir)
{
    //OFString PatientName;
    //if (moveRequestIdentifiers->findAndGetOFString(DCM_PatientName, PatientName).bad())
    //{
    //    PatientName.clear();
    //}
    OFString StudyInstanceUID;
    if (moveRequestIdentifiers->findAndGetOFString(DCM_StudyInstanceUID, StudyInstanceUID).bad())
    {
        StudyInstanceUID.clear();
        status->setStatus(STATUS_Success);
        return EC_Normal;
    }
    OFString SeriesInstanceUID;
    if (moveRequestIdentifiers->findAndGetOFString(DCM_SeriesInstanceUID, SeriesInstanceUID).bad())
    {
        SeriesInstanceUID.clear();
    }

    OFHashValue path = CreateHashValue(StudyInstanceUID.c_str(), StudyInstanceUID.length());
    OFString hash_dir = longToString(path.first) + "/" + longToString(path.second);
    OFString temp_dir = "/Images/" + hash_dir + "/" + StudyInstanceUID;
    //OFLOG_INFO(dcmqrscpLogger, "---------hash_dir:" + hash_dir + " ----------------------");
    //OFLOG_INFO(dcmqrscpLogger, "---------study_dir:" + hash_dir + " ----------------------");
    OFString find_dir;
    OFBool find = OFFalse;
    OFListIterator(OFString) if_iter;
    OFListIterator(OFString) if_last;
    if (imagedir != NULL)
    {
        if_iter = imagedir->begin();
        if_last = imagedir->end();
    }
    else
    {
        if_iter = m_config->getStoreDir()->begin();
        if_last = m_config->getStoreDir()->end();
    }
    while (if_iter != if_last)
    {
        OFString dir = *if_iter;
        if (OFStandard::dirExists(dir + temp_dir))
        {
            find = OFTrue;
            find_dir = dir + temp_dir;
            break;
        }
        if_iter++;
    }

    if (find && !find_dir.empty())
    {
        SearchDirFile(find_dir, "dcm", m_matchingFiles);
    }
    else
    {
        status->setStatus(STATUS_Success);
        return EC_Normal;
    }

    if (m_matchingFiles.size() > 0)
    {
        status->setStatus(STATUS_Pending);
    }
    else
    {
        status->setStatus(STATUS_Success);
    }
    return EC_Normal;
}

void DcmQueryRetrieveMoveContext::callbackHandler(
    /* in */
    OFBool cancelled, T_DIMSE_C_MoveRQ *request,
    DcmDataset *requestIdentifiers, int responseCount,
    /* out */
    T_DIMSE_C_MoveRSP *response, DcmDataset **stDetail,
    DcmDataset **responseIdentifiers, OFList<OFString> *imagedir, OFList<QueryClientInfo> *queryclient)
{
    OFCondition cond = EC_Normal;
    OFCondition dbcond = EC_Normal;
    DcmQueryRetrieveDatabaseStatus dbStatus(priorStatus);

    if (responseCount == 1)
    {
        m_matchingFiles.clear();
        /* start the database search */
        DCMQRDB_INFO("Move SCP Request Identifiers:" << OFendl << DcmObject::PrintHelper(*requestIdentifiers));

        //if (false)
        //{
        //    dbcond = dbHandle.startMoveRequest(request->AffectedSOPClassUID, requestIdentifiers, &dbStatus);
        //}
        //else
        //{
        dbcond = startMoveRequest(request->AffectedSOPClassUID, requestIdentifiers, &dbStatus,imagedir);
        //}

        if (dbcond.bad())
        {
            DCMQRDB_ERROR("moveSCP: Database: startMoveRequest Failed ("
                << DU_cmoveStatusString(dbStatus.status()) << "):");
        }

        if (dbStatus.status() == STATUS_Pending)
        {
            /* If we are going to be performing sub-operations, build
             * a new association to the move destination.
             */
            cond = buildSubAssociation(request,queryclient);
            if (cond == QR_EC_InvalidPeer)
            {
                dbStatus.setStatus(STATUS_MOVE_Failed_MoveDestinationUnknown);
            }
            else if (cond.bad())
            {
                /* failed to build association, must fail move */
                failAllSubOperations(&dbStatus);
            }
        }
    }

    /* only cancel if we have pending status */
    if (cancelled && dbStatus.status() == STATUS_Pending)
    {
        dbHandle.cancelMoveRequest(&dbStatus);
    }

    if (dbStatus.status() == STATUS_Pending)
    {
        moveNextImage(&dbStatus);
    }

    if (dbStatus.status() != STATUS_Pending)
    {
        /*
         * Tear down sub-association (if it exists).
         */
        closeSubAssociation();

        /*
         * Need to adjust the final status if any sub-operations failed or
         * had warnings
         */
        if (nFailed > 0 || nWarning > 0)
        {
            dbStatus.setStatus(STATUS_MOVE_Warning_SubOperationsCompleteOneOrMoreFailures);
        }
        /*
         * if all the sub-operations failed then we need to generate a failed or refused status.
         * cf. DICOM part 4, C.4.2.3.1
         * we choose to generate a "Refused - Out of Resources - Unable to perform suboperations" status.
         */
        if ((nFailed > 0) && ((nCompleted + nWarning) == 0))
        {
            dbStatus.setStatus(STATUS_MOVE_Refused_OutOfResourcesSubOperations);
        }
    }

    if (dbStatus.status() != STATUS_Success &&
        dbStatus.status() != STATUS_Pending)
    {
        /*
         * May only include response identifiers if not Success
         * and not Pending
         */
        buildFailedInstanceList(responseIdentifiers);
    }

    /* set response status */
    response->DimseStatus = dbStatus.status();
    response->NumberOfRemainingSubOperations = nRemaining;
    response->NumberOfCompletedSubOperations = nCompleted;
    response->NumberOfFailedSubOperations = nFailed;
    response->NumberOfWarningSubOperations = nWarning;
    *stDetail = dbStatus.extractStatusDetail();

    OFString str;
    DCMQRDB_INFO("Move SCP Response " << responseCount << " [status: "
        << DU_cmoveStatusString(dbStatus.status()) << "]");
    DCMQRDB_DEBUG(DIMSE_dumpMessage(str, *response, DIMSE_OUTGOING));
    if (DICOM_PENDING_STATUS(dbStatus.status()) && (*responseIdentifiers != NULL))
    {
        DCMQRDB_DEBUG("Move SCP Response Identifiers:" << OFendl << DcmObject::PrintHelper(**responseIdentifiers));
    }
    if (*stDetail)
    {
        DCMQRDB_DEBUG("  Status detail:" << OFendl << DcmObject::PrintHelper(**stDetail));
    }
}

void DcmQueryRetrieveMoveContext::addFailedUIDInstance(const char *sopInstance)
{
    size_t len;

    if (failedUIDs == NULL)
    {
        if ((failedUIDs = (char*)malloc(DIC_UI_LEN + 1)) == NULL)
        {
            DCMQRDB_ERROR("malloc failure: addFailedUIDInstance");
            return;
        }
        strcpy(failedUIDs, sopInstance);
    }
    else
    {
        len = strlen(failedUIDs);
        if ((failedUIDs = (char*)realloc(failedUIDs,
            (len + strlen(sopInstance) + 2))) == NULL)
        {
            DCMQRDB_ERROR("realloc failure: addFailedUIDInstance");
            return;
        }
        /* tag sopInstance onto end of old with '\' between */
        strcat(failedUIDs, "\\");
        strcat(failedUIDs, sopInstance);
    }
}

OFCondition DcmQueryRetrieveMoveContext::performMoveSubOp(DIC_UI sopClass, DIC_UI sopInstance, char *fname)
{
    OFCondition cond = EC_Normal;
    T_DIMSE_C_StoreRQ req;
    T_DIMSE_C_StoreRSP rsp;
    DIC_US msgId;
    T_ASC_PresentationContextID presId;
    DcmDataset *stDetail = NULL;

#ifdef LOCK_IMAGE_FILES
    /* shared lock image file */
    int lockfd;
#ifdef O_BINARY
    lockfd = open(fname, O_RDONLY | O_BINARY, 0666);
#else
    lockfd = open(fname, O_RDONLY, 0666);
#endif
    if (lockfd < 0)
    {
        /* due to quota system the file could have been deleted */
        DCMQRDB_ERROR("Move SCP: storeSCU: [file: " << fname << "]: "
            << OFStandard::getLastSystemErrorCode().message());
        nFailed++;
        addFailedUIDInstance(sopInstance);
        return EC_Normal;
    }
    dcmtk_flock(lockfd, LOCK_SH);
#endif

    msgId = subAssoc->nextMsgID++;

    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(subAssoc, sopClass);
    if (presId == 0)
    {
        nFailed++;
        addFailedUIDInstance(sopInstance);
        DCMQRDB_ERROR("Move SCP: storeSCU: [file: " << fname << "] No presentation context for: ("
            << dcmSOPClassUIDToModality(sopClass, "OT") << ") " << sopClass);
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }

    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID, sopClass);
    strcpy(req.AffectedSOPInstanceUID, sopInstance);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = priority;
    req.opts = (O_STORE_MOVEORIGINATORAETITLE | O_STORE_MOVEORIGINATORID);
    strcpy(req.MoveOriginatorApplicationEntityTitle, origAETitle);
    req.MoveOriginatorID = origMsgId;

    DCMQRDB_INFO("Store SCU RQ: MsgID " << msgId << ", ("
        << dcmSOPClassUIDToModality(sopClass, "OT") << ")");

    cond = DIMSE_storeUser(subAssoc, presId, &req,
        fname, NULL, moveSubOpProgressCallback, this,
        options_.blockMode_, options_.dimse_timeout_,
        &rsp, &stDetail);

#ifdef LOCK_IMAGE_FILES
    /* unlock image file */
    dcmtk_flock(lockfd, LOCK_UN);
    close(lockfd);
#endif

    if (cond.good())
    {
        DCMQRDB_INFO("Move SCP: Received Store SCU RSP [Status="
            << DU_cstoreStatusString(rsp.DimseStatus) << "]");
        if (rsp.DimseStatus == STATUS_Success)
        {
            /* everything ok */
            nCompleted++;
        }
        else if ((rsp.DimseStatus & 0xf000) == 0xb000)
        {
            /* a warning status message */
            nWarning++;
            DCMQRDB_ERROR("Move SCP: Store Warning: Response Status: " <<
                DU_cstoreStatusString(rsp.DimseStatus));
        }
        else
        {
            nFailed++;
            addFailedUIDInstance(sopInstance);
            /* print a status message */
            DCMQRDB_ERROR("Move SCP: Store Failed: Response Status: " <<
                DU_cstoreStatusString(rsp.DimseStatus));
        }
    }
    else
    {
        nFailed++;
        addFailedUIDInstance(sopInstance);
        OFString temp_str;
        DCMQRDB_ERROR("Move SCP: storeSCU: Store Request Failed: " << DimseCondition::dump(temp_str, cond));
    }
    if (stDetail != NULL)
    {
        DCMQRDB_INFO("  Status Detail:" << OFendl << DcmObject::PrintHelper(*stDetail));
        delete stDetail;
    }
    return cond;
}

OFCondition DcmQueryRetrieveMoveContext::buildSubAssociation(T_DIMSE_C_MoveRQ *request, OFList<QueryClientInfo> *queryclient)
{
    OFCondition cond = EC_Normal;
    DIC_NODENAME dstHostName;
    DIC_NODENAME dstHostNamePlusPort;
    int dstPortNumber = 0;
    T_ASC_Parameters *params;
    OFString temp_str;

    strcpy(dstAETitle, request->MoveDestination);

    /*
     * We must map the destination AE Title into a host name and port
     * address.  Further, we must make sure that the RSNA'93 demonstration
     * rules are observed regarding move destinations.
     */

    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(origAssoc->params, origAETitle, aeTitle, NULL);
    ourAETitle = aeTitle;

    ASC_getPresentationAddresses(origAssoc->params, origHostName, NULL);

    //get ae ip  port 2019.07.23
    OFString desAE = origAETitle;
    OFString desHN, desIP;
    if (queryclient != NULL)
    {
        OFListIterator(QueryClientInfo) of_iter = queryclient->begin();
        OFListIterator(QueryClientInfo) of_last = queryclient->end();
        while (of_iter != of_last)
        {
            if (desAE == of_iter->AEtitle)
            {
                desHN =  of_iter->HostName;
                desIP = of_iter->IpAddress;
                dstPortNumber = of_iter->port;
                break;
            }
            of_iter++;
        }
    }
    if (dstPortNumber > 0)
    {
        strcpy(dstHostName, desIP.c_str());
    }
    else
    {
        if (!mapMoveDestination(origHostName, origAETitle,
            request->MoveDestination, dstHostName, &dstPortNumber))
        {
            return QR_EC_InvalidPeer;
        }
    }

    if (cond.good())
    {
        cond = ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
        if (cond.bad())
        {
            DCMQRDB_ERROR("moveSCP: Cannot create Association-params for sub-ops: " << DimseCondition::dump(temp_str, cond));
        }
    }
    if (cond.good())
    {
        //dstHostName = "192.168.1.102";
        sprintf(dstHostNamePlusPort, "%s:%d", dstHostName, dstPortNumber);
        ASC_setPresentationAddresses(params, OFStandard::getHostName().c_str(), dstHostNamePlusPort);
        ASC_setAPTitles(params, ourAETitle.c_str(), dstAETitle, NULL);

        if (options_.outgoingProfile.empty())
        {
            cond = addAllStoragePresentationContexts(params);
        }
        else
        {
            cond = associationConfiguration_.setAssociationParameters(options_.outgoingProfile.c_str(), *params);
        }
        if (cond.bad())
        {
            DCMQRDB_ERROR(DimseCondition::dump(temp_str, cond));
        }
        DCMQRDB_DEBUG("Request Parameters:" << OFendl << ASC_dumpParameters(temp_str, params, ASC_ASSOC_RQ));
    }
    if (cond.good())
    {
        /* create association */
        DCMQRDB_INFO("Requesting Sub-Association");
        cond = ASC_requestAssociation(options_.net_, params, &subAssoc);
        if (cond.bad())
        {
            if (cond == DUL_ASSOCIATIONREJECTED)
            {
                T_ASC_RejectParameters rej;

                ASC_getRejectParameters(params, &rej);
                DCMQRDB_ERROR("moveSCP: Sub-Association Rejected" << OFendl << ASC_printRejectParameters(temp_str, &rej));
            }
            else
            {
                DCMQRDB_ERROR("moveSCP: Sub-Association Request Failed: " << DimseCondition::dump(temp_str, cond));
            }
        }
    }

    if (cond.good())
    {
        assocStarted = OFTrue;
    }
    return cond;
}

OFCondition DcmQueryRetrieveMoveContext::closeSubAssociation()
{
    OFCondition cond = EC_Normal;

    if (subAssoc != NULL)
    {
        /* release association */
        OFString temp_str;
        DCMQRDB_INFO("Releasing Sub-Association");
        cond = ASC_releaseAssociation(subAssoc);
        if (cond.bad())
        {
            DCMQRDB_ERROR("moveSCP: Sub-Association Release Failed: " << DimseCondition::dump(temp_str, cond));
        }
        cond = ASC_dropAssociation(subAssoc);
        if (cond.bad())
        {
            DCMQRDB_ERROR("moveSCP: Sub-Association Drop Failed: " << DimseCondition::dump(temp_str, cond));
        }
        cond = ASC_destroyAssociation(&subAssoc);
        if (cond.bad())
        {
            DCMQRDB_ERROR("moveSCP: Sub-Association Destroy Failed: " << DimseCondition::dump(temp_str, cond));
        }
    }

    if (assocStarted)
    {
        assocStarted = OFFalse;
    }

    return cond;
}

void DcmQueryRetrieveMoveContext::moveNextImage(DcmQueryRetrieveDatabaseStatus * dbStatus)
{
    OFCondition cond = EC_Normal;
    OFCondition dbcond = EC_Normal;
    DIC_UI subImgSOPClass;      /* sub-operation image SOP Class */
    DIC_UI subImgSOPInstance;   /* sub-operation image SOP Instance */
    char subImgFileName[MAXPATHLEN + 1];    /* sub-operation image file */

    /* clear out strings */
    bzero(subImgFileName, sizeof(subImgFileName));
    bzero(subImgSOPClass, sizeof(subImgSOPClass));
    bzero(subImgSOPInstance, sizeof(subImgSOPInstance));

    OFString ImgFileName;
    OFString strImgSOPClass, strImgSOPInstance;
    /* get DB response */
    if (false)
    {
        dbcond = dbHandle.nextMoveResponse(subImgSOPClass, subImgSOPInstance, subImgFileName, &nRemaining, dbStatus);
    }
    else
    {
        int size = m_matchingFiles.size();
        if (size > 0)
        {
            OFString filename(m_matchingFiles.back());
            m_matchingFiles.pop_back();
            DcmFileFormat data;
            if (data.loadFile(filename) != EC_Normal)
            {
                dbStatus->setStatus(STATUS_Success);
                return;
            }

            if (data.getDataset()->findAndGetOFString(DCM_SOPClassUID, strImgSOPClass).bad())
            {
                strImgSOPClass.clear();
            }
            if (data.getDataset()->findAndGetOFString(DCM_SOPInstanceUID, strImgSOPInstance).bad())
            {
                strImgSOPInstance.clear();

            }
            strcmp(subImgFileName, (char *)filename.c_str());
            ImgFileName = filename;
            ////////
            dbcond = EC_Normal;
            dbStatus->setStatus(STATUS_Pending);
        }
        else
        {
            dbStatus->setStatus(STATUS_Success);
        }
    }
    if (dbcond.bad())
    {
        DCMQRDB_ERROR("moveSCP: Database: nextMoveResponse Failed ("
            << DU_cmoveStatusString(dbStatus->status()) << "):");
    }

    if (dbStatus->status() == STATUS_Pending)
    {
        /* perform sub-op */
        strImgSOPClass, strImgSOPInstance;
        cond = performMoveSubOp(/*subImgSOPClass*/(char *)strImgSOPClass.c_str(), (char *)strImgSOPInstance.c_str()/*subImgSOPInstance*/, (char *)ImgFileName.c_str()/*subImgFileName*/);
        if (cond != EC_Normal)
        {
            OFString temp_str;
            DCMQRDB_ERROR("moveSCP: Move Sub-Op Failed: " << DimseCondition::dump(temp_str, cond));
            /* clear condition stack */
        }
    }
}

void DcmQueryRetrieveMoveContext::failAllSubOperations(DcmQueryRetrieveDatabaseStatus * dbStatus)
{
    OFCondition dbcond = EC_Normal;
    DIC_UI subImgSOPClass;      /* sub-operation image SOP Class */
    DIC_UI subImgSOPInstance;   /* sub-operation image SOP Instance */
    char subImgFileName[MAXPATHLEN + 1];    /* sub-operation image file */

    /* clear out strings */
    bzero(subImgFileName, sizeof(subImgFileName));
    bzero(subImgSOPClass, sizeof(subImgSOPClass));
    bzero(subImgSOPInstance, sizeof(subImgSOPInstance));

    while (dbStatus->status() == STATUS_Pending)
    {
        /* get DB response */
        dbcond = dbHandle.nextMoveResponse(
            subImgSOPClass, subImgSOPInstance, subImgFileName, &nRemaining, dbStatus);
        if (dbcond.bad())
        {
            DCMQRDB_ERROR("moveSCP: Database: nextMoveResponse Failed ("
                << DU_cmoveStatusString(dbStatus->status()) << "):");
        }

        if (dbStatus->status() == STATUS_Pending)
        {
            nFailed++;
            addFailedUIDInstance(subImgSOPInstance);
        }
    }
    dbStatus->setStatus(STATUS_MOVE_Warning_SubOperationsCompleteOneOrMoreFailures);
}

void DcmQueryRetrieveMoveContext::buildFailedInstanceList(DcmDataset ** rspIds)
{
    OFBool ok;

    if (failedUIDs != NULL)
    {
        *rspIds = new DcmDataset();
        ok = DU_putStringDOElement(*rspIds, DCM_FailedSOPInstanceUIDList, failedUIDs);
        if (!ok)
        {
            DCMQRDB_ERROR("moveSCP: failed to build DCM_FailedSOPInstanceUIDList");
        }
        free(failedUIDs);
        failedUIDs = NULL;
    }
}

OFBool DcmQueryRetrieveMoveContext::mapMoveDestination( const char *origPeer, const char *origAE,
    const char *dstAE, char *dstPeer, int *dstPort)
{
    /*
     * This routine enforces RSNA'93 Demo Requirements regarding
     * the destination of move commands.
     *
     */
    OFBool ok = OFFalse;
    const char *dstPeerName; /* the CNF utility returns us a static char* */

    if (options_.restrictMoveToSameAE_)
    {
        /* AE Titles the same ? */
        ok = (strcmp(origAE, dstAE) == 0);
        if (!ok)
        {
            DCMQRDB_INFO("mapMoveDestination: strictMove Reqs: '" << origAE << "' != '" << dstAE << "'");
            return OFFalse;
        }
    }

    ok = config->peerForAETitle((char*)dstAE, &dstPeerName, dstPort) > 0;
    if (!ok)
    {
        DCMQRDB_INFO("mapMoveDestination: unknown AE: '" << dstAE << "'");
        return OFFalse;     /* dstAE not known */
    }

    strcpy(dstPeer, dstPeerName);

    if (options_.restrictMoveToSameHost_)
    {
        /* hosts the same ? */
        ok = (strcmp(origPeer, dstPeer) == 0);
        if (!ok)
        {
            DCMQRDB_INFO("mapMoveDestination: different hosts: '" << origPeer << "', '" << dstPeer << "'");
            return OFFalse;
        }
    }

    if (options_.restrictMoveToSameVendor_)
    {
        /* AE titles belong to the same vendor */
        ok = config->checkForSameVendor((char*)origAE, (char*)dstAE) > 0;
        if (!ok)
        {
            DCMQRDB_INFO("mapMoveDestination: different vendors: '" << origAE << "', '" << dstAE << "'");
            return OFFalse;
        }
    }

    return ok;
}

OFCondition DcmQueryRetrieveMoveContext::addAllStoragePresentationContexts(T_ASC_Parameters *params)
{
    // this would be the place to add support for compressed transfer syntaxes
    OFCondition cond = EC_Normal;

    int i;
    int pid = 1;

    //const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 10
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // 21
        NULL
    };
    int numTransferSyntaxes = 0;

#ifdef DISABLE_COMPRESSION_EXTENSION
    /* gLocalByteOrder is defined in dcxfer.h */
    if (gLocalByteOrder == EBO_LittleEndian)
    {
        /* we are on a little endian machine */
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
    }
    else
    {
        /* we are on a big endian machine */
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
}
#else
    switch (options_.networkTransferSyntaxOut_)
    {
    case EXS_LittleEndianImplicit:
        /* we only propose Little Endian Implicit */
        //transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
        //numTransferSyntaxes = 1;
        transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_JPEGProcess14SV1TransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_LittleEndianExplicit:
        /* we prefer Little Endian Explicit */
        transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    case EXS_BigEndianExplicit:
        /* we prefer Big Endian Explicit */
        transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 3;
        break;
    case EXS_JPEGProcess14SV1:
        /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) */
        transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_JPEGProcess1:
        /* we prefer JPEGBaseline (default lossy for 8 bit images) */
        transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_JPEGProcess2_4:
        /* we prefer JPEGExtended (default lossy for 12 bit images) */
        transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_JPEG2000LosslessOnly:
        /* we prefer JPEG 2000 lossless */
        transferSyntaxes[0] = UID_JPEG2000LosslessOnlyTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_JPEG2000:
        /* we prefer JPEG 2000 lossy or lossless */
        transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_JPEGLSLossless:
        /* we prefer JPEG-LS Lossless */
        transferSyntaxes[0] = UID_JPEGLSLosslessTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_JPEGLSLossy:
        /* we prefer JPEG-LS Lossy */
        transferSyntaxes[0] = UID_JPEGLSLossyTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
    case EXS_MPEG2MainProfileAtMainLevel:
        /* we only propose MPEG2 MP@ML since we don't want to decompress */
        transferSyntaxes[0] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_MPEG2MainProfileAtHighLevel:
        /* we only propose MPEG2 MP@HL since we don't want to decompress */
        transferSyntaxes[0] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_MPEG4HighProfileLevel4_1:
        /* we only propose MPEG4 HP/L4.1 since we don't want to decompress */
        transferSyntaxes[0] = UID_MPEG4HighProfileLevel4_1TransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_MPEG4BDcompatibleHighProfileLevel4_1:
        /* we only propose MPEG4 BD HP/L4.1 since we don't want to decompress */
        transferSyntaxes[0] = UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_MPEG4HighProfileLevel4_2_For2DVideo:
        /* we only propose MPEG4 HP/L4.2 for 2D Videos since we don't want to decompress */
        transferSyntaxes[0] = UID_MPEG4HighProfileLevel4_2_For2DVideoTransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_MPEG4HighProfileLevel4_2_For3DVideo:
        /* we only propose MPEG4 HP/L4.2 for 3D Videos since we don't want to decompress */
        transferSyntaxes[0] = UID_MPEG4HighProfileLevel4_2_For3DVideoTransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_MPEG4StereoHighProfileLevel4_2:
        /* we only propose MPEG4 Stereo HP/L4.2 since we don't want to decompress */
        transferSyntaxes[0] = UID_MPEG4StereoHighProfileLevel4_2TransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_HEVCMainProfileLevel5_1:
        /* we only propose HEVC/H.265 Main Profile/L5.1 since we don't want to decompress */
        transferSyntaxes[0] = UID_HEVCMainProfileLevel5_1TransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_HEVCMain10ProfileLevel5_1:
        /* we only propose HEVC/H.265 Main 10 Profile/L5.1 since we don't want to decompress */
        transferSyntaxes[0] = UID_HEVCMain10ProfileLevel5_1TransferSyntax;
        numTransferSyntaxes = 1;
        break;
    case EXS_RLELossless:
        /* we prefer RLE Lossless */
        transferSyntaxes[0] = UID_RLELosslessTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
#ifdef WITH_ZLIB
    case EXS_DeflatedLittleEndianExplicit:
        /* we prefer deflated transmission */
        transferSyntaxes[0] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
        transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 4;
        break;
#endif
    default:
        /* We prefer explicit transfer syntaxes.
         * If we are running on a Little Endian machine we prefer
         * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
         */
       /* transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
        transferSyntaxes[1] = UID_JPEG2000LosslessOnlyTransferSyntax;
        transferSyntaxes[2] = UID_JPEGProcess2_4TransferSyntax;
        transferSyntaxes[3] = UID_JPEGProcess1TransferSyntax;
        transferSyntaxes[4] = UID_JPEGProcess14SV1TransferSyntax;
        transferSyntaxes[5] = UID_JPEGLSLossyTransferSyntax;
        transferSyntaxes[6] = UID_JPEGLSLosslessTransferSyntax;
        transferSyntaxes[7] = UID_RLELosslessTransferSyntax;
        transferSyntaxes[8] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
        transferSyntaxes[9] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
        transferSyntaxes[10] = UID_MPEG4HighProfileLevel4_1TransferSyntax;
        transferSyntaxes[11] = UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax;
        transferSyntaxes[12] = UID_MPEG4HighProfileLevel4_2_For2DVideoTransferSyntax;
        transferSyntaxes[13] = UID_MPEG4HighProfileLevel4_2_For3DVideoTransferSyntax;
        transferSyntaxes[14] = UID_MPEG4StereoHighProfileLevel4_2TransferSyntax;
        transferSyntaxes[15] = UID_HEVCMainProfileLevel5_1TransferSyntax;
        transferSyntaxes[16] = UID_HEVCMain10ProfileLevel5_1TransferSyntax;
        transferSyntaxes[17] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
        transferSyntaxes[18] = UID_LittleEndianExplicitTransferSyntax;
        transferSyntaxes[19] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[20] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 21;*/
        //if (gLocalByteOrder == EBO_LittleEndian)
        //{
        //}
        /* else {
        transferSyntaxes[18] = UID_BigEndianExplicitTransferSyntax;
        transferSyntaxes[19] = UID_LittleEndianExplicitTransferSyntax;
        }*/

        ///
        ///-----
        //if (gLocalByteOrder != EBO_BigEndian/*EBO_LittleEndian*/)  /* defined in dcxfer.h */
        //{
        //    transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
        //    transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
        //}
        //else
        //{
        //    transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
        //    transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
        //}
        //transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
        //numTransferSyntaxes = 3;
        //---------------------
        //1.2.840.10008.1.2 = Implicit VR Little Endian : Default Transfer Syntax for DICOM
        transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
        numTransferSyntaxes = 1;
        break;
    }
#endif

    for (i = 0; i < numberOfDcmLongSCUStorageSOPClassUIDs && cond.good(); i++)
    {
        cond = ASC_addPresentationContext(
            params, pid, dcmLongSCUStorageSOPClassUIDs[i],
            transferSyntaxes, numTransferSyntaxes);
        pid += 2;   /* only odd presentation context id's */
    }
    return cond;
}

void DcmQueryRetrieveMoveContext::SetDcmQueryRetrieveConfig(const DcmQueryRetrieveConfig* config)
{
    m_config = config;
}
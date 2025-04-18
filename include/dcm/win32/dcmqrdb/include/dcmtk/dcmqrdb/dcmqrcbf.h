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

#ifndef DCMQRCBF_H
#define DCMQRCBF_H

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmqrdb/qrdefine.h"
//
#include "dcmtk/dcmqrdb/dcmqrdbi.h"
//
class DcmQueryRetrieveDatabaseHandle;
class DcmQueryRetrieveOptions;
class DcmQueryRetrieveCharacterSetOptions;

/** this class maintains the context information that is passed to the
 *  callback function called by DIMSE_findProvider.
 */

class DCMTK_DCMQRDB_EXPORT DcmQueryRetrieveFindContext
{
public:
    /** constructor
     *  @param handle reference to database handle
     *  @param options options for the Q/R service
     *  @param priorStat prior DIMSE status
     */
    DcmQueryRetrieveFindContext(
      DcmQueryRetrieveDatabaseHandle& handle,
      const DcmQueryRetrieveOptions& options,
      DIC_US priorStat,
      const DcmQueryRetrieveCharacterSetOptions& characterSetOptions)
    : dbHandle(handle)
    , options_(options)
    , priorStatus(priorStat)
    , ourAETitle()
    , characterSetOptions(characterSetOptions)
    {
    }

    /** set the AEtitle under which this application operates
     *  @param ae AEtitle, is copied into this object.
     */
    void setOurAETitle(const char *ae)
    {
      if (ae)
          ourAETitle = ae;
      else
          ourAETitle.clear();
    }

    /** callback handler called by the DIMSE_storeProvider callback function.
     *  @param cancelled (in) flag indicating whether a C-CANCEL was received
     *  @param request original find request (in)
     *  @param requestIdentifiers original find request identifiers (in)
     *  @param responseCount find response count (in)
     *  @param response find response (out)
     *  @param responseIdentifiers find response identifiers (out)
     *  @param stDetail status detail for find response (out)
     */
    void callbackHandler(
        OFBool cancelled, T_DIMSE_C_FindRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        MySqlInfo *mysql,
        T_DIMSE_C_FindRSP *response,
        DcmDataset **responseIdentifiers,
        DcmDataset **stDetail);

    //find all record;
    OFCondition startFindRequestFromSql(
        const char      *SOPClassUID,
        DcmDataset      *findRequestIdentifiers,
        DcmQueryRetrieveDatabaseStatus  *status, MySqlInfo *mysql = NULL);

    //set config;
    void SetDcmQueryRetrieveConfig(const DcmQueryRetrieveConfig* config);

    // find all by sqlite
    OFCondition querySqlite(DcmDataset *findRequestIdentifiers, DB_LEVEL queryLevel, DcmQueryRetrieveDatabaseStatus  *status,
        DB_ElementList *findRequestList, DB_ElementList *findResponseList);

private:

    /// reference to database handle
    DcmQueryRetrieveDatabaseHandle& dbHandle;

    /// reference to Q/R service options
    const DcmQueryRetrieveOptions& options_;

    /// prior DIMSE status
    DIC_US priorStatus;

    /// our current title
    OFString ourAETitle;

    /// Specific Character Set related options
    const DcmQueryRetrieveCharacterSetOptions& characterSetOptions;

    /// array of matching datasets
    OFList<DcmDataset> m_matchingDatasets;

    //
    const DcmQueryRetrieveConfig *m_config;

};

#endif

/*
 *
 *  Copyright (C) 2007-2017, OFFIS e.V.
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
 *  Module:  dcmdata
 *
 *  Author:  Michael Onken
 *
 *  Purpose: Implements utility for converting standard image formats to DICOM
 *
 */


#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

#include "dcmtk/dcmdata/libi2d/i2d.h"
#include "dcmtk/dcmdata/dcpxitem.h"
#include "dcmtk/dcmdata/dcfilefo.h"  /* for DcmFileFormat */
#include "dcmtk/dcmdata/dcdeftag.h"  /* for DCM_ defines */
#include "dcmtk/dcmdata/dcuid.h"     /* for SITE_SERIES_UID_ROOT */
#include "dcmtk/dcmdata/dcpixseq.h"  /* for DcmPixelSequence */
#include "dcmtk/dcmdata/dcpath.h"    /* for override keys */

OFLogger DCM_dcmdataLibi2dLogger = OFLog::getLogger("dcmtk.dcmdata.libi2d");


Image2Dcm::Image2Dcm() : m_overrideKeys(), m_templateFile(""),
  m_readStudyLevel(OFFalse), m_readSeriesLevel(OFFalse), m_studySeriesFile(),
  m_incInstNoFromFile(OFFalse), m_disableAttribChecks(OFFalse),
  m_inventMissingType2Attribs(OFTrue), m_inventMissingType1Attribs(OFFalse),
  m_insertLatin1(OFTrue)
{

}


OFCondition Image2Dcm::convert(I2DImgSource *inputPlug,
                               I2DOutputPlug *outPlug,
                               DcmDataset*& resultDset,
                               E_TransferSyntax& proposedTS)

{
  if (!inputPlug || !outPlug)
    return EC_IllegalParameter;

  OFCondition cond;
  resultDset = NULL;
  OFunique_ptr<DcmDataset> tempDataset;
  DCMDATA_LIBI2D_DEBUG("Image2Dcm: Starting conversion of file: " << inputPlug->getImageFile());

  // If specified, copy DICOM template file to export file
  if (!m_templateFile.empty())
  {
    DcmFileFormat dcmff;
    cond = dcmff.loadFile(m_templateFile.c_str());
    if (cond.bad())
      return cond;
    // remove problematic attributes from dataset
    cleanupTemplate(dcmff.getDataset());
    // copy from input file
    tempDataset.reset(new DcmDataset(*(dcmff.getDataset())));
  }
  else // otherwise, start with an empty DICOM file
    tempDataset.reset(new DcmDataset());
  if (!tempDataset.get())
    return EC_MemoryExhausted;

  // Read patient and study or series information if desired and write to export file
  if (m_readStudyLevel || m_readSeriesLevel)
  {
    cond = applyStudyOrSeriesFromFile(tempDataset.get());
    if (cond.bad())
    {
      return cond;
    }
  }

  // Increment instance number
  if (m_incInstNoFromFile)
  {
    cond = incrementInstanceNumber(tempDataset.get());
    if (cond.bad())
    {
      return cond;
    }
  }

  // Insert Latin 1 as standard character set if desired
  if (m_insertLatin1)
    cond = insertLatin1(tempDataset.get());
  if (cond.bad())
    return cond;

  // Generate and insert UIDs as necessary
  generateUIDs(tempDataset.get());

  // Read and insert pixel data
  cond = readAndInsertPixelData(inputPlug, tempDataset.get(), proposedTS);
  if (cond.bad())
  {
    return cond;
  }

  // Insert Lossy Image Compression and Lossy Image Compression Method attributes if necessary
  OFBool srcIsLossy = OFFalse; OFString comprMethod;
  if (inputPlug->getLossyComprInfo(srcIsLossy, comprMethod).good()) //TODO)
  {
    if (srcIsLossy)
    {
      cond = tempDataset->putAndInsertOFStringArray(DCM_LossyImageCompression, "01");
      if (cond.good() && !comprMethod.empty())
        cond = tempDataset->putAndInsertOFStringArray(DCM_LossyImageCompressionMethod, comprMethod);
      if (cond.bad()) return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write attribute Lossy Image Compression and/or Lossy Image Compression Method to result dataset");
    }
  }
  else
    DCMDATA_LIBI2D_DEBUG("Image2Dcm: No information regarding lossy compression available");

  // Insert SOP Class specific attributes (and values)
  cond = outPlug->convert(*tempDataset);
  if (cond.bad())
  {
    return cond;
  }

  // At last, apply override keys on dataset
  cond = applyOverrideKeys(tempDataset.get());
  if (cond.bad())
  {
    return cond;
  }

  // Do some very basic attribute checking (e. g. existence (type 2) and values (type 1))
  if (!m_disableAttribChecks)
  {
    OFString err;
    err = isValid(*tempDataset);
    err += outPlug->isValid(*tempDataset);
    if (!err.empty())
    {
      return makeOFCondition(OFM_dcmdata, 18, OF_error, err.c_str());
    }
  }

  resultDset = tempDataset.release();
  return EC_Normal;
}


OFCondition Image2Dcm::insertLatin1(DcmDataset *outputDset)
{
  if (outputDset == NULL)
    return EC_IllegalParameter;
  return outputDset->putAndInsertString(DCM_SpecificCharacterSet, "ISO_IR 100");
}


void Image2Dcm::cleanupTemplate(DcmDataset *targetDset)
{
  if (!targetDset)
    return;
  // Remove any existing image pixel module attribute
  targetDset->findAndDeleteElement(DCM_PixelDataProviderURL);
  targetDset->findAndDeleteElement(DCM_PhotometricInterpretation);
  targetDset->findAndDeleteElement(DCM_SamplesPerPixel);
  targetDset->findAndDeleteElement(DCM_Rows);
  targetDset->findAndDeleteElement(DCM_Columns);
  targetDset->findAndDeleteElement(DCM_BitsAllocated);
  targetDset->findAndDeleteElement(DCM_BitsStored);
  targetDset->findAndDeleteElement(DCM_HighBit);
  targetDset->findAndDeleteElement(DCM_PixelRepresentation);
  targetDset->findAndDeleteElement(DCM_PixelData);
  targetDset->findAndDeleteElement(DCM_PlanarConfiguration);
  targetDset->findAndDeleteElement(DCM_PixelAspectRatio);
  targetDset->findAndDeleteElement(DCM_SmallestImagePixelValue);
  targetDset->findAndDeleteElement(DCM_LargestImagePixelValue);
  targetDset->findAndDeleteElement(DCM_RedPaletteColorLookupTableDescriptor);
  targetDset->findAndDeleteElement(DCM_GreenPaletteColorLookupTableDescriptor);
  targetDset->findAndDeleteElement(DCM_BluePaletteColorLookupTableDescriptor);
  targetDset->findAndDeleteElement(DCM_RedPaletteColorLookupTableData);
  targetDset->findAndDeleteElement(DCM_GreenPaletteColorLookupTableData);
  targetDset->findAndDeleteElement(DCM_BluePaletteColorLookupTableData);
  targetDset->findAndDeleteElement(DCM_ICCProfile);
  // Remove SOP Class / Instance information
  targetDset->findAndDeleteElement(DCM_SOPClassUID);
  targetDset->findAndDeleteElement(DCM_SOPInstanceUID);

}


OFCondition Image2Dcm::applyStudyOrSeriesFromFile(DcmDataset *targetDset)
{
  DCMDATA_LIBI2D_DEBUG("Image2Dcm: Applying study and/or series information from file");
  if ( (!m_readSeriesLevel && !m_readStudyLevel) || m_studySeriesFile.empty() )
    return EC_IllegalCall;
  DcmFileFormat dcmff;
  OFString errMsg;
  OFCondition cond;

  // Open DICOM file to read patient/study/series information from
  cond = dcmff.loadFile(m_studySeriesFile.c_str());
  if (cond.bad())
  {
    errMsg = "Error: Unable to open study / series file "; errMsg += m_studySeriesFile;
    return makeOFCondition(OFM_dcmdata, 18, OF_error, errMsg.c_str());
  }

  DcmDataset *srcDset = NULL;
  srcDset = dcmff.getDataset();
  if (srcDset == NULL)
    return EC_IllegalCall;

  // Patient level attributes (type 2 - if value cannot be read, insert empty value
  OFString value;
  srcDset->findAndGetOFString(DCM_PatientName, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_PatientName, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Patient's Name to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_PatientID, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_PatientID, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Patient ID to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_PatientSex, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_PatientSex, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Patient's Sex to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_PatientBirthDate, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_PatientBirthDate, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Patient's Birth Date to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_SpecificCharacterSet, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_SpecificCharacterSet, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Specific Character Set to file");
  value.clear();

  // Study level attributes (type 2 except Study Instance UID)
  cond = srcDset->findAndGetOFString(DCM_StudyInstanceUID, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to read Study Instance UID (type 1) from file");
  cond = targetDset->putAndInsertOFStringArray(DCM_StudyInstanceUID, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Study Instance UID to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_StudyDate, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_StudyDate, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Study Date to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_StudyTime, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_StudyTime, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Study Time to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_ReferringPhysicianName, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_ReferringPhysicianName, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Referring Physician's Name to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_StudyID, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_StudyID, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Study ID to file");
  value.clear();

  srcDset->findAndGetOFString(DCM_AccessionNumber, value);
  cond = targetDset->putAndInsertOFStringArray(DCM_AccessionNumber, value);
  if (cond.bad())
    return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to trite Accession Number to file");
  value.clear();

  // Series Level attributes (type 2 except Series Instance UID which is type 1)
  if (m_readSeriesLevel)
  {
    // General Series Module attributes
    cond = srcDset->findAndGetOFString(DCM_SeriesInstanceUID, value);
    if (cond.bad())
      return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to read Series Instance UID (type 1) from file");
    cond = targetDset->putAndInsertOFStringArray(DCM_SeriesInstanceUID, value);
    if (cond.bad())
      return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Series Instance UID to file");
    value.clear();

    srcDset->findAndGetOFString(DCM_SeriesNumber, value);
    cond = targetDset->putAndInsertOFStringArray(DCM_SeriesNumber, value);
    if (cond.bad())
      return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Series Number to file");
    value.clear();

    // General Equipment Module attributes
    srcDset->findAndGetOFString(DCM_Manufacturer, value);
    cond = targetDset->putAndInsertOFStringArray(DCM_Manufacturer, value);
    if (cond.bad())
      return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Manufacturer to file");
    value.clear();
  }

  return EC_Normal;
}


OFCondition Image2Dcm::incrementInstanceNumber(DcmDataset *targetDset)
{
  // Read and increment Instance Number if desired
  if (m_incInstNoFromFile)
  {
    DCMDATA_LIBI2D_DEBUG("Image2Dcm: Trying to read and increment instance number");
    Sint32 instanceNumber;
    if ( targetDset->findAndGetSint32(DCM_InstanceNumber, instanceNumber).good() )
    {
      instanceNumber++;
      char buf[100];
      sprintf(buf, "%ld", OFstatic_cast(long, instanceNumber));
      OFCondition cond = targetDset->putAndInsertOFStringArray(DCM_InstanceNumber, buf);
      if (cond.bad())
        return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable write Instance Number to dataset");
    }
    else
      return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to read Instance Number from dataset");
  }
  return EC_Normal;
}


OFCondition Image2Dcm::generateUIDs(DcmDataset *dset)
{
  OFString value;
  OFCondition cond;

  DCMDATA_LIBI2D_DEBUG("Image2Dcm: Generate and insert new UIDs if necessary");
  // Generate and write Series Instance UID if not already present
  if (!m_readSeriesLevel)
  {
    cond = dset->findAndGetOFString(DCM_SeriesInstanceUID, value);
    if (cond.bad() || value.empty())
    {
      char newUID[100];
      dcmGenerateUniqueIdentifier(newUID, SITE_SERIES_UID_ROOT);
      cond = dset->putAndInsertOFStringArray(DCM_SeriesInstanceUID, newUID);
      if (cond.bad())
        return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Series Instance UID to file");
    }
    value.clear();
  }

  // Generate and write Study Instance UID if not already present
  if (!m_readStudyLevel)
  {
    cond = dset->findAndGetOFString(DCM_StudyInstanceUID, value);
    if (cond.bad() || value.empty())
    {
      char newUID[100];
      dcmGenerateUniqueIdentifier(newUID, SITE_STUDY_UID_ROOT);
      cond = dset->putAndInsertOFStringArray(DCM_StudyInstanceUID, newUID);
      if (cond.bad())
        return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write Study Instance UID to file");
    }
    value.clear();
  }

  // Generate SOP Instance UID if not already present
  cond = dset->findAndGetOFString(DCM_SOPInstanceUID, value);
  if (cond.bad() || value.empty())
  {
    char newUID[100];
    dcmGenerateUniqueIdentifier(newUID, SITE_INSTANCE_UID_ROOT);
    cond = dset->putAndInsertOFStringArray(DCM_SOPInstanceUID, newUID);
    if (cond.bad())
      return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to write SOP Instance UID to file");
  }

  return EC_Normal;
}


void Image2Dcm::setISOLatin1(OFBool insLatin1)
{
  m_insertLatin1 = insLatin1;
}


OFCondition Image2Dcm::insertEncapsulatedPixelData(DcmDataset* dset,
                                                   char *pixData,
                                                   Uint32 length,
                                                   const E_TransferSyntax& outputTS) const
{
  OFCondition cond;

  DCMDATA_LIBI2D_DEBUG("Image2Dcm: Storing imported pixel data to DICOM file");
  // create initial pixel sequence
  DcmPixelSequence* pixelSequence = new DcmPixelSequence(DCM_PixelSequenceTag);
  if (pixelSequence == NULL)
    return EC_MemoryExhausted;

  // insert empty offset table into sequence
  DcmPixelItem *offsetTable = new DcmPixelItem(DCM_PixelItemTag);
  if (offsetTable == NULL)
  {
    delete pixelSequence; pixelSequence = NULL;
    return EC_MemoryExhausted;
  }
  cond = pixelSequence->insert(offsetTable);
  if (cond.bad())
  {
    delete offsetTable; offsetTable = NULL;
    delete pixelSequence; pixelSequence = NULL;
    return cond;
  }

  // store compressed frame into pixel sequence
  DcmOffsetList dummyList;
  cond = pixelSequence->storeCompressedFrame(dummyList, OFreinterpret_cast(Uint8*,pixData), length, 0);
  // storeCompressedFrame(..) does a deep copy, so the pixData memory can be freed now
  delete[] pixData;
  if (cond.bad())
  {
    delete pixelSequence; pixelSequence = NULL;
    return cond;
  }

  // insert pixel data attribute incorporating pixel sequence into dataset
  DcmPixelData *pixelData = new DcmPixelData(DCM_PixelData);
  if (pixelData == NULL)
  {
    delete pixelSequence; pixelSequence = NULL;
    return EC_MemoryExhausted;
  }
  /* tell pixel data element that this is the original presentation of the pixel data
   * pixel data and how it compressed
   */
  pixelData->putOriginalRepresentation(outputTS, NULL, pixelSequence);
  cond = dset->insert(pixelData);
  if (cond.bad())
  {
    delete pixelData; pixelData = NULL; // also deletes contained pixel sequence
    return cond;
  }

  return EC_Normal;
}


OFCondition Image2Dcm::readAndInsertPixelData(I2DImgSource* imgSource,
                                              DcmDataset* dset,
                                              E_TransferSyntax& outputTS)
{
  Uint16 samplesPerPixel, rows, cols, bitsAlloc, bitsStored, highBit, pixelRepr, planConf;
  Uint16 pixAspectH =1; Uint16 pixAspectV = 1;
  OFString photoMetrInt;
  outputTS = EXS_Unknown;
  char* pixData = NULL;
  Uint32 length;

  OFCondition cond = imgSource->readPixelData(rows, cols,
    samplesPerPixel, photoMetrInt, bitsAlloc, bitsStored, highBit, pixelRepr,
    planConf, pixAspectH, pixAspectV, pixData, length, outputTS);

  if (cond.bad())
    return cond;

  DcmXfer transport(outputTS);
  if (transport.isEncapsulated())
    insertEncapsulatedPixelData(dset, pixData, length, outputTS);
  else
  {
    /* Not encapsulated */
    dset->putAndInsertUint8Array(DCM_PixelData, OFreinterpret_cast(Uint8*, pixData), length);
    delete[] pixData;
  }

  DCMDATA_LIBI2D_DEBUG("Image2Dcm: Inserting Image Pixel module information");

  cond = dset->putAndInsertUint16(DCM_SamplesPerPixel, samplesPerPixel);
  if (cond.bad())
    return cond;

  cond = dset->putAndInsertOFStringArray(DCM_PhotometricInterpretation, photoMetrInt);
  if (cond.bad())
    return cond;

  // Should only be written if Samples per Pixel > 1
  if (samplesPerPixel > 1)
  {
    cond = dset->putAndInsertUint16(DCM_PlanarConfiguration, planConf);
    if (cond.bad())
      return cond;
  }

  cond = dset->putAndInsertUint16(DCM_Rows, rows);
  if (cond.bad())
    return cond;

  cond = dset->putAndInsertUint16(DCM_Columns, cols);
  if (cond.bad())
    return cond;

  cond = dset->putAndInsertUint16(DCM_BitsAllocated, bitsAlloc);
  if (cond.bad())
    return cond;

  cond = dset->putAndInsertUint16(DCM_BitsStored, bitsStored);
  if (cond.bad())
    return cond;

  cond = dset->putAndInsertUint16(DCM_HighBit, highBit);
  if (cond.bad())
    return cond;

  if ( pixAspectH != pixAspectV )
  {
    char buf[200];
    int err = sprintf(buf, "%u\\%u", pixAspectV, pixAspectH);
    if (err == -1) return EC_IllegalCall;
    cond = dset->putAndInsertOFStringArray(DCM_PixelAspectRatio, buf);
    if (cond.bad())
      return cond;
  }

  return dset->putAndInsertUint16(DCM_PixelRepresentation, pixelRepr);
}


OFString Image2Dcm::isValid(DcmDataset& dset) const
{
  DCMDATA_LIBI2D_DEBUG("Image2Dcm: Checking validity of DICOM output dataset");
  OFString dummy, err; OFCondition cond;
  // General Patient module attributes
  err += checkAndInventType2Attrib(DCM_PatientName, &dset);
  err += checkAndInventType2Attrib(DCM_PatientSex, &dset);
  err += checkAndInventType2Attrib(DCM_PatientBirthDate, &dset);
  err += checkAndInventType2Attrib(DCM_PatientID, &dset);

  // General Study module attributes
  err += checkAndInventType1Attrib(DCM_StudyInstanceUID, &dset);
  err += checkAndInventType2Attrib(DCM_StudyDate, &dset);
  err += checkAndInventType2Attrib(DCM_StudyTime, &dset);
  err += checkAndInventType2Attrib(DCM_ReferringPhysicianName, &dset);
  err += checkAndInventType2Attrib(DCM_StudyID, &dset);
  err += checkAndInventType2Attrib(DCM_AccessionNumber, &dset);

  // General Series module attributes
  err += checkAndInventType1Attrib(DCM_SeriesInstanceUID, &dset);
  err += checkAndInventType2Attrib(DCM_SeriesNumber, &dset);
  err += checkAndInventType2Attrib(DCM_InstanceNumber, &dset);

  // General Image module attributes
  /* Patient Orientation is of type 2C and must be written if not
     Image Orientation (Patient) (0020,0037) and Image Position (Patient)
     are required for the IOD. The current output IODs (SC, new SC, VLP)
     therefore need Patient Orientation. Make sure any new output plugin
     takes care about this attribute
   */
  err += checkAndInventType2Attrib(DCM_PatientOrientation, &dset);

  // Image Pixel Module
  err += checkAndInventType1Attrib(DCM_Rows, &dset);
  err += checkAndInventType1Attrib(DCM_Columns, &dset);
  err += checkAndInventType1Attrib(DCM_SamplesPerPixel, &dset);
  err += checkAndInventType1Attrib(DCM_PhotometricInterpretation, &dset);
  err += checkAndInventType1Attrib(DCM_BitsAllocated, &dset);
  err += checkAndInventType1Attrib(DCM_BitsStored, &dset);
  err += checkAndInventType1Attrib(DCM_HighBit, &dset);
  err += checkAndInventType1Attrib(DCM_PixelRepresentation, &dset);
  err += checkAndInventType1Attrib(DCM_SOPInstanceUID, &dset);

  return err;
}



void Image2Dcm::setSeriesFrom(const OFString& file)
{
  m_readSeriesLevel = OFTrue;
  m_studySeriesFile = file;
}


void Image2Dcm::setStudyFrom(const OFString& file)
{
  m_readStudyLevel = OFTrue;
  m_studySeriesFile = file;
}


void Image2Dcm::setValidityChecking(OFBool doChecks,
                                    OFBool insertMissingType2,
                                    OFBool inventMissingType1)
{
  m_disableAttribChecks = !doChecks;
  m_inventMissingType2Attribs = insertMissingType2;
  m_inventMissingType1Attribs = inventMissingType1;
}


void Image2Dcm::setTemplateFile(const OFString& file)
{
  m_templateFile = file;
}



void Image2Dcm::setIncrementInstanceNumber(OFBool incInstNo)
{
  m_incInstNoFromFile = incInstNo;
}


void Image2Dcm::setOverrideKeys(const OFList<OFString>& ovkeys)
{
  OFListConstIterator(OFString) it = ovkeys.begin();
  OFListConstIterator(OFString) end = ovkeys.end();
  while (it != end)
  {
    m_overrideKeys.push_back(*it);
    it++;
  }
}


OFCondition Image2Dcm::applyOverrideKeys(DcmDataset *outputDset)

{
  /* replace specific keys by those in overrideKeys, copied from findscu */
  OFListConstIterator(OFString) path = m_overrideKeys.begin();
  OFListConstIterator(OFString) endOfList = m_overrideKeys.end();
  OFCondition cond;
  DcmPathProcessor proc;
  while (path != endOfList)
  {
    cond = proc.applyPathWithValue(outputDset, *path);
    if (cond.bad()) {
      OFString err;
      err += "Bad override key/path: "; err += *path; err += ": "; err += cond.text();
      return makeOFCondition(OFM_dcmdata, 18, OF_error, err.c_str());
    }
    path++;
  }
  return cond;
}


OFString Image2Dcm::checkAndInventType1Attrib(const DcmTagKey& key,
                                              DcmDataset* targetDset,
                                              const OFString& defaultValue) const
{
  OFBool exists = targetDset->tagExists(key);
  if (!exists)
  {
    OFString err = "Image2Dcm: Missing type 1 attribute: "; err += DcmTag(key).getTagName(); err += "\n";
    return err;
  }
  DcmElement *elem;
  OFCondition cond = targetDset->findAndGetElement(key, elem);
  if (cond.bad() || !elem || (elem->getLength() == 0))
  {
    if (!m_inventMissingType1Attribs)
    {
      OFString err = "Image2Dcm: Empty value for type 1 attribute: ";
      err += DcmTag(key).getTagName();
      err += "\n";
      return err;
    }
    // holds element to insert in item
    elem = NULL;
    DcmTag tag(key); OFBool wasError = OFFalse;
    // if DICOM element could be created, insert in to item and modify to value
    if ( DcmItem::newDicomElement(elem, tag).good())
    {
        if (targetDset->insert(elem, OFTrue).good())
        {
          if (elem->putString(defaultValue.c_str()).good())
          {
            DCMDATA_LIBI2D_DEBUG("Image2Dcm: Inserting missing type 1 attribute " << tag.getTagName()
                << " with value " << defaultValue);
          } else wasError = OFTrue;
        } else wasError = OFTrue;
    } else wasError = OFTrue;
    if (wasError)
    {
      OFString err = "Unable to insert type 1 attribute "; err += tag.getTagName(); err += " with value "; err += defaultValue; err += "\n";
      return err;
    }
  }
  return "";
}


OFString Image2Dcm::checkAndInventType2Attrib(const DcmTagKey& key,
                                              DcmDataset* targetDset) const
{
  OFString err;
  OFBool exists = targetDset->tagExists(key);
  if (!exists)
  {
    if (m_inventMissingType2Attribs)
    {
      DcmTag tag(key);
      DCMDATA_LIBI2D_DEBUG("Image2Dcm: Inserting missing type 2 attribute: " << tag.getTagName());
      targetDset->insertEmptyElement(tag);
    }
    else
    {
      err = "Image2Dcm: Missing type 2 attribute: "; err += DcmTag(key).getTagName(); err += "\n";
      return err;
    }
  }
  return err;
}


Image2Dcm::~Image2Dcm()
{
  DCMDATA_LIBI2D_DEBUG("Freeing memory");
}

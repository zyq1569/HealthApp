include(defaultdirectories.pri)

INCLUDEPATH += $${GDCMINCLUDEDIR} \
               $${GDCMINCLUDEDIR}/../ \
               $${GDCMINCLUDEDIR}/Source/Common/ \
               $${GDCMINCLUDEDIR}/Source/DataDictionary \
               $${GDCMINCLUDEDIR}/Source/InformationObjectDefinition \
               $${GDCMINCLUDEDIR}/Source/MediaStorageAndFileFormat/ \
               $${GDCMINCLUDEDIR}/Source/DataStructureAndEncodingDefinition/ \
               $${GDCMINCLUDEDIR}/Utilities/VTK/

LIBS += -L$${GDCMLIBDIR} \
        -lgdcmcharls \
        -lgdcmCommon \
        -lgdcmDICT \
        -lgdcmDSED \
        -lgdcmexpat \
        -lgdcmIOD \
        -lgdcmjpeg8 \
        -lgdcmjpeg12 \
        -lgdcmjpeg16 \
        -lgdcmMSFF \
        -lgdcmopenjpeg \
        -lgdcmzlib \
        -lvtkgdcm

win32 {
    LIBS += -lgdcmgetopt
}

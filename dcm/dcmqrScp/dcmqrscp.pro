TEMPLATE = app
CONFIG += console

CONFIG -= app_bundle
CONFIG -= qt

#DEFINES += HAVE_POPEN
#DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES  -= UNICODE
DESTDIR = ../../bin/win32/profile/bin
SOURCES += \
        dcmqrscp.cc

INCLUDEPATH +=../../include/mysql/win32
INCLUDEPATH +=../../include/dcm/win32/dcmUnits

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include
INCLUDEPATH +=../../include/dcm/win32/dcmtls/include
INCLUDEPATH +=../../include/dcm/win32/dcmwlm/include

win32 {
msvc{

}else
{
    DEFINES += HAVE_POPEN
    DEFINES += HAVE_PCLOSE
}
 include(../../rootdir.pri)
 LIB_DIR = $$ROOTDIR/bin/win32/profile/lib
 LIBS   +=  -L$${LIB_DIR} \
             -ldcmnet \
             -ldcmdata \
             -ldcmimgle \
             -ldcmimage \
             -ldcmsr \
             -ldcmqrdb \
             -ldcmtls \
             -ldcmjpeg \
             -llibijg8 \
             -llibijg12 \
             -llibijg16 \
             -loflog \
             -lofstd \
             -ldcmUnits

}


LIBS += -liphlpapi
LIBS += -lwsock32
LIBS += -lws2_32
LIBS += -lole32
LIBS += -lnetapi32

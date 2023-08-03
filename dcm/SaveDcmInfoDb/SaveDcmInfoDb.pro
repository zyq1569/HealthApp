TEMPLATE = app
CONFIG += console
#CONFIG += no_lflags_merge
CONFIG -= app_bundle
CONFIG -= qt

#DEFINES += HAVE_POPEN
#DEFINES += HAVE_PCLOSE
DEFINES  -= UNICODE
DEFINES  -= _UNICODE
include(../../rootdir.pri)
DESTDIR = $$ROOTDIR/bin/win32/profile/bin
win32 {
    msvc{
        LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
        DESTDIR = $$ROOTDIR/bin/win32/vs/bin
    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        LIB_DIR = $$ROOTDIR/bin/win32/Mingw/lib
        DESTDIR = $$ROOTDIR/bin/win32/Mingw/bin
    }

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
             -ldcmUnits \
             -llibmariadbclient
}
SOURCES += \
        SaveDcmInfoDb.cc

INCLUDEPATH +=../../include/mysql/win32
INCLUDEPATH +=../../include/dcm/win32/dcmUnits

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include
INCLUDEPATH +=../../include/dcm/win32/dcmtls/include

#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmnet.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmdata.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/liboflog.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libofstd.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmtls.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmUnits.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libmariadbclient.a
LIBS += -liphlpapi
LIBS += -lwsock32
LIBS += -lws2_32
LIBS += -lole32
LIBS += -lnetapi32

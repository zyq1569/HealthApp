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
include(../../rootdir.pri)
#MOC_DIR = ../../temp/win32/scp
win32 {
    msvc{
       LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
       DESTDIR = $$ROOTDIR/bin/win32/vs/bin
#       LIBS += -liphlpapi
#       LIBS += -lwsock32
#       LIBS += -lws2_32
#       LIBS += -lole32
#       LIBS += -lnetapi32
#       LIBS += -lShlwapi
#       LIBS += -lKernel32
#       LIBS += -lShlwapi
#       LIBS += -lAdvapi32
    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        LIB_DIR = $$ROOTDIR/bin/win32/Mingw/lib
        DESTDIR = $$ROOTDIR/bin/win32/Mingw/bin
    }
}
SOURCES += \
        storescp.cpp

INCLUDEPATH +=../../include/mysql/win32
INCLUDEPATH +=../../include/dcm/win32/dcmUnits

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include
INCLUDEPATH +=../../include/dcm/win32/dcmtls/include


LIBS   +=  -L$${LIB_DIR} \
             -ldcmjpeg \
             -lijg8 \
             -lijg12 \
             -lijg16 \
             -ldcmqrdb \
             -ldcmnet \
             -ldcmdata \
             -ldcmimgle \
             -ldcmimage \
             -ldcmsr \
             -ldcmtls \
             -loflog \
             -lofstd \
             -ldcmUnits \
             -lopenjp2 \
             -lmariadb

win32 {

            LIBS += -liphlpapi
            LIBS += -lwsock32
            LIBS += -lws2_32
            LIBS += -lole32
            LIBS += -lnetapi32
            LIBS += -lShlwapi
            LIBS += -lKernel32
            LIBS += -lShlwapi
            LIBS += -lAdvapi32
}

#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmnet.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmdata.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/liboflog.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libofstd.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmtls.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmUnits.a



#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmnet.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmdata.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/liboflog.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libofstd.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmtls.a
#LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmUnits.a
#iphlpapi.lib
#ws2_32.lib
#netapi32.lib
#wsock32.lib
#kernel32.lib
#user32.lib
#gdi32.lib
#winspool.lib
#shell32.lib
#ole32.lib
#oleaut32.lib
#uuid.lib
#comdlg32.lib
#advapi32.lib
#LIBS +=-out-implib
#LIBS +=-lwpcap
#LIBS +=-no-whole-archive
#LIBS +=Wl
#LIBS +=-lpthread
#LIBS +=-licuuc
#LIBS +=-licudata
#LIBS +=-lnsl
#LIBS +=-lz
#iphlpapi.lib;ws2_32.lib;netapi32.lib;
#wsock32.lib;kernel32.lib;user32.lib;
#gdi32.lib;winspool.lib;shell32.lib;
#ole32.lib;oleaut32.lib;uuid.lib;
#comdlg32.lib;advapi32.lib
#LIBS += icuuc
#LIBS += licudata
#LIBS += lnsl
#LIBS += lpthread
#LIBS += lssl
#LIBS += lcrypto


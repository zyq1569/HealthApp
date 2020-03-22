TEMPLATE = app
CONFIG += console

CONFIG -= app_bundle
CONFIG -= qt

#-DDCMTK_BUILD_IN_PROGRESS -DUSE_NULL_SAFE_OFSTRING -D_REENTRANT
DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES -= UNICODE


SOURCES += \
        wlcefs.cc\
        wlmscpfs.cc

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

LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmdata.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmnet.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/liboflog.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libofstd.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmtls.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmUnits.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libDcmWlm.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libmariadbclient.a


LIBS += -liphlpapi
LIBS += -lwsock32
LIBS += -lws2_32
LIBS += -lole32
LIBS += -lnetapi32

LIBS += -lkernel32
LIBS += -lwinspool
LIBS += -ladvapi32
LIBS += -lcomdlg32
LIBS += -luuid
#LIBS += -out-implib
#LIBS += -enable-auto-import
#LIBS += -fno-whole-file

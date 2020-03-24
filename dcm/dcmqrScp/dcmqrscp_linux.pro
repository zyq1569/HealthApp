TEMPLATE = app
CONFIG += console
TARGET = dcmqrScp
CONFIG -= app_bundle
CONFIG -= qt

#-DDCMTK_BUILD_IN_PROGRESS -DDCMTK_HAVE_POLL=1 -DUSE_NULL_SAFE_OFSTRING -D_REENTRANT
DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += DCMTK_HAVE_POLL=1
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES  -= UNICODE


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

LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmqrdb.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmnet.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmdata.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/liboflog.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libofstd.a

LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmUnits.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libmariadbclient.a
LIBS +=-lz
LIBS +=-licuuc
LIBS +=-licudata
LIBS +=-lnsl
LIBS +=-lpthread
LIBS +=-ldl
LIBS +=-lm
LIBS +=-lssl
LIBS +=-lcrypto
LIBS +=-luuid
#-o ../../bin/dcmqrscp  ../../lib/libdcmqrdb.a
# ../../lib/libdcmnet.a ../../lib/libdcmdata.a
# ../../lib/liboflog.a ../../lib/libofstd.a
# -lz -licuuc -licudata -lnsl -lpthread

TEMPLATE = app
CONFIG += console
TARGET = WorklistScp
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES  -= UNICODE


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


LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmnet.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmdata.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/liboflog.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libofstd.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmtls.a
#LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmnet.a
#LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmdata.a
#LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/liboflog.a
#LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libofstd.a
LIBS +=/home/zyq/windows_share/HealthApp/lib/dcm/linux/libdcmUnits.a
LIBS +=-luuid
LIBS +=-lpthread
LIBS +=-licuuc
LIBS +=-licudata
LIBS +=-lnsl
LIBS +=-lz

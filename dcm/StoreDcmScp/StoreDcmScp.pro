TEMPLATE = app
CONFIG += console
#CONFIG += no_lflags_merge
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES  -= UNICODE

#MOC_DIR = ../../temp/win32/scp

SOURCES += \
        storescp.cc

INCLUDEPATH +=../../include/mysql/win32
INCLUDEPATH +=../../include/dcm/win32/dcmUnits

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include
INCLUDEPATH +=../../include/dcm/win32/dcmtls/include

LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmnet.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmdata.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/liboflog.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libofstd.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmtls.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmUnits.a

LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmnet.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmdata.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/liboflog.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libofstd.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmtls.a
LIBS +=F:/temp/HealthApp/lib/dcm/win32/libdcmUnits.a
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
#LIBS += -lwsock32

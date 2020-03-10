TEMPLATE = app
CONFIG += console
#CONFIG += no_lflags_merge
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES  -= UNICODE

MOC_DIR = ../../temp/win32/scp

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


LIBS += ../../lib/dcm/win32/libofstd.a
LIBS += ../../lib/dcm/win32/libdcmtls.a
LIBS += ../../lib/dcm/win32/liboflog.a
LIBS += ../../lib/dcm/win32/libdcmdata.a
LIBS += ../../lib/dcm/win32/libdcmUnits.a
LIBS += ../../lib/dcm/win32/libdcmnet.a

LIBS += -lwsock32

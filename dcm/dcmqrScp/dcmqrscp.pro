TEMPLATE = app
CONFIG += console

CONFIG -= app_bundle
CONFIG -= qt

#DEFINES += HAVE_POPEN
#DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES -= UNICODE
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

include(../../rootdir.pri)

win32{

    msvc{
        LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
        DESTDIR = $$ROOTDIR/bin/win32/vs/bin

        LIBS += -liphlpapi
        LIBS += -lwsock32
        LIBS += -lws2_32
        LIBS += -lole32
        LIBS += -lnetapi32
        LIBS += -lShlwapi
        LIBS += -lKernel32
        LIBS += -lShlwapi
        LIBS += -lAdvapi32
#        LIBS += -luser32
#        LIBS += -lgdi32
#        LIBS += -lwinspool

#        LIBS += -lshell32
#        LIBS += -loleaut32
#        LIBS += -luuid
#        LIBS += -lcomdlg32

        #Shlwapi.lib
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
    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        LIB_DIR = $$ROOTDIR/bin/win32/Mingw/lib
        DESTDIR = $$ROOTDIR/bin/win32/Mingw/bin
    }
}


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
             -lmariadb \
             -lopenjp2





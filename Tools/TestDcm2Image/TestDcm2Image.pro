QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG  += c++11
DEFINES -= UNICODE
DEFINES += NEED_SHORT_EXTERNAL_NAMES  ON_THE_FLY_COMPRESSION
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# Avoid definition of min and max macros by windows.h
DEFINES += NOMINMAX
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
# EasyLogging++ is set to be thread safe, to not have a default log file, and to not handle crashes
DEFINES += ELPP_THREAD_SAFE ELPP_NO_DEFAULT_LOG_FILE ELPP_DISABLE_DEFAULT_CRASH_HANDLING

SOURCES += \
            easylogging++.cc \
            logging.cpp \
            main.cpp \
            mainwindow.cpp

HEADERS += \
           easylogging++.h \
           logging.h \
           mainwindow.h

FORMS   += \
           mainwindow.ui

include(../../rootdir.pri)

win32 {

    msvc{
        LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
        LIB_DIR = $$ROOTDIR/lib/dcm/win32
        DESTDIR = $$PWD/bin/windows/vs

# we define that for visual studio-based windows compilation systems
# compilation is done in as many cores as possible
#QMAKE_CXXFLAGS += /MP

## We indicate that for debug compilations, Runtime Library
## is Multi-threaded DLL (as in release) and not Multi-threaded Debug DLL
QMAKE_CXXFLAGS_DEBUG                  -= -MD
QMAKE_CXXFLAGS_DEBUG                  += -MT

QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -MD
QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += -MT

QMAKE_CXXFLAGS_RELEASE                -= -MD
QMAKE_CXXFLAGS_RELEASE                += -MT

QMAKE_CXXFLAGS                        -= -Zc:strictStrings

    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE NEED_SHORT_EXTERNAL_NAMES
        LIB_DIR = $$ROOTDIR/bin/win32/Mingw/lib
        DESTDIR = $$PWD/bin/windows/Mingw

    }
}

#message($$LIB_DIR)

LIBS   +=  -L$${LIB_DIR} \
#             -ldcmqrdb \
             -ldcmnet \
             -ldcmdata \
             -ldcmimgle \
             -ldcmimage \
             -ldcmsr \
             -ldcmtls \
             -loflog \
             -lofstd \
             -ldcmUnits \
             -ldcmjpls \
             -ldcmcharls \
             -ldcmjpeg \
             -llibijg8 \
             -llibijg12 \
             -llibijg16 \
             -lfmjpeg2k \
             -lsqlite3 \
             -lopenjp2
#             \
#             -lzlibstatic

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

INCLUDEPATH +=  ../../include/dcm/win32/ofstd/include \
                ../../include/dcm/win32/dcmdata/include \
                ../../include/dcm/win32/dcmimgle/include \
                ../../include/dcm/win32/dcmnet/include \
                ../../include/dcm/win32/dcmsign/include \
                ../../include/dcm/win32/ofstd/include \
                ../../include/dcm/win32/oflog/include \
                ../../include/dcm/win32/dcmimage/include \
                ../../include/dcm/win32/config/include \
                ../../include/dcm/win32/dcmpsta/includet \
                ../../include/dcm/win32/dcmsr/include \
                ../../include/dcm/win32/dcmjpeg/include \
                ../../include/dcm/win32/dcmjpls/include \
                ../../include/dcm/win32/dcmUnits \
                ../../Tools/fmjpeg2k/include


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

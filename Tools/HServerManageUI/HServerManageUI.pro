#-------------------------------------------------
#
# Project created by QtCreator 2019-12-04T16:11:34
#
#-------------------------------------------------

QT       += core gui sql
#QT       += sql
#QMAKE_CXXFLAGS += /utf-8
DEFINES  -= UNICODE
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HServerUI
TEMPLATE = app
DESTDIR = ./bin
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# EasyLogging++ is set to be thread safe, to not have a default log file, and to not handle crashes
DEFINES += ELPP_THREAD_SAFE ELPP_NO_DEFAULT_LOG_FILE ELPP_DISABLE_DEFAULT_CRASH_HANDLING

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

include(../../rootdir.pri)

SOURCES += \
        easylogging++.cc \
        logging.cpp \
        main.cpp \
        mainwindow.cpp \
        units.cpp

HEADERS += \
        easylogging++.h \
        logging.h \
        mainwindow.h \
        units.h

FORMS += \
        mainwindow.ui

INCLUDEPATH +=../../DB/mariadb

win32 {

    msvc{
        LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
        DESTDIR = $$PWD/bin/windows/vs

    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        LIB_DIR = $$ROOTDIR/bin/win32/Mingw/lib
        DESTDIR = $$PWD/bin/windows/Mingw
        contains(CONFIG, static){
            LIB_DIR = $$ROOTDIR/bin/win32/Mingw/staticlib
            DESTDIR = $$PWD/bin/windows/MingwStatic
        }else{
            LIBS += -liphlpapi
            LIBS += -lwsock32
            LIBS += -lws2_32
            LIBS += -lole32
            LIBS += -lnetapi32
            LIBS += -lShlwapi
            LIBS += -lKernel32
            LIBS += -lShlwapi
            LIBS += -lAdvapi32
            #message("win32 share")
        }


    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
             systray.qrc

LIBS      +=  -L$${LIB_DIR} \
              -lmariadb


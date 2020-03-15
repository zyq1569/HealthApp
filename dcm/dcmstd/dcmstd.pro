#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T20:52:45
#
#-------------------------------------------------

QT       -= core gui

TARGET = ofstd
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES += ofstd_EXPORTS
DEFINES += O3
DEFINES  -= UNICODE


SOURCES += \
    math.cc \
    ofchrenc.cc \
    ofcmdln.cc \
    ofconapp.cc \
    ofcond.cc \
    ofconfig.cc \
    ofconsol.cc \
    ofcrc32.cc \
    ofdate.cc \
    ofdatime.cc \
    oferror.cc \
    offile.cc \
    offname.cc \
    oflist.cc \
    ofmath.cc \
    ofsockad.cc \
    ofstd.cc \
    ofstring.cc \
    oftempf.cc \
    ofthread.cc \
    oftime.cc \
    oftimer.cc \
    ofuuid.cc \
    ofxml.cc

HEADERS +=
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include

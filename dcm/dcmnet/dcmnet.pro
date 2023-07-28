#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T21:30:26
#
#-------------------------------------------------

QT       -= core gui

TARGET = dcmnet
TEMPLATE = lib
CONFIG += staticlib

include(../../rootdir.pri)
DESTDIR = $$ROOTDIR/bin/win32/profile/lib
#message($$DESTDIR)
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += HAVE_POPEN
#DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES += dcmnet_EXPORTS
DEFINES -= UNICODE
win32 {
    msvc{

    }else
    {
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
    }
}

SOURCES += \
    assoc.cc \
    cond.cc \
    dcasccff.cc \
    dcasccfg.cc \
    dccfenmp.cc \
    dccfpcmp.cc \
    dccfprmp.cc \
    dccfrsmp.cc \
    dccftsmp.cc \
    dccfuidh.cc \
    dcmlayer.cc \
    dcmtrans.cc \
    dcompat.cc \
    dcuserid.cc \
    dfindscu.cc \
    dimcancl.cc \
    dimcmd.cc \
    dimdump.cc \
    dimecho.cc \
    dimfind.cc \
    dimget.cc \
    dimmove.cc \
    dimse.cc \
    dimstore.cc \
    diutil.cc \
    dstorscp.cc \
    dstorscu.cc \
    dul.cc \
    dulconst.cc \
    dulextra.cc \
    dulfsm.cc \
    dulparse.cc \
    dulpres.cc \
    dwrap.c \
    extneg.cc \
    lst.cc \
    scp.cc \
    scpcfg.cc \
    scppool.cc \
    scpthrd.cc \
    scu.cc

HEADERS +=
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmjpeg/include
INCLUDEPATH +=../../include/dcm/win32/dcmimgle/include



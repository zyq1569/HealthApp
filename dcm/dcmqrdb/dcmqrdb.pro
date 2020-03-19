#-------------------------------------------------
#
# Project created by QtCreator 2020-03-09T09:25:57
#
#-------------------------------------------------

QT       -= core gui

TARGET = dcmqrdb
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES += dcmqrdb_EXPORTS
DEFINES  -= UNICODE
DEFINES  -= _UNICODE


SOURCES += \
    dcmqrcbf.cc \
    dcmqrcbg.cc \
    dcmqrcbm.cc \
    dcmqrcbs.cc \
    dcmqrcnf.cc \
    dcmqrdbi.cc \
    dcmqrdbs.cc \
    dcmqropt.cc \
    dcmqrptb.cc \
    dcmqrsrv.cc \
    dcmqrtis.cc

HEADERS +=

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH +=../../include/mysql/win32
INCLUDEPATH +=../../include/dcm/win32/dcmUnits

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include

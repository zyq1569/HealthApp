#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T21:50:42
#
#-------------------------------------------------

QT       -= core gui

TARGET = dcmimgle
TEMPLATE = lib
CONFIG += staticlib
include(../../rootdir.pri)
DESTDIR = $$ROOTDIR/bin/win32/profile/lib
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
DEFINES += dcmimgle_EXPORTS
DEFINES += O3
DEFINES -= UNICODE
SOURCES += \
            dcmimage.cc \
            dibaslut.cc \
            diciefn.cc \
            dicielut.cc \
            didislut.cc \
            didispfn.cc \
            didocu.cc \
            digsdfn.cc \
            digsdlut.cc \
            diimage.cc \
            diinpx.cc \
            diluptab.cc \
            dimo1img.cc \
            dimo2img.cc \
            dimoimg.cc \
            dimoimg3.cc \
            dimoimg4.cc \
            dimoimg5.cc \
            dimomod.cc \
            dimoopx.cc \
            dimopx.cc \
            diovdat.cc \
            diovlay.cc \
            diovlimg.cc \
            diovpln.cc \
            diutils.cc


HEADERS +=

unix {
    target.path = /usr/lib
    INSTALLS += target
}


INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmtls/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmimage/include
INCLUDEPATH +=../../include/dcm/win32/dcmimgle/include

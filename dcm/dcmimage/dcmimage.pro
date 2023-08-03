#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T21:50:42
#
#-------------------------------------------------

QT       -= core gui

TARGET = dcmimage
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
DEFINES += dcmimage_EXPORTS
DEFINES += O3
DEFINES  -= UNICODE

linux {
    DEFINES += HAVE_POPEN
    DEFINES += HAVE_PCLOSE
    DEFINES += DCMTK_BUILD_IN_PROGRESS
    DEFINES += USE_NULL_SAFE_OFSTRING
    DEFINES += _REENTRANT
    DEFINES += dcmimage_EXPORTS
}
win32 {
    msvc{
        DESTDIR = $$ROOTDIR/bin/win32/vs/lib
    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        DESTDIR = $$ROOTDIR/bin/win32/Mingw/lib
    }
    DEFINES += dcmimage_EXPORTS  DCMTK_BUILD_IN_PROGRESS  NOMINMAX
}
SOURCES += \
            diargimg.cc \
            dicmyimg.cc \
            dicoimg.cc \
            dicoopx.cc \
            dicopx.cc \
            dihsvimg.cc \
            dilogger.cc \
            dipalimg.cc \
            dipipng.cc \
            dipitiff.cc \
            diqtctab.cc \
            diqtfs.cc \
            diqthash.cc \
            diqthitl.cc \
            diqtpbox.cc \
            diquant.cc \
            diregist.cc \
            dirgbimg.cc \
            diybrimg.cc \
            diyf2img.cc \
            diyp2img.cc


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

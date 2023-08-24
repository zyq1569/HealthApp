#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T21:13:53
#
#-------------------------------------------------

QT       -= core gui

TARGET = dcmjpeg
TEMPLATE = lib
CONFIG += staticlib
include(../../rootdir.pri)
DESTDIR = $$ROOTDIR/bin/win32/profile/lib
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#-DDCMTK_BUILD_IN_PROGRESS -DUSE_NULL_SAFE_OFSTRING -D_REENTRANT -Ddcmdata_EXPORTS
#DEFINES += HAVE_POPEN
#DEFINES += HAVE_PCLOSE
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES += dcmjpeg_EXPORTS  NEED_SHORT_EXTERNAL_NAMES
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES  -= UNICODE
win32 {
    msvc{
        LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
        DESTDIR = $$ROOTDIR/bin/win32/vs/lib

DEFINES        += USE_NULL_SAFE_OFSTRING DCMTK_BUILD_IN_PROGRESS NOMINMAX _CRT_FAR_MAPPINGS_NO_DEPRECATE _CRT_IS_WCTYPE_NO_DEPRECATE _CRT_MANAGED_FP_NO_DEPRECATE
DEFINES        += _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE_GLOBALS _CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE _CRT_TIME_FUNCTIONS_NO_DEPRECATE
DEFINES        += _CRT_VCCLRIT_NO_DEPRECATE _SCL_SECURE_NO_DEPRECATE _REENTRANT CMAKE_INTDIR=\"Release\"

#LIBS           +=  -L$${LIB_DIR} \
#                   -lijg8 \
#                   -lijg12 \
#                   -lijg16

    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        DESTDIR = $$ROOTDIR/bin/win32/Mingw/lib
    }
}
SOURCES += \
            ddpiimpl.cc \
            dipijpeg.cc \
            djcodecd.cc \
            djcodece.cc \
            djcparam.cc \
            djdecbas.cc \
            djdecext.cc \
            djdeclol.cc \
            djdecode.cc \
            djdecpro.cc \
            djdecsps.cc \
            djdecsv1.cc \
            djdijg12.cc \
            djdijg16.cc \
            djdijg8.cc \
            djeijg12.cc \
            djeijg16.cc \
            djeijg8.cc \
            djencbas.cc \
            djencext.cc \
            djenclol.cc \
            djencode.cc \
            djencpro.cc \
            djencsps.cc \
            djencsv1.cc \
            djrplol.cc \
            djrploss.cc \
            djutils.cc


HEADERS +=
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmjpls/include
INCLUDEPATH +=../../include/dcm/win32/dcmimage/include
INCLUDEPATH +=../../include/dcm/win32/dcmimgle/include
INCLUDEPATH +=../../include/dcm/win32/dcmjpeg/include
INCLUDEPATH += ./libijg8
INCLUDEPATH += ./libijg12
INCLUDEPATH += ./libijg16

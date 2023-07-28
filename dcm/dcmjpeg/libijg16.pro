#TEMPLATE = lib
#TARGET = libijg16
#DESTDIR = ../../lib/dcm/win32
#CONFIG += staticlib debug
#DEFINES += ijg16_EXPORTS USE_NULL_SAFE_OFSTRING DCMTK_BUILD_IN_PROGRESS NOMINMAX _CRT_FAR_MAPPINGS_NO_DEPRECATE _CRT_IS_WCTYPE_NO_DEPRECATE _CRT_MANAGED_FP_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE_GLOBALS _CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE _CRT_TIME_FUNCTIONS_NO_DEPRECATE _CRT_VCCLRIT_NO_DEPRECATE _SCL_SECURE_NO_DEPRECATE _REENTRANT CMAKE_INTDIR=\"Release\"


QT       -= core gui
TARGET    = libijg16
TEMPLATE  = lib
CONFIG   += staticlib

include(../../rootdir.pri)
DESTDIR = $$ROOTDIR/bin/win32/profile/lib

DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES += ijg16_EXPORTS
win32 {
    msvc{

    }else
    {
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
    }
}
SOURCES += \
            libijg16/jaricom.c \
            libijg16/jcapimin.c \
            libijg16/jcapistd.c \
            libijg16/jcarith.c \
            libijg16/jccoefct.c \
            libijg16/jccolor.c \
            libijg16/jcdctmgr.c \
            libijg16/jcdiffct.c \
            libijg16/jchuff.c \
            libijg16/jcinit.c \
            libijg16/jclhuff.c \
            libijg16/jclossls.c \
            libijg16/jclossy.c \
            libijg16/jcmainct.c \
            libijg16/jcmarker.c \
            libijg16/jcmaster.c \
            libijg16/jcodec.c \
            libijg16/jcomapi.c \
            libijg16/jcparam.c \
            libijg16/jcphuff.c \
            libijg16/jcpred.c \
            libijg16/jcprepct.c \
            libijg16/jcsample.c \
            libijg16/jcscale.c \
            libijg16/jcshuff.c \
            libijg16/jctrans.c \
            libijg16/jdapimin.c \
            libijg16/jdapistd.c \
            libijg16/jdarith.c \
            libijg16/jdatadst.c \
            libijg16/jdatasrc.c \
            libijg16/jdcoefct.c \
            libijg16/jdcolor.c \
            libijg16/jddctmgr.c \
            libijg16/jddiffct.c \
            libijg16/jdhuff.c \
            libijg16/jdinput.c \
            libijg16/jdlhuff.c \
            libijg16/jdlossls.c \
            libijg16/jdlossy.c \
            libijg16/jdmainct.c \
            libijg16/jdmarker.c \
            libijg16/jdmaster.c \
            libijg16/jdmerge.c \
            libijg16/jdphuff.c \
            libijg16/jdpostct.c \
            libijg16/jdpred.c \
            libijg16/jdsample.c \
            libijg16/jdscale.c \
            libijg16/jdshuff.c \
            libijg16/jdtrans.c \
            libijg16/jerror.c \
            libijg16/jfdctflt.c \
            libijg16/jfdctfst.c \
            libijg16/jfdctint.c \
            libijg16/jidctflt.c \
            libijg16/jidctfst.c \
            libijg16/jidctint.c \
            libijg16/jidctred.c \
            libijg16/jmemmgr.c \
            libijg16/jmemnobs.c \
            libijg16/jquant1.c \
            libijg16/jquant2.c \
            libijg16/jutils.c



INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmtls/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmimage/include
INCLUDEPATH +=../../include/dcm/win32/dcmimgle/include

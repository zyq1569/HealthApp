

#TEMPLATE = lib
#TARGET = libijg12
#DESTDIR = ../../lib/dcm/win32
#CONFIG += staticlib debug
#DEFINES += ijg12_EXPORTS USE_NULL_SAFE_OFSTRING DCMTK_BUILD_IN_PROGRESS NOMINMAX _CRT_FAR_MAPPINGS_NO_DEPRECATE _CRT_IS_WCTYPE_NO_DEPRECATE _CRT_MANAGED_FP_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE_GLOBALS _CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE _CRT_TIME_FUNCTIONS_NO_DEPRECATE _CRT_VCCLRIT_NO_DEPRECATE _SCL_SECURE_NO_DEPRECATE _REENTRANT CMAKE_INTDIR=\"Release\"



QT       -= core gui
TARGET   = libijg12
TEMPLATE  = lib
CONFIG   += staticlib

include(../../rootdir.pri)
DESTDIR = $$ROOTDIR/bin/win32/profile/lib

DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES += ijg12_EXPORTS
win32 {
    msvc{

    }else
    {
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
    }
}
SOURCES += \
    libijg12/jaricom.c \
    libijg12/jcapimin.c \
    libijg12/jcapistd.c \
    libijg12/jcarith.c \
    libijg12/jccoefct.c \
    libijg12/jccolor.c \
    libijg12/jcdctmgr.c \
    libijg12/jcdiffct.c \
    libijg12/jchuff.c \
    libijg12/jcinit.c \
    libijg12/jclhuff.c \
    libijg12/jclossls.c \
    libijg12/jclossy.c \
    libijg12/jcmainct.c \
    libijg12/jcmarker.c \
    libijg12/jcmaster.c \
    libijg12/jcodec.c \
    libijg12/jcomapi.c \
    libijg12/jcparam.c \
    libijg12/jcphuff.c \
    libijg12/jcpred.c \
    libijg12/jcprepct.c \
    libijg12/jcsample.c \
    libijg12/jcscale.c \
    libijg12/jcshuff.c \
    libijg12/jctrans.c \
    libijg12/jdapimin.c \
    libijg12/jdapistd.c \
    libijg12/jdarith.c \
    libijg12/jdatadst.c \
    libijg12/jdatasrc.c \
    libijg12/jdcoefct.c \
    libijg12/jdcolor.c \
    libijg12/jddctmgr.c \
    libijg12/jddiffct.c \
    libijg12/jdhuff.c \
    libijg12/jdinput.c \
    libijg12/jdlhuff.c \
    libijg12/jdlossls.c \
    libijg12/jdlossy.c \
    libijg12/jdmainct.c \
    libijg12/jdmarker.c \
    libijg12/jdmaster.c \
    libijg12/jdmerge.c \
    libijg12/jdphuff.c \
    libijg12/jdpostct.c \
    libijg12/jdpred.c \
    libijg12/jdsample.c \
    libijg12/jdscale.c \
    libijg12/jdshuff.c \
    libijg12/jdtrans.c \
    libijg12/jerror.c \
    libijg12/jfdctflt.c \
    libijg12/jfdctfst.c \
    libijg12/jfdctint.c \
    libijg12/jidctflt.c \
    libijg12/jidctfst.c \
    libijg12/jidctint.c \
    libijg12/jidctred.c \
    libijg12/jmemmgr.c \
    libijg12/jmemnobs.c \
    libijg12/jquant1.c \
    libijg12/jquant2.c \
    libijg12/jutils.c



INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmtls/include
INCLUDEPATH +=../../include/dcm/win32/dcmdata/include
INCLUDEPATH +=../../include/dcm/win32/dcmnet/include
INCLUDEPATH +=../../include/dcm/win32/dcmimage/include
INCLUDEPATH +=../../include/dcm/win32/dcmimgle/include

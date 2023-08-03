
QT       -= core gui
TARGET    = ijg8
TEMPLATE  = lib
CONFIG   += staticlib

include(../../rootdir.pri)
DESTDIR  = $$ROOTDIR/bin/win32/profile/lib

DEFINES  += DCMTK_BUILD_IN_PROGRESS
DEFINES  += USE_NULL_SAFE_OFSTRING
DEFINES  += _REENTRANT
DEFINES  += ijg8_EXPORTS
#DEFINES += ijg8_EXPORTS USE_NULL_SAFE_OFSTRING DCMTK_BUILD_IN_PROGRESS NOMINMAX _CRT_FAR_MAPPINGS_NO_DEPRECATE _CRT_IS_WCTYPE_NO_DEPRECATE _CRT_MANAGED_FP_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE_GLOBALS _CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE _CRT_TIME_FUNCTIONS_NO_DEPRECATE _CRT_VCCLRIT_NO_DEPRECATE _SCL_SECURE_NO_DEPRECATE _REENTRANT CMAKE_INTDIR=\"Release\"
win32 {
    msvc{
        DESTDIR = $$ROOTDIR/bin/win32/vs/lib
    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        DESTDIR = $$ROOTDIR/bin/win32/Mingw/lib
    }
}
SOURCES += \
            libijg8/jaricom.c \
            libijg8/jcapimin.c \
            libijg8/jcapistd.c \
            libijg8/jcarith.c \
            libijg8/jccoefct.c \
            libijg8/jccolor.c \
            libijg8/jcdctmgr.c \
            libijg8/jcdiffct.c \
            libijg8/jchuff.c \
            libijg8/jcinit.c \
            libijg8/jclhuff.c \
            libijg8/jclossls.c \
            libijg8/jclossy.c \
            libijg8/jcmainct.c \
            libijg8/jcmarker.c \
            libijg8/jcmaster.c \
            libijg8/jcodec.c \
            libijg8/jcomapi.c \
            libijg8/jcparam.c \
            libijg8/jcphuff.c \
            libijg8/jcpred.c \
            libijg8/jcprepct.c \
            libijg8/jcsample.c \
            libijg8/jcscale.c \
            libijg8/jcshuff.c \
            libijg8/jctrans.c \
            libijg8/jdapimin.c \
            libijg8/jdapistd.c \
            libijg8/jdarith.c \
            libijg8/jdatadst.c \
            libijg8/jdatasrc.c \
            libijg8/jdcoefct.c \
            libijg8/jdcolor.c \
            libijg8/jddctmgr.c \
            libijg8/jddiffct.c \
            libijg8/jdhuff.c \
            libijg8/jdinput.c \
            libijg8/jdlhuff.c \
            libijg8/jdlossls.c \
            libijg8/jdlossy.c \
            libijg8/jdmainct.c \
            libijg8/jdmarker.c \
            libijg8/jdmaster.c \
            libijg8/jdmerge.c \
            libijg8/jdphuff.c \
            libijg8/jdpostct.c \
            libijg8/jdpred.c \
            libijg8/jdsample.c \
            libijg8/jdscale.c \
            libijg8/jdshuff.c \
            libijg8/jdtrans.c \
            libijg8/jerror.c \
            libijg8/jfdctflt.c \
            libijg8/jfdctfst.c \
            libijg8/jfdctint.c \
            libijg8/jidctflt.c \
            libijg8/jidctfst.c \
            libijg8/jidctint.c \
            libijg8/jidctred.c \
            libijg8/jmemmgr.c \
            libijg8/jmemnobs.c \
            libijg8/jquant1.c \
            libijg8/jquant2.c \
            libijg8/jutils.c



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

#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T21:13:53
#
#-------------------------------------------------

QT       -= core gui

TARGET    = dcmdata
TEMPLATE  = lib
CONFIG   += staticlib
include(../../rootdir.pri)
DESTDIR = $$ROOTDIR/bin/win32/profile/lib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#-DDCMTK_BUILD_IN_PROGRESS -DUSE_NULL_SAFE_OFSTRING -D_REENTRANT -Ddcmdata_EXPORTS

#    DCMTK_BUILD_IN_PROGRESS
#                NOMINMAX \
#                _CRT_FAR_MAPPINGS_NO_DEPRECATE _CRT_IS_WCTYPE_NO_DEPRECATE USE_NULL_SAFE_OFSTRING \
#                _CRT_MANAGED_FP_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE \
#                _CRT_SECURE_NO_DEPRECATE_GLOBALS _CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE \
#                _CRT_TIME_FUNCTIONS_NO_DEPRECATE _CRT_VCCLRIT_NO_DEPRECATE   _SCL_SECURE_NO_DEPRECATE _REENTRANT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += dcmdata_EXPORTS
linux{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE
        DEFINES += DCMTK_BUILD_IN_PROGRESS
        DEFINES += USE_NULL_SAFE_OFSTRING
        DEFINES += _REENTRANT
}
win32{
       msvc{                      
           DEFINES += dcmdata_EXPORTS
           DEFINES += _REENTRANT
           DEFINES += USE_NULL_SAFE_OFSTRING
           DEFINES += O3
           DEFINES -= UNICODE
           DEFINES += NOMINMAX
           #DEFINES +=  _CRT_FAR_MAPPINGS_NO_DEPRECATE _CRT_IS_WCTYPE_NO_DEPRECATE _CRT_MANAGED_FP_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE_GLOBALS _CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE _CRT_TIME_FUNCTIONS_NO_DEPRECATE _CRT_VCCLRIT_NO_DEPRECATE _SCL_SECURE_NO_DEPRECATE
           DESTDIR  = $$ROOTDIR/bin/win32/vs/lib
       }else{

           DEFINES += HAVE_POPEN
           DEFINES += HAVE_PCLOSE
           DESTDIR  = $$ROOTDIR/bin/win32/Mingw/lib
       }
}
SOURCES += \
        cmdlnarg.cc \
        dcbytstr.cc \
        dcchrstr.cc \
        dccodec.cc \
        dcdatset.cc \
        dcdatutl.cc \
        dcddirif.cc \
        dcdicdir.cc \
        dcdicent.cc \
        dcdict.cc \
        dcdictbi.cc \
        dcdirrec.cc \
        dcelem.cc \
        dcerror.cc \
        dcfilefo.cc \
        dcfilter.cc \
        dchashdi.cc \
        dcistrma.cc \
        dcistrmb.cc \
        dcistrmf.cc \
        dcistrmz.cc \
        dcitem.cc \
        dcjson.cc \
        dclist.cc \
        dcmatch.cc \
        dcmetinf.cc \
        dcobject.cc \
        dcostrma.cc \
        dcostrmb.cc \
        dcostrmf.cc \
        dcostrmz.cc \
        dcpath.cc \
        dcpcache.cc \
        dcpixel.cc \
        dcpixseq.cc \
        dcpxitem.cc \
        dcrleccd.cc \
        dcrlecce.cc \
        dcrlecp.cc \
        dcrledrg.cc \
        dcrleerg.cc \
        dcrlerp.cc \
        dcsequen.cc \
        dcspchrs.cc \
        dcstack.cc \
        dcswap.cc \
        dctag.cc \
        dctagkey.cc \
        dctypes.cc \
        dcuid.cc \
        dcvr.cc \
        dcvrae.cc \
        dcvras.cc \
        dcvrat.cc \
        dcvrcs.cc \
        dcvrda.cc \
        dcvrds.cc \
        dcvrdt.cc \
        dcvrfd.cc \
        dcvrfl.cc \
        dcvris.cc \
        dcvrlo.cc \
        dcvrlt.cc \
        dcvrobow.cc \
        dcvrod.cc \
        dcvrof.cc \
        dcvrol.cc \
        dcvrpn.cc \
        dcvrpobw.cc \
        dcvrsh.cc \
        dcvrsl.cc \
        dcvrss.cc \
        dcvrst.cc \
        dcvrtm.cc \
        dcvruc.cc \
        dcvrui.cc \
        dcvrul.cc \
        dcvrulup.cc \
        dcvrur.cc \
        dcvrus.cc \
        dcvrut.cc \
        dcwcache.cc \
        dcxfer.cc \
        mkdeftag.cc \
        mkdictbi.cc \
        vrscan.cc \
        vrscanl.c

HEADERS += \
    vrscani.h \
    vrscanl.h


unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH +=  ../../include/dcm/win32/config/include \
                ../../include/dcm/win32/ofstd/include \
                ../../include/dcm/win32/oflog/include \
                ../../include/dcm/win32/dcmqrdb/include \
                ../../include/dcm/win32/dcmdata/include \
                ./../../include/dcm/win32/dcmimgle/include \
                ./../../include/dcm/win32/dcmimage/include \
                ./../../include/dcm/win32/dcmjpeg/include \
                ./../../include/dcm/win32/dcmjpls/include \
                ./../../include/dcm/win32/dcmtls/include \
                ./../../include/dcm/win32/dcmnet/include \
                ./../../include/dcm/win32/dcmsr/include \
                ./../../include/dcm/win32/dcmsign/include \
                ./../../include/dcm/win32/dcmwlm/include \
                ./../../include/dcm/win32/dcmqrdb/include \
                ./../../include/dcm/win32/dcmpstat/include \
                ./../../include/dcm/win32/dcmrt/include \
                ./../../include/dcm/win32/dcmiod/include \
                ./../../include/dcm/win32/dcmfg/include \
                ./../../include/dcm/win32/dcmseg/include \
                ./../../include/dcm/win32/dcmtract/include \
                ./../../include/dcm/win32/dcmpmap/include

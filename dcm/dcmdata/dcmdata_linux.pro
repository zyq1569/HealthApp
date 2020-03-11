#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T21:13:53
#
#-------------------------------------------------

QT       -= core gui

TARGET = dcmdata
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11 console
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += DCMTK_INSIDE_LOG4CPLUS
DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES += QTCREATOR
DEFINES += _MSC_VER
DEFINES  -= UNICODE


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

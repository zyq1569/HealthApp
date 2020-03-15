#-------------------------------------------------
#
# Project created by QtCreator 2020-03-08T11:29:24
#
#-------------------------------------------------

QT       -= core gui

TARGET = oflog
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += DCMTK_BUILD_IN_PROGRESS
DEFINES += USE_NULL_SAFE_OFSTRING
DEFINES += _REENTRANT
DEFINES += O3
DEFINES += oflog_EXPORTS
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += DCMTK_INSIDE_LOG4CPLUS
DEFINES += HAVE_POPEN
DEFINES += HAVE_PCLOSE
DEFINES += QTCREATOR
DEFINES  -= UNICODE

SOURCES += \
    apndimpl.cc \
    appender.cc \
    asyncap.cc \
    clogger.cc \
    config.cc \
    consap.cc \
    cygwin32.cc \
    env.cc \
    factory.cc \
    fileap.cc \
    fileinfo.cc \
    filter.cc \
    globinit.cc \
    hierarchy.cc \
    hierlock.cc \
    layout.cc \
    lloguser.cc \
    lockfile.cc \
    log4judp.cc \
    logevent.cc \
    logger.cc \
    logimpl.cc \
    loglevel.cc \
    loglog.cc \
    logmacro.cc \
    mdc.cc \
    ndc.cc \
    ntelogap.cc \
    nullap.cc \
    objreg.cc \
    oflog.cc \
    patlay.cc \
    pointer.cc \
    property.cc \
    queue.cc \
    rootlog.cc \
    sleep.cc \
    snprintf.cc \
    sockbuff.cc \
    socket.cc \
    socketap.cc \
    strccloc.cc \
    strcloc.cc \
    strhelp.cc \
    striconv.cc \
    syncprims.cc \
    syslogap.cc \
    threads.cc \
    timehelp.cc \
    tls.cc \
    unixsock.cc \
    version.cc \
    winconap.cc \
    windebap.cc \
    winsock.cc

HEADERS +=
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH +=../../include/dcm/win32/config/include
INCLUDEPATH +=../../include/dcm/win32/ofstd/include
INCLUDEPATH +=../../include/dcm/win32/oflog/include
INCLUDEPATH +=../../include/dcm/win32/dcmqrdb/include

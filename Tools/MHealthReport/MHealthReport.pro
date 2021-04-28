QT       += core gui network webenginewidgets
QMAKE_CXXFLAGS += /utf-8
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# EasyLogging++ is set to be thread safe, to not have a default log file, and to not handle crashes
DEFINES += ELPP_THREAD_SAFE ELPP_NO_DEFAULT_LOG_FILE ELPP_DISABLE_DEFAULT_CRASH_HANDLING

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config.cpp \
    easylogging++.cc \
    httpclient.cpp \
    ipcinterfaceinterface.cpp \
    logging.cpp \
    main.cpp \
    mainapp.cpp \
    patientdata.cpp \
    patientsform.cpp \
    patientstudyregister.cpp \
    studyimage.cpp

HEADERS += \
    config.h \
    easylogging++.h \
    httpclient.h \
    ipcinterfaceinterface.h \
    logging.h \
    mainapp.h \
    patientdata.h \
    patientsform.h \
    patientstudyregister.h \
    studyimage.h

FORMS += \
    config.ui \
    mainapp.ui \
    patientsform.ui \
    patientstudyregister.ui \
    studyimage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

RC_FILE += app.rc

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

#QMAKE_CXXFLAGS  += /utf-8
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sender.cpp \
    util.cpp

HEADERS += \
    mainwindow.h \
    sender.h \
    util.h

FORMS += \
    mainwindow.ui

include(../../rootdir.pri)

win32 {

    LIBS += -liphlpapi
    LIBS += -lwsock32
    LIBS += -lws2_32
    LIBS += -lole32
    LIBS += -lnetapi32

    LIBS += -lkernel32
    LIBS += -lwinspool
    LIBS += -ladvapi32
    LIBS += -lcomdlg32
    LIBS += -luuid

    msvc{
        LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
        DESTDIR = $$PWD/bin/windows/vs

    }else{
        LIB_DIR = $$ROOTDIR/bin/win32/Mingw/lib
        DESTDIR = $$PWD/bin/windows/Mingw

    }
}

#message($$LIB_DIR)

LIBS   +=  -L$${LIB_DIR} \
             -lijg8 \
             -lijg12 \
             -lijg16 \
             -ldcmjpeg \
             -ldcmqrdb \
             -ldcmnet \
             -ldcmdata \
             -ldcmimgle \
             -ldcmimage \
             -ldcmsr \
             -ldcmtls \
             -loflog \
             -lofstd \
             -lmariadb \
             -ldcmUnits

INCLUDEPATH +=  ../../include/dcm/win32/ofstd/include \
                ../../include/dcm/win32/dcmdata/include \
                ../../include/dcm/win32/dcmimgle/include \
                ../../include/dcm/win32/dcmnet/include \
                ../../include/dcm/win32/dcmsign/include \
                ../../include/dcm/win32/ofstd/include \
                ../../include/dcm/win32/oflog/include \
                ../../include/dcm/win32/dcmimage/include \
                ../../include/dcm/win32/config/include \
                ../../include/dcm/win32/dcmpsta/includet \
                ../../include/dcm/win32/dcmsr/include \
                ../../include/dcm/win32/dcmjpeg/include


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

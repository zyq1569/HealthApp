QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG  += c++11
DEFINES -= UNICODE
DEFINES += NEED_SHORT_EXTERNAL_NAMES  ON_THE_FLY_COMPRESSION
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

    msvc{
        LIB_DIR = $$ROOTDIR/bin/win32/vs/lib
        LIB_DIR = $$ROOTDIR/lib/dcm/win32
        DESTDIR = $$PWD/bin/windows/vs

# we define that for visual studio-based windows compilation systems
# compilation is done in as many cores as possible
#QMAKE_CXXFLAGS += /MP

## We indicate that for debug compilations, Runtime Library
## is Multi-threaded DLL (as in release) and not Multi-threaded Debug DLL
QMAKE_CXXFLAGS_DEBUG -= -MD
QMAKE_CXXFLAGS_DEBUG += -MT

QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -MD
QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += -MT

QMAKE_CXXFLAGS_RELEASE -= -MD
QMAKE_CXXFLAGS_RELEASE += -MT

QMAKE_CXXFLAGS -= -Zc:strictStrings

    }else{
        DEFINES += HAVE_POPEN
        DEFINES += HAVE_PCLOSE NEED_SHORT_EXTERNAL_NAMES
        LIB_DIR = $$ROOTDIR/bin/win32/Mingw/lib
        DESTDIR = $$PWD/bin/windows/Mingw

    }
}

#message($$LIB_DIR)

LIBS   +=  -L$${LIB_DIR} \
#             -ldcmqrdb \
             -ldcmnet \
             -ldcmdata \
             -ldcmimgle \
             -ldcmimage \
             -ldcmsr \
             -ldcmtls \
             -loflog \
             -lofstd \
             -ldcmUnits \
             -ldcmjpls \
             -ldcmcharls \
             -ldcmjpeg \
             -llibijg8 \
             -llibijg12 \
             -llibijg16 \
             -lfmjpeg2k \
             -lopenjp2

win32 {

            LIBS += -liphlpapi
            LIBS += -lwsock32
            LIBS += -lws2_32
            LIBS += -lole32
            LIBS += -lnetapi32
            LIBS += -lShlwapi
            LIBS += -lKernel32
            LIBS += -lShlwapi
            LIBS += -lAdvapi32

}

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
                ../../include/dcm/win32/dcmjpeg/include \
                ../../include/dcm/win32/dcmjpls/include \
                ../../Tools/fmjpeg2k/include


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#Qt4VSv1.0
#https://stackoverflow.com/questions/72031733/the-online-service-is-not-available-issue-in-visual-studio-professional-2013-w
#尝试这个; 它对我有用。只需将两个DWORD值添加到 Windows 注册表即可。网站go.microsoft.com目前仅支持 TLS1.2 协议。

#[HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\.NETFramework\v4.0.30319]
#"SystemDefaultTlsVersions"=dword:00000001
#"SchUseStrongCrypto"=dword:00000001

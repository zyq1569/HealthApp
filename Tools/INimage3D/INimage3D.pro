QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ConfigForm.cpp \
    main.cpp \
    mainwindow.cpp \
    q3dviewer.cpp \
    qfourpaneviewer.cpp

HEADERS += \
    ConfigForm.h \
    mainwindow.h \
    q3dviewer.h \
    qfourpaneviewer.h

FORMS += \
    ConfigForm.ui \
    mainwindow.ui \
    q3dviewer.ui \
    qfourpaneviewer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

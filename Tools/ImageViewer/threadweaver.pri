include(defaultdirectories.pri)

LIBS += -L$${THREADWEAVERLIBDIR} -lKF5ThreadWeaver

INCLUDEPATH += $${THREADWEAVERINCLUDEDIR}

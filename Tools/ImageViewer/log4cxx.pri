include(defaultdirectories.pri)

#LIBS += -L$${GDCMLIBDIR} \
#        -lgdcmcharls \
#        -lvtkgdcm

LIBS += -L$${LOG4CXXLIBDIR} \
        -llog4cxx

INCLUDEPATH += $${LOG4CXXINCLUDEDIR}

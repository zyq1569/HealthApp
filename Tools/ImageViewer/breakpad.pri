macx {
    LIBS += -lcrypto
}

# Problema de suport de breakpad per linux 64bits
include(detectlinux64.pri)
linux_x86_64 {
    DEFINES += NO_CRASH_REPORTER
}

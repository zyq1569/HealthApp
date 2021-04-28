QT            += dbus widgets

DBUS_ADAPTORS += car.xml
HEADERS       += car.h
SOURCES       += car.cpp \
                 main.cpp \
                 car_adaptor.cpp

# install
target.path   = $$[QT_INSTALL_EXAMPLES]/dbus/remotecontrolledcar/car
INSTALLS     += target
DLLDESTDIR    = ../../bin

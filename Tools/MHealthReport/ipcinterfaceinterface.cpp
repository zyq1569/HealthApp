#include "ipcinterfaceinterface.h"

IPCInterfaceInterface::IPCInterfaceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr)
    :QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{

}

IPCInterfaceInterface::~IPCInterfaceInterface()
{

}

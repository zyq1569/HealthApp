#include "ipcinterfaceinterface.h"

IPCInterfaceInterface::IPCInterfaceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    :QDBusAbstractInterface(service, path, IPCInterfaceName.toLatin1().data(), connection, parent)
{

}

IPCInterfaceInterface::~IPCInterfaceInterface()
{

}

#ifndef IPCINTERFACEINTERFACE_H
#define IPCINTERFACEINTERFACE_H

#include <QString>
#include <QtDBus/QtDBus>

class IPCInterfaceInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    IPCInterfaceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);
    ~IPCInterfaceInterface();
public:
    virtual InterfaceName() = 0;
public:
    QString InterfaceName;
};

#endif // IPCINTERFACEINTERFACE_H

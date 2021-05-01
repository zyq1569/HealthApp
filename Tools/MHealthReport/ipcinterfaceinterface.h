#ifndef IPCINTERFACEINTERFACE_H
#define IPCINTERFACEINTERFACE_H

#include <QString>
#include <QtDBus/QtDBus>


//---https://blog.csdn.net/lvliang2008/article/details/6304003
//--DBusProxy DBusClientProxy DBusServer
class IPCInterfaceInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    IPCInterfaceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~IPCInterfaceInterface();

public:
    virtual void InterfaceName() = 0;

public:
    QString IPCInterfaceName;
};

#endif // IPCINTERFACEINTERFACE_H

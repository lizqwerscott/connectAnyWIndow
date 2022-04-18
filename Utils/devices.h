#ifndef DEVICES_H
#define DEVICES_H

#include <QString>

class DeviceS
{
public:
    DeviceS(QString id, QString ip, bool livep = true);
public:
    QString deviceId;
    QString hostIp;
    bool livep;
};

#endif // DEVICES_H

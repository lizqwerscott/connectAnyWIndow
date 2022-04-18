#include "devices.h"

DeviceS::DeviceS(QString id, QString ip, bool _livep) :
    deviceId(id),
    hostIp(ip),
    livep(_livep)
{

}

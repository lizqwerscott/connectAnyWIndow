#ifndef CONNECTSERVICE_H
#define CONNECTSERVICE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QMutex>

#include "Utils/devices.h"

class ConnectService : public QObject
{
    Q_OBJECT
public:
    explicit ConnectService(QObject *parent = nullptr);
    ~ConnectService();
signals:
    void working();
    void finish();

    void searchIpFinish();
    void searchDeviceFinish();
    void refreshDevice();
    void refreshDeviceLive();

public:
    void begin();

public slots:

private:
    QString deviceId;
    QString userName;

    QMutex finishMutex;
    int finishNum;
    QMutex deviceFinishMutex;
    int deviceFinishN;
    QMutex ipsMutex;

    QList<QString> ips;
    QMutex deviceMutex;
    QMap<QString, DeviceS *> devices;
private:
    void loadUserInfo();

public:
    void addIp(QString ip)
    {
        ipsMutex.lock();
        this->ips.append(ip);
        ipsMutex.unlock();
    }

    QList<QString> & getIps()
    {
        return this->ips;
    }
    void addDevices(DeviceS * device) 
    {
        deviceMutex.lock();
        this->devices[device->hostIp] = device;
        qInfo("add devices: %s", device->deviceId.toStdString().c_str());
        deviceMutex.unlock();
    }

    void addDevices(QString id, QString ip)
    {
        deviceMutex.lock();
        this->devices[ip] = new DeviceS(id, ip);
        qInfo("add devices: %s", id.toStdString().c_str());
        deviceMutex.unlock();
    }

    QMap<QString, DeviceS *> & getDevices()
    {
        return this->devices;
    }

    void searchIp();
    void searchDevices();
};

#endif // CONNECTSERVICE_H

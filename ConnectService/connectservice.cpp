#include "connectservice.h"

#include <QList>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <QString>
#include <QProcess>
#include <QList>
#include <QSysInfo>

#include "Utils/httputils.h"

ConnectService::ConnectService(QObject * parent): QObject(parent), finishNum(0), deviceFinishN(0)
{
}

ConnectService::~ConnectService()
{
    foreach (auto value, this->devices) {
        delete value;
        value = nullptr;
    }
    this->devices.clear();
}

void ConnectService::begin()
{
    this->loadUserInfo();
}

void ConnectService::loadUserInfo()
{
    QSettings settings;
    auto deviceId = settings.value("user/id", "-1").toString();
    auto userName = settings.value("user/name", "-1").toString();
    if (deviceId != "-1" && userName != "-1") {
        this->deviceId = deviceId;
        this->userName = userName;
    } else {
        qInfo("the userName and deviceId is null, please login");
    }
}

void ConnectService::searchIp()
{
    auto localIp = HttpUtils::getLocalIp();
    auto gateWayIp = HttpUtils::getGateWay(localIp);
    qInfo("localIp: %s", localIp.toStdString().c_str());
    qInfo("localIp: %s", gateWayIp.toStdString().c_str());
    QString tempIp;
    for (int i = 2; i < 255; i++) {
        tempIp = HttpUtils::getIp(gateWayIp, i);
        //connect(temp, SIGNAL(finish()), this, SLOT(pingFinish()));
        //QThreadPool::globalInstance()->start(temp);
        auto future = QtConcurrent::run([tempIp, this] {

            auto type = QSysInfo::productType();
            auto pingProcess = new QProcess();
            qInfo("system type: %s", type.toStdString().c_str());
            qInfo("ping: %s", tempIp.toStdString().c_str());
            if (type == "windows")
            {
                qInfo("is windows");
                QStringList lst;
                lst << tempIp;
                lst << "-n" << "1";
                lst << "-w" << "1000";
                int exitCode = pingProcess->execute("ping", lst);
                if (exitCode == 0) {
                    this->addIp(tempIp);
                }
            } else if (type == "arch" || type == "ubuntu")
            {
                qInfo("is linux");
                QStringList lst;
                lst << tempIp;
                lst << "-c" << "1";
                lst << "-w" << "1";
                int exitCode = pingProcess->execute("ping", lst);
                if (exitCode == 0) {
                    this->addIp(tempIp);
                }
            } else
            {
                qDebug("[Error]: can't support");
                delete pingProcess;
            }
            this->finishMutex.lock();
            this->finishNum++;
            this->finishMutex.unlock();
        });
    }

    auto finish = QtConcurrent::run([this] {
        while (1)
        {
            qInfo("finishNum: %d", this->finishNum);
            if (this->finishNum == 253) {
                emit(searchIpFinish());
                qInfo("search ip Finish --------------------------------");
                this->finishMutex.lock();
                this->finishNum = 0;
                this->finishMutex.unlock();
                break;
            }
        }
    });
}

void ConnectService::searchDevices()
{
    qInfo("searchDevices start");
    for (int i = 0; i < this->ips.size(); i++) {
        auto ip = this->ips[i];
        QUrl url;
        url.setScheme("http");
        url.setHost(ip);
        url.setPort(7677);
        url.setPath("/connect");
        url.setQuery("name=" + this->userName + "&id=" + this->deviceId);
        qInfo("connect(url): %s", url.toString().toStdString().c_str());

        HttpUtils::get(url, [this, ip](QString replyText, bool errorp) {
            this->deviceFinishMutex.lock();
            this->deviceFinishN++;
            this->deviceFinishMutex.unlock();
            if (!errorp) {
                qInfo("(%s)reply: %s", ip.toStdString().c_str(), replyText.toUtf8().toStdString().c_str());
                QJsonObject obj = QJsonDocument::fromJson(replyText.toUtf8()).object();
                QString name = obj.value(QString("name")).toString("-1");
                QString device = obj.value(QString("device")).toString("-1");

                if (name != "-1" && device != "-1") {
                    qInfo("device: %s, name: %s", device.toStdString().c_str(), name.toStdString().c_str());
                    if (name == this->userName) {
                        this->addDevices(device, ip);
                    } else {
                        qInfo("another userName: %s", name.toStdString().c_str());
                    }
                }
            } else {
                qInfo("(%s)reply: error", ip.toStdString().c_str());
            }
        });
    }
    auto finish = QtConcurrent:: run([this] {
        while (1)
        {
            if (this->deviceFinishN == this->ips.size()) {
                emit(searchDeviceFinish());
                this->deviceFinishMutex.lock();
                this->deviceFinishN = 0;
                this->deviceFinishMutex.unlock();
                break;
            }
        }
    });
}

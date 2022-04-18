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

ConnectService::ConnectService(QObject * parent): QObject(parent), finishNum(0)
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
    qInfo("localIp: %s\n", localIp.toStdString().c_str());
    qInfo("localIp: %s\n", gateWayIp.toStdString().c_str());
    QString tempIp;
    for (int i = 2; i < 255; i++) {
        tempIp = HttpUtils::getIp(gateWayIp, i);
        //connect(temp, SIGNAL(finish()), this, SLOT(pingFinish()));
        //QThreadPool::globalInstance()->start(temp);
        auto future = QtConcurrent::run([tempIp, this] {

            auto type = QSysInfo::productType();
            auto pingProcess = new QProcess();
            qInfo("system type: %s\n", type.toStdString().c_str());
            qInfo("ping: %s\n", tempIp.toStdString().c_str());
            if (type == "windows")
            {
                qInfo("is windows\n");
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
                qInfo("is linux\n");
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
            qInfo("finishNum: %d\n", this->finishNum);
            if (this->finishNum == 253) {
                emit(searchFinish());
                this->finishMutex.lock();
                this->finishNum = 0;
                this->finishMutex.unlock();
                break;
            }
        }
    });
    qInfo("search ip Finish --------------------------------\n");
}

void ConnectService::searchDevices()
{
    qInfo("searchDevices start\n");
    QUrl baiduUrl;
    baiduUrl.setScheme("http");
    baiduUrl.setHost("api.vvhan.com");
    baiduUrl.setPath("/api/love");
    //baiduUrl.setQuery("");
    QNetworkRequest baiduRequest;
    baiduRequest.setUrl(baiduUrl);
    auto breply = HttpUtils::get(baiduRequest);
    qInfo("connect: %s\n", baiduUrl.toString().toStdString().c_str());
    QObject::connect(breply, &QNetworkReply::finished, [breply]() {
        if (breply->error() == QNetworkReply::NoError) {
            auto replyText = breply->readAll();
            qInfo("io reply: %s\n", replyText.toStdString().c_str());
        }
        breply->deleteLater();
    });
    QObject::connect(breply, &QNetworkReply::errorOccurred, [breply]() {
        qInfo("json errorOccurred: %s\n", breply->errorString().toStdString().c_str());
    });
    for (int i = 0; i < this->ips.size(); i++) {
        auto ip = this->ips[i];
        QNetworkRequest request;
        QUrl url;
        url.setScheme("http");
        url.setHost(ip);
        url.setPort(7677);
        url.setPath("/connect");
        url.setQuery("name=" + this->userName + "&id=" + this->deviceId);
        //QString urlStr = "http://" + ip + ":7677/connect?name=" + this->userName + "&" + "id=" + this->deviceId;
        QString urlStr = "http://" + ip + ":7677";
        qInfo("connect(url): %s\n", url.toString().toStdString().c_str());
        request.setUrl(url);
        auto reply = HttpUtils::get(request);
        qInfo("connect: %s\n", ip.toStdString().c_str());
        QObject::connect(reply, &QNetworkReply::finished, [reply, this, ip]() {
            if (reply->error() == QNetworkReply::NoError) {
                QString replyText = reply->readAll();
                qInfo("a: %d\n", (int)replyText.size());
                qInfo("(%s)reply: %s\n", ip.toStdString().c_str(), replyText.toUtf8().toStdString().c_str());
                QJsonDocument doc = QJsonDocument::fromJson(replyText.toUtf8());
                QJsonObject obj = doc.object();
                QString name = obj.value(QString("name")).toString("-1");
                QString device = obj.value(QString("device")).toString("-1");
                qInfo("device: %s, name: %s\n", device.toStdString().c_str(), device.toStdString().c_str());
                if (name != "-1" && device != "-1") {
                    if (name == this->userName) {
                        this->addDevices(device, ip);
                    } else {
                        qInfo("another userName: %s\n", name.toStdString().c_str());
                    }
                } else {
                }
            }
            reply->deleteLater();
        });
        QObject::connect(reply, &QNetworkReply::errorOccurred, [ip, reply]() {
            qInfo("%s errorOccurred: %s\n", ip.toStdString().c_str(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString().toStdString().c_str());
            qInfo("%s errorOccurred: %s\n", ip.toStdString().c_str(), reply->errorString().toStdString().c_str());
        });
    }
}

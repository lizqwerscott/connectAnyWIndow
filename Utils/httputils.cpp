#include "httputils.h"

#include <QtNetwork/QNetworkInterface>
#include <QStringList>
#include <QSysInfo>

HttpUtils::HttpUtils(QObject * parent)
{
    manager = new QNetworkAccessManager(parent);
}

HttpUtils::~HttpUtils()
{
    manager->deleteLater();
    manager = nullptr;
}

HttpUtils * httpUtilsInstance = nullptr;

HttpUtils * HttpUtils::getInstance(QObject * parent)
{
    if (httpUtilsInstance == nullptr) {
        httpUtilsInstance = new HttpUtils(parent);
    }
    return httpUtilsInstance;
}

void HttpUtils::Destory()
{
    delete httpUtilsInstance;
    httpUtilsInstance = nullptr;
}

QString HttpUtils::getLocalIp()
{
    QStringList ips;
    for (auto address : QNetworkInterface::allAddresses()) {
        qInfo("ip: %s\n", address.toString().toStdString().c_str());
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            ips.push_back(address.toString());
        }
    }
    qInfo("search finish\n");
    auto type = QSysInfo::productType();
    if (type == "windows") {
        return ips[0];
    } else {
        return ips[1];
    }
}

QString HttpUtils::getGateWay(QString &localIp)
{
    auto result = localIp.split(".");
    result[result.size() - 1] = "1";
    return result.join(".");
}

QString HttpUtils::getIp(QString &gateWay, int i)
{
    auto result = gateWay.split(".");
    result[result.size() - 1] = QString::number(i, 10);
    return result.join(".");
}

QNetworkReply * HttpUtils::get(QNetworkRequest & request)
{
    request.setRawHeader( "User-Agent" , "Mozilla Firefox" );
    return httpUtilsInstance->manager->get(request);;
}

QString HttpUtils::post(QString url, QString data)
{
    return "";
}

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
        qInfo("ip: %s", address.toString().toStdString().c_str());
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            ips.push_back(address.toString());
        }
    }
    qInfo("search finish");
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

QNetworkReply * HttpUtils::get(const QUrl &url, std::function<void (QNetworkReply * reply)> const &finish, std::function<void (QNetworkReply * reply)> const &errorOccurred)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8,zh-TW;q=0.7");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
    request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36");
    auto reply =  httpUtilsInstance->manager->get(request);;
    QObject::connect(reply, &QNetworkReply::finished, [reply, finish] {
        finish(reply);
    });
    QObject::connect(reply, &QNetworkReply::errorOccurred, [reply, errorOccurred] {
        errorOccurred(reply);
    });
    return reply;
}

QNetworkReply * HttpUtils::get(const QUrl &url, std::function<void (QNetworkReply * reply)> const &finish)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8,zh-TW;q=0.7");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
    request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36");
    auto reply =  httpUtilsInstance->manager->get(request);;
    QObject::connect(reply, &QNetworkReply::finished, [reply, finish] {
        finish(reply);
    });
    QObject::connect(reply, &QNetworkReply::errorOccurred, [reply, url] {
        qInfo("%s errorOccurred: %s", url.toString().toStdString().c_str(), reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString().toStdString().c_str());
        qInfo("%s errorOccurred: %s", url.toString().toStdString().c_str(), reply->errorString().toStdString().c_str());
    });
    return reply;
}
void HttpUtils::get(const QUrl &url, std::function<void (QString replyText, bool errorp)> const &finish, bool showError)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8,zh-TW;q=0.7");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
    request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36");

    auto reply =  httpUtilsInstance->manager->get(request);

    QObject::connect(reply, &QNetworkReply::finished, [reply, finish, url] {
        QString replyText = reply->readAll();
        finish(replyText, reply->error() != QNetworkReply::NoError);
        reply->deleteLater();
    });
    if (showError) {
        QObject::connect(reply, &QNetworkReply::errorOccurred, [reply, url] {
            auto errorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString().toStdString();
            qInfo("%s error code: %s", url.toString().toStdString().c_str(), errorCode.c_str());
            qInfo(" %s", reply->errorString().toStdString().c_str());
        });
    }
}

QString HttpUtils::post(QString url, QString data)
{
    return "";
}

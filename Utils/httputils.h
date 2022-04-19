#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <functional>

class HttpUtils
{
private:
    HttpUtils(QObject * parent);
    ~HttpUtils();

public:
    static HttpUtils * getInstance(QObject * parent = nullptr);
    static void Destory();

private:
    QNetworkAccessManager * manager;

public:
    static QString getLocalIp();
    static QString getGateWay(QString &localIp);
    static QString getIp(QString &gateWay, int i);

public:
    static QNetworkReply * get(const QUrl &url, std::function<void (QNetworkReply * reply)> const &finish, std::function<void (QNetworkReply * reply)> const &errorOccurred);
    static QNetworkReply * get(const QUrl &url, std::function<void (QNetworkReply * reply)> const &finish);
    static void get(const QUrl &url, std::function<void (QString replyText, bool errorp)> const &finish, bool showError = false);
    static QString post(QString url, QString data);
};

#endif // HTTPUTILS_H

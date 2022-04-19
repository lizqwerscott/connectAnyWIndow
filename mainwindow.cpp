#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Utils/httputils.h"

#include <QSettings>
#include <QDebug>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    ipModel(new QStandardItemModel()),
    deviceModel(new QStandardItemModel()),
    t1(new QThread()),
    service(new ConnectService())
{
    HttpUtils::getInstance(this);

    ui->setupUi(this);
    QCoreApplication::setOrganizationName("flydog");
    QCoreApplication::setOrganizationDomain("flydog.com");
    QCoreApplication::setApplicationName("connectAny-Windows");

    //UI ip table settings
    ipModel->setColumnCount(1);
    ipModel->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("ip"));

    ui->ipTableView->setModel(this->ipModel);
    ui->ipTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    //UI device table settings
    deviceModel->setColumnCount(3);
    deviceModel->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("name"));
    deviceModel->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("ip"));
    deviceModel->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("livep"));

    ui->devicesTableView->setModel(this->deviceModel);
    ui->devicesTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(onLoginButtonClicked()));
    connect(ui->sendMessageButton, SIGNAL(clicked()), this, SLOT(onSendButtonClicked()));
    connect(ui->searchDevicesButton, SIGNAL(clicked()), this, SLOT(onSearchButtonClicked()));

    if (!loadUserInfo()) {
        this->loginDialog = new LoginDialog();
        connect(this->loginDialog, SIGNAL(login()), this, SLOT(onLoginFinish()));
        loginDialog->show();
    }

    updateLoading(false);

    qInfo("Start get LocalIp");
    auto localIp = HttpUtils::getLocalIp();
    auto gateWay = HttpUtils::getGateWay(localIp);
    qInfo("localIp:%s", localIp.toStdString().c_str());
    qInfo("gateWay:%s", gateWay.toStdString().c_str());

    service->moveToThread(t1);
    service->begin();

    connect(service, SIGNAL(searchIpFinish()), this, SLOT(onSearchIpFinish()));
    connect(service, SIGNAL(searchDeviceFinish()), this, SLOT(onSearchDeviceFinish()));
}

MainWindow::~MainWindow()
{
    HttpUtils::Destory();
    service->deleteLater();
    t1->quit();
    t1->wait();
    t1->deleteLater();
    delete ui;
    if (this->loginDialog != nullptr) {
        delete this->loginDialog;
    }
}

bool MainWindow::loadUserInfo()
{
    auto isLoad = false;
    QSettings settings;
    auto deviceId = settings.value("user/id", "-1").toString();
    auto userName = settings.value("user/name", "-1").toString();
    if (deviceId != "-1" && userName != "-1") {
        this->deviceId = deviceId;
        this->userName = userName;
        this->ui->deviceId->setText(this->deviceId);
        this->ui->userName->setText(this->userName);
        isLoad = true;
    } else {
        qInfo("the userName and deviceId is null, please login");
    }
    return isLoad;
}

void MainWindow::updateLoading(bool isrun)
{
    if (isrun) {
        this->ui->loadingLabel->setText("Loading...");
    } else {
        this->ui->loadingLabel->setText("");
    }
}

void MainWindow::onLoginButtonClicked()
{
    if (this->loginDialog != nullptr) {
        this->loginDialog->clearInput();
    } else {
        this->loginDialog = new LoginDialog();
        connect(this->loginDialog, SIGNAL(login()), this, SLOT(onLoginFinish()));
    }
    loginDialog->show();
}

void MainWindow::onLoginFinish()
{
    qInfo("Login Finish");
    this->loadUserInfo();
}

void MainWindow::onSearchButtonClicked()
{
    updateLoading(true);
    this->service->searchIp();
}

void MainWindow::onSendButtonClicked()
{

}

void MainWindow::onSearchIpFinish()
{
    qInfo("MainWindow::search ip Finish");
    auto ips = this->service->getIps();
    qInfo("MainWindow::ips: %d", (int)ips.size());
    this->ipModel->clear();
    for (int i = 0; i < ips.size(); i++) {
        this->ipModel->setItem(i, 0, new QStandardItem(ips[i]));
    }
    this->service->searchDevices();
}
void MainWindow::onSearchDeviceFinish()
{
    updateLoading(false);
    auto devices = this->service->getDevices();
    qInfo("MainWindow::devices: %d", (int)devices.size());
    this->deviceModel->clear();
    int i = 0;
    QString livepStr;
    foreach (auto value, devices) {
        this->deviceModel->setItem(i, 0, new QStandardItem(value->deviceId));
        this->deviceModel->setItem(i, 1, new QStandardItem(value->hostIp));
        if (value->livep) {
            livepStr = "Live";
        } else {
            livepStr = "false";
        }
        this->deviceModel->setItem(i, 2, new QStandardItem(livepStr));
        i++;
    }
}


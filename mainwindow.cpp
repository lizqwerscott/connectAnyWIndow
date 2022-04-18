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
    t1(new QThread()),
    service(new ConnectService())
{
    HttpUtils::getInstance(this);

    ui->setupUi(this);
    QCoreApplication::setOrganizationName("flydog");
    QCoreApplication::setOrganizationDomain("flydog.com");
    QCoreApplication::setApplicationName("connectAny-Windows");

    //UI settings
    ipModel->setColumnCount(1);
    ipModel->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("ip"));

    ui->ipTableView->setModel(this->ipModel);

    ui->ipTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(onLoginButtonClicked()));
    connect(ui->sendMessageButton, SIGNAL(clicked()), this, SLOT(onSendButtonClicked()));
    connect(ui->searchDevicesButton, SIGNAL(clicked()), this, SLOT(onSearchButtonClicked()));

    if (!loadUserInfo()) {
        this->loginDialog = new LoginDialog();
        connect(this->loginDialog, SIGNAL(login()), this, SLOT(onLoginFinish()));
        loginDialog->show();
    }

    updateLoading(false);

    qInfo("Start get LocalIp\n");
    auto localIp = HttpUtils::getLocalIp();
    auto gateWay = HttpUtils::getGateWay(localIp);
    qInfo("localIp:%s", localIp.toStdString().c_str());
    qInfo("gateWay:%s", gateWay.toStdString().c_str());

    service->moveToThread(t1);
    service->begin();

    connect(service, SIGNAL(searchFinish()), this, SLOT(onSearchIpFinish()));
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
    updateLoading(false);
    qInfo("MainWindow::search ip Finish\n");
    auto ips = this->service->getIps();
    qInfo("MainWindow::ips: %d\n", (int)ips.size());
    for (int i = 0; i < ips.size(); i++) {
        this->ipModel->setItem(i, 0, new QStandardItem(ips[i]));
    }
    this->service->searchDevices();
}


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QPushButton>
#include <QString>
#include <QStandardItemModel>

#include "loginDialog/logindialog.h"
#include "ConnectService/connectservice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onLoginButtonClicked();
    void onLoginFinish();

    void onSearchButtonClicked();
    void onSendButtonClicked();

    void onSearchIpFinish();

private:
    bool loadUserInfo();

    void updateLoading(bool isrun);

private:
    Ui::MainWindow *ui;

    QStandardItemModel * ipModel;
    QTableView * ipView;
    QTableView * devicesView;
    QPushButton * searchIpButton;
    QPushButton * searchDevicesButton;
    QPushButton * loginButton;

    LoginDialog * loginDialog = nullptr;

    QString deviceId;
    QString userName;

    QThread * t1;
    ConnectService * service = nullptr;

};
#endif // MAINWINDOW_H

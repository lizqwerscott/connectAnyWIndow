#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QString>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

signals:
    void login();

public slots:
    void onLoginClicked();
    void onCancelClicked();

public:
    void clearInput();

private:
    Ui::LoginDialog *ui;
    QLineEdit * deviceIdLineEdit;
    QLineEdit * userNameLineEdit;

    QString deviceId;
    QString userName;
};

#endif // LOGINDIALOG_H

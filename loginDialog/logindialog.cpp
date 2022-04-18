#include "logindialog.h"
#include "ui_logindialog.h"

#include <QSettings>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->deviceIdLineEdit = ui->idLineEdit;
    this->userNameLineEdit = ui->userNameLineEdit;

    connect(ui->okPushButton, SIGNAL(clicked()), this, SLOT(onLoginClicked()));
    connect(ui->cancelPushButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::onLoginClicked()
{
    auto id = this->deviceIdLineEdit->text().trimmed();
    auto userName = this->userNameLineEdit->text().trimmed();

    QSettings settings;
    settings.setValue("user/id", id);
    settings.setValue("user/name", userName);

    emit(login());
    this->close();
}

void LoginDialog::onCancelClicked()
{
    this->close();
}

void LoginDialog::clearInput()
{
    this->deviceIdLineEdit->clear();
    this->userNameLineEdit->clear();
}


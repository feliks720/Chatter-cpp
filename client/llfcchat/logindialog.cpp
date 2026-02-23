#include "logindialog.h"
#include "ui_logindialog.h"
#include "httpmgr.h"
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);

}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_login_btn_clicked()
{
    const QString user = ui->user_edit->text().trimmed();
    const QString pass = ui->pass_edit->text();
    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, tr("登录"), tr("请输入用户名和密码"));
        return;
    }

    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["password"] = pass;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/login"),
                                        json_obj, ReqId::ID_LOGIN_USER, Modules::LOGINMOD);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (id != ReqId::ID_LOGIN_USER) {
        return;
    }

    if (err != ErrorCodes::SUCCESS) {
        QMessageBox::warning(this, tr("登录"), tr("网络请求错误"));
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        QMessageBox::warning(this, tr("登录"), tr("服务器返回格式错误"));
        return;
    }

    const QJsonObject obj = jsonDoc.object();
    const int error = obj["error"].toInt();
    if (error != ErrorCodes::SUCCESS) {
        const QString msg = obj["message"].toString();
        QMessageBox::warning(this, tr("登录"), msg.isEmpty() ? tr("登录失败") : msg);
        return;
    }

    QMessageBox::information(this, tr("登录"), tr("登录成功，WebSocket地址: %1/ws").arg(gate_url_prefix));
}

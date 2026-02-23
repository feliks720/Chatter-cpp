#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QRegularExpression>
#include <QMessageBox>
#include "global.h"
#include "httpmgr.h"

namespace {
bool IsValidEmail(const QString& email) {
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    return regex.match(email).hasMatch();
}
}

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    //设置密码格式隐藏
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state","normal");
    repolish(ui->err_tip);
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    if(IsValidEmail(email)){
        //发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_varifycode"),
                     json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);

    }else{
        //提示邮箱不正确
        showTip(tr("邮箱地址不正确"),false);
    }
}

void RegisterDialog::on_pushButton_2_clicked()
{
    const QString user = ui->user_edit->text().trimmed();
    const QString email = ui->email_edit->text().trimmed();
    const QString pass = ui->pass_edit->text();
    const QString confirm = ui->confirm_edit->text();
    const QString code = ui->varify_edit->text().trimmed();

    if (user.isEmpty() || email.isEmpty() || pass.isEmpty() || confirm.isEmpty() || code.isEmpty()) {
        showTip(tr("请完整填写注册信息"), false);
        return;
    }

    if (!IsValidEmail(email)) {
        showTip(tr("邮箱地址不正确"), false);
        return;
    }

    if (pass != confirm) {
        showTip(tr("两次密码输入不一致"), false);
        return;
    }

    if (pass.length() < 6) {
        showTip(tr("密码长度至少6位"), false);
        return;
    }

    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["email"] = email;
    json_obj["password"] = pass;
    json_obj["code"] = code;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/register"),
                                        json_obj, ReqId::ID_REG_USER, Modules::REGISTERMOD);
}

void RegisterDialog::on_pushButton_clicked()
{
    emit sigSwitchLogin();
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }

    //调用对应的逻辑,根据id回调。
    if (!_handlers.contains(id)) {
        showTip(tr("未知的响应类型"), false);
        return;
    }

    _handlers[id](jsonDoc.object());

    return;
}

void RegisterDialog::initHttpHandlers()
{
    //注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        const QString code = jsonObj["code"].toString();
        if (!code.isEmpty()) {
            ui->varify_edit->setText(code);
        }
        showTip(tr("验证码已发送到邮箱，注意查收"), true);
        qDebug()<< "email is " << email ;
    });

    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            const QString msg = jsonObj["message"].toString();
            showTip(msg.isEmpty() ? tr("注册失败") : msg, false);
            return;
        }

        showTip(tr("注册成功，请登录"), true);
        QMessageBox::information(this, tr("注册"), tr("注册成功，请返回登录"));
        emit sigSwitchLogin();
    });
}

void RegisterDialog::showTip(QString str, bool b_ok)
{
    if(b_ok){
         ui->err_tip->setProperty("state","normal");
    }else{
        ui->err_tip->setProperty("state","err");
    }

    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

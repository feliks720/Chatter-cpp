#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
private slots:
    void on_login_btn_clicked();
public slots:
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);

private:
    Ui::LoginDialog *ui;
signals:
    void switchRegister();
};

#endif // LOGINDIALOG_H

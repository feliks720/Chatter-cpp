#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"
#include <functional>
#include <QMap>
#include <QJsonObject>
namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
public slots:
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
private:
    void initHttpHandlers();
    Ui::RegisterDialog *ui;
    void showTip(QString str,bool b_ok);
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
};

#endif // REGISTERDIALOG_H

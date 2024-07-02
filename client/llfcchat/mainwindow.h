#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
/******************************************************************************
 *
 * @file       mainwindow.h
 * @brief      主界面功能 Function
 *
 * @author     恋恋风辰
 * @date       2024/02/27
 * @history
 *****************************************************************************/
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void SlotSwitchReg();
private:
    Ui::MainWindow *ui;
    LoginDialog* _login_dlg;
    RegisterDialog* _reg_dlg;
};

#endif // MAINWINDOW_H

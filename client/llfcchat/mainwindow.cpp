#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _stacked = new QStackedWidget(this);
    _login_dlg = new LoginDialog(this);
    _reg_dlg = new RegisterDialog(this);
    _stacked->addWidget(_login_dlg);
    _stacked->addWidget(_reg_dlg);
    setCentralWidget(_stacked);
    _stacked->setCurrentWidget(_login_dlg);

    //创建和注册消息的连接
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    connect(_reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
    _stacked->setCurrentWidget(_reg_dlg);
}

void MainWindow::SlotSwitchLogin()
{
    _stacked->setCurrentWidget(_login_dlg);
}

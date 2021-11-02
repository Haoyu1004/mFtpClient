#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(clientWindow, &ClientWindow::destroyed, this, &MainWindow::on_ClientWindow_destroyed);
}

MainWindow::~MainWindow()
{
    if(clientWindow != nullptr) {
        delete clientWindow;
    }
    if(ftp != nullptr) {
        delete ftp;
    }
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ftp = new FtpClient();
    if(ftp->login(ui->lineEdit->text(), ui->lineEdit_2->text(), ui->lineEdit_3->text())) {
        clientWindow = new ClientWindow(ftp);
        clientWindow->show();
        this->hide();
    }
}

void MainWindow::on_MainWindow_destroyed()
{
    this->~MainWindow();
}

void MainWindow::on_ClientWindow_destroyed()
{
    delete clientWindow;
    clientWindow = nullptr;
}


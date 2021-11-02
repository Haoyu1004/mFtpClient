#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "clientwindow.h"
#include "ftpclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_MainWindow_destroyed();
    void on_ClientWindow_destroyed();

private:
    Ui::MainWindow *ui;

    ClientWindow *clientWindow = nullptr;

    FtpClient *ftp = nullptr;

};
#endif // MAINWINDOW_H

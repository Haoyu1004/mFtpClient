#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include "ftpclient.h"
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFont>

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(FtpClient *ftp = nullptr, QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void on_pushButton_upload_clicked();
    void on_pushButton_download_clicked();

    void on_listWidget_local_itemDoubleClicked(QListWidgetItem *item);
    void on_listWidget_remote_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_localdir_clicked();
    void on_pushButton_remotedir_clicked();

    void on_pushButton_remoteDel_clicked();

    void on_pushButton_localDel_clicked();

private:
    Ui::ClientWindow *ui;
    FtpClient *ftp;

    QDir localDir;
    QDir remoteDir;
    void listLocalFile();
    void listRemoteFile();

    QFont defaultFileFont = QFont("Microsoft YaHei Ui", 9, QFont::Normal);
    QFont defaultDirFont = QFont("Microsoft YaHei Ui", 9, QFont::Bold);

};

#endif // CLIENTWINDOW_H

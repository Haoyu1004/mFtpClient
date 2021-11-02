#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(FtpClient* ftp, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientWindow),
    ftp(ftp)
{
    ui->setupUi(this);

    localDir = QDir::current();
    ui->lineEdit_localDir->setText(localDir.path());
    listLocalFile();

    remoteDir = ftp->getDir();
    ui->lineEdit_remoteDir->setText(remoteDir.path());
    listRemoteFile();

}

ClientWindow::~ClientWindow()
{
    delete ui;
    ftp->logout();
    /* ftp is deleted in ~MainWindow */
}

void ClientWindow::on_pushButton_upload_clicked()
{
    QString fileName = ui->listWidget_local->currentItem()->text();
    ftp->uploadFile(localDir, fileName);
    listRemoteFile();
}


void ClientWindow::on_pushButton_download_clicked()
{
    QMessageBox::information(this, "info", ui->listWidget_remote->currentItem()->text());
}

void ClientWindow::listLocalFile()
{
    QFileInfoList infoList = localDir.entryInfoList();
    ui->listWidget_local->clear();
//    ui->listWidget_local->addItems(fileList);

    for(int row = 0; row < infoList.size(); ++row) {
        QFileInfo &fileinfo = infoList[row];

        ui->listWidget_local->addItem(fileinfo.fileName());
        if(fileinfo.isDir()) {
            ui->listWidget_local->item(row)->setFont(defaultDirFont);
        }
    }
}

void ClientWindow::listRemoteFile()
{
    QStringList fileList = ftp->getFileList(remoteDir).split("\r\n");
    ui->listWidget_remote->clear();
//    ui->listWidget_remote->addItems(fileList);

    ui->listWidget_remote->addItem(".");
    ui->listWidget_remote->item(0)->setFont(defaultDirFont);
    ui->listWidget_remote->addItem("..");
    ui->listWidget_remote->item(1)->setFont(defaultDirFont);

    for(int row = 2; row < 2 + fileList.size(); ++row) {
        QString &fileinfo = fileList[row-2];

        if(fileinfo == "") {
            continue;
        }
        QTextStream fileinfoStream(&fileinfo);

        QString autority, _1, _ftp1, _ftp2, filesize,  month, date, clock_year,fileName;
        fileinfoStream >> autority >> _1 >> _ftp1 >> _ftp2 >> filesize >> month >> date >> clock_year;

        fileName = fileinfoStream.readAll();

        ui->listWidget_remote->addItem(fileName);
        if(autority[0] == 'd') {
            ui->listWidget_remote->item(row)->setFont(defaultDirFont);
        }

    }
    /* "LIST" recive reply like
     *
     * -rw-rw-rw- 1 ftp ftp      1020264448 May 23 11:53 CentOS-7-x86_64-Minimal-2009.iso\r\n
     * -rw-rw-rw- 1 ftp ftp            1448 Dec 02  2020 BOOTEX.LOG\r\n
     * drwxrwxrwx 1 ftp ftp               0 Oct 11 12:41 Windows 10 x64
     * -rw-rw-rw- 1 ftp ftp      2193522688 Sep 11  2020 ubuntu-18.04.5-desktop-amd64.iso
     * drwxrwxrwx 1 ftp ftp               0 Oct 11 12:47 Ubuntu 18.04 64bit
    */
}


void ClientWindow::on_listWidget_local_itemDoubleClicked(QListWidgetItem *item)
{
    qDebug() << "local clickkkkkkkk" << item;
}


void ClientWindow::on_listWidget_remote_itemDoubleClicked(QListWidgetItem *item)
{
    qDebug() << "remote clickkkkkk " << item;
}


void ClientWindow::on_pushButton_localdir_clicked()
{
    QDir targetDir = QDir(ui->lineEdit_localDir->text());
    if(targetDir.exists()) {
        localDir = targetDir;
        listLocalFile();
    } else {
        ui->lineEdit_localDir->setText(localDir.path());
        QString criticalInfo = "Target rocal directory " + targetDir.path() + " does not exist.";
        QMessageBox::critical(this, "Criticle", criticalInfo);
    }
}


void ClientWindow::on_pushButton_remotedir_clicked()
{
    QDir targetDir = QDir(ui->lineEdit_remoteDir->text());
    if(ftp->chDir(targetDir)) {
        remoteDir = targetDir;
        listRemoteFile();
    } else {
        ui->lineEdit_remoteDir->setText(remoteDir.path());
        QString criticalInfo = "Target remote directory " + targetDir.path() + " does not exist.";
        QMessageBox::critical(this, "Criticle", criticalInfo);
    }
}


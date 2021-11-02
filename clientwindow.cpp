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
    QStringList fileList = localDir.entryList();
    ui->listWidget_local->clear();
    ui->listWidget_local->addItems(fileList);
}

void ClientWindow::listRemoteFile()
{
    QStringList fileList = ftp->getFileList(remoteDir).split("\r\n");
    ui->listWidget_remote->clear();
    ui->listWidget_remote->addItems(fileList);

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


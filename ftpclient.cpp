#include "ftpclient.h"

FtpClient::FtpClient(QObject *parent) : QObject(parent)
{
    ctrl = new QTcpSocket(this);

    connect(ctrl, &QTcpSocket::connected, this, &FtpClient::socketConnected);
    connect(ctrl, &QTcpSocket::readyRead, this, &FtpClient::ctrlReceive);
    connect(ctrl, &QTcpSocket::disconnected, this, &FtpClient::socketDisConnected);

    connect(data, &QTcpSocket::connected, this, &FtpClient::socketConnected);
    connect(data, &QTcpSocket::disconnected, this, &FtpClient::socketDisConnected);
}

void FtpClient::sendCommand(QString cmd)
{
    QByteArray rawcmd = cmd.toLatin1() + "\r\n";
    ctrl->write(rawcmd);
    ctrl->waitForReadyRead();
}

void FtpClient::socketConnected()
{
    qDebug() << "socket connected";
}

void FtpClient::socketDisConnected()
{
    qDebug() << "socket disconnected";
}

void FtpClient::ctrlReceive()
{
    while (ctrl->bytesAvailable()) {
        QByteArray rawReply = ctrl->readAll();
        QString reply = QString::fromLocal8Bit(rawReply);
        qDebug() << reply;

        replyCode = reply.mid(0, 3).toUInt();
        replyData = reply.mid(4);
    }
}

void FtpClient::dataReceive()
{

}

bool FtpClient::login(QString hostname, QString username, QString password)
{
    openCSocket(hostname, username, password);
    return ctrl->state() == QTcpSocket::ConnectedState;
}

void FtpClient::openCSocket(QString hostname, QString username, QString password)
{
    static QString save_hostname, save_username, save_password;
    if(hostname != "") {
        save_hostname = hostname;
        save_username = username;
        save_password = password;
    }

    if(ctrl->state() != QTcpSocket::ConnectedState) {

        ctrl->connectToHost(save_hostname, 21, QTcpSocket::ReadWrite, QTcpSocket::IPv4Protocol);
        ctrl->waitForConnected();
        ctrl->waitForReadyRead();
        qDebug() << "reading reply code" << replyCode;
        if(replyCode != 220) {
            return;
        }
    }
    assert(ctrl->state() == QTcpSocket::ConnectedState);

    sendCommand("USER " + save_username);
    if(replyCode != 331 && replyCode != 230) {    // 331 username ok, need password
        logout();
        return;
    }

    sendCommand("PASS " + save_password);
    if(replyCode != 230) {   // 230 user login;
        logout();
        return;
    }
}

void FtpClient::logout()
{
    sendCommand("QUIT");
    closeCSocket();
}

void FtpClient::closeCSocket()
{
    if(ctrl->state() != QTcpSocket::UnconnectedState) {
        ctrl->close();
        ctrl->waitForDisconnected();
    }
}

bool FtpClient::checkCSocket()
{
    if(ctrl->state() == QTcpSocket::ConnectedState) {
        return true;
    }
    openCSocket();
    return ctrl->state() == QTcpSocket::ConnectedState;
}

bool FtpClient::chDir(QDir dir)
{
    checkCSocket();
    sendCommand("CWD " + dir.absolutePath());
    if(replyCode != 250) {
        qDebug() << "chDir fail";
        return false;
    }
    return true;
}

void FtpClient::mkDir(QDir dir)
{
    checkCSocket();
    sendCommand("MKD " + dir.absolutePath());
    if(replyCode != 257) {
        qDebug() << "mkDir fail";
    }
}

void FtpClient::rmDir(QDir dir)
{
    checkCSocket();
    sendCommand("RMD " + dir.absolutePath());
    if(replyCode != 250) {
        qDebug() << "rmDir fail";
    }
}

QDir FtpClient::getDir()
{
    checkCSocket();
    sendCommand("PWD");
    if(replyCode != 250 && replyCode != 257) { // to be spcified
        qDebug() << "getDir fail";
    }
    qsizetype lpos = replyData.indexOf('"');
    qsizetype rpos = replyData.indexOf('"', lpos+1);
    return QDir(replyData.mid(lpos + 1, rpos - lpos -1));
}

QString FtpClient::getFileList(QDir dir)
{
    checkCSocket();
    openDSocket();

    sendCommand("LIST " + dir.absolutePath());
    if(replyCode != 150 && replyCode != 125 && replyCode != 226) { // to be spcified
        qDebug() << "list file fail";
    }

    data->waitForReadyRead();
    QByteArray rawReply = data->readAll();
    QString dataReply = QString::fromLocal8Bit(rawReply);
    qDebug() << "DSocket Received: (decoded) \n" << dataReply;

    closeDSocket();

    return dataReply;
}

void FtpClient::openDSocket()
{
    sendCommand("PASV");
    if(replyCode != 227) {
        qDebug() << "passive mode fail";
        return;
    }

    qsizetype lbracket = replyData.indexOf('(');
    qsizetype rbracket = replyData.indexOf(')');
    QString ip = replyData.mid(lbracket + 1, rbracket - lbracket - 1);

    QStringList parts = ip.split(',');
    QString hostname = parts[0] + '.' + parts[1] + '.' + parts[2] + '.' + parts[3];
    quint16 port = (parts[4].toUInt() << 8) | parts[5].toUInt();

    if(data == nullptr) {
        data = new QTcpSocket(this);
    }

    data->connectToHost(hostname, port, QTcpSocket::ReadWrite, QTcpSocket::IPv4Protocol);
    data->waitForConnected();
    if(data->state() != QTcpSocket::ConnectedState) { // to be spcified
        qDebug() << "data socket connection failed";
        return;
    }

    assert(data->state() == QTcpSocket::ConnectedState);
}

void FtpClient::closeDSocket()
{
    if(data->state() != QTcpSocket::UnconnectedState) {
        data->close();
        data->waitForDisconnected();
    }
}

void FtpClient::setTransferType(transferType type)
{
    checkCSocket();
    if(type==ascii) {
        sendCommand("TYPE A");
    } else if (type==binary) {
        sendCommand("TYPE I");
    }
    if(replyCode != 200) {
        qDebug() << "setTransferType fail";
    }
}

qsizetype FtpClient::getFileSize(QString filename)
{
    checkCSocket();

    openDSocket();
    sendCommand("SIZE " + filename);
    closeDSocket();

    if(replyCode == 213) {
        return replyData.toLong();
    } else {
        qDebug() << "file " + filename + " does not exist";
        return -1;
    }
}

void FtpClient::downloadFile(QDir localDir, QString fileName)
{
    checkCSocket();

    qsizetype fileSize = getFileSize(fileName);

    if(fileSize == -1) {       // file does not exist in remote server
        qDebug() << "file does not exits in remote server";
        return;
    }

    QDir filePath = localDir.filePath(fileName);
    QFile *localFile = new QFile(filePath.absolutePath());

    if(localFile->exists()) {       // filename exist in local disk

        qDebug() << "file already exists";
        delete localFile;
        return;
    } else {
        setTransferType(binary);

        openDSocket();

        sendCommand("RETR " + fileName);
        if (!(replyCode == 150) && !(replyCode == 125) && !(replyCode == 226) && !(replyCode == 250)) {
            qDebug() << "download fail: RETR command fail";
            closeDSocket();
            delete localFile;
            return;
        }

        localFile->open(QFile::WriteOnly);
        qsizetype bytesTotalWritten = 0;
        while(true) {
            qsizetype bytesBufferWritten = localFile->write(data->read(BUFFER_SIZE));

            if(bytesBufferWritten < 0) {
                qDebug() << "local file writing fail";
                break;
            }

            if(bytesBufferWritten == 0) {
                if(data->waitForReadyRead(TIMEOUT_MS)) {
                    continue;
                } else {
                    qDebug() << "download timeout";
                    break;
                }
            }

            localFile->waitForBytesWritten(TIMEOUT_MS);
            qDebug() << "writing to local file";

            bytesTotalWritten += bytesBufferWritten;
            if(bytesTotalWritten == fileSize) {
                break;
            }
        }

        localFile->close();
        delete localFile;
        closeDSocket();

        ctrl->waitForReadyRead();
        if (!(replyCode == 226) && !(replyCode == 250)) {   //
            qDebug() << "download fail(?): no control reply after download";
        }

        setTransferType(ascii);
    }
    qDebug() << "ctrl reaches end of downloadFile()";
}

void FtpClient::uploadFile(QDir localDir, QString fileName)
{
    checkCSocket();

    QDir filePath = localDir.filePath(fileName);
    QFile *localFile = new QFile(filePath.absolutePath());

    if(!localFile->exists() || !localFile->open(QFile::ReadOnly)) {     // file does not exist in local disk
        qDebug() << "upload fail: cannot open local file";
        delete localFile;
        return;
    }

    if(getFileSize(fileName) != -1) {   // filename exists in remote server

        qDebug() << "file already exist in remote server";
        delete localFile;
        return;
    } else {
        setTransferType(binary);

        openDSocket();

        sendCommand("STOR " + fileName);
        if (!(replyCode == 125) && !(replyCode == 150)) // 150
        {
            qDebug() << "upload fail: STOR command fail";
            closeDSocket();
            delete localFile;
            return;
        }

        while(!localFile->atEnd()) {
            if(data->write(localFile->read(BUFFER_SIZE)) <= 0) {
                qDebug() << "socket writing fail";
                break;
            }
            data->waitForBytesWritten();
            qDebug() << "writing to data socket";
        }

        delete localFile;
        closeDSocket();

        ctrl->waitForReadyRead();
        if (!(replyCode == 226) && !(replyCode == 250)) {   // 226
            qDebug() << "upload fail(?): no control reply after upload";
        }

        setTransferType(ascii);
    }
    qDebug() << "ctrl reaches end of uploadFile()";
}


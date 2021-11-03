#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QObject>

#include <qtcpsocket.h>
#include <qdir.h>

class FtpClient : public QObject
{
    Q_OBJECT
public:
    explicit FtpClient(QObject *parent = nullptr);

    bool login(QString hostname, QString username, QString password);
    void logout();

    bool chDir(QDir dir);
    void mkDir(QDir dir);
    void rmDir(QDir dir);
    QDir getDir();
    QString getFileList(QDir dir);

    void downloadFile(QDir localDir, QString fileName);
    void uploadFile(QDir localDir, QString fileName);

private:
    QTcpSocket *ctrl = nullptr;
    void openCSocket(QString hostname="", QString username="", QString password="");
    void closeCSocket();
    bool checkCSocket();
    void sendCommand(QString cmd);

    QTcpSocket *data = nullptr;
    void openDSocket();
    void closeDSocket();

    quint16 replyCode;
    QString replyData;

    enum transferType {
        ascii, binary
    };
    void setTransferType(transferType type);
    qsizetype getFileSize(QString filename);

    static const qsizetype BUFFER_SIZE = 16*1024;
    static const uint TIMEOUT_MS = 2000;

private slots:
    void socketConnected();
    void socketDisConnected();
    void ctrlReceive();
    void dataReceive();

signals:

};

#endif // FTPCLIENT_H

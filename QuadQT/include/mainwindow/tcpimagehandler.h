#ifndef TCPIMAGEHANDLER_H
#define TCPIMAGEHANDLER_H

#include <QObject>
#include <QSslSocket>
#include <QByteArray>

class TcpImageHandler : public QObject {
    Q_OBJECT
public:
    explicit TcpImageHandler(QObject *parent = nullptr);

    // 이미지 요청과 동시에 서버 연결 시도
    void connectToServerThenRequestImage(const QString& host, quint16 port, const QString& imagePath);

signals:
    void imageDataReady(const QString& imagePath, const QByteArray& data);
    void errorOccurred(const QString& errorMsg);

private slots:
    void onConnected();
    void onEncrypted();
    void onSslErrors(const QList<QSslError>& errs);
    void onReadyRead();
    void onDisconnected();

private:
    void sendGetImageRequest();

    QSslSocket* socket_;
    QByteArray buffer_;
    QString currentPath_;
    QString host_;
    quint16 port_;
    quint64 expectedSize_ = 0;
    bool headerParsed_ = false;
};

#endif // TCPIMAGEHANDLER_H

// tcphistoryhandler.h
#pragma once

#include <QObject>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslError>
#include <QJsonObject>

class TcpHistoryHandler : public QObject
{
    Q_OBJECT

public:
    explicit TcpHistoryHandler(QObject *parent = nullptr);

    /// 서버(SSL) 연결
    void connectToServer(const QString &host, quint16 port);

    /// API 호출
    void getHistory(const QString &email, int limit, int offset);
    void getHistoryByEventType(const QString &email, int eventType, int limit, int offset);
    void getHistoryByDateRange(const QString &email,
                               const QString &startDate,
                               const QString &endDate,
                               int limit, int offset);
    void getHistoryByEventTypeAndDateRange(const QString &email,
                                           int eventType,
                                           const QString &startDate,
                                           const QString &endDate,
                                           int limit, int offset);

signals:
    /// 서버 연결 완료 시
    void connected();
    /// 서버로부터 JSON 응답 수신 시
    void historyDataReady(const QJsonObject &response);
    /// 오류 발생 시
    void errorOccurred(const QString &errorString);

private slots:
    void onEncrypted();
    void onReadyRead();
    void onSslErrors(const QList<QSslError> &errors);
    void onSocketError(QAbstractSocket::SocketError error);
    void onDisconnected();

private:
    QSslSocket *socket_;
    void sendCommand(const QString &cmd);
    QString findCertificateFile(const QString &filename);
};

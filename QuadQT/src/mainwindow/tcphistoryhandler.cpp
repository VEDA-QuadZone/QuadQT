// tcphistoryhandler.cpp
#include "mainwindow/tcphistoryhandler.h"
#include <QFile>
#include <QJsonDocument>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslConfiguration>
#include <QFile>
TcpHistoryHandler::TcpHistoryHandler(QObject *parent)
    : QObject(parent),
    socket_(new QSslSocket(this))
{
    // 1) client private key 로드
    {
        QFile keyFile("../../client.key.pem");
        if (keyFile.open(QIODevice::ReadOnly)) {
            QByteArray keyData = keyFile.readAll();
            QSslKey key(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
            socket_->setPrivateKey(key);
        }
    }

    // 2) client certificate 로드
    {
        QFile certFile("../../client.cert.pem");
        if (certFile.open(QIODevice::ReadOnly)) {
            QByteArray certData = certFile.readAll();
            QSslCertificate cert(certData, QSsl::Pem);
            socket_->setLocalCertificate(cert);
        }
    }

    // 3) CA certificate 로드해서 SSL config 에 추가
    {
        QFile caFile("../../ca.cert.pem");
        if (caFile.open(QIODevice::ReadOnly)) {
            QByteArray caData = caFile.readAll();
            auto caCerts = QSslCertificate::fromData(caData, QSsl::Pem);
            QSslConfiguration cfg = socket_->sslConfiguration();
            cfg.setCaCertificates(caCerts);
            socket_->setSslConfiguration(cfg);
        }
    }

    // 필수: 서버 인증서 검증 켜기
    socket_->setPeerVerifyMode(QSslSocket::VerifyPeer);

    connect(socket_, &QSslSocket::encrypted,
            this, &TcpHistoryHandler::onEncrypted);
    connect(socket_, &QSslSocket::readyRead,
            this, &TcpHistoryHandler::onReadyRead);
    connect(socket_, &QSslSocket::sslErrors,
            this, &TcpHistoryHandler::onSslErrors);
}

void TcpHistoryHandler::connectToServer(const QString &host, quint16 port)
{
    socket_->connectToHostEncrypted(host, port);
}

void TcpHistoryHandler::getHistory(const QString &email, int limit, int offset)
{
    sendCommand(QString("GET_HISTORY %1 %2 %3")
                    .arg(email).arg(limit).arg(offset));
}

void TcpHistoryHandler::getHistoryByEventType(const QString &email,
                                              int eventType,
                                              int limit, int offset)
{
    sendCommand(QString("GET_HISTORY_BY_EVENT_TYPE %1 %2 %3 %4")
                    .arg(email).arg(eventType).arg(limit).arg(offset));
}

void TcpHistoryHandler::getHistoryByDateRange(const QString &email,
                                              const QString &startDate,
                                              const QString &endDate,
                                              int limit, int offset)
{
    sendCommand(QString("GET_HISTORY_BY_DATE_RANGE %1 %2 %3 %4 %5")
                    .arg(email)
                    .arg(startDate)
                    .arg(endDate)
                    .arg(limit)
                    .arg(offset));
}

void TcpHistoryHandler::getHistoryByEventTypeAndDateRange(const QString &email,
                                                          int eventType,
                                                          const QString &startDate,
                                                          const QString &endDate,
                                                          int limit, int offset)
{
    sendCommand(QString("GET_HISTORY_BY_EVENT_TYPE_AND_DATE_RANGE %1 %2 %3 %4 %5 %6")
                    .arg(email)
                    .arg(eventType)
                    .arg(startDate)
                    .arg(endDate)
                    .arg(limit)
                    .arg(offset));
}

void TcpHistoryHandler::sendCommand(const QString &cmd)
{
    // 그냥 write() 하면 QSslSocket이 연결 완료 후 자동으로 버퍼를 flush 해 줍니다.
    QByteArray line = cmd.toUtf8() + "\n";
    socket_->write(line);
}

void TcpHistoryHandler::onEncrypted()
{
    // 연결된 직후 필요한 초기 요청이 있으면 여기서…
}

void TcpHistoryHandler::onReadyRead()
{
    // 서버가 보내는 JSON 전체를 라인 단위로 받는다 가정
    while (socket_->canReadLine()) {
        QByteArray line = socket_->readLine().trimmed();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            emit historyDataReady(doc.object());
        } else {
            emit errorOccurred(tr("Invalid JSON response"));
        }
    }
}

void TcpHistoryHandler::onSslErrors(const QList<QSslError> &errors)
{
    // 인증서 문제 무시(필요 시 정책 변경)
    socket_->ignoreSslErrors();
    Q_UNUSED(errors);
}

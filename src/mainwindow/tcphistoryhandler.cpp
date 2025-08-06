// tcphistoryhandler.cpp
#include "mainwindow/tcphistoryhandler.h"
#include <QFile>
#include <QJsonDocument>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslConfiguration>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
TcpHistoryHandler::TcpHistoryHandler(QObject *parent)
    : QObject(parent),
    socket_(new QSslSocket(this))
{
    // 1) client private key 로드
    {
        QString keyPath = findCertificateFile("client.key.pem");
        if (!keyPath.isEmpty()) {
            QFile keyFile(keyPath);
            if (keyFile.open(QIODevice::ReadOnly)) {
                QByteArray keyData = keyFile.readAll();
                QSslKey key(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
                socket_->setPrivateKey(key);
                qDebug() << "Client private key loaded successfully from:" << keyPath;
            } else {
                qDebug() << "Failed to load client private key:" << keyFile.errorString();
            }
        } else {
            qDebug() << "Failed to load client private key: 지정된 파일을 찾을 수 없습니다.";
        }
    }

    // 2) client certificate 로드
    {
        QString certPath = findCertificateFile("client.cert.pem");
        if (!certPath.isEmpty()) {
            QFile certFile(certPath);
            if (certFile.open(QIODevice::ReadOnly)) {
                QByteArray certData = certFile.readAll();
                QSslCertificate cert(certData, QSsl::Pem);
                socket_->setLocalCertificate(cert);
                qDebug() << "Client certificate loaded successfully from:" << certPath;
            } else {
                qDebug() << "Failed to load client certificate:" << certFile.errorString();
            }
        } else {
            qDebug() << "Failed to load client certificate: 지정된 파일을 찾을 수 없습니다.";
        }
    }

    // 3) CA certificate 로드해서 SSL config 에 추가
    {
        QString caPath = findCertificateFile("ca.cert.pem");
        if (!caPath.isEmpty()) {
            QFile caFile(caPath);
            if (caFile.open(QIODevice::ReadOnly)) {
                QByteArray caData = caFile.readAll();
                auto caCerts = QSslCertificate::fromData(caData, QSsl::Pem);
                QSslConfiguration cfg = socket_->sslConfiguration();
                cfg.setCaCertificates(caCerts);
                socket_->setSslConfiguration(cfg);
                qDebug() << "CA certificate loaded successfully from:" << caPath;
            } else {
                qDebug() << "Failed to load CA certificate:" << caFile.errorString();
            }
        } else {
            qDebug() << "Failed to load CA certificate: 지정된 파일을 찾을 수 없습니다.";
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
    connect(socket_, &QSslSocket::errorOccurred,
            this, &TcpHistoryHandler::onSocketError);
    connect(socket_, &QSslSocket::disconnected,
            this, &TcpHistoryHandler::onDisconnected);
}

void TcpHistoryHandler::connectToServer(const QString &host, quint16 port)
{
    qDebug() << "Attempting to connect to" << host << ":" << port;
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
    if (socket_->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Socket not connected, cannot send command:" << cmd;
        emit errorOccurred("Socket not connected");
        return;
    }
    
    qDebug() << "Sending command:" << cmd;
    QByteArray line = cmd.toUtf8() + "\n";
    qint64 written = socket_->write(line);
    if (written == -1) {
        qDebug() << "Failed to write command to socket";
        emit errorOccurred("Failed to send command");
    } else {
        socket_->flush();
    }
}

void TcpHistoryHandler::onEncrypted()
{
    qDebug() << "SSL connection established successfully";
    emit connected();
}

void TcpHistoryHandler::onReadyRead()
{
    // 서버가 보내는 JSON 전체를 라인 단위로 받는다 가정
    while (socket_->canReadLine()) {
        QByteArray line = socket_->readLine().trimmed();
        qDebug() << "Received data:" << line;
        
        if (line.isEmpty()) continue;
        
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            qDebug() << "Valid JSON received, emitting historyDataReady";
            emit historyDataReady(doc.object());
        } else {
            qDebug() << "JSON parse error:" << err.errorString();
            emit errorOccurred(tr("Invalid JSON response: %1").arg(err.errorString()));
        }
    }
}

void TcpHistoryHandler::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "SSL errors occurred:";
    for (const auto& error : errors) {
        qDebug() << "  -" << error.errorString();
    }
    // 인증서 문제 무시(필요 시 정책 변경)
    socket_->ignoreSslErrors();
}

void TcpHistoryHandler::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QString errorMsg = QString("Socket error: %1").arg(socket_->errorString());
    qDebug() << errorMsg;
    
    // 연결 관련 오류인 경우 connectionFailed 시그널 발생
    if (error == QAbstractSocket::ConnectionRefusedError ||
        error == QAbstractSocket::HostNotFoundError ||
        error == QAbstractSocket::NetworkError ||
        error == QAbstractSocket::SocketTimeoutError) {
        emit connectionFailed();
    }
    
    emit errorOccurred(errorMsg);
}

void TcpHistoryHandler::onDisconnected()
{
    qDebug() << "Socket disconnected";
}

QString TcpHistoryHandler::findCertificateFile(const QString &filename)
{
    // 여러 경로에서 인증서 파일을 찾아봅니다
    QStringList searchPaths = {
        filename,                    // 원본 경로
        "./" + filename,            // 현재 디렉토리
        "../" + filename,           // 상위 디렉토리
        "../../" + filename,        // 상위의 상위 디렉토리
        "resources/certs/" + QFileInfo(filename).fileName(),  // resources/certs 디렉토리
        QCoreApplication::applicationDirPath() + "/" + filename,  // 실행 파일 디렉토리
        QDir::currentPath() + "/" + filename  // 현재 작업 디렉토리
    };
    
    for (const QString &path : searchPaths) {
        QFile file(path);
        if (file.exists()) {
            qDebug() << "인증서 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "다음 경로들에서 인증서 파일을 찾을 수 없습니다:";
    for (const QString &path : searchPaths) {
        qDebug() << "  -" << path;
    }
    
    return QString(); // 빈 문자열 반환
}

bool TcpHistoryHandler::isConnected() const
{
    return socket_ && socket_->state() == QAbstractSocket::ConnectedState;
}

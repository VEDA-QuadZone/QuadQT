// tcpimagehandler.cpp
#include "mainwindow/tcpimagehandler.h"

#include <QFile>
#include <QDebug>
#include <QSslCertificate>
#include <QSslKey>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork/QSslConfiguration>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

TcpImageHandler::TcpImageHandler(QObject *parent)
    : QObject(parent),
    socket_(new QSslSocket(this))
{
    // 1) client private key 로드
    QString keyPath = findCertificateFile("client.key.pem");
    if (!keyPath.isEmpty()) {
        QFile keyFile(keyPath);
        if (keyFile.open(QIODevice::ReadOnly)) {
            QByteArray keyData = keyFile.readAll();
            QSslKey key(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
            socket_->setPrivateKey(key);
            qDebug() << "클라이언트 개인키 로드 성공:" << keyPath;
        }
    }

    // 2) client certificate 로드
    QString certPath = findCertificateFile("client.cert.pem");
    if (!certPath.isEmpty()) {
        QFile certFile(certPath);
        if (certFile.open(QIODevice::ReadOnly)) {
            QByteArray certData = certFile.readAll();
            QSslCertificate cert(certData, QSsl::Pem);
            socket_->setLocalCertificate(cert);
            qDebug() << "클라이언트 인증서 로드 성공:" << certPath;
        }
    }

    // 3) CA certificate 로드해서 SSL config 에 추가
    QString caPath = findCertificateFile("ca.cert.pem");
    if (!caPath.isEmpty()) {
        QFile caFile(caPath);
        if (caFile.open(QIODevice::ReadOnly)) {
            QByteArray caData = caFile.readAll();
            auto caCerts = QSslCertificate::fromData(caData, QSsl::Pem);
            QSslConfiguration cfg = socket_->sslConfiguration();
            cfg.setCaCertificates(caCerts);
            socket_->setSslConfiguration(cfg);
            qDebug() << "CA 인증서 로드 성공:" << caPath;
        }
    }

    connect(socket_, &QSslSocket::encrypted,        this, &TcpImageHandler::onEncrypted);
    connect(socket_, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors), this, &TcpImageHandler::onSslErrors);
    connect(socket_, &QSslSocket::connected,        this, &TcpImageHandler::onConnected);
    connect(socket_, &QSslSocket::readyRead,        this, &TcpImageHandler::onReadyRead);
    connect(socket_, &QSslSocket::disconnected,     this, &TcpImageHandler::onDisconnected);
    
    // 에러 처리 추가
    connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError error) {
                qDebug() << "[TcpImageHandler] Socket 에러 발생:" << error << socket_->errorString();
                emit errorOccurred(QString("Socket error: %1").arg(socket_->errorString()));
            });
}

void TcpImageHandler::connectToServerThenRequestImage(const QString &host, quint16 port, const QString &imagePath) {
    qDebug() << "[TcpImageHandler] connectToServerThenRequestImage 호출됨";
    qDebug() << "[TcpImageHandler] Host:" << host << "Port:" << port << "Path:" << imagePath;
    
    host_ = host;
    port_ = port;
    currentPath_ = imagePath;

    buffer_.clear();
    expectedSize_ = 0;
    headerParsed_ = false;

    qDebug() << "[TcpImageHandler] 기존 연결 중단 후 새 연결 시도...";
    socket_->abort();
    socket_->connectToHostEncrypted(host_, port_);
    qDebug() << "[TcpImageHandler] connectToHostEncrypted 호출 완료";
}

void TcpImageHandler::onConnected() {
    qDebug() << "[TcpImageHandler] TCP connected, starting SSL handshake";
}

void TcpImageHandler::onEncrypted() {
    qDebug() << "[TcpImageHandler] SSL/TLS handshake complete";
    sendGetImageRequest();
}

void TcpImageHandler::sendGetImageRequest() {
    qDebug() << "[TcpImageHandler] sendGetImageRequest 호출됨";
    qDebug() << "[TcpImageHandler] Socket 상태:" << socket_->state();
    
    if (socket_->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[TcpImageHandler] 서버에 연결되지 않음, 에러 발생";
        emit errorOccurred("Not connected to server");
        return;
    }

    QByteArray cmd = "GET_IMAGE " + currentPath_.toUtf8() + "\n";
    qDebug() << "[TcpImageHandler] 전송할 명령:" << cmd;
    
    qint64 written = socket_->write(cmd);
    socket_->flush();

    qDebug() << "[TcpImageHandler] GET_IMAGE 명령 전송 완료, 전송된 바이트:" << written;
    qDebug() << "[TcpImageHandler] 요청한 이미지 경로:" << currentPath_;
}

void TcpImageHandler::onSslErrors(const QList<QSslError>& errs) {
    socket_->ignoreSslErrors();
    for (const auto &e : errs) {
        qWarning() << "[TcpImageHandler] Ignored SSL error:" << e.errorString();
    }
}

void TcpImageHandler::onReadyRead() {
    if (!headerParsed_ && socket_->bytesAvailable() >= 8) {
        QByteArray header = socket_->read(8);
        QDataStream stream(&header, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream >> expectedSize_;
        headerParsed_ = true;
        qDebug() << "[TcpImageHandler] Expecting image size:" << expectedSize_;
    }

    buffer_.append(socket_->readAll());

    if (headerParsed_ && buffer_.size() >= expectedSize_) {
        qDebug() << "[TcpImageHandler] Received full image data, size:" << buffer_.size();

        QFile file("C:/test_received_image.jpg");
        if (file.open(QIODevice::WriteOnly)) {
            file.write(buffer_);
            file.close();
            qDebug() << "파일로 이미지 저장 성공! 경로: C:/test_received_image.jpg";
        } else {
            qDebug() << "파일 저장 실패!";
        }

        emit imageDataReady(currentPath_, buffer_);
        socket_->disconnectFromHost();
    }
}

void TcpImageHandler::onDisconnected() {
    qDebug() << "[TcpImageHandler] Disconnected. Final buffer size:" << buffer_.size();

    if (!headerParsed_ && buffer_.startsWith('{')) {
        QJsonDocument doc = QJsonDocument::fromJson(buffer_);
        if (doc.isObject()) {
            QString msg = doc.object().value("message").toString();
            emit errorOccurred(msg.isEmpty() ? "Unknown server error" : msg);
        }
    } else if (!buffer_.isEmpty()) {
        emit imageDataReady(currentPath_, buffer_);
    }
}

QString TcpImageHandler::findCertificateFile(const QString &filename)
{
    // 여러 경로에서 인증서 파일을 찾아봅니다
    QStringList searchPaths = {
        QString(":/certs/%1").arg(filename),  // 리소스 경로 우선
        filename,                    // 원본 경로
        "./" + filename,            // 현재 디렉토리
        "../" + filename,           // 상위 디렉토리
        "../../" + filename,        // 상위의 상위 디렉토리
        "resources/certs/" + QFileInfo(filename).fileName(),  // resources/certs 디렉토리
        QCoreApplication::applicationDirPath() + "/" + filename,  // 실행 파일 디렉토리
        QDir::currentPath() + "/" + filename  // 현재 작업 디렉토리
    };
    
    for (const QString &path : searchPaths) {
        if (path.startsWith(":/")) {
            // 리소스 파일인 경우
            QFile file(path);
            if (file.exists()) {
                qDebug() << "인증서 파일 발견 (리소스):" << path;
                return path;
            }
        } else {
            // 일반 파일인 경우
            QFile file(path);
            if (file.exists()) {
                qDebug() << "인증서 파일 발견:" << path;
                return path;
            }
        }
    }
    
    qDebug() << "다음 경로들에서 인증서 파일을 찾을 수 없습니다:";
    for (const QString &path : searchPaths) {
        qDebug() << "  -" << path;
    }
    
    return QString(); // 빈 문자열 반환
}

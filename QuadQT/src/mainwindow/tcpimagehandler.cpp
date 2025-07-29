// tcpimagehandler.cpp
#include "mainwindow/tcpimagehandler.h"

#include <QFile>
#include <QDebug>
#include <QSslCertificate>
#include <QSslKey>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork/QSslConfiguration>

TcpImageHandler::TcpImageHandler(QObject *parent)
    : QObject(parent),
    socket_(new QSslSocket(this))
{
    // 1) client private key 로드
    QFile keyFile("../../client.key.pem");
    if (keyFile.open(QIODevice::ReadOnly)) {
        QByteArray keyData = keyFile.readAll();
        QSslKey key(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        socket_->setPrivateKey(key);
    }

    // 2) client certificate 로드
    QFile certFile("../../client.cert.pem");
    if (certFile.open(QIODevice::ReadOnly)) {
        QByteArray certData = certFile.readAll();
        QSslCertificate cert(certData, QSsl::Pem);
        socket_->setLocalCertificate(cert);
    }

    // 3) CA certificate 로드해서 SSL config 에 추가
    QFile caFile("../../ca.cert.pem");
    if (caFile.open(QIODevice::ReadOnly)) {
        QByteArray caData = caFile.readAll();
        auto caCerts = QSslCertificate::fromData(caData, QSsl::Pem);
        QSslConfiguration cfg = socket_->sslConfiguration();
        cfg.setCaCertificates(caCerts);
        socket_->setSslConfiguration(cfg);
    }

    connect(socket_, &QSslSocket::encrypted,        this, &TcpImageHandler::onEncrypted);
    connect(socket_, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors), this, &TcpImageHandler::onSslErrors);
    connect(socket_, &QSslSocket::connected,        this, &TcpImageHandler::onConnected);
    connect(socket_, &QSslSocket::readyRead,        this, &TcpImageHandler::onReadyRead);
    connect(socket_, &QSslSocket::disconnected,     this, &TcpImageHandler::onDisconnected);
}

void TcpImageHandler::connectToServerThenRequestImage(const QString &host, quint16 port, const QString &imagePath) {
    host_ = host;
    port_ = port;
    currentPath_ = imagePath;

    buffer_.clear();
    expectedSize_ = 0;
    headerParsed_ = false;

    socket_->abort();
    socket_->connectToHostEncrypted(host_, port_);
}

void TcpImageHandler::onConnected() {
    qDebug() << "[TcpImageHandler] TCP connected, starting SSL handshake";
}

void TcpImageHandler::onEncrypted() {
    qDebug() << "[TcpImageHandler] SSL/TLS handshake complete";
    sendGetImageRequest();
}

void TcpImageHandler::sendGetImageRequest() {
    if (socket_->state() != QAbstractSocket::ConnectedState) {
        emit errorOccurred("Not connected to server");
        return;
    }

    QByteArray cmd = "GET_IMAGE " + currentPath_.toUtf8() + "\n";
    socket_->write(cmd);
    socket_->flush();

    qDebug() << "[TcpImageHandler] Sent GET_IMAGE for" << currentPath_;
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

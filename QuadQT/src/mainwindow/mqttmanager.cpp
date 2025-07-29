#include "MqttManager.h"
#include <QFile>
#include <QDebug>
#include <QSslKey>
#include <QMqttTopicFilter>

MqttManager::MqttManager(QObject *parent)
    : QObject(parent)
{
    loadCertificates();

    client.setHostname("localhost");   // 브로커 IP
    client.setPort(8883);

    connect(&client, &QMqttClient::connected, this, [this]() {
        qDebug() << "[MQTT] Connected!";
        emit connected();
        QMqttTopicFilter filter("alert");
        auto sub = client.subscribe(filter, 1);
        if (sub)
            qDebug() << "[MQTT] Subscribed to alert";
        else
            qDebug() << "[MQTT] Subscribe failed";
    });

    connect(&client, &QMqttClient::messageReceived, this,
            [this](const QByteArray &msg, const QMqttTopicName &topic) {
                emit newMessage(topic.name(), msg);
            });

    connect(&client, &QMqttClient::errorChanged, this, [this]() {
        qDebug() << "[MQTT] Error:" << client.error();
    });

    connect(&client, &QMqttClient::messageReceived, this,
            [this](const QByteArray &msg, const QMqttTopicName &topic) {
                Q_UNUSED(topic);
                emit messageReceived(msg);
            });
}

void MqttManager::loadCertificates()
{
    QFile caFile(":/certs/ca.cert.pem");
    if (caFile.open(QIODevice::ReadOnly)) {
        QSslCertificate caCert(&caFile, QSsl::Pem);
        sslConfig.addCaCertificate(caCert);
        qDebug() << "[TLS] CA cert loaded";
    } else {
        qDebug() << "[TLS] Failed to load CA cert";
    }

    QFile certFile(":/certs/client.cert.pem");
    QFile keyFile(":/certs/client.key.pem");
    if (certFile.open(QIODevice::ReadOnly) && keyFile.open(QIODevice::ReadOnly)) {
        QSslCertificate clientCert(&certFile, QSsl::Pem);
        QSslKey clientKey(&keyFile, QSsl::Rsa, QSsl::Pem);
        sslConfig.setLocalCertificate(clientCert);
        sslConfig.setPrivateKey(clientKey);
        qDebug() << "[TLS] Client cert/key loaded";
    } else {
        qDebug() << "[TLS] Failed to load client cert or key";
    }
}

void MqttManager::publish(const QString &topic, const QByteArray &payload)
{
    client.publish(topic, payload, 1, false);  // QoS 1, retain false
}

void MqttManager::connectToBroker()
{
    qDebug() << "[DEBUG] Connecting to" << client.hostname() << client.port();
    qDebug() << "[DEBUG] CA count:" << sslConfig.caCertificates().size();
    qDebug() << "[DEBUG] Local cert valid:" << !sslConfig.localCertificate().isNull();
    client.connectToHostEncrypted(sslConfig);
}

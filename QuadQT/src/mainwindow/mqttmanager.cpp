#include "MqttManager.h"
#include <QFile>
#include <QDebug>
#include <QSslKey>
#include <QMqttTopicFilter>
#include <QSettings>
#include <QUrl>

MqttManager::MqttManager(QObject *parent)
    : QObject(parent)
{
    loadCertificates();

    // config.ini에서 MQTT 설정 읽기
    QSettings settings("config.ini", QSettings::IniFormat);
    QString brokerUrl = settings.value("mqtt/broker_url").toString();
    subscribeTopic = settings.value("mqtt/subscribe_topic").toString();
    publishTopic = settings.value("mqtt/publish_topic").toString();
    
    // URL 파싱해서 호스트와 포트 추출
    QUrl url(brokerUrl);
    client.setHostname(url.host());
    client.setPort(url.port(1883)); // 기본 포트 1883
    
    connect(&client, &QMqttClient::connected, this, [this]() {
        qDebug() << "[MQTT] Connected!";
        emit connected();
        QMqttTopicFilter filter(subscribeTopic);
        auto sub = client.subscribe(filter, 1);
        if (sub)
            qDebug() << "[MQTT] Subscribed to" << subscribeTopic;
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

void MqttManager::publish(const QByteArray &payload)
{
    client.publish(publishTopic, payload, 1, false);  // 기본 토픽 사용
}

void MqttManager::connectToBroker()
{
    qDebug() << "[DEBUG] Connecting to" << client.hostname() << client.port();
    qDebug() << "[DEBUG] CA count:" << sslConfig.caCertificates().size();
    qDebug() << "[DEBUG] Local cert valid:" << !sslConfig.localCertificate().isNull();
    client.connectToHostEncrypted(sslConfig);
}

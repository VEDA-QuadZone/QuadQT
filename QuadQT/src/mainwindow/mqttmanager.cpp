#include "mainwindow/mqttmanager.h"
#include <QFile>
#include <QDebug>
#include <QSslKey>
#include <QMqttTopicFilter>
#include <QSettings>
#include <QUrl>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

MqttManager::MqttManager(QObject *parent)
    : QObject(parent)
{
    loadCertificates();

    // config.ini에서 MQTT 설정 읽기
    QString configPath = findConfigFile();
    if (!configPath.isEmpty()) {
        QSettings settings(configPath, QSettings::IniFormat);
        brokerUrl = settings.value("mqtt/broker_url").toString();
        subscribeTopic = settings.value("mqtt/subscribe_topic").toString();
        publishTopic = settings.value("mqtt/publish_topic").toString();
        
        qDebug() << "MQTT 설정 로드 - Broker URL:" << brokerUrl;
        
        // URL 파싱해서 호스트와 포트 추출
        QUrl url(brokerUrl);
        client.setHostname(url.host());
        client.setPort(url.port(1883)); // 기본 포트 1883
        
        // SSL 사용 여부 결정 (mqtts:// 또는 ssl:// 스키마인 경우)
        useSSL = (url.scheme().toLower() == "mqtts" || url.scheme().toLower() == "ssl");
        
        qDebug() << "MQTT 설정 완료 - Host:" << url.host() << "Port:" << url.port(1883) << "SSL:" << useSSL;
    } else {
        qDebug() << "MQTT: config.ini 파일을 찾을 수 없습니다.";
    }
    
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
    // CA 인증서 로드 (리소스 우선, 실패 시 파일 시스템)
    QString caPath = findCertificateFile("ca.cert.pem");
    if (!caPath.isEmpty()) {
        QFile caFile(caPath);
        if (caFile.open(QIODevice::ReadOnly)) {
            QSslCertificate caCert(&caFile, QSsl::Pem);
            sslConfig.addCaCertificate(caCert);
            qDebug() << "[TLS] CA cert loaded from:" << caPath;
        } else {
            qDebug() << "[TLS] Failed to open CA cert file:" << caPath;
        }
    } else {
        qDebug() << "[TLS] Failed to load CA cert";
    }

    // 클라이언트 인증서와 키 로드
    QString certPath = findCertificateFile("client.cert.pem");
    QString keyPath = findCertificateFile("client.key.pem");
    
    if (!certPath.isEmpty() && !keyPath.isEmpty()) {
        QFile certFile(certPath);
        QFile keyFile(keyPath);
        if (certFile.open(QIODevice::ReadOnly) && keyFile.open(QIODevice::ReadOnly)) {
            QSslCertificate clientCert(&certFile, QSsl::Pem);
            QSslKey clientKey(&keyFile, QSsl::Rsa, QSsl::Pem);
            sslConfig.setLocalCertificate(clientCert);
            sslConfig.setPrivateKey(clientKey);
            qDebug() << "[TLS] Client cert/key loaded from:" << certPath << "and" << keyPath;
        } else {
            qDebug() << "[TLS] Failed to open client cert or key files";
        }
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
    qDebug() << "[DEBUG] SSL enabled:" << useSSL;
    
    if (useSSL) {
        qDebug() << "[DEBUG] CA count:" << sslConfig.caCertificates().size();
        qDebug() << "[DEBUG] Local cert valid:" << !sslConfig.localCertificate().isNull();
        client.connectToHostEncrypted(sslConfig);
    } else {
        qDebug() << "[DEBUG] Connecting without SSL";
        client.connectToHost();
    }
}

QString MqttManager::findCertificateFile(const QString &filename)
{
    // 1. 리소스 경로 우선 시도
    QString resourcePath = ":/certs/" + filename;
    if (QFile::exists(resourcePath)) {
        qDebug() << "MQTT 인증서 파일 발견 (리소스):" << resourcePath;
        return resourcePath;
    }
    
    // 2. 파일 시스템 경로들 시도
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
            qDebug() << "MQTT 인증서 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "MQTT: 다음 경로들에서 인증서 파일을 찾을 수 없습니다:";
    qDebug() << "  - 리소스:" << resourcePath;
    for (const QString &path : searchPaths) {
        qDebug() << "  -" << path;
    }
    
    return QString(); // 빈 문자열 반환
}

QString MqttManager::findConfigFile()
{
    // config.ini 파일을 여러 경로에서 찾아봅니다
    QStringList searchPaths = {
        "config.ini",                    // 현재 디렉토리
        "./config.ini",                  // 명시적 현재 디렉토리
        "../config.ini",                 // 상위 디렉토리
        "../../config.ini",              // 상위의 상위 디렉토리
        QCoreApplication::applicationDirPath() + "/config.ini",  // 실행 파일 디렉토리
        QDir::currentPath() + "/config.ini"  // 현재 작업 디렉토리
    };
    
    for (const QString &path : searchPaths) {
        QFile file(path);
        if (file.exists()) {
            qDebug() << "MQTT 설정 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "MQTT: 다음 경로들에서 config.ini 파일을 찾을 수 없습니다:";
    for (const QString &path : searchPaths) {
        qDebug() << "  -" << path;
    }
    
    return QString(); // 빈 문자열 반환
}

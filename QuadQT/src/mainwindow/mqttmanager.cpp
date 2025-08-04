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
    // SSL 인증서 로드
    loadCertificates();

    // config.ini에서 MQTT 설정 읽기
    QString configPath = findConfigFile();
    if (!configPath.isEmpty()) {
        QSettings settings(configPath, QSettings::IniFormat);
        brokerUrl = settings.value("mqtt/broker_url").toString();
        subscribeTopic = settings.value("mqtt/subscribe_topic").toString();
        publishTopic = settings.value("mqtt/publish_topic").toString();
        
        qDebug() << "[MQTT] 브로커 URL 로드:" << brokerUrl;
        
        // URL 파싱하여 호스트와 포트 추출
        QUrl url(brokerUrl);
        client.setHostname(url.host());
        client.setPort(url.port(1883)); // 기본 포트 1883
        
        // SSL 사용 여부 결정 (mqtts:// 또는 ssl:// 스키마인 경우)
        useSSL = (url.scheme().toLower() == "mqtts" || url.scheme().toLower() == "ssl");
        
        qDebug() << "[MQTT] 연결 설정 완료 - Host:" << url.host() << "Port:" << url.port(1883) << "SSL:" << useSSL;
    } else {
        qDebug() << "[MQTT] config.ini 파일을 찾을 수 없습니다.";
    }
    
    // MQTT 클라이언트 연결 성공 시 처리
    connect(&client, &QMqttClient::connected, this, [this]() {
        qDebug() << "[MQTT] 브로커 연결 성공";
        emit connected();
        
        // 구독 토픽 설정
        QMqttTopicFilter filter(subscribeTopic);
        auto sub = client.subscribe(filter, 1);
        if (sub)
            qDebug() << "[MQTT] 토픽 구독 성공:" << subscribeTopic;
        else
            qDebug() << "[MQTT] 토픽 구독 실패";
    });

    // MQTT 메시지 수신 처리
    connect(&client, &QMqttClient::messageReceived, this,
            [this](const QByteArray &msg, const QMqttTopicName &topic) {
                emit newMessage(topic.name(), msg);
            });

    // MQTT 에러 처리
    connect(&client, &QMqttClient::errorChanged, this, [this]() {
        qDebug() << "[MQTT] 연결 오류:" << client.error();
    });

    // MQTT 메시지 수신 시그널 처리
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
            qDebug() << "[TLS] CA 인증서 로드 성공:" << caPath;
        } else {
            qDebug() << "[TLS] CA 인증서 파일 열기 실패:" << caPath;
        }
    } else {
        qDebug() << "[TLS] CA 인증서 로드 실패";
    }

    // 클라이언트 인증서와 개인키 로드
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
            qDebug() << "[TLS] 클라이언트 인증서/키 로드 성공:" << certPath << "," << keyPath;
        } else {
            qDebug() << "[TLS] 클라이언트 인증서/키 파일 열기 실패";
        }
    } else {
        qDebug() << "[TLS] 클라이언트 인증서/키 로드 실패";
    }
}

void MqttManager::publish(const QString &topic, const QByteArray &payload)
{
    // 지정된 토픽으로 메시지 발행 (QoS 1, retain false)
    client.publish(topic, payload, 1, false);
}

void MqttManager::publish(const QByteArray &payload)
{
    // 기본 토픽으로 메시지 발행
    client.publish(publishTopic, payload, 1, false);
}

void MqttManager::connectToBroker()
{
    qDebug() << "[MQTT] 브로커 연결 시도:" << client.hostname() << ":" << client.port();
    
    if (useSSL) {
        qDebug() << "[MQTT] SSL 암호화 연결 시도";
        client.connectToHostEncrypted(sslConfig);
    } else {
        qDebug() << "[MQTT] 일반 연결 시도";
        client.connectToHost();
    }
}

QString MqttManager::findCertificateFile(const QString &filename)
{
    qDebug() << "[MQTT] 인증서 파일 검색:" << filename;
    
    // 1. 리소스 경로 우선 시도
    QString resourcePath = ":/certs/" + filename;
    qDebug() << "[MQTT] 리소스 경로 확인:" << resourcePath << "존재:" << QFile::exists(resourcePath);
    if (QFile::exists(resourcePath)) {
        qDebug() << "[MQTT] 리소스에서 인증서 파일 발견:" << resourcePath;
        return resourcePath;
    }
    
    // 2. 파일 시스템 경로들 시도
    QStringList searchPaths = {
        filename,                    // 원본 경로
        "./" + filename,            // 현재 디렉토리
        "../" + filename,           // 상위 디렉토리
        "../../" + filename,        // 상위의 상위 디렉토리
        "../../../" + filename,     // 상위의 상위의 상위 디렉토리
        "resources/certs/" + QFileInfo(filename).fileName(),  // resources/certs 디렉토리
        "../../../resources/certs/" + QFileInfo(filename).fileName(),  // 프로젝트 루트의 resources/certs
        "../../../../resources/certs/" + QFileInfo(filename).fileName(),  // 더 상위의 resources/certs
        QCoreApplication::applicationDirPath() + "/" + filename,  // 실행 파일 디렉토리
        QDir::currentPath() + "/" + filename  // 현재 작업 디렉토리
    };
    
    qDebug() << "[MQTT] 현재 작업 디렉토리:" << QDir::currentPath();
    
    for (const QString &path : searchPaths) {
        QFile file(path);
        qDebug() << "[MQTT] 검색 경로:" << path << "존재:" << file.exists();
        if (file.exists()) {
            qDebug() << "[MQTT] 인증서 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "[MQTT] 인증서 파일을 찾을 수 없습니다:" << filename;
    return QString(); // 빈 문자열 반환
}

QString MqttManager::findConfigFile()
{
    // config.ini 파일을 여러 경로에서 찾기
    QStringList searchPaths = {
        "config.ini",                    // 현재 디렉토리
        "./config.ini",                  // 명시적 현재 디렉토리
        "../config.ini",                 // 상위 디렉토리
        "../../config.ini",              // 상위의 상위 디렉토리
        QCoreApplication::applicationDirPath() + "/config.ini",  // 실행 파일 디렉토리
        QDir::currentPath() + "/config.ini"  // 현재 작업 디렉토리
    };
    
    qDebug() << "[MQTT] config.ini 파일 검색 중...";
    qDebug() << "[MQTT] 현재 작업 디렉토리:" << QDir::currentPath();
    qDebug() << "[MQTT] 실행 파일 디렉토리:" << QCoreApplication::applicationDirPath();
    
    for (const QString &path : searchPaths) {
        QFile file(path);
        qDebug() << "[MQTT] 검색 경로:" << path << "존재:" << file.exists();
        if (file.exists()) {
            qDebug() << "[MQTT] config.ini 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "[MQTT] config.ini 파일을 찾을 수 없습니다";
    return QString(); // 빈 문자열 반환
}

#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <QObject>
#include <QMqttClient>
#include <QSslConfiguration>

class MqttManager : public QObject
{
    Q_OBJECT
public:
    explicit MqttManager(QObject *parent = nullptr);
    void connectToBroker();
    void publish(const QString &topic, const QByteArray &payload);
    void publish(const QByteArray &payload); // 기본 토픽 사용

signals:
    void newMessage(const QString &topic, const QByteArray &payload);
    void connected();
    void messageReceived(const QByteArray &message);

private:
    QMqttClient client;
    QSslConfiguration sslConfig;
    QString subscribeTopic;
    QString publishTopic;
    void loadCertificates();
};

#endif // MQTTMANAGER_H

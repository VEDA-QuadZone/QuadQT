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

signals:
    void newMessage(const QString &topic, const QByteArray &payload);
    void connected();

private:
    QMqttClient client;
    QSslConfiguration sslConfig;
    void loadCertificates();
};

#endif // MQTTMANAGER_H

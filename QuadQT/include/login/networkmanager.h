#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslError>
#include <QSslCipher>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    void connectToServer();
    void connectToServerSSL();
    void disconnectFromServer();
    bool isConnected() const;
    
    // SSL 설정 메서드
    bool setupSSLConfiguration();
    void setSSLEnabled(bool enabled) { m_sslEnabled = enabled; }

    void registerUser(const QString &email, const QString &password);
    void loginUser(const QString &email, const QString &password);
    void resetPassword(const QString &email, const QString &newPassword);
    
    // History API
    void getHistory(const QString &email, int limit = 10, int offset = 0);
    void getHistoryByEventType(const QString &email, int eventType, int limit = 10, int offset = 0);
    void getHistoryByDateRange(const QString &email, const QString &startDate, const QString &endDate, int limit = 10, int offset = 0);
    void getHistoryByEventTypeAndDateRange(const QString &email, int eventType, const QString &startDate, const QString &endDate, int limit = 10, int offset = 0);
    void addHistory(const QString &date, const QString &imagePath, const QString &plateNumber, int eventType);
    
    // Frame API
    void changeFrame(int menuNum, const QString &value);
    void getFrame();
    
    // Log API
    void getLog();

signals:
    void connected();
    void disconnected();
    void registerResponse(const QJsonObject &response);
    void loginResponse(const QJsonObject &response);
    void resetPasswordResponse(const QJsonObject &response);
    void networkError(const QString &error);
    
    // History API signals
    void historyResponse(const QJsonObject &response);
    void historyByEventTypeResponse(const QJsonObject &response);
    void historyByDateRangeResponse(const QJsonObject &response);
    void historyByEventTypeAndDateRangeResponse(const QJsonObject &response);
    void addHistoryResponse(const QJsonObject &response);
    
    // Frame API signals
    void changeFrameResponse(const QJsonObject &response);
    void getFrameResponse(const QJsonObject &response);
    
    // Log API signals
    void getLogResponse(const QJsonObject &response);
public slots:
    void sendCommand(const QString &command);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onTimeout();
    void onSslErrors(const QList<QSslError> &errors);
    void onEncrypted();

private:
    void loadConfig();
    QJsonObject parseResponse(const QString &response);
    QString findCertificateFile(const QString &filename);
    QString findConfigFile();

    QSslSocket *m_socket;
    QTimer *m_timeoutTimer;
    QString m_serverIp;
    int m_serverPort;
    int m_timeout;
    QString m_pendingCommand;
    QString m_responseBuffer;
    
    // SSL 관련 멤버 변수
    bool m_sslEnabled;
    QString m_caCertPath;
    QString m_clientCertPath;
    QString m_clientKeyPath;
    QSslConfiguration m_sslConfig;
};

#endif // NETWORKMANAGER_H

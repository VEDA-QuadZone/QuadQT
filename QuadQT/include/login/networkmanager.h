#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QSettings>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    void connectToServer();
    void disconnectFromServer();
    bool isConnected() const;

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

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onTimeout();

private:
    void loadConfig();
    void sendCommand(const QString &command);
    QJsonObject parseResponse(const QString &response);

    QTcpSocket *m_socket;
    QTimer *m_timeoutTimer;
    QString m_serverIp;
    int m_serverPort;
    int m_timeout;
    QString m_pendingCommand;
    QString m_responseBuffer;
};

#endif // NETWORKMANAGER_H
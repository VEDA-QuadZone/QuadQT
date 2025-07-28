#include "networkmanager.h"
#include <QDebug>
#include <QJsonParseError>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_timeoutTimer(new QTimer(this))
    , m_serverPort(8080)
    , m_timeout(5000)
{
    loadConfig();
    
    // 소켓 시그널 연결
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &NetworkManager::onError);
    
    // 타임아웃 타이머 설정
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &NetworkManager::onTimeout);
}

NetworkManager::~NetworkManager()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void NetworkManager::loadConfig()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    
    m_serverIp = settings.value("Server/ip", "127.0.0.1").toString();
    m_serverPort = settings.value("Server/port", 8080).toInt();
    m_timeout = settings.value("Server/timeout", 5000).toInt();
    
    qDebug() << "설정 로드됨 - IP:" << m_serverIp << "포트:" << m_serverPort << "타임아웃:" << m_timeout;
}

void NetworkManager::connectToServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        qDebug() << "이미 연결되었거나 연결 중";
        return;
    }
    
    qDebug() << "서버에 연결 중:" << m_serverIp << ":" << m_serverPort;
    m_socket->connectToHost(m_serverIp, m_serverPort);
    
    // 연결 타임아웃 설정
    m_timeoutTimer->start(m_timeout);
}

void NetworkManager::disconnectFromServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool NetworkManager::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkManager::registerUser(const QString &email, const QString &password)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("REGISTER %1 %2").arg(email, password);
    m_pendingCommand = "REGISTER";
    sendCommand(command);
}

void NetworkManager::loginUser(const QString &email, const QString &password)
{
    if (!isConnected()) {
        qDebug() << "❌ 로그인 요청을 보낼 수 없음: 서버에 연결되지 않음";
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("LOGIN %1 %2").arg(email, password);
    m_pendingCommand = "LOGIN";
    
    qDebug() << "🔐 TCP를 통해 LOGIN 명령어 전송:" << command;
    qDebug() << "⏳ 서버 응답 대기 중...";
    
    sendCommand(command);
}

void NetworkManager::resetPassword(const QString &email, const QString &newPassword)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("RESET_PASSWORD %1 %2").arg(email, newPassword);
    m_pendingCommand = "RESET_PASSWORD";
    sendCommand(command);
}

void NetworkManager::onConnected()
{
    m_timeoutTimer->stop();
    qDebug() << "서버에 연결됨";
    emit connected();
}

void NetworkManager::onDisconnected()
{
    m_timeoutTimer->stop();
    qDebug() << "서버와의 연결 끊김";
    emit disconnected();
}

void NetworkManager::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    m_responseBuffer.append(QString::fromUtf8(data));
    
    // 🔍 TCP 데이터 수신 디버그
    qDebug() << "📡 TCP 데이터 수신:" << QString::fromUtf8(data);
    qDebug() << "📦 현재 버퍼:" << m_responseBuffer;
    qDebug() << "⏳ 대기 중인 명령어:" << m_pendingCommand;
    
    // JSON 응답이 완전한지 확인 (간단한 방법으로 } 로 끝나는지 확인)
    if (m_responseBuffer.contains('}')) {
        QJsonObject response = parseResponse(m_responseBuffer);
        
        if (!response.isEmpty()) {
            // 명령어에 따라 적절한 시그널 발생
            if (m_pendingCommand == "REGISTER") {
                emit registerResponse(response);
            } else if (m_pendingCommand == "LOGIN") {
                qDebug() << "🔐 LOGIN 응답 처리 중, loginResponse 시그널 발생";
                emit loginResponse(response);
            } else if (m_pendingCommand == "RESET_PASSWORD") {
                emit resetPasswordResponse(response);
            } else if (m_pendingCommand == "GET_HISTORY") {
                emit historyResponse(response);
            } else if (m_pendingCommand == "GET_HISTORY_BY_EVENT_TYPE") {
                emit historyByEventTypeResponse(response);
            } else if (m_pendingCommand == "GET_HISTORY_BY_DATE_RANGE") {
                emit historyByDateRangeResponse(response);
            } else if (m_pendingCommand == "GET_HISTORY_BY_EVENT_TYPE_AND_DATE_RANGE") {
                emit historyByEventTypeAndDateRangeResponse(response);
            } else if (m_pendingCommand == "ADD_HISTORY") {
                emit addHistoryResponse(response);
            } else if (m_pendingCommand == "CHANGE_FRAME") {
                emit changeFrameResponse(response);
            } else if (m_pendingCommand == "GET_FRAME") {
                emit getFrameResponse(response);
            } else if (m_pendingCommand == "GET_LOG") {
                emit getLogResponse(response);
            }
        }
        
        m_responseBuffer.clear();
        m_pendingCommand.clear();
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError error)
{
    m_timeoutTimer->stop();
    QString errorString;
    
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        errorString = "서버 연결이 거부되었습니다.";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorString = "서버가 연결을 종료했습니다.";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorString = "서버를 찾을 수 없습니다.";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorString = "연결 시간이 초과되었습니다.";
        break;
    default:
        errorString = QString("네트워크 오류: %1").arg(m_socket->errorString());
        break;
    }
    
    qDebug() << "소켓 오류:" << errorString;
    emit networkError(errorString);
}

void NetworkManager::onTimeout()
{
    if (m_socket->state() == QAbstractSocket::ConnectingState) {
        m_socket->abort();
        emit networkError("서버 연결 시간이 초과되었습니다.");
    }
}

void NetworkManager::sendCommand(const QString &command)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "명령어 전송:" << command;
        m_socket->write(command.toUtf8() + "\n");
        m_socket->flush();
    }
}

QJsonObject NetworkManager::parseResponse(const QString &response)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON 파싱 오류:" << error.errorString();
        return QJsonObject();
    }
    
    return doc.object();
}

// History API implementations
void NetworkManager::getHistory(const QString &email, int limit, int offset)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("GET_HISTORY %1 %2 %3").arg(email).arg(limit).arg(offset);
    m_pendingCommand = "GET_HISTORY";
    sendCommand(command);
}

void NetworkManager::getHistoryByEventType(const QString &email, int eventType, int limit, int offset)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("GET_HISTORY_BY_EVENT_TYPE %1 %2 %3 %4").arg(email).arg(eventType).arg(limit).arg(offset);
    m_pendingCommand = "GET_HISTORY_BY_EVENT_TYPE";
    sendCommand(command);
}

void NetworkManager::getHistoryByDateRange(const QString &email, const QString &startDate, const QString &endDate, int limit, int offset)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("GET_HISTORY_BY_DATE_RANGE %1 %2 %3 %4 %5").arg(email, startDate, endDate).arg(limit).arg(offset);
    m_pendingCommand = "GET_HISTORY_BY_DATE_RANGE";
    sendCommand(command);
}

void NetworkManager::getHistoryByEventTypeAndDateRange(const QString &email, int eventType, const QString &startDate, const QString &endDate, int limit, int offset)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("GET_HISTORY_BY_EVENT_TYPE_AND_DATE_RANGE %1 %2 %3 %4 %5 %6").arg(email).arg(eventType).arg(startDate, endDate).arg(limit).arg(offset);
    m_pendingCommand = "GET_HISTORY_BY_EVENT_TYPE_AND_DATE_RANGE";
    sendCommand(command);
}

void NetworkManager::addHistory(const QString &date, const QString &imagePath, const QString &plateNumber, int eventType)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("ADD_HISTORY %1 %2 %3 %4").arg(date, imagePath, plateNumber).arg(eventType);
    m_pendingCommand = "ADD_HISTORY";
    sendCommand(command);
}

// Frame API implementations
void NetworkManager::changeFrame(int menuNum, const QString &value)
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("CHANGE_FRAME %1 %2").arg(menuNum).arg(value);
    m_pendingCommand = "CHANGE_FRAME";
    sendCommand(command);
}

void NetworkManager::getFrame()
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = "GET_FRAME";
    m_pendingCommand = "GET_FRAME";
    sendCommand(command);
}

// Log API implementation
void NetworkManager::getLog()
{
    if (!isConnected()) {
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = "GET_LOG";
    m_pendingCommand = "GET_LOG";
    sendCommand(command);
}
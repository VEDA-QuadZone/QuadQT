#include "login/networkmanager.h"
#include <QDebug>
#include <QJsonParseError>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
    , m_timeoutTimer(new QTimer(this))
    , m_serverPort(8080)
    , m_timeout(5000)
    , m_sslEnabled(false)
    , m_caCertPath("ca.cert.pem")
    , m_clientCertPath("client.cert.pem")
    , m_clientKeyPath("client.key.pem")
{
    // 소켓 시그널 연결
    connect(m_socket, &QSslSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QSslSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QSslSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &NetworkManager::onError);
    
    // SSL 관련 시그널 연결
    connect(m_socket, &QSslSocket::encrypted, this, &NetworkManager::onEncrypted);
    connect(m_socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            this, &NetworkManager::onSslErrors);
    
    // 타임아웃 타이머 설정
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &NetworkManager::onTimeout);
    
    // SSL 지원 확인
    if (!QSslSocket::supportsSsl()) {
        qDebug() << "경고: 이 시스템에서 SSL이 지원되지 않습니다.";
        qDebug() << "SSL 라이브러리 버전:" << QSslSocket::sslLibraryVersionString();
        m_sslEnabled = false;
    } else {
        qDebug() << "SSL 지원 확인됨. 라이브러리 버전:" << QSslSocket::sslLibraryVersionString();
    }
    
    // 설정 로드 (SSL 지원 확인 후)
    loadConfig();
    
    // SSL이 활성화되어 있고 지원되는 경우에만 SSL 설정 초기화
    if (m_sslEnabled && QSslSocket::supportsSsl()) {
        setupSSLConfiguration();
    }
}

NetworkManager::~NetworkManager()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void NetworkManager::loadConfig()
{
    // config.ini 파일 경로 찾기
    QString configPath = findConfigFile();
    if (configPath.isEmpty()) {
        qDebug() << "config.ini 파일을 찾을 수 없습니다. 기본 설정을 사용합니다.";
        return;
    }
    
    QSettings settings(configPath, QSettings::IniFormat);
    qDebug() << "설정 파일 로드:" << configPath;
    
    // 설정 파일 상태 확인
    if (settings.status() != QSettings::NoError) {
        qDebug() << "설정 파일 읽기 오류:" << settings.status();
        return;
    }
    
    m_serverIp = settings.value("TCP/ip", "127.0.0.1").toString();
    m_serverPort = settings.value("TCP/port", 8080).toInt();
    m_timeout = settings.value("TCP/timeout", 5000).toInt();
    
    // SSL 설정 로드
    m_sslEnabled = settings.value("SSL/enabled", false).toBool();
    m_caCertPath = settings.value("SSL/ca_cert", "ca.cert.pem").toString();
    m_clientCertPath = settings.value("SSL/client_cert", "client.cert.pem").toString();
    m_clientKeyPath = settings.value("SSL/client_key", "client.key.pem").toString();
    

    
    qDebug() << "설정 로드됨 - IP:" << m_serverIp << "포트:" << m_serverPort << "타임아웃:" << m_timeout;
    qDebug() << "SSL 활성화:" << m_sslEnabled << "CA 인증서:" << m_caCertPath;
    
    // 디버깅: 모든 키 출력
    qDebug() << "설정 파일의 모든 키:";
    for (const QString &key : settings.allKeys()) {
        qDebug() << "  " << key << "=" << settings.value(key);
    }
}

void NetworkManager::connectToServer()
{
    if (m_sslEnabled) {
        connectToServerSSL();
    } else {
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            qDebug() << "이미 연결되었거나 연결 중";
            return;
        }
        
        qDebug() << "서버에 연결 중 (비암호화):" << m_serverIp << ":" << m_serverPort;
        m_socket->connectToHost(m_serverIp, m_serverPort);
        
        // 연결 타임아웃 설정
        m_timeoutTimer->start(m_timeout);
    }
}

void NetworkManager::connectToServerSSL()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        qDebug() << "Already connected or connecting";
        return;
    }
    
    if (!setupSSLConfiguration()) {
        emit networkError("SSL 설정 실패");
        return;
    }
    
    qDebug() << "서버에 SSL 연결 중:" << m_serverIp << ":" << m_serverPort;
    m_socket->setSslConfiguration(m_sslConfig);
    m_socket->connectToHostEncrypted(m_serverIp, m_serverPort);
    
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
    if (m_sslEnabled) {
        return m_socket->state() == QAbstractSocket::ConnectedState && 
               m_socket->isEncrypted();
    } else {
        return m_socket->state() == QAbstractSocket::ConnectedState;
    }
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
        qDebug() << "로그인 요청을 보낼 수 없음: 서버에 연결되지 않음";
        emit networkError("서버에 연결되지 않았습니다.");
        return;
    }
    
    QString command = QString("LOGIN %1 %2").arg(email, password);
    m_pendingCommand = "LOGIN";
    
    qDebug() << "TCP를 통해 LOGIN 명령어 전송:" << command;
    qDebug() << "서버 응답 대기 중...";
    
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
    qDebug() << "Connected to server";
    emit connected();
}

void NetworkManager::onDisconnected()
{
    m_timeoutTimer->stop();
    qDebug() << "Disconnected from server";
    emit disconnected();
}

void NetworkManager::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    m_responseBuffer.append(QString::fromUtf8(data));
    
    // TCP 데이터 수신 디버그
    qDebug() << "TCP 데이터 수신:" << QString::fromUtf8(data);
    qDebug() << "현재 버퍼:" << m_responseBuffer;
    qDebug() << "대기 중인 명령어:" << m_pendingCommand;
    
    // JSON 응답이 완전한지 확인 (간단한 방법으로 } 로 끝나는지 확인)
    if (m_responseBuffer.contains('}')) {
        QJsonObject response = parseResponse(m_responseBuffer);
        
        if (!response.isEmpty()) {
            // 명령어에 따라 적절한 시그널 발생
            if (m_pendingCommand == "REGISTER") {
                emit registerResponse(response);
            } else if (m_pendingCommand == "LOGIN") {
                qDebug() << "LOGIN 응답 처리 중, loginResponse 시그널 발생";
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
    
    qDebug() << "Socket error:" << errorString;
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
        qDebug() << "Sending command:" << command;
        m_socket->write(command.toUtf8() + "\n");
        m_socket->flush();
    }
}

QJsonObject NetworkManager::parseResponse(const QString &response)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString();
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

bool NetworkManager::setupSSLConfiguration()
{
    if (!QSslSocket::supportsSsl()) {
        qDebug() << "SSL이 지원되지 않아 SSL 설정을 건너뜁니다.";
        return false;
    }
    
    // CA 인증서 로드 (여러 경로 시도)
    QString caCertPath = findCertificateFile(m_caCertPath);
    if (caCertPath.isEmpty()) {
        qDebug() << "CA 인증서 파일을 찾을 수 없습니다:" << m_caCertPath;
        return false;
    }
    
    QFile caCertFile(caCertPath);
    if (!caCertFile.open(QIODevice::ReadOnly)) {
        qDebug() << "CA 인증서 파일을 열 수 없습니다:" << caCertPath;
        return false;
    }
    
    QSslCertificate caCert(&caCertFile, QSsl::Pem);
    caCertFile.close();
    
    if (caCert.isNull()) {
        qDebug() << "CA 인증서가 유효하지 않습니다";
        return false;
    }
    
    // 클라이언트 인증서 로드
    QString clientCertPath = findCertificateFile(m_clientCertPath);
    if (clientCertPath.isEmpty()) {
        qDebug() << "클라이언트 인증서 파일을 찾을 수 없습니다:" << m_clientCertPath;
        return false;
    }
    
    QFile clientCertFile(clientCertPath);
    if (!clientCertFile.open(QIODevice::ReadOnly)) {
        qDebug() << "클라이언트 인증서 파일을 열 수 없습니다:" << clientCertPath;
        return false;
    }
    
    QSslCertificate clientCert(&clientCertFile, QSsl::Pem);
    clientCertFile.close();
    
    if (clientCert.isNull()) {
        qDebug() << "클라이언트 인증서가 유효하지 않습니다";
        return false;
    }
    
    // 클라이언트 개인키 로드
    QString clientKeyPath = findCertificateFile(m_clientKeyPath);
    if (clientKeyPath.isEmpty()) {
        qDebug() << "클라이언트 개인키 파일을 찾을 수 없습니다:" << m_clientKeyPath;
        return false;
    }
    
    QFile clientKeyFile(clientKeyPath);
    if (!clientKeyFile.open(QIODevice::ReadOnly)) {
        qDebug() << "클라이언트 개인키 파일을 열 수 없습니다:" << clientKeyPath;
        return false;
    }
    
    QSslKey clientKey(&clientKeyFile, QSsl::Rsa, QSsl::Pem);
    clientKeyFile.close();
    
    if (clientKey.isNull()) {
        qDebug() << "클라이언트 개인키가 유효하지 않습니다";
        return false;
    }
    
    // SSL 설정 구성 (정상적인 SSL 검증)
    m_sslConfig = QSslConfiguration::defaultConfiguration();
    m_sslConfig.setLocalCertificate(clientCert);
    m_sslConfig.setPrivateKey(clientKey);
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
    m_sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    
    // CA 인증서 설정
    m_sslConfig.setCaCertificates(QList<QSslCertificate>() << caCert);
    
    qDebug() << "SSL 엄격 검증 활성화";
    
    qDebug() << "SSL 설정 완료";
    qDebug() << "CA 인증서 주체:" << caCert.subjectInfo(QSslCertificate::CommonName);
    qDebug() << "클라이언트 인증서 주체:" << clientCert.subjectInfo(QSslCertificate::CommonName);
    
    return true;
}

void NetworkManager::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "SSL 오류 발생:";
    for (const QSslError &error : errors) {
        qDebug() << "  -" << error.errorString();
        qDebug() << "  - 오류 코드:" << error.error();
    }
    
    // 호스트명 불일치 오류만 무시하고 다른 SSL 오류는 거부
    QList<QSslError> ignorableErrors;
    bool hasOnlyHostnameError = true;
    
    for (const QSslError &error : errors) {
        if (error.error() == QSslError::HostNameMismatch) {
            ignorableErrors.append(error);
            qDebug() << "호스트명 불일치 오류를 무시합니다:" << error.errorString();
        } else {
            hasOnlyHostnameError = false;
            qDebug() << "심각한 SSL 오류 발견:" << error.errorString();
        }
    }
    
    if (hasOnlyHostnameError && !ignorableErrors.isEmpty()) {
        qDebug() << "호스트명 불일치 오류만 발생했으므로 연결을 허용합니다.";
        m_socket->ignoreSslErrors(ignorableErrors);
    } else {
        qDebug() << "심각한 SSL 오류로 인해 연결을 거부합니다.";
        emit networkError("SSL 인증 실패");
    }
}

void NetworkManager::onEncrypted()
{
    qDebug() << "SSL 연결이 암호화되었습니다";
    
    // SSL 세션 정보 출력 (안전한 방식)
    QSslCipher cipher = m_socket->sessionCipher();
    if (!cipher.isNull()) {
        qDebug() << "사용된 암호화 방식:" << cipher.name();
        qDebug() << "프로토콜:" << cipher.protocolString();
    }
    
    // 서버 인증서 정보 출력
    QSslCertificate cert = m_socket->peerCertificate();
    if (!cert.isNull()) {
        qDebug() << "서버 인증서 주체:" << cert.subjectInfo(QSslCertificate::CommonName);
    }
}

QString NetworkManager::findCertificateFile(const QString &filename)
{
    // 여러 경로에서 인증서 파일을 찾아봅니다
    QStringList searchPaths = {
        filename,                    // 원본 경로
        "./" + filename,            // 현재 디렉토리
        "../" + filename,           // 상위 디렉토리
        "../../" + filename,        // 상위의 상위 디렉토리
        QCoreApplication::applicationDirPath() + "/" + filename,  // 실행 파일 디렉토리
        QDir::currentPath() + "/" + filename  // 현재 작업 디렉토리
    };
    
    for (const QString &path : searchPaths) {
        QFile file(path);
        if (file.exists()) {
            qDebug() << "인증서 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "다음 경로들에서 인증서 파일을 찾을 수 없습니다:";
    for (const QString &path : searchPaths) {
        qDebug() << "  -" << path;
    }
    
    return QString(); // 빈 문자열 반환
}

QString NetworkManager::findConfigFile()
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
            qDebug() << "설정 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "다음 경로들에서 config.ini 파일을 찾을 수 없습니다:";
    for (const QString &path : searchPaths) {
        qDebug() << "  -" << path;
    }
    
    return QString(); // 빈 문자열 반환
}

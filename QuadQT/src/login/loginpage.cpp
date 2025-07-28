#include "login/loginpage.h"
#include "ui_loginpage.h"
#include <QRegularExpression>
#include <QDebug>

LoginPage::LoginPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginPage),
    m_networkManager(new NetworkManager(this))
{
    ui->setupUi(this);

    // 윈도우 크기 설정
    setFixedSize(800, 600);

    // 기본적으로 Sign In 페이지 표시
    ui->stackedWidget->setCurrentIndex(0);

    // 연결 설정
    setupConnections();

    // 서버 연결 시도
    m_networkManager->connectToServer();
}

LoginPage::~LoginPage()
{
    delete ui;
}

void LoginPage::setupConnections()
{
    // 페이지 네비게이션 버튼
    connect(ui->pushButton_GoToRegister, &QPushButton::clicked, this, &LoginPage::showRegisterPage);
    connect(ui->pushButton_BackToSignIn, &QPushButton::clicked, this, &LoginPage::showSignInPage);
    connect(ui->pushButton_ForgotPassword, &QPushButton::clicked, this, &LoginPage::showResetPasswordPage);
    connect(ui->pushButton_Cancel, &QPushButton::clicked, this, &LoginPage::showSignInPage);

    // 액션 버튼들
    connect(ui->pushButton_SignIn, &QPushButton::clicked, this, &LoginPage::onSignInClicked);
    connect(ui->pushButton_Register, &QPushButton::clicked, this, &LoginPage::onRegisterClicked);
    connect(ui->pushButton_ResetPassword, &QPushButton::clicked, this, &LoginPage::onResetPasswordClicked);

    // 네트워크 매니저 시그널
    connect(m_networkManager, &NetworkManager::connected, this, &LoginPage::onNetworkConnected);
    connect(m_networkManager, &NetworkManager::disconnected, this, &LoginPage::onNetworkDisconnected);
    connect(m_networkManager, &NetworkManager::loginResponse, this, &LoginPage::onLoginResponse);
    connect(m_networkManager, &NetworkManager::registerResponse, this, &LoginPage::onRegisterResponse);
    connect(m_networkManager, &NetworkManager::resetPasswordResponse, this, &LoginPage::onResetPasswordResponse);
    connect(m_networkManager, &NetworkManager::networkError, this, &LoginPage::onNetworkError);
}

void LoginPage::showSignInPage()
{
    ui->stackedWidget->setCurrentIndex(0);
    clearInputFields();
}

void LoginPage::showRegisterPage()
{
    ui->stackedWidget->setCurrentIndex(1);
    clearInputFields();
}

void LoginPage::showResetPasswordPage()
{
    ui->stackedWidget->setCurrentIndex(2);
    clearInputFields();
}

void LoginPage::onSignInClicked()
{
    QString email = ui->lineEdit_SignInEmail->text().trimmed();
    QString password = ui->lineEdit_SignInPassword->text();

    qDebug() << "🔐 로그인 시도 시작";
    qDebug() << "📧 이메일:" << email;
    qDebug() << "🔒 비밀번호 길이:" << password.length();

    if (email.isEmpty() || password.isEmpty()) {
        qDebug() << "❌ 로그인 실패: 이메일 또는 비밀번호가 비어있음";
        showMessage("입력 오류", "이메일과 비밀번호를 모두 입력해주세요.", QMessageBox::Warning);
        return;
    }

    if (!validateEmail(email)) {
        qDebug() << "❌ 로그인 실패: 잘못된 이메일 형식";
        showMessage("입력 오류", "올바른 이메일 형식을 입력해주세요.", QMessageBox::Warning);
        return;
    }

    if (!m_networkManager->isConnected()) {
        qDebug() << "❌ 로그인 실패: 서버에 연결되지 않음";
        showMessage("연결 오류", "서버에 연결되지 않았습니다. 잠시 후 다시 시도해주세요.", QMessageBox::Critical);
        m_networkManager->connectToServer();
        return;
    }

    qDebug() << "📡 TCP를 통해 서버로 로그인 요청 전송 중...";
    m_networkManager->loginUser(email, password);
}

void LoginPage::onRegisterClicked()
{
    QString email = ui->lineEdit_RegisterEmail->text().trimmed();
    QString password = ui->lineEdit_RegisterPassword->text();

    if (email.isEmpty() || password.isEmpty()) {
        showMessage("입력 오류", "이메일과 비밀번호를 모두 입력해주세요.", QMessageBox::Warning);
        return;
    }

    if (!validateEmail(email)) {
        showMessage("입력 오류", "올바른 이메일 형식을 입력해주세요.", QMessageBox::Warning);
        return;
    }

    if (!m_networkManager->isConnected()) {
        showMessage("연결 오류", "서버에 연결되지 않았습니다. 잠시 후 다시 시도해주세요.", QMessageBox::Critical);
        m_networkManager->connectToServer();
        return;
    }

    m_networkManager->registerUser(email, password);
}

void LoginPage::onResetPasswordClicked()
{
    QString email = ui->lineEdit_ResetEmail->text().trimmed();
    QString newPassword = ui->lineEdit_NewPassword->text();

    if (email.isEmpty() || newPassword.isEmpty()) {
        showMessage("입력 오류", "이메일과 새 비밀번호를 모두 입력해주세요.", QMessageBox::Warning);
        return;
    }

    if (!validateEmail(email)) {
        showMessage("입력 오류", "올바른 이메일 형식을 입력해주세요.", QMessageBox::Warning);
        return;
    }

    if (!m_networkManager->isConnected()) {
        showMessage("연결 오류", "서버에 연결되지 않았습니다. 잠시 후 다시 시도해주세요.", QMessageBox::Critical);
        m_networkManager->connectToServer();
        return;
    }

    m_networkManager->resetPassword(email, newPassword);
}

void LoginPage::onNetworkConnected()
{
    qDebug() << "네트워크 연결됨";
    // 연결 상태 UI 업데이트 (필요시)
}

void LoginPage::onNetworkDisconnected()
{
    qDebug() << "네트워크 연결 끊김";
    showMessage("연결 끊김", "서버와의 연결이 끊어졌습니다.", QMessageBox::Warning);
}

void LoginPage::onLoginResponse(const QJsonObject &response)
{
    QString status = response["status"].toString();
    int code = response["code"].toInt();
    QString message = response["message"].toString();

    // 🔍 TCP 응답 디버그 출력
    qDebug() << "=== TCP 로그인 응답 ===";
    qDebug() << "상태:" << status;
    qDebug() << "코드:" << code;
    qDebug() << "메시지:" << message;
    qDebug() << "전체 응답:" << response;
    qDebug() << "====================";

    if (status == "success" && code == 200) {
        // 🎉 로그인 성공 디버그 메시지
        qDebug() << "🎉 로그인 성공! TCP에서 'login success' 메시지 수신";
        qDebug() << "✅ 사용자 인증 성공";
        qDebug() << "🚀 메인 윈도우로 전환 중...";
        
        showMessage("로그인 성공", "로그인에 성공했습니다!", QMessageBox::Information);

        // ✅ 시그널-슬롯 연결 해제
        disconnect(m_networkManager, nullptr, this, nullptr);

        emit loginSuccessful();
        accept();  // 다이얼로그 닫기 (MainWindow에서는 exec() 상태 종료)
    } else {
        // ❌ 로그인 실패 디버그 메시지
        qDebug() << "❌ 로그인 실패! TCP 응답이 실패를 나타냄";
        qDebug() << "오류 코드:" << code << "오류 메시지:" << message;
        
        QString errorMsg;
        switch (code) {
        case 404: errorMsg = "사용자를 찾을 수 없습니다."; break;
        case 401: errorMsg = "이메일 또는 비밀번호가 올바르지 않습니다."; break;
        default: errorMsg = message.isEmpty() ? "로그인에 실패했습니다." : message; break;
        }
        showMessage("로그인 실패", errorMsg, QMessageBox::Critical);
    }
}



void LoginPage::onRegisterResponse(const QJsonObject &response)
{
    QString status = response["status"].toString();
    int code = response["code"].toInt();
    QString message = response["message"].toString();

    if (status == "success" && code == 200) {
        showMessage("회원가입 성공", "회원가입이 완료되었습니다!", QMessageBox::Information);
        showSignInPage();
    } else {
        QString errorMsg;
        switch (code) {
        case 409:
            errorMsg = "이미 존재하는 이메일입니다.";
            break;
        case 400:
            errorMsg = "이메일 또는 비밀번호 형식이 올바르지 않습니다.";
            break;
        default:
            errorMsg = message.isEmpty() ? "회원가입에 실패했습니다." : message;
            break;
        }
        showMessage("회원가입 실패", errorMsg, QMessageBox::Critical);
    }
}

void LoginPage::onResetPasswordResponse(const QJsonObject &response)
{
    QString status = response["status"].toString();
    int code = response["code"].toInt();
    QString message = response["message"].toString();

    if (status == "success" && code == 200) {
        showMessage("비밀번호 재설정 성공", "비밀번호가 성공적으로 변경되었습니다!", QMessageBox::Information);
        showSignInPage();
    } else {
        QString errorMsg;
        switch (code) {
        case 404:
            errorMsg = "사용자를 찾을 수 없습니다.";
            break;
        default:
            errorMsg = message.isEmpty() ? "비밀번호 재설정에 실패했습니다." : message;
            break;
        }
        showMessage("비밀번호 재설정 실패", errorMsg, QMessageBox::Critical);
    }
}

void LoginPage::onNetworkError(const QString &error)
{
    showMessage("네트워크 오류", error, QMessageBox::Critical);
}

void LoginPage::showMessage(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.exec();
}

bool LoginPage::validateEmail(const QString &email)
{
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return emailRegex.match(email).hasMatch();
}

bool LoginPage::validatePassword(const QString &password)
{
    return password.length() >= 6;
}

void LoginPage::clearInputFields()
{
    // Sign In 페이지
    ui->lineEdit_SignInEmail->clear();
    ui->lineEdit_SignInPassword->clear();

    // Register 페이지
    ui->lineEdit_RegisterEmail->clear();
    ui->lineEdit_RegisterPassword->clear();

    // Reset Password 페이지
    ui->lineEdit_ResetEmail->clear();
    ui->lineEdit_NewPassword->clear();
}

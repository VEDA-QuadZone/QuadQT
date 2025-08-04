#include "login/loginpage.h"
#include "login/custommessagebox.h"
#include "mainwindow/overlaywidget.h"
#include "ui_loginpage.h"
#include <QRegularExpression>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

LoginPage::LoginPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginPage),
    m_networkManager(new NetworkManager(this)),
    overlay_(nullptr)
{
    ui->setupUi(this);


    // 폰트 설정
    setupFonts();

    // 윈도우 크기 설정
    setFixedSize(800, 600);

    // 기본적으로 Sign In 페이지 표시
    ui->stackedWidget->setCurrentIndex(0);

    ui->pushButton_SignIn->setDefault(true);

    // 연결 설정
    setupConnections();

    // 서버 연결 시도
    m_networkManager->connectToServer();
}

LoginPage::~LoginPage()
{
    delete ui;
}

void LoginPage::setupFonts()
{
    // 한화 폰트 패밀리 찾기
    QStringList allFamilies = QFontDatabase().families();
    QString hanwhaBFamily;      // HanwhaB 폰트용 (로고)
    QString hanwhaGothicFamily; // HanwhaGothicR 폰트용 (일반 텍스트)
    
    // HanwhaB 폰트 찾기 (QuadZone 로고용)
    for (const QString &family : allFamilies) {
        if (family.contains("Hanwha", Qt::CaseInsensitive) && 
            (family.contains("B", Qt::CaseInsensitive) || family.contains("Bold", Qt::CaseInsensitive)) &&
            !family.contains("Gothic", Qt::CaseInsensitive)) {
            hanwhaBFamily = family;
            break;
        }
    }
    
    // HanwhaGothicR 폰트 찾기 (일반 텍스트용)
    for (const QString &family : allFamilies) {
        if (family.contains("Hanwha", Qt::CaseInsensitive) && family.contains("Gothic", Qt::CaseInsensitive)) {
            hanwhaGothicFamily = family;
            break;
        }
    }
    
    if (hanwhaGothicFamily.isEmpty()) {
        // Gothic이 없으면 일반 Hanwha 폰트 찾기
        for (const QString &family : allFamilies) {
            if (family.contains("Hanwha", Qt::CaseInsensitive)) {
                hanwhaGothicFamily = family;
                break;
            }
        }
    }
    
    // QuadZone 로고에 HanwhaB 폰트 적용
    QFont logoFont(hanwhaBFamily.isEmpty() ? "Arial" : hanwhaBFamily, 32);
    logoFont.setBold(true);
    ui->label_QuadZone->setFont(logoFont);
    
    // 기본 폰트 설정 (일반 텍스트)
    QFont defaultFont(hanwhaGothicFamily.isEmpty() ? "Malgun Gothic" : hanwhaGothicFamily, 12);
    defaultFont.setBold(false);
    
    // 버튼용 볼드 폰트 설정
    QFont buttonFont(hanwhaGothicFamily.isEmpty() ? "Malgun Gothic" : hanwhaGothicFamily, 12);
    buttonFont.setBold(true);
    
    // 링크 버튼용 일반 폰트 설정
    QFont linkFont(hanwhaGothicFamily.isEmpty() ? "Malgun Gothic" : hanwhaGothicFamily, 12);
    linkFont.setBold(false);
    
    // 모든 라벨에 일반 폰트 적용
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label != ui->label_QuadZone) { // QuadZone 로고는 제외
            label->setFont(defaultFont);
        }
    }
    
    // 버튼별로 폰트 적용 구분
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        // 포커스 아웃라인 제거
        button->setFocusPolicy(Qt::NoFocus);
        
        // 링크 버튼들은 일반 폰트 적용
        if (button == ui->pushButton_ForgotPassword ||
            button == ui->pushButton_GoToRegister ||
            button == ui->pushButton_BackToSignIn ||
            button == ui->pushButton_Cancel) {
            button->setFont(linkFont);
        } else {
            // 주요 액션 버튼들은 볼드 폰트 적용
            button->setFont(buttonFont);
        }
    }
    
    // 모든 입력 필드에 일반 폰트 적용
    QList<QLineEdit*> lineEdits = findChildren<QLineEdit*>();
    for (QLineEdit* lineEdit : lineEdits) {
        lineEdit->setFont(defaultFont);
    }
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

    // 입력 값 검증
    if (email.isEmpty() || password.isEmpty()) {
        showMessage("입력 오류", "이메일과 비밀번호를 모두 입력해주세요.", QMessageBox::Warning);
        return;
    }

    if (!validateEmail(email)) {
        showMessage("입력 오류", "올바른 이메일 형식을 입력해주세요.", QMessageBox::Warning);
        return;
    }

    // 서버 연결 상태 확인
    if (!m_networkManager->isConnected()) {
        showMessage("연결 오류", "서버에 연결되지 않았습니다. 잠시 후 다시 시도해주세요.", QMessageBox::Critical);
        m_networkManager->connectToServer();
        return;
    }

    // 로그인 시도할 이메일 저장
    m_loggedInEmail = email;
    
    // 서버로 로그인 요청 전송
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
    // 네트워크 연결 성공 시 처리 (필요시 UI 업데이트)
}

void LoginPage::onNetworkDisconnected()
{
    showMessage("연결 끊김", "서버와의 연결이 끊어졌습니다.", QMessageBox::Warning);
}

void LoginPage::onLoginResponse(const QJsonObject &response)
{
    QString status = response["status"].toString();
    int code = response["code"].toInt();
    QString message = response["message"].toString();

    if (status == "success" && code == 200) {
        // 로그인 성공
        qDebug() << "[Login] 로그인 성공";
        
        showMessage("로그인 성공", "로그인에 성공했습니다!", QMessageBox::Information);

        // 시그널-슬롯 연결 해제
        disconnect(m_networkManager, nullptr, this, nullptr);

        emit loginSuccessful();
        accept();  // 다이얼로그 닫기
    } else {
        // 로그인 실패
        qDebug() << "[Login] 로그인 실패 - 코드:" << code;
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
    QString iconKey;
    switch (icon) {
    case QMessageBox::Information:
        iconKey = "info";
        break;
    case QMessageBox::Warning:
        iconKey = "warning";
        break;
    case QMessageBox::Critical:
        iconKey = "error";
        break;
    default:
        iconKey = "";
        break;
    }

    CustomMessageBox::showMessage(this, title, message);
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

void LoginPage::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    
    // 최상위 윈도우를 찾아서 오버레이 생성
    QWidget* topLevelWidget = this;
    while (topLevelWidget->parentWidget()) {
        topLevelWidget = topLevelWidget->parentWidget();
    }
    
    if (topLevelWidget && topLevelWidget != this) {
        overlay_ = new OverlayWidget(topLevelWidget);
        overlay_->resize(topLevelWidget->size());
        overlay_->show();
        
        // 다이얼로그를 최상위로
        raise();
    }
}

void LoginPage::hideEvent(QHideEvent* event) {
    // 오버레이 제거
    if (overlay_) {
        overlay_->deleteLater();
        overlay_ = nullptr;
    }
    
    QDialog::hideEvent(event);
}

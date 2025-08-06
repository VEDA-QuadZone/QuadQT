#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QDialog>
#include <QJsonObject>
#include <QMessageBox>
#include "login/networkmanager.h"
#include "login/custommessagebox.h"

class OverlayWidget;

namespace Ui {
class LoginPage;
}

class LoginPage : public QDialog
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();
    
    QString getLoggedInEmail() const { return m_loggedInEmail; }

signals:
    void loginSuccessful();

private slots:
    void showSignInPage();
    void showRegisterPage();
    void showResetPasswordPage();

    // 버튼 클릭 핸들러
    void onSignInClicked();
    void onRegisterClicked();
    void onResetPasswordClicked();

    // 네트워크 응답 핸들러
    void onNetworkConnected();
    void onNetworkDisconnected();
    void onLoginResponse(const QJsonObject &response);
    void onRegisterResponse(const QJsonObject &response);
    void onResetPasswordResponse(const QJsonObject &response);
    void onNetworkError(const QString &error);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void setupConnections();
    void setupFonts();
    void showMessage(const QString &title, const QString &message, QMessageBox::Icon icon = QMessageBox::Information);
    bool validateEmail(const QString &email);
    bool validatePassword(const QString &password);
    void clearInputFields();

    Ui::LoginPage *ui;
    NetworkManager *m_networkManager;
    QString m_loggedInEmail;
    
    // 오버레이 효과
    OverlayWidget* overlay_;
};

#endif // LOGINPAGE_H

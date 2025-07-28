#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QDialog>
#include <QJsonObject>
#include <QMessageBox>
#include "networkmanager.h"

namespace Ui {
class LoginPage;
}

class LoginPage : public QDialog
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

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

private:
    void setupConnections();
    void showMessage(const QString &title, const QString &message, QMessageBox::Icon icon = QMessageBox::Information);
    bool validateEmail(const QString &email);
    bool validatePassword(const QString &password);
    void clearInputFields();

    Ui::LoginPage *ui;
    NetworkManager *m_networkManager;
};

#endif // LOGINPAGE_H

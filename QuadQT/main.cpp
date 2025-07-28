#include <QApplication>
#include <QDebug>
#include "login/loginpage.h"
#include "mainwindow/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "🚀 애플리케이션 시작...";
    qDebug() << "📋 로그인 페이지 표시";

    LoginPage login;
    login.setFixedSize(800, 600);

    if (login.exec() == QDialog::Accepted) {
        qDebug() << "✅ 로그인 대화상자 승인됨 - 사용자 인증 성공!";
        qDebug() << "🏠 메인 윈도우 생성 및 표시 중...";
        
        MainWindow w;
        w.show();
        
        qDebug() << "🎯 메인 윈도우 표시됨, 메인 이벤트 루프 진입";
        return app.exec();
    } else {
        qDebug() << "❌ 로그인 대화상자 거부되거나 취소됨";
        qDebug() << "🔚 애플리케이션 종료";
    }
    
    return 0;
}

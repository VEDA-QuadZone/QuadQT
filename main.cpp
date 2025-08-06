#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QPixmap>
#include "login/loginpage.h"
#include "mainwindow/mainwindow.h"
#include <QFontDatabase>
#include <QFont>
#include <QIcon>

QString g_hanwhaGothicFontFamily;
QString g_hanwhaFontFamily;

void loadFonts()
{
    // 한화 폰트들 로드
    int fontId1 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/01HanwhaB.ttf");
    int fontId2 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/02HanwhaR.ttf");
    int fontId3 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/03HanwhaL.ttf");
    int fontId4 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/04HanwhaGothicB.ttf");
    int fontId5 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/05HanwhaGothicR.ttf");
    int fontId6 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/06HanwhaGothicL.ttf");
    int fontId7 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/07HanwhaGothicEL.ttf");
    int fontId8 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/08HanwhaGothicT.ttf");
    
    // 로드된 폰트 패밀리 확인
    QStringList allFamilies = QFontDatabase().families();
    for (const QString &family : allFamilies) {
        if (family.contains("Hanwha", Qt::CaseInsensitive)) {
            if (family.contains("Gothic", Qt::CaseInsensitive) && g_hanwhaGothicFontFamily.isEmpty()) {
                g_hanwhaGothicFontFamily = family;
            }
            if (!family.contains("Gothic", Qt::CaseInsensitive) && g_hanwhaFontFamily.isEmpty()) {
                g_hanwhaFontFamily = family;
            }
        }
    }
    
    // 기본 폰트 설정
    if (!g_hanwhaGothicFontFamily.isEmpty()) {
        QFont defaultFont(g_hanwhaGothicFontFamily, 10);
        QApplication::setFont(defaultFont);
    } else if (!g_hanwhaFontFamily.isEmpty()) {
        QFont defaultFont(g_hanwhaFontFamily, 10);
        QApplication::setFont(defaultFont);
    }
}

QString getHanwhaGothicFontFamily() { return g_hanwhaGothicFontFamily; }
QString getHanwhaFontFamily() { return g_hanwhaFontFamily; }

int main(int argc, char *argv[])
{
    // DPI 스케일링 설정
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    
    // 애플리케이션 아이콘 설정
    app.setWindowIcon(QIcon(":/images/logo.png"));
    
    // 한화 폰트 로드
    loadFonts();
    
    // 로그인-메인윈도우 루프
    while (true) {
        LoginPage login;
        login.setFixedSize(800, 600);

        if (login.exec() == QDialog::Accepted) {
            // 로그인 성공 - 메인 윈도우 생성
            QString loggedInEmail = login.getLoggedInEmail();
            
            MainWindow *w = new MainWindow();
            w->setUserEmail(loggedInEmail);  // 사용자 이메일 설정
            w->setAttribute(Qt::WA_DeleteOnClose);  // 닫힐 때 자동 삭제
            w->show();
            
            // 메인 윈도우가 닫힐 때까지 대기
            int result = app.exec();
            
            if (result == 1) {
                // X 버튼으로 인한 종료 - 프로그램 완전 종료
                break;
            } else {
                // 로그아웃으로 인한 종료 - 로그인 페이지로 돌아감
                continue;
            }
        } else {
            // 로그인 취소 - 애플리케이션 종료
            break;
        }
    }
    
    return 0;
}

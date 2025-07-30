#include <QApplication>
#include <QGuiApplication>
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
    // 한화 폰트들 로드 및 확인
    int fontId1 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/01HanwhaB.ttf");
    int fontId2 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/02HanwhaR.ttf");
    int fontId3 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/03HanwhaL.ttf");
    int fontId4 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/04HanwhaGothicB.ttf");
    int fontId5 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/05HanwhaGothicR.ttf");
    int fontId6 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/06HanwhaGothicL.ttf");
    int fontId7 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/07HanwhaGothicEL.ttf");
    int fontId8 = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/08HanwhaGothicT.ttf");
    
    qDebug() << "폰트 로드 결과:";
    qDebug() << "  HanwhaB:" << (fontId1 != -1 ? "성공" : "실패") << "ID:" << fontId1;
    qDebug() << "  HanwhaR:" << (fontId2 != -1 ? "성공" : "실패") << "ID:" << fontId2;
    qDebug() << "  HanwhaGothicB:" << (fontId4 != -1 ? "성공" : "실패") << "ID:" << fontId4;
    qDebug() << "  HanwhaGothicR:" << (fontId5 != -1 ? "성공" : "실패") << "ID:" << fontId5;
    
    // 각 폰트 ID에서 실제 패밀리명 확인
    if (fontId1 != -1) {
        QStringList families1 = QFontDatabase::applicationFontFamilies(fontId1);
        qDebug() << "  HanwhaB 패밀리명:" << families1;
    }
    if (fontId5 != -1) {
        QStringList families5 = QFontDatabase::applicationFontFamilies(fontId5);
        qDebug() << "  HanwhaGothicR 패밀리명:" << families5;
    }
    
    // 모든 로드된 폰트 패밀리 확인
    QStringList allFamilies = QFontDatabase().families();
    qDebug() << "시스템에 로드된 모든 폰트 중 Hanwha 관련:";
    for (const QString &family : allFamilies) {
        if (family.contains("Hanwha", Qt::CaseInsensitive)) {
            qDebug() << "  발견된 폰트:" << family;
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
        qDebug() << "기본 폰트 설정 완료:" << g_hanwhaGothicFontFamily;
    } else if (!g_hanwhaFontFamily.isEmpty()) {
        QFont defaultFont(g_hanwhaFontFamily, 10);
        QApplication::setFont(defaultFont);
        qDebug() << "대체 폰트 설정 완료:" << g_hanwhaFontFamily;
    } else {
        qDebug() << "한화 폰트 로드 실패 - 시스템 기본 폰트 사용";
    }
}

QString getHanwhaGothicFontFamily() { return g_hanwhaGothicFontFamily; }
QString getHanwhaFontFamily() { return g_hanwhaFontFamily; }

int main(int argc, char *argv[])
{
    qputenv("QT_MEDIA_BACKEND", "ffmpeg");
    qputenv("QT_FFMPEG_EXTRA_FLAGS",
            "-fflags nobuffer -flags low_delay -framedrop -avioflags direct -rtsp_transport tcp");

    qDebug() << "[Debug] QT_MEDIA_BACKEND =" << qgetenv("QT_MEDIA_BACKEND");
    qDebug() << "[Debug] QT_FFMPEG_EXTRA_FLAGS =" << qgetenv("QT_FFMPEG_EXTRA_FLAGS");

    // DPI 스케일링 설정 (Qt 5.6+)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);

    qDebug() << "애플리케이션 시작...";
    
    // 리소스 시스템 디버깅
    qDebug() << "=== 리소스 시스템 디버깅 ===";
    QDir resourceDir(":/");
    qDebug() << "리소스 루트 디렉토리 존재:" << resourceDir.exists();
    
    QDir imagesDir(":/images");
    qDebug() << "이미지 디렉토리 존재:" << imagesDir.exists();
    if (imagesDir.exists()) {
        QStringList imageFiles = imagesDir.entryList();
        qDebug() << "이미지 디렉토리 내용:" << imageFiles;
    }
    
    // 개별 이미지 파일 테스트
    QStringList testImages = {":/images/logo.png", ":/images/camera.png", ":/images/video.png"};
    for (const QString &imagePath : testImages) {
        QPixmap testPixmap(imagePath);
        qDebug() << "이미지 테스트" << imagePath << ":" << (!testPixmap.isNull() ? "성공" : "실패");
        if (!testPixmap.isNull()) {
            qDebug() << "  - 크기:" << testPixmap.size();
        }
    }
    qDebug() << "========================";
    
    // 애플리케이션 아이콘 설정
    app.setWindowIcon(QIcon(":/images/logo.png"));
    qDebug() << "애플리케이션 아이콘 설정 완료";
    
    // 폰트 로드
    loadFonts();
    
    // 로그인-메인윈도우 루프
    while (true) {
        qDebug() << "로그인 페이지 표시";

        LoginPage login;
        login.setFixedSize(800, 600);

        if (login.exec() == QDialog::Accepted) {
            qDebug() << "로그인 대화상자 승인됨 - 사용자 인증 성공!";
            qDebug() << "메인 윈도우 생성 및 표시 중...";
            
            QString loggedInEmail = login.getLoggedInEmail();
            qDebug() << "로그인된 사용자 이메일:" << loggedInEmail;
            
            MainWindow *w = new MainWindow();
            w->setUserEmail(loggedInEmail);  // 이메일 설정
            w->setAttribute(Qt::WA_DeleteOnClose);  // 닫힐 때 자동 삭제
            w->show();
            
            qDebug() << "메인 윈도우 표시됨, 메인 이벤트 루프 진입";
            
            // MainWindow가 닫힐 때까지 대기
            int result = app.exec();
            
            if (result == 1) {
                // X 버튼으로 인한 종료 - 프로그램 완전 종료
                qDebug() << "X 버튼으로 인한 프로그램 완전 종료";
                break;
            } else {
                // 로그아웃으로 인한 종료 - 로그인 페이지로 돌아감
                qDebug() << "로그아웃으로 인한 종료, 로그인 페이지로 돌아감";
                // 루프 계속 (다시 로그인 페이지로)
            }
        } else {
            qDebug() << "로그인 대화상자 거부되거나 취소됨";
            qDebug() << "애플리케이션 종료";
            break;
        }
    }
    
    return 0;
}

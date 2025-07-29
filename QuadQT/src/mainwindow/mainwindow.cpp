#include "mainwindow/mainwindow.h"
#include "mainwindow/topbarwidget.h"
#include "mainwindow/displaysettingbox.h"

#include <QResizeEvent>
#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    topBar(nullptr),
    stackedWidget(nullptr),
    cameraPage(nullptr),
    documentPage(nullptr),
    settingsPage(nullptr),
    videoSettingTitle(nullptr),
    displayTitle(nullptr),
    procTitle(nullptr),
    displayBox(nullptr),
    procBox(nullptr),
    videoSettingLine(nullptr),
    cameraTitle(nullptr),
    notifTitleLabel(nullptr),
    videoArea(nullptr),
    notificationPanel(nullptr),
    m_isLogout(false)
{
    qDebug() << "MainWindow 생성자 시작";
    
    QWidget *centralW = new QWidget(this);
    centralW->setStyleSheet("background-color: white;");
    setCentralWidget(centralW);
    setMinimumSize(1600, 900);
    showMaximized();

    qDebug() << "UI 설정 시작";
    setupUI();
    
    qDebug() << "폰트 설정 시작";
    setupFonts();
    
    qDebug() << "페이지 설정 시작";
    setupPages();
    
    qDebug() << "레이아웃 업데이트 시작";
    updateLayout();
    
    // 기본적으로 카메라 페이지 표시
    showPage(PageType::Camera);
    
    qDebug() << "MainWindow 생성 완료";
}

MainWindow::~MainWindow() {}

void MainWindow::setUserEmail(const QString &email)
{
    qDebug() << "MainWindow::setUserEmail 호출됨:" << email;
    if (topBar) {
        topBar->setUserEmail(email);
        qDebug() << "TopBarWidget에 이메일 설정 완료";
    } else {
        qDebug() << "TopBarWidget이 아직 초기화되지 않음";
    }
}

void MainWindow::setupUI()
{
    QWidget *parent = centralWidget();
    qDebug() << "중앙 위젯 설정됨:" << (parent ? "성공" : "실패");

    // TopBar 생성
    topBar = new TopBarWidget(parent);
    qDebug() << "TopBar 생성됨:" << (topBar ? "성공" : "실패");
    
    // TopBar 시그널 연결
    connect(topBar, &TopBarWidget::cameraClicked, this, &MainWindow::onCameraClicked);
    connect(topBar, &TopBarWidget::documentClicked, this, &MainWindow::onDocumentClicked);
    connect(topBar, &TopBarWidget::settingsClicked, this, &MainWindow::onSettingsClicked);
    connect(topBar, &TopBarWidget::logoutRequested, this, &MainWindow::onLogoutRequested);
    qDebug() << "TopBar 시그널 연결 완료";

    // StackedWidget 생성 (페이지 전환용)
    stackedWidget = new QStackedWidget(parent);
    qDebug() << "StackedWidget 생성됨:" << (stackedWidget ? "성공" : "실패");
    
    topBar->show();
    stackedWidget->show();
    
    qDebug() << "기본 UI 요소 생성 완료";
}

void MainWindow::setupFonts()
{
    // 기본 폰트 설정
    QFont defaultFont("HanwhaGothicR", 12);
    QFont titleFont("HanwhaGothicR", 15);
    titleFont.setBold(true);
    
    // 전체 위젯에 기본 폰트 적용
    this->setFont(defaultFont);
    
    qDebug() << "MainWindow 폰트 설정 완료";
}

void MainWindow::setupPages()
{
    // 각 페이지 생성
    cameraPage = createCameraPage();
    documentPage = createDocumentPage();
    settingsPage = createSettingsPage();
    
    // StackedWidget에 페이지들 추가
    stackedWidget->addWidget(cameraPage);
    stackedWidget->addWidget(documentPage);
    stackedWidget->addWidget(settingsPage);
    
    qDebug() << "모든 페이지 생성 및 추가 완료";
}

QWidget* MainWindow::createCameraPage()
{
    QWidget *page = new QWidget();
    
    // 폰트 설정
    QFont titleFont("HanwhaGothicR", 15);
    titleFont.setBold(true);
    
    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", page);
    cameraTitle->setFont(titleFont);
    cameraTitle->setStyleSheet("font-weight: bold;");
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    notifTitleLabel = new QLabel("실시간 알림", page);
    notifTitleLabel->setFont(titleFont);
    notifTitleLabel->setStyleSheet("font-weight: bold;");
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    videoArea = new QLabel(page);
    
    // video.png 이미지 로드 및 설정
    QPixmap videoPixmap(":/images/resources/images/video.png");
    if (!videoPixmap.isNull()) {
        videoArea->setPixmap(videoPixmap);
        videoArea->setScaledContents(true);  // 영역에 맞게 스케일링
        videoArea->setAlignment(Qt::AlignCenter);
        videoArea->setStyleSheet("border: 1px solid #ccc; background-color: #000;"); // 검은 배경으로 비디오 느낌
        qDebug() << "video.png 이미지 로드 성공 - 크기:" << videoPixmap.size();
    } else {
        // 이미지 로드 실패 시 fallback
        videoPixmap = QPixmap("resources/images/video.png");
        if (!videoPixmap.isNull()) {
            videoArea->setPixmap(videoPixmap);
            videoArea->setScaledContents(true);
            videoArea->setAlignment(Qt::AlignCenter);
            videoArea->setStyleSheet("border: 1px solid #ccc; background-color: #000;");
            qDebug() << "video.png 절대경로로 로드 성공 - 크기:" << videoPixmap.size();
        } else {
            videoArea->setText("비디오 영역\n(video.png 로드 실패)");
            videoArea->setAlignment(Qt::AlignCenter);
            videoArea->setStyleSheet("background-color: #e2e7ec; border: 1px solid #ccc; font-size: 16px; color: #666;");
            qDebug() << "video.png 로드 실패 - 텍스트로 fallback";
        }
    }

    notificationPanel = new QWidget(page);
    notificationPanel->setStyleSheet("background-color: #f8f9fa; border-left: 1px solid #ccc;");

    QFont settingTitleFont("HanwhaGothicR", 16);
    settingTitleFont.setBold(true);
    QFont subTitleFont("HanwhaGothicR", 13);
    subTitleFont.setBold(true);
    
    videoSettingTitle = new QLabel("영상 설정", page);
    videoSettingTitle->setFont(settingTitleFont);
    videoSettingTitle->setStyleSheet("font-weight: bold;");
    videoSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    videoSettingLine = new QWidget(page);
    videoSettingLine->setStyleSheet("background-color: #999;");

    displayTitle = new QLabel("화면 표시", page);
    displayTitle->setFont(subTitleFont);
    displayTitle->setStyleSheet("font-weight: bold;");
    displayTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    procTitle = new QLabel("영상 처리", page);
    procTitle->setFont(subTitleFont);
    procTitle->setStyleSheet("font-weight: bold;");
    procTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    displayBox = new DisplaySettingBox(page);
    displayBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

    procBox = new QWidget(page);
    procBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    
    qDebug() << "카메라 페이지 생성 완료";
    return page;
}

QWidget* MainWindow::createDocumentPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f5f5f5;");
    
    QFont pageTitleFont("HanwhaGothicR", 24);
    pageTitleFont.setBold(true);
    QFont pageContentFont("HanwhaGothicR", 16);
    
    QLabel *titleLabel = new QLabel("문서 페이지", page);
    titleLabel->setFont(pageTitleFont);
    titleLabel->setStyleSheet("font-weight: bold; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 100, 800, 50);
    
    QLabel *contentLabel = new QLabel("문서 관련 기능이 여기에 표시됩니다.", page);
    contentLabel->setFont(pageContentFont);
    contentLabel->setStyleSheet("color: #666;");
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setGeometry(0, 200, 800, 30);
    
    qDebug() << "문서 페이지 생성 완료";
    return page;
}

QWidget* MainWindow::createSettingsPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f0f8ff;");
    
    QFont pageTitleFont("HanwhaGothicR", 24);
    pageTitleFont.setBold(true);
    QFont pageContentFont("HanwhaGothicR", 16);
    
    QLabel *titleLabel = new QLabel("설정 페이지", page);
    titleLabel->setFont(pageTitleFont);
    titleLabel->setStyleSheet("font-weight: bold; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 100, 800, 50);
    
    QLabel *contentLabel = new QLabel("설정 관련 기능이 여기에 표시됩니다.", page);
    contentLabel->setFont(pageContentFont);
    contentLabel->setStyleSheet("color: #666;");
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setGeometry(0, 200, 800, 30);
    
    qDebug() << "설정 페이지 생성 완료";
    return page;
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    qDebug() << "MainWindow 크기 변경 이벤트:" << event->size();
    QMainWindow::resizeEvent(event);
    updateLayout();
}

void MainWindow::onCameraClicked()
{
    qDebug() << "카메라 페이지로 전환";
    showPage(PageType::Camera);
}

void MainWindow::onDocumentClicked()
{
    qDebug() << "문서 페이지로 전환";
    showPage(PageType::Document);
}

void MainWindow::onSettingsClicked()
{
    qDebug() << "설정 페이지로 전환";
    showPage(PageType::Settings);
}

void MainWindow::onLogoutRequested()
{
    qDebug() << "로그아웃 요청됨";
    
    // 사용자 데이터 클리어
    if (topBar) {
        topBar->clearUserData();
    }
    
    // 로그아웃 플래그 설정
    m_isLogout = true;
    
    // 이벤트 루프 종료하여 로그인 화면으로 돌아가기
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isLogout) {
        qDebug() << "로그아웃으로 인한 창 닫기 - 로그인 페이지로 이동";
        // 로그아웃인 경우 정상적으로 닫기
        event->accept();
    } else {
        qDebug() << "X 버튼으로 인한 창 닫기 - 프로그램 완전 종료";
        // X 버튼 클릭인 경우 프로그램 완전 종료
        QApplication::exit(1);  // 종료 코드 1로 완전 종료
        event->accept();
    }
}

void MainWindow::showPage(PageType pageType)
{
    if (!stackedWidget) return;
    
    switch (pageType) {
    case PageType::Camera:
        stackedWidget->setCurrentWidget(cameraPage);
        qDebug() << "카메라 페이지 표시됨";
        break;
    case PageType::Document:
        stackedWidget->setCurrentWidget(documentPage);
        qDebug() << "문서 페이지 표시됨";
        break;
    case PageType::Settings:
        stackedWidget->setCurrentWidget(settingsPage);
        qDebug() << "설정 페이지 표시됨";
        break;
    }
}

void MainWindow::updateLayout()
{
    qDebug() << "레이아웃 업데이트 시작";
    
    if (!topBar || !stackedWidget) {
        qDebug() << "필수 UI 요소가 null이어서 레이아웃 업데이트 중단";
        return;
    }

    int w = width();
    int h = height();
    qDebug() << "윈도우 크기: " << w << "x" << h;
    
    double h_unit = h / 24.0;

    // TopBar 레이아웃
    topBar->setGeometry(0, 0, w, h_unit * 3);
    topBar->updateLayout(w, h);
    qDebug() << "TopBar 레이아웃 설정됨";

    // StackedWidget 레이아웃 (TopBar 아래 전체 영역)
    stackedWidget->setGeometry(0, h_unit * 3, w, h - h_unit * 3);
    qDebug() << "StackedWidget 레이아웃 설정됨";
    
    // 카메라 페이지의 세부 레이아웃 (현재 페이지가 카메라 페이지인 경우에만)
    if (stackedWidget->currentWidget() == cameraPage && cameraTitle && videoArea) {
        updateCameraPageLayout();
    }
    
    qDebug() << "레이아웃 업데이트 완료";
}

void MainWindow::updateCameraPageLayout()
{
    if (!cameraTitle || !notifTitleLabel || !videoArea || !notificationPanel) return;
    
    int w = stackedWidget->width();
    int h = stackedWidget->height();
    
    double w_unit = w / 24.0;
    double h_unit = h / 21.0; // TopBar를 제외한 높이 기준
    
    double cctv_w = w_unit * 16.5;
    double padding = w_unit * 1;
    double middle_pad = w_unit * 0.5;
    double notif_w = w_unit * 5.0;
    double cctv_x = padding;
    double notif_x = cctv_x + cctv_w + middle_pad;

    cameraTitle->setGeometry(cctv_x, 0, cctv_w, h_unit);
    notifTitleLabel->setGeometry(notif_x, 0, notif_w, h_unit);

    videoArea->setGeometry(cctv_x, h_unit, cctv_w, h_unit * 13);
    notificationPanel->setGeometry(notif_x, h_unit, notif_w, h_unit * 19);

    double settingTop = h_unit * 14;
    double labelTop = h_unit * 15;
    double boxTop = h_unit * 16;

    if (videoSettingTitle) {
        videoSettingTitle->setGeometry(cctv_x, settingTop, cctv_w, h_unit);
    }
    
    if (videoSettingLine) {
        videoSettingLine->setGeometry(cctv_x, settingTop + h_unit - 1, cctv_w, 1);
    }

    if (displayTitle) {
        displayTitle->setGeometry(cctv_x, labelTop, w_unit * 6, h_unit);
    }
    
    if (procTitle) {
        procTitle->setGeometry(cctv_x + w_unit * (6 + 0.5), labelTop, w_unit * 10, h_unit);
    }

    if (displayBox) {
        displayBox->setGeometry(cctv_x, boxTop, w_unit * 6, h_unit * 4);
    }
    
    if (procBox) {
        procBox->setGeometry(cctv_x + w_unit * (6 + 0.5), boxTop, w_unit * 10, h_unit * 4);
    }
    
    qDebug() << "카메라 페이지 레이아웃 업데이트 완료";
}

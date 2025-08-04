#include "mainwindow/mainwindow.h"
#include "mainwindow/topbarwidget.h"
#include "mainwindow/displaysettingbox.h"
#include "mainwindow/procsettingbox.h"
#include "mainwindow/notificationpanel.h"
#include "mainwindow/mqttmanager.h"
#include "login/networkmanager.h"
#include "login/custommessagebox.h"

#include <QResizeEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QFile>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QTimer>
#include <QSettings>
#include <QPalette>
#include <QFontDatabase>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    topBar(nullptr),
    stackedWidget(nullptr),
    cameraPage(nullptr),
    documentPage(nullptr),
    cameraTitle(nullptr),
    notifTitleLabel(nullptr),
    videoSettingTitle(nullptr),
    displayTitle(nullptr),
    procTitle(nullptr),
    videoSettingLine(nullptr),
    notificationPanel(nullptr),
    displayBox(nullptr),
    procBox(nullptr),
    mqttManager(nullptr),
    networkManager(nullptr),
    m_isLogout(false),
    historyView(nullptr)
{
    // SSL 기본 설정 (개발 환경용)
    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConf);

    // 중앙 위젯 설정
    QWidget *centralW = new QWidget(this);
    centralW->setStyleSheet("background-color: #FFFFFF;");
    centralW->setAutoFillBackground(true);
    setCentralWidget(centralW);

    this->setStyleSheet("MainWindow { background-color: #FFFFFF; }");
    this->setAutoFillBackground(true);

    // 윈도우 크기 설정
    setMinimumSize(1600, 900);
    showMaximized();

    // TCP 네트워크 매니저 초기화
    networkManager = new NetworkManager(this);
    networkManager->connectToServer();

    // TCP 연결 상태 모니터링
    connect(networkManager, &NetworkManager::connected, this, []() {
        qDebug() << "[TCP] 서버 연결 성공";
    });
    connect(networkManager, &NetworkManager::disconnected, this, []() {
        qDebug() << "[TCP] 서버 연결 해제";
    });
    connect(networkManager, &NetworkManager::networkError, this, [](const QString &msg) {
        qDebug() << "[TCP] 네트워크 오류:" << msg;
    });

    // UI 구성 요소 초기화
    setupUI();
    setupFonts();
    setupPages();
    showPage(PageType::Camera);

    // 레이아웃 초기화
    QTimer::singleShot(100, this, &MainWindow::forceLayoutUpdate);

    // RTSP 스트리밍 초기화
    QSettings settings("config.ini", QSettings::IniFormat);
    QString rtspUrl = settings.value("rtsp/url", "rtsps://192.168.219.68:8555/test").toString();

    rtspThread = new RtspThread(rtspUrl, this);

    // RTSP 프레임 수신 처리
    connect(rtspThread, &RtspThread::frameReady, this, [this](const QImage &img) {
        if (rtspLabel)
            rtspLabel->setPixmap(QPixmap::fromImage(img).scaled(rtspLabel->size(), Qt::KeepAspectRatio));
    });

    rtspThread->start();

    // MQTT 매니저 초기화
    mqttManager = new MqttManager(this);

    // MQTT 메시지를 알림 패널로 전달
    connect(mqttManager, &MqttManager::messageReceived,
            notificationPanel, &NotificationPanel::handleMqttMessage);

    // MQTT 브로커 연결
    QTimer::singleShot(0, this, [this]() {
        mqttManager->connectToBroker();
    });
}

MainWindow::~MainWindow()
{
    if (rtspThread) {
        rtspThread->stop();
        rtspThread->wait();
    }
}

void MainWindow::setUserEmail(const QString &email)
{
    if (topBar)
        topBar->setUserEmail(email);
}

void MainWindow::setupUI()
{
    QWidget *parent = centralWidget();

    topBar = new TopBarWidget(parent);
    topBar->setStyleSheet("background-color: #FFFFFF !important;");
    topBar->setAutoFillBackground(true);
    
    // QPalette을 사용한 추가 설정
    QPalette topBarPalette = topBar->palette();
    topBarPalette.setColor(QPalette::Window, QColor(255, 255, 255));
    topBar->setPalette(topBarPalette);
    connect(topBar, &TopBarWidget::cameraClicked, this, &MainWindow::onCameraClicked);
    connect(topBar, &TopBarWidget::documentClicked, this, &MainWindow::onDocumentClicked);
    connect(topBar, &TopBarWidget::logoutRequested, this, &MainWindow::onLogoutRequested);

    stackedWidget = new QStackedWidget(parent);
    topBar->show();
    stackedWidget->show();
}

void MainWindow::setupFonts()
{
    // 한화 글꼴 로드 확인
    QStringList allFamilies = QFontDatabase().families();
    QString hanwhaFont;
    
    // HanwhaGothicB 폰트 찾기
    for (const QString &family : allFamilies) {
        if (family.contains("HanwhaGothicB", Qt::CaseInsensitive) || 
            family.contains("HanwhaGothic", Qt::CaseInsensitive)) {
            hanwhaFont = family;
            break;
        }
    }
    
    // 폰트가 없으면 시스템 기본 폰트 사용
    if (hanwhaFont.isEmpty()) {
        hanwhaFont = "Malgun Gothic"; // 한글 지원 기본 폰트
    }
    
    QFont defaultFont(hanwhaFont, 12);
    QFont titleFont(hanwhaFont, 15);
    titleFont.setBold(true);
    setFont(defaultFont);
}

void MainWindow::setupPages()
{
    cameraPage = createCameraPage();
    documentPage = createDocumentPage();

    stackedWidget->addWidget(cameraPage);
    stackedWidget->addWidget(documentPage);
}

QWidget* MainWindow::createCameraPage()
{
    QWidget *page = new QWidget();

    // 한화 글꼴 확인 및 fallback 설정
    QStringList allFamilies = QFontDatabase().families();
    QString hanwhaFont;
    
    for (const QString &family : allFamilies) {
        if (family.contains("HanwhaGothicB", Qt::CaseInsensitive) || 
            family.contains("HanwhaGothic", Qt::CaseInsensitive)) {
            hanwhaFont = family;
            break;
        }
    }
    
    if (hanwhaFont.isEmpty()) {
        hanwhaFont = "Malgun Gothic"; // 한글 지원 기본 폰트
    }

    QFont titleFont(hanwhaFont, 15);
    titleFont.setBold(true);

    // CCTV 제목 라벨
    cameraTitle = new QLabel("상계 초등학교 앞 CCTV", page);
    cameraTitle->setFont(titleFont);
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 실시간 알림 제목 라벨
    notifTitleLabel = new QLabel("실시간 알림", page);
    notifTitleLabel->setFont(titleFont);
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // RTSP 영상 표시 라벨
    rtspLabel = new QLabel(page);
    rtspLabel->setStyleSheet("background-color: black; border: 1px solid #ccc;");
    rtspLabel->setAlignment(Qt::AlignCenter);
    rtspLabel->setMinimumSize(640, 480);

    // 알림 패널
    notificationPanel = new NotificationPanel(page);
    notificationPanel->setStyleSheet("background-color: #FFFFFF; border-left: 1px solid #ccc;");

    // 영상 설정 관련 폰트
    QFont settingTitleFont(hanwhaFont, 16); settingTitleFont.setBold(true);
    QFont subTitleFont(hanwhaFont, 13);     subTitleFont.setBold(true);

    // 영상 설정 제목
    videoSettingTitle = new QLabel("영상 설정", page);
    videoSettingTitle->setFont(settingTitleFont);
    videoSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 영상 설정 구분선
    videoSettingLine = new QWidget(page);
    videoSettingLine->setStyleSheet("background-color: #ccc;");

    // 화면 표시 제목
    displayTitle = new QLabel("화면 표시", page);
    displayTitle->setFont(subTitleFont);
    displayTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // 영상 처리 제목
    procTitle = new QLabel("영상 처리", page);
    procTitle->setFont(subTitleFont);
    procTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // 화면 표시 설정 박스
    displayBox = new DisplaySettingBox(page);
    displayBox->setStyleSheet("background-color: #FFFFFF; border: 1px solid #ccc;");

    // 영상 처리 설정 박스
    procBox = new ProcSettingBox(page);
    procBox->setStyleSheet("background-color: #FFFFFF; border: 1px solid #ccc;");
    
    // 설정 박스들을 NetworkManager와 연결
    connect(displayBox, &DisplaySettingBox::requestCommand,networkManager, &NetworkManager::sendCommand);
    connect(procBox, &ProcSettingBox::requestCommand, networkManager, &NetworkManager::sendCommand);

    return page;
}

QWidget* MainWindow::createDocumentPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #FFFFFF;");

    // 히스토리 뷰 생성
    historyView = new HistoryView(page);

    // 레이아웃 설정
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(historyView);

    page->setLayout(layout);
    return page;
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // 약간의 지연을 두고 레이아웃 업데이트 (리사이즈 완료 후)
    QTimer::singleShot(10, this, &MainWindow::forceLayoutUpdate);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Ctrl+0으로 1600x900 크기로 복원
    if (event->modifiers() == Qt::ControlModifier && 
        (event->key() == Qt::Key_0 || event->key() == Qt::Key_Equal)) {
        
        // 최대화 상태 해제
        if (isMaximized()) {
            showNormal();
        }
        
        // 크기를 1600x900으로 설정
        resize(1600, 900);
        
        // 화면 중앙에 위치시키기
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - 1600) / 2;
        int y = (screenGeometry.height() - 900) / 2;
        move(x, y);
        
        // 레이아웃 업데이트
        QTimer::singleShot(50, this, &MainWindow::forceLayoutUpdate);
        
        event->accept();
        return;
    }
    
    // 기본 키 이벤트 처리
    QMainWindow::keyPressEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    // 윈도우가 표시될 때 레이아웃 강제 업데이트
    QTimer::singleShot(50, this, &MainWindow::forceLayoutUpdate);
}


void MainWindow::onCameraClicked()   { showPage(PageType::Camera); }
void MainWindow::onDocumentClicked() { showPage(PageType::Document); }

void MainWindow::onLogoutRequested()
{
    bool confirmed = CustomMessageBox::showConfirm(this, "로그아웃 확인", "정말 로그아웃 하시겠습니까?");

    if (confirmed) {
        if (topBar) topBar->clearUserData();
        m_isLogout = true;
        close();  // 창을 닫고 main.cpp에서 로그인 페이지로 복귀
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isLogout)
        event->accept();
    else {
        QApplication::exit(1);
        event->accept();
    }
}

bool MainWindow::wasLogout() const {
    return m_isLogout;
}

void MainWindow::showPage(PageType pageType)
{
    if (!stackedWidget) return;

    switch (pageType) {
    case PageType::Camera:   stackedWidget->setCurrentWidget(cameraPage); break;
    case PageType::Document: stackedWidget->setCurrentWidget(documentPage); break;
    }
    
    // 페이지 전환 후 레이아웃 강제 업데이트
    QTimer::singleShot(0, this, [this]() {
        updateLayout();
        // 현재 페이지의 모든 자식 위젯들도 업데이트
        if (stackedWidget->currentWidget()) {
            stackedWidget->currentWidget()->update();
            stackedWidget->currentWidget()->repaint();
        }
    });
}

void MainWindow::updateLayout()
{
    int w = width();
    int h = height();

    double h_unit = h / 24.0;

    // 상단 바 레이아웃 업데이트
    if (topBar) {
        topBar->setGeometry(0, 0, w, h_unit * 3);
        topBar->updateLayout(w, h);
    }

    // 스택 위젯 레이아웃 업데이트
    if (stackedWidget) {
        stackedWidget->setGeometry(0, h_unit * 3, w, h - h_unit * 3);
        
        // 현재 페이지에 따라 레이아웃 업데이트
        QWidget* currentPage = stackedWidget->currentWidget();
        if (currentPage == cameraPage) {
            updateCameraPageLayout();
        } else if (currentPage == documentPage && historyView) {
            // 문서 페이지 레이아웃 업데이트
            historyView->setGeometry(0, 0, stackedWidget->width(), stackedWidget->height());
            historyView->update();
        }
        
        // 현재 페이지 강제 업데이트
        if (currentPage) {
            currentPage->updateGeometry();
            currentPage->update();
        }
    }
}


void MainWindow::updateCameraPageLayout()
{
    if (!cameraTitle || !notifTitleLabel || !rtspLabel || !notificationPanel) return;

    int w = stackedWidget->width();
    int h = stackedWidget->height();

    double w_unit = w / 24.0;
    double h_unit = h / 21.0;

    // 레이아웃 계산
    double cctv_w = w_unit * 16.5;
    double padding = w_unit * 1;
    double middle_pad = w_unit * 0.5;
    double notif_w = w_unit * 5.0;
    double cctv_x = padding;
    double notif_x = cctv_x + cctv_w + middle_pad;

    // 제목 라벨들 배치
    cameraTitle->setGeometry(cctv_x, h_unit * 0, cctv_w, h_unit);
    cameraTitle->update();
    
    notifTitleLabel->setGeometry(notif_x, h_unit * 0, notif_w, h_unit);
    notifTitleLabel->update();

    // RTSP 영상 라벨 배치
    if (rtspLabel) {
        rtspLabel->setGeometry(cctv_x, h_unit * 1, cctv_w, h_unit * 13);
        rtspLabel->update();
    }
    
    // 알림 패널 배치 (영상처리 박스 아래까지 확장)
    double notifHeight = h_unit * 19;
    notificationPanel->setGeometry(notif_x, h_unit * 1, notif_w, notifHeight);
    notificationPanel->setMinimumHeight(notifHeight);
    notificationPanel->setMaximumHeight(notifHeight);
    notificationPanel->updateGeometry();
    notificationPanel->update();

    // 영상 설정 영역 배치
    double settingTop = h_unit * 14;
    double labelTop   = h_unit * 15;
    double boxTop     = h_unit * 16;

    if (videoSettingTitle) {
        videoSettingTitle->setGeometry(cctv_x, settingTop, cctv_w, h_unit);
        videoSettingTitle->update();
    }
    if (videoSettingLine) {
        videoSettingLine->setGeometry(cctv_x, settingTop + h_unit - 1, cctv_w, 1);
        videoSettingLine->update();
    }
    if (displayTitle) {
        displayTitle->setGeometry(cctv_x, labelTop, w_unit * 6, h_unit);
        displayTitle->update();
    }
    if (procTitle) {
        procTitle->setGeometry(cctv_x + w_unit * (6 + 0.5), labelTop, w_unit * 10, h_unit);
        procTitle->update();
    }
    if (displayBox) {
        displayBox->setGeometry(cctv_x, boxTop, w_unit * 6, h_unit * 4);
        displayBox->updateGeometry();
        displayBox->update();
    }
    if (procBox) {
        procBox->setGeometry(cctv_x + w_unit * (6 + 0.5), boxTop, w_unit * 10, h_unit * 4);
        procBox->updateGeometry();
        procBox->update();
    }
}

void MainWindow::forceLayoutUpdate()
{
    // 현재 윈도우 상태 확인
    if (!isVisible() || isMinimized()) {
        return;
    }
    
    updateLayout();
    
    // 모든 자식 위젯들의 geometry 강제 업데이트
    if (stackedWidget && stackedWidget->currentWidget()) {
        QWidget* currentPage = stackedWidget->currentWidget();
        
        // 현재 페이지의 모든 자식 위젯들 업데이트
        QList<QWidget*> allChildren = currentPage->findChildren<QWidget*>();
        for (QWidget* child : allChildren) {
            child->updateGeometry();
            child->update();
        }
        
        currentPage->updateGeometry();
        currentPage->update();
        currentPage->repaint();
    }
    
    // 전체 윈도우 repaint
    update();
    repaint();
}

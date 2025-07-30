#include "mainwindow/mainwindow.h"
#include "mainwindow/topbarwidget.h"
#include "mainwindow/displaysettingbox.h"
#include "mainwindow/procsettingbox.h"
#include "mainwindow/notificationpanel.h"
#include "mainwindow/mqttmanager.h"


#include <QResizeEvent>
#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QFile>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
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
    settingsPage(nullptr),
    videoSettingTitle(nullptr),
    displayTitle(nullptr),
    procTitle(nullptr),
    displayBox(nullptr),
    procBox(nullptr),
    videoSettingLine(nullptr),
    cameraTitle(nullptr),
    notifTitleLabel(nullptr),
    videoWidget(nullptr),
    mqttManager(nullptr),
    //videoArea(nullptr),
    notificationPanel(nullptr),
    player(nullptr),
    m_isLogout(false)
{
    qDebug() << "🏠 MainWindow 생성자 시작";

    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConf);

    QWidget *centralW = new QWidget(this);
    centralW->setStyleSheet("background-color: #FFFFFF;");
    centralW->setAutoFillBackground(true);
    setCentralWidget(centralW);
    
    // MainWindow 자체 배경도 설정
    this->setStyleSheet("MainWindow { background-color: #FFFFFF; }");
    this->setAutoFillBackground(true);
    
    setMinimumSize(1600, 900);
    showMaximized();

    setupUI();
    setupFonts();
    setupPages();
    updateLayout();
    showPage(PageType::Camera);

    // QMediaPlayer 초기화 및 RTSP 재생
    player = new QMediaPlayer(this);
    player->setVideoOutput(videoWidget);
    
    // config.ini에서 RTSP URL 읽기
    QSettings settings("config.ini", QSettings::IniFormat);
    QString rtspUrl = settings.value("rtsp/url", "rtsps://192.168.0.10:8555/test").toString();
    player->setSource(QUrl(rtspUrl));
    player->play();

    // === MQTT 매니저 초기화 ===
    mqttManager = new MqttManager(this);

    // 연결 성공 시 테스트 메시지 발행
    connect(mqttManager, &MqttManager::connected, this, [this]() {
        qDebug() << "[MQTT] Connected signal received!";
        QByteArray testPayload = "{\"event\":99,\"timestamp\":\"test-message\"}";
        qDebug() << "[MQTT] Publishing test message:" << testPayload;
        mqttManager->publish(testPayload); // 기본 토픽 사용
    });

    connect(mqttManager, &MqttManager::messageReceived,
            notificationPanel, &NotificationPanel::handleMqttMessage);

    // 이벤트 루프 시작 후 연결 시도 (TransportInvalid 방지)
    QTimer::singleShot(0, this, [this]() {
        mqttManager->connectToBroker();
    });


    qDebug() << "MainWindow 생성 완료";
}

MainWindow::~MainWindow() {}

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
    connect(topBar, &TopBarWidget::settingsClicked, this, &MainWindow::onSettingsClicked);
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
        hanwhaFont = "Arial"; // 또는 "Malgun Gothic" (한글 지원)
        qDebug() << "한화 글꼴을 찾을 수 없어 기본 글꼴 사용:" << hanwhaFont;
    } else {
        qDebug() << "한화 글꼴 발견:" << hanwhaFont;
    }
    
    QFont defaultFont(hanwhaFont, 12);
    QFont titleFont(hanwhaFont, 15);
    titleFont.setBold(true);
    setFont(defaultFont);
    
    qDebug() << "기본 글꼴 설정 완료:" << defaultFont.family();
}

void MainWindow::setupPages()
{
    cameraPage = createCameraPage();
    documentPage = createDocumentPage();
    settingsPage = createSettingsPage();

    stackedWidget->addWidget(cameraPage);
    stackedWidget->addWidget(documentPage);
    stackedWidget->addWidget(settingsPage);
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
        qDebug() << "카메라 페이지: 한화 글꼴을 찾을 수 없어 기본 글꼴 사용:" << hanwhaFont;
    }

    QFont titleFont(hanwhaFont, 15);
    titleFont.setBold(true);

    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", page);
    cameraTitle->setFont(titleFont);
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    notifTitleLabel = new QLabel("실시간 알림", page);
    notifTitleLabel->setFont(titleFont);
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    videoWidget = new QVideoWidget(page);
    videoWidget->setStyleSheet("background-color: black; border: 1px solid #ccc;");

    notificationPanel = new NotificationPanel(page);
    notificationPanel->setStyleSheet("background-color: #FFFFFF; border-left: 1px solid #ccc;");

    QFont settingTitleFont(hanwhaFont, 16); settingTitleFont.setBold(true);
    QFont subTitleFont(hanwhaFont, 13);     subTitleFont.setBold(true);

    videoSettingTitle = new QLabel("영상 설정", page);
    videoSettingTitle->setFont(settingTitleFont);
    videoSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    videoSettingLine = new QWidget(page);
    videoSettingLine->setStyleSheet("background-color: #ccc;");

    displayTitle = new QLabel("화면 표시", page);
    displayTitle->setFont(subTitleFont);
    displayTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    procTitle = new QLabel("영상 처리", page);
    procTitle->setFont(subTitleFont);
    procTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    displayBox = new DisplaySettingBox(page);
    displayBox->setStyleSheet("background-color: #FFFFFF; border: 1px solid #ccc;");

    procBox = new ProcSettingBox(page);
    procBox->setStyleSheet("background-color: #FFFFFF; border: 1px solid #ccc;");

    return page;
}

QWidget* MainWindow::createDocumentPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #FFFFFF;");

    // 1) HistoryView 인스턴스 생성 (멤버로 만들어 관리)
    historyView = new HistoryView(page);  // parent 지정해도 되고 안 해도 됨

    // 2) 레이아웃에 추가
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(historyView);

    page->setLayout(layout);
    return page;
}
QWidget* MainWindow::createSettingsPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #FFFFFF;");

    QFont titleFont("HanwhaGothicR", 24); titleFont.setBold(true);
    QFont contentFont("HanwhaGothicR", 16);

    QLabel *titleLabel = new QLabel("설정 페이지", page);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 100, 800, 50);

    QLabel *contentLabel = new QLabel("설정 관련 기능이 여기에 표시됩니다.", page);
    contentLabel->setFont(contentFont);
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setGeometry(0, 200, 800, 30);

    return page;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateLayout();
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

void MainWindow::onCameraClicked()   { showPage(PageType::Camera); }
void MainWindow::onDocumentClicked() { showPage(PageType::Document); }
void MainWindow::onSettingsClicked() { showPage(PageType::Settings); }

void MainWindow::onLogoutRequested()
{
    if (topBar) topBar->clearUserData();
    m_isLogout = true;
    QApplication::quit();
}

void MainWindow::showPage(PageType pageType)
{
    if (!stackedWidget) return;

    switch (pageType) {
    case PageType::Camera:   stackedWidget->setCurrentWidget(cameraPage); break;
    case PageType::Document: stackedWidget->setCurrentWidget(documentPage); break;
    case PageType::Settings: stackedWidget->setCurrentWidget(settingsPage); break;
    }
}

void MainWindow::updateLayout()
{
    qDebug() << "📐 레이아웃 업데이트 시작";
    if (!topBar || !cameraTitle || !notifTitleLabel || !videoWidget || !notificationPanel) {
        qDebug() << "❌ 필수 UI 요소가 null이어서 레이아웃 업데이트 중단";
        return;
    }

    int w = width();
    int h = height();

    double w_unit = w / 24.0;
    double h_unit = h / 24.0;

    topBar->setGeometry(0, 0, w, h_unit * 3);
    topBar->updateLayout(w, h);

    stackedWidget->setGeometry(0, h_unit * 3, w, h - h_unit * 3);

    if (stackedWidget->currentWidget() == cameraPage)
        updateCameraPageLayout();
}

void MainWindow::updateCameraPageLayout()
{
    if (!cameraTitle || !notifTitleLabel || !videoWidget || !notificationPanel) return;

    int w = stackedWidget->width();
    int h = stackedWidget->height();

    double w_unit = w / 24.0;
    double h_unit = h / 21.0;

    double cctv_w = w_unit * 16.5;
    double padding = w_unit * 1;
    double middle_pad = w_unit * 0.5;
    double notif_w = w_unit * 5.0;
    double cctv_x = padding;
    double notif_x = cctv_x + cctv_w + middle_pad;

    cameraTitle->setGeometry(cctv_x, h_unit * 0, cctv_w, h_unit);
    notifTitleLabel->setGeometry(notif_x, h_unit * 0, notif_w, h_unit);

    videoWidget->setGeometry(cctv_x, h_unit * 1, cctv_w, h_unit * 13);
    notificationPanel->setGeometry(notif_x, h_unit * 1, notif_w, h_unit * 13);
    notificationPanel->setMinimumHeight(h_unit * 13);
    notificationPanel->setMaximumHeight(h_unit * 13);

    double settingTop = h_unit * 14;
    double labelTop   = h_unit * 15;
    double boxTop     = h_unit * 16;

    if (videoSettingTitle)
        videoSettingTitle->setGeometry(cctv_x, settingTop, cctv_w, h_unit);
    if (videoSettingLine)
        videoSettingLine->setGeometry(cctv_x, settingTop + h_unit - 1, cctv_w, 1);
    if (displayTitle)
        displayTitle->setGeometry(cctv_x, labelTop, w_unit * 6, h_unit);
    if (procTitle)
        procTitle->setGeometry(cctv_x + w_unit * (6 + 0.5), labelTop, w_unit * 10, h_unit);
    if (displayBox)
        displayBox->setGeometry(cctv_x, boxTop, w_unit * 6, h_unit * 4);
    if (procBox)
        procBox->setGeometry(cctv_x + w_unit * (6 + 0.5), boxTop, w_unit * 10, h_unit * 4);
}

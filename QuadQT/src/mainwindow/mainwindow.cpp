#include "mainwindow/mainwindow.h"
#include "mainwindow/topbarwidget.h"
#include "mainwindow/displaysettingbox.h"

#include <QResizeEvent>
#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QDebug>
#include <QFile>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    videoSettingTitle(nullptr),
    displayTitle(nullptr),
    procTitle(nullptr),
    displayBox(nullptr),
    procBox(nullptr),
    videoSettingLine(nullptr),
    topBar(nullptr),
    cameraTitle(nullptr),
    notifTitleLabel(nullptr),
    videoWidget(nullptr),
    notificationPanel(nullptr),
    player(nullptr)
{
    qDebug() << "🏠 MainWindow 생성자 시작";

    // 단방향 TLS 검증 해제
    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConf);

    QWidget *centralW = new QWidget(this);
    setCentralWidget(centralW);
    setMinimumSize(1600, 900);
    showMaximized();

    setupUI();
    updateLayout();

    // QMediaPlayer 초기화
    player = new QMediaPlayer(this);
    player->setVideoOutput(videoWidget);
    player->setSource(QUrl("rtsps://192.168.0.10:8555/test"));
    player->play();

    qDebug() << "✅ MainWindow 생성 완료";
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *parent = centralWidget();
    qDebug() << "📋 중앙 위젯 설정됨:" << (parent ? "성공" : "실패");

    topBar = new TopBarWidget(parent);

    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", parent);
    cameraTitle->setStyleSheet("font-weight: bold; font-size: 15px;");
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    notifTitleLabel = new QLabel("실시간 알림", parent);
    notifTitleLabel->setStyleSheet("font-weight: bold; font-size: 15px;");
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // QVideoWidget 사용
    videoWidget = new QVideoWidget(parent);
    videoWidget->setStyleSheet("background-color: black; border: 1px solid #ccc;");
    videoWidget->show();

    notificationPanel = new NotificationPanel(parent);
    notificationPanel->setStyleSheet("background: transparent; border: none;");

    videoSettingTitle = new QLabel("영상 설정", parent);
    videoSettingTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    videoSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    videoSettingLine = new QWidget(parent);
    videoSettingLine->setStyleSheet("background-color: #999;");

    displayTitle = new QLabel("화면 표시", parent);
    displayTitle->setStyleSheet("font-size: 13px; font-weight: bold;");
    displayTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    procTitle = new QLabel("영상 처리", parent);
    procTitle->setStyleSheet("font-size: 13px; font-weight: bold;");
    procTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    displayBox = new DisplaySettingBox(parent);
    displayBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

    procBox = new ProcSettingBox(parent);
    procBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

    // Z-Order 조정: 영상은 뒤로, UI는 앞으로
    videoWidget->lower();
    topBar->raise();
    cameraTitle->raise();
    notifTitleLabel->raise();
    notificationPanel->raise();
    videoSettingTitle->raise();
    videoSettingLine->raise();
    displayTitle->raise();
    procTitle->raise();
    displayBox->raise();
    procBox->raise();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    qDebug() << "📏 MainWindow 크기 변경 이벤트:" << event->size();
    QMainWindow::resizeEvent(event);
    updateLayout();
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

    double cctv_w = w_unit * 16.5;
    double padding = w_unit * 1;
    double middle_pad = w_unit * 0.5;
    double notif_w = w_unit * 5.0;
    double cctv_x = padding;
    double notif_x = cctv_x + cctv_w + middle_pad;

    cameraTitle->setGeometry(cctv_x, h_unit * 3, cctv_w, h_unit);
    notifTitleLabel->setGeometry(notif_x, h_unit * 3, notif_w, h_unit);

    // QVideoWidget 레이아웃 적용
    videoWidget->setGeometry(cctv_x, h_unit * 4, cctv_w, h_unit * 13);
    notificationPanel->setGeometry(notif_x, h_unit * 4, notif_w, h_unit * 19);

    double settingTop = h_unit * 17;
    double labelTop   = h_unit * 18;
    double boxTop     = h_unit * 19;

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

    qDebug() << "✅ 레이아웃 업데이트 완료";
}

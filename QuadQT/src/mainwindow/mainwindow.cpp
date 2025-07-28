#include "mainwindow/mainwindow.h"
#include "mainwindow/topbarwidget.h"
#include "mainwindow/displaysettingbox.h"

#include <QResizeEvent>
#include <QLabel>
#include <QWidget>
#include <QFont>
#include <QDebug>

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
    videoArea(nullptr),
    notificationPanel(nullptr)
{
    qDebug() << "🏠 MainWindow 생성자 시작";
    
    QWidget *centralW = new QWidget(this);
    centralW->setStyleSheet("background-color: white;"); // 배경색을 흰색으로 설정
    setCentralWidget(centralW);
    setMinimumSize(1600, 900);
    showMaximized();

    qDebug() << "🎨 UI 설정 시작";
    setupUI();
    
    qDebug() << "📐 레이아웃 업데이트 시작";
    updateLayout();
    
    qDebug() << "✅ MainWindow 생성 완료";
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *parent = centralWidget();
    qDebug() << "📋 중앙 위젯 설정됨:" << (parent ? "성공" : "실패");

    // 항상 생성 (조건 없이)
    topBar = new TopBarWidget(parent);
    qDebug() << "🔝 TopBar 생성됨:" << (topBar ? "성공" : "실패");

    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", parent);
    cameraTitle->setStyleSheet("font-weight: bold; font-size: 15px;");
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    qDebug() << "📹 카메라 제목 생성됨:" << (cameraTitle ? "성공" : "실패");

    notifTitleLabel = new QLabel("실시간 알림", parent);
    notifTitleLabel->setStyleSheet("font-weight: bold; font-size: 15px;");
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    qDebug() << "🔔 알림 제목 생성됨:" << (notifTitleLabel ? "성공" : "실패");

    videoArea = new QWidget(parent);
    videoArea->setStyleSheet("background-color: #e2e7ec; border: 1px solid #ccc;");
    qDebug() << "🎥 비디오 영역 생성됨:" << (videoArea ? "성공" : "실패");

    notificationPanel = new QWidget(parent);
    notificationPanel->setStyleSheet("background-color: #f8f9fa; border-left: 1px solid #ccc;");
    qDebug() << "📢 알림 패널 생성됨:" << (notificationPanel ? "성공" : "실패");

    videoSettingTitle = new QLabel("영상 설정", parent);
    videoSettingTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    videoSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    qDebug() << "⚙️ 영상 설정 제목 생성됨:" << (videoSettingTitle ? "성공" : "실패");

    videoSettingLine = new QWidget(parent);
    videoSettingLine->setStyleSheet("background-color: #999;");
    qDebug() << "➖ 영상 설정 라인 생성됨:" << (videoSettingLine ? "성공" : "실패");

    displayTitle = new QLabel("화면 표시", parent);
    displayTitle->setStyleSheet("font-size: 13px; font-weight: bold;");
    displayTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    qDebug() << "🖥️ 화면 표시 제목 생성됨:" << (displayTitle ? "성공" : "실패");

    procTitle = new QLabel("영상 처리", parent);
    procTitle->setStyleSheet("font-size: 13px; font-weight: bold;");
    procTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    qDebug() << "🔄 영상 처리 제목 생성됨:" << (procTitle ? "성공" : "실패");

    displayBox = new DisplaySettingBox(parent);
    displayBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    qDebug() << "📦 화면 설정 박스 생성됨:" << (displayBox ? "성공" : "실패");

    procBox = new QWidget(parent);
    procBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
    qDebug() << "🔧 처리 박스 생성됨:" << (procBox ? "성공" : "실패");
    
    // 모든 위젯을 명시적으로 보이도록 설정
    topBar->show();
    cameraTitle->show();
    notifTitleLabel->show();
    videoArea->show();
    notificationPanel->show();
    videoSettingTitle->show();
    videoSettingLine->show();
    displayTitle->show();
    procTitle->show();
    displayBox->show();
    procBox->show();
    
    qDebug() << "👁️ 모든 UI 요소 표시 설정 완료";
    qDebug() << "🎨 모든 UI 요소 생성 완료";
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
    qDebug() << "🔍 UI 요소 null 체크:";
    qDebug() << "  - topBar:" << (topBar ? "OK" : "NULL");
    qDebug() << "  - cameraTitle:" << (cameraTitle ? "OK" : "NULL");
    qDebug() << "  - notifTitleLabel:" << (notifTitleLabel ? "OK" : "NULL");
    qDebug() << "  - videoArea:" << (videoArea ? "OK" : "NULL");
    qDebug() << "  - notificationPanel:" << (notificationPanel ? "OK" : "NULL");
    
    if (!topBar || !cameraTitle || !notifTitleLabel || !videoArea || !notificationPanel) {
        qDebug() << "❌ 필수 UI 요소가 null이어서 레이아웃 업데이트 중단";
        return;
    }

    int w = width();
    int h = height();
    qDebug() << "📏 윈도우 크기: " << w << "x" << h;
    
    double w_unit = w / 24.0;
    double h_unit = h / 24.0;

    topBar->setGeometry(0, 0, w, h_unit * 3);
    topBar->updateLayout(w, h);
    qDebug() << "🔝 TopBar 레이아웃 설정됨";

    double cctv_w = w_unit * 16.5;
    double padding = w_unit * 1;
    double middle_pad = w_unit * 0.5;
    double notif_w = w_unit * 5.0;
    double cctv_x = padding;
    double notif_x = cctv_x + cctv_w + middle_pad;

    cameraTitle->setGeometry(cctv_x, h_unit * 3, cctv_w, h_unit);
    notifTitleLabel->setGeometry(notif_x, h_unit * 3, notif_w, h_unit);
    qDebug() << "📝 제목 레이블들 레이아웃 설정됨";

    videoArea->setGeometry(cctv_x, h_unit * 4, cctv_w, h_unit * 13);
    notificationPanel->setGeometry(notif_x, h_unit * 4, notif_w, h_unit * 19);
    qDebug() << "🎥 비디오 영역 및 알림 패널 레이아웃 설정됨";

    double settingTop = h_unit * 17;
    double labelTop   = h_unit * 18;
    double boxTop     = h_unit * 19;

    if (videoSettingTitle) {
        videoSettingTitle->setGeometry(cctv_x, settingTop, cctv_w, h_unit);
        qDebug() << "⚙️ 영상 설정 제목 레이아웃 설정됨";
    }
    
    if (videoSettingLine) {
        videoSettingLine->setGeometry(cctv_x, settingTop + h_unit - 1, cctv_w, 1);
        qDebug() << "➖ 영상 설정 라인 레이아웃 설정됨";
    }

    if (displayTitle) {
        displayTitle->setGeometry(cctv_x, labelTop, w_unit * 6, h_unit);
        qDebug() << "🖥️ 화면 표시 제목 레이아웃 설정됨";
    }
    
    if (procTitle) {
        procTitle->setGeometry(cctv_x + w_unit * (6 + 0.5), labelTop, w_unit * 10, h_unit);
        qDebug() << "🔄 영상 처리 제목 레이아웃 설정됨";
    }

    if (displayBox) {
        displayBox->setGeometry(cctv_x, boxTop, w_unit * 6, h_unit * 4);
        qDebug() << "📦 화면 설정 박스 레이아웃 설정됨";
    }
    
    if (procBox) {
        procBox->setGeometry(cctv_x + w_unit * (6 + 0.5), boxTop, w_unit * 10, h_unit * 4);
        qDebug() << "🔧 처리 박스 레이아웃 설정됨";
    }
    
    qDebug() << "✅ 레이아웃 업데이트 완료";
}

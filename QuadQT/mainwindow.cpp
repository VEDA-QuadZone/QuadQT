#include "mainwindow.h"
#include "topbarwidget.h"
#include "displaysettingbox.h"

#include <QResizeEvent>
#include <QLabel>
#include <QWidget>
#include <QFont>
#include "ui_mainwindow.h"
#include "loginpage.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    videoSettingTitle(nullptr),
    displayTitle(nullptr),
    procTitle(nullptr),
    displayBox(nullptr),
    procBox(nullptr),
    videoSettingLine(nullptr)
{
    setMinimumSize(1600, 900);
    setupUI();
    showMaximized();
    ui->setupUi(this);
    
    LoginPage *loginPage = new LoginPage(this);
    setCentralWidget(loginPage);
    
    // 로그인 성공 시그널 연결
    connect(loginPage, &LoginPage::loginSuccessful, this, &MainWindow::onLoginSuccessful);
    
    // LoginPage 크기에 맞춰 MainWindow 크기 조정
    setFixedSize(800, 600);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    topBar = new TopBarWidget(this);

    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", this);
    cameraTitle->setStyleSheet("font-weight: bold; font-size: 15px;");
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    notifTitleLabel = new QLabel("실시간 알림", this);
    notifTitleLabel->setStyleSheet("font-weight: bold; font-size: 15px;");
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    videoArea = new QWidget(this);
    videoArea->setStyleSheet("background-color: #e2e7ec; border: 1px solid #ccc;");

    notificationPanel = new QWidget(this);
    notificationPanel->setStyleSheet("background-color: #f8f9fa; border-left: 1px solid #ccc;");
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateLayout();
}

void MainWindow::updateLayout()
{
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

    videoArea->setGeometry(cctv_x, h_unit * 4, cctv_w, h_unit * 13);
    notificationPanel->setGeometry(notif_x, h_unit * 4, notif_w, h_unit * 19);

    //
    // ───── [최종 수정: 영상 설정 레이아웃 개선] ─────
    //

    double settingTop = h_unit * 17;     // 영상 설정 시작 (videoArea 아래)
    double labelTop   = h_unit * 18;     // 텍스트 라벨 (1칸 아래)
    double boxTop     = h_unit * 19;     // 박스 시작 (그 아래 3칸)


    // 영상 설정 텍스트
    if (!videoSettingTitle)
        videoSettingTitle = new QLabel("영상 설정", this);
    videoSettingTitle->setGeometry(cctv_x, settingTop, cctv_w, h_unit);
    videoSettingTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    videoSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 영상 설정 하단 라인 (QWidget로 선 구현)
    if (!videoSettingLine)
        videoSettingLine = new QWidget(this);
    videoSettingLine->setGeometry(cctv_x, settingTop + h_unit - 1, cctv_w, 1);  // 1px 선
    videoSettingLine->setStyleSheet("background-color: #999;");

    // 화면 표시 라벨
    if (!displayTitle)
        displayTitle = new QLabel("화면 표시", this);
    displayTitle->setGeometry(cctv_x, labelTop, w_unit * 6, h_unit);
    displayTitle->setStyleSheet("font-size: 13px; font-weight: bold;");
    displayTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // 영상 처리 라벨
    if (!procTitle)
        procTitle = new QLabel("영상 처리", this);
    procTitle->setGeometry(cctv_x + w_unit * (6 + 0.5), labelTop, w_unit * 10, h_unit);
    procTitle->setStyleSheet("font-size: 13px; font-weight: bold;");
    procTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // 화면 표시 박스
    if (!displayBox)
        displayBox = new DisplaySettingBox(this);
    displayBox->setGeometry(cctv_x, boxTop, w_unit * 6, h_unit * 4);
    displayBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

    // 영상 처리 박스
    if (!procBox)
        procBox = new QWidget(this);
    procBox->setGeometry(cctv_x + w_unit * (6 + 0.5), boxTop, w_unit * 10, h_unit * 4);
    procBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

}


void MainWindow::onLoginSuccessful()
{
    // 로그인 성공 후 메인 애플리케이션 화면으로 전환
    QLabel *welcomeLabel = new QLabel("로그인 성공! 메인 애플리케이션 화면입니다.", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 18px; color: #333333;");
    setCentralWidget(welcomeLabel);
}

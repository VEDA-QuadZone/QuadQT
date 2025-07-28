#include "mainwindow.h"
#include "topbarwidget.h"
#include "displaysettingbox.h"

#include <QResizeEvent>
#include <QLabel>
#include <QWidget>
#include <QFont>

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
    setCentralWidget(new QWidget(this));
    setMinimumSize(1600, 900);
    showMaximized();

    setupUI();
    updateLayout();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *parent = centralWidget();

    // 항상 생성 (조건 없이)
    topBar = new TopBarWidget(parent);

    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", parent);
    cameraTitle->setStyleSheet("font-weight: bold; font-size: 15px;");
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    notifTitleLabel = new QLabel("실시간 알림", parent);
    notifTitleLabel->setStyleSheet("font-weight: bold; font-size: 15px;");
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    videoArea = new QWidget(parent);
    videoArea->setStyleSheet("background-color: #e2e7ec; border: 1px solid #ccc;");

    notificationPanel = new QWidget(parent);
    notificationPanel->setStyleSheet("background-color: #f8f9fa; border-left: 1px solid #ccc;");

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

    procBox = new QWidget(parent);
    procBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateLayout();
}

void MainWindow::updateLayout()
{
    if (!topBar || !cameraTitle || !notifTitleLabel || !videoArea || !notificationPanel)
        return;

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

    double settingTop = h_unit * 17;
    double labelTop   = h_unit * 18;
    double boxTop     = h_unit * 19;

    videoSettingTitle->setGeometry(cctv_x, settingTop, cctv_w, h_unit);
    videoSettingLine->setGeometry(cctv_x, settingTop + h_unit - 1, cctv_w, 1);

    displayTitle->setGeometry(cctv_x, labelTop, w_unit * 6, h_unit);
    procTitle->setGeometry(cctv_x + w_unit * (6 + 0.5), labelTop, w_unit * 10, h_unit);

    displayBox->setGeometry(cctv_x, boxTop, w_unit * 6, h_unit * 4);
    procBox->setGeometry(cctv_x + w_unit * (6 + 0.5), boxTop, w_unit * 10, h_unit * 4);
}

#include "mainwindow.h"
#include <QResizeEvent>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(1600, 900);   // 최소 크기 제한
    setupUI();                   // 위젯 생성
    showMaximized();             // 실행 시 전체화면
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    logoLabel = new QLabel("QuardZone", this);
    QFont logoFont;
    logoFont.setBold(true);
    logoLabel->setFont(logoFont);
    logoLabel->setStyleSheet("color: orange;");

    cameraIcon = new QLabel("CAM", this);
    docIcon = new QLabel("DOC", this);
    settingIcon = new QLabel("SET", this);
    loginStatus = new QLabel("USER", this);

    QList<QLabel*> icons = {cameraIcon, docIcon, settingIcon, loginStatus};
    for (auto icon : icons) {
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("font-size: 14px; font-weight: bold; background-color: #eee; border: 1px solid #ccc;");
    }

    topLine = new QWidget(this);
    topLine->setStyleSheet("background-color: #ccc;");

    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", this);
    cameraTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    notifTitleLabel = new QLabel("Notifications", this);
    notifTitleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    videoArea = new QWidget(this);
    videoArea->setStyleSheet("background-color: #e2e7ec; border: 1px solid #ccc;");

    notificationPanel = new QWidget(this);
    notificationPanel->setStyleSheet("background-color: #f8f9fa; border-left: 1px solid #ccc;");

    bottomControlBar = new QWidget(this);

    bottomLeftPanel = new QWidget(bottomControlBar);
    bottomLeftPanel->setStyleSheet("background-color: #ffffff; border: 1px solid #ccc;");

    bottomRightPanel = new QWidget(bottomControlBar);
    bottomRightPanel->setStyleSheet("background-color: #f5f5f5; border: 1px solid #ccc;");
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

    logoLabel->setGeometry(w_unit * 1, h_unit * 1, w_unit * 4, h_unit * 2);
    QFont logoFont = logoLabel->font();
    logoFont.setPointSizeF(h_unit * 0.65);
    logoLabel->setFont(logoFont);

    topLine->setGeometry(w_unit * 1, h_unit * 3 - 1, w_unit * 22, 1);

    cameraIcon->setGeometry(w_unit * 9, h_unit * 1, w_unit * 2, h_unit * 2);
    docIcon->setGeometry(w_unit * 11, h_unit * 1, w_unit * 2, h_unit * 2);
    settingIcon->setGeometry(w_unit * 13, h_unit * 1, w_unit * 2, h_unit * 2);
    loginStatus->setGeometry(w_unit * 21.0, h_unit * 1, w_unit * 2, h_unit * 2);

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

    bottomControlBar->setGeometry(cctv_x, h_unit * 18, cctv_w, h_unit * 5);
    bottomLeftPanel->setGeometry(0, 0, cctv_w * 0.8, h_unit * 5);
    bottomRightPanel->setGeometry(cctv_w * 0.8, 0, cctv_w * 0.2, h_unit * 5);
}

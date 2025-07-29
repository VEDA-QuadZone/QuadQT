#include "mainwindow/mainwindow.h"
#include "mainwindow/topbarwidget.h"
#include "mainwindow/displaysettingbox.h"
#include "mainwindow/procsettingbox.h"
#include "mainwindow/notificationpanel.h"

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

    setupUI();
    setupFonts();
    setupPages();
    updateLayout();
    showPage(PageType::Camera);

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
    QFont defaultFont("HanwhaGothicR", 12);
    QFont titleFont("HanwhaGothicR", 15);
    titleFont.setBold(true);

    setFont(defaultFont);
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

    QFont titleFont("HanwhaGothicR", 15);
    titleFont.setBold(true);

    cameraTitle = new QLabel("역삼 초등학교 앞 CCTV", page);
    cameraTitle->setFont(titleFont);
    cameraTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    notifTitleLabel = new QLabel("실시간 알림", page);
    notifTitleLabel->setFont(titleFont);
    notifTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    videoArea = new QLabel(page);
    QPixmap videoPixmap(":/images/resources/images/video.png");
    if (!videoPixmap.isNull()) {
        videoArea->setPixmap(videoPixmap);
        videoArea->setScaledContents(true);
        videoArea->setAlignment(Qt::AlignCenter);
        videoArea->setStyleSheet("border: 1px solid #ccc; background-color: #000;");
    } else {
        videoArea->setText("비디오 영역\n(video.png 로드 실패)");
        videoArea->setAlignment(Qt::AlignCenter);
        videoArea->setStyleSheet("background-color: #e2e7ec; border: 1px solid #ccc; font-size: 16px; color: #666;");
    }

    notificationPanel = new NotificationPanel(page);
    notificationPanel->setStyleSheet("background-color: #f8f9fa; border-left: 1px solid #ccc;");

    QFont settingTitleFont("HanwhaGothicR", 16); settingTitleFont.setBold(true);
    QFont subTitleFont("HanwhaGothicR", 13);     subTitleFont.setBold(true);

    videoSettingTitle = new QLabel("영상 설정", page);
    videoSettingTitle->setFont(settingTitleFont);
    videoSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    videoSettingLine = new QWidget(page);
    videoSettingLine->setStyleSheet("background-color: #999;");

    displayTitle = new QLabel("화면 표시", page);
    displayTitle->setFont(subTitleFont);
    displayTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    procTitle = new QLabel("영상 처리", page);
    procTitle->setFont(subTitleFont);
    procTitle->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    displayBox = new DisplaySettingBox(page);
    displayBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

    procBox = new ProcSettingBox(page);
    procBox->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

    return page;
}

QWidget* MainWindow::createDocumentPage()
{
    QWidget *page = new QWidget();
    page->setStyleSheet("background-color: #f5f5f5;");

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
    page->setStyleSheet("background-color: #f0f8ff;");

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

void MainWindow::onCameraClicked()  { showPage(PageType::Camera); }
void MainWindow::onDocumentClicked(){ showPage(PageType::Document); }
void MainWindow::onSettingsClicked(){ showPage(PageType::Settings); }

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
    if (!topBar || !stackedWidget) return;

    int w = width();
    int h = height();
    double h_unit = h / 24.0;

    topBar->setGeometry(0, 0, w, h_unit * 3);
    topBar->updateLayout(w, h);

    stackedWidget->setGeometry(0, h_unit * 3, w, h - h_unit * 3);

    if (stackedWidget->currentWidget() == cameraPage && cameraTitle && videoArea)
        updateCameraPageLayout();
}

void MainWindow::updateCameraPageLayout()
{
    if (!cameraTitle || !notifTitleLabel || !videoArea || !notificationPanel) return;

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

    // ✅ 위치 위로 올림
    cameraTitle->setGeometry(cctv_x, h_unit * 0, cctv_w, h_unit);
    notifTitleLabel->setGeometry(notif_x, h_unit * 0, notif_w, h_unit);
    videoArea->setGeometry(cctv_x, h_unit * 1, cctv_w, h_unit * 13);
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


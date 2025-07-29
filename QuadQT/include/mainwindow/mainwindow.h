#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QCloseEvent>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include "mainwindow/topbarwidget.h"
#include "mainwindow/notificationpanel.h"
#include "mainwindow/procsettingbox.h"

class QLabel;
class DisplaySettingBox;

enum class PageType {
    Camera,
    Document,
    Settings
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public:
    void setUserEmail(const QString &email);

private:
    void setupUI();
    void setupFonts();
    void updateLayout();
    void setupPages();
    void showPage(PageType pageType);
    void updateCameraPageLayout();

    QWidget* createCameraPage();
    QWidget* createDocumentPage();
    QWidget* createSettingsPage();

    // 공통 UI 요소
    TopBarWidget *topBar;
    QStackedWidget *stackedWidget;

    // 카메라 페이지 구성 요소
    QLabel *cameraTitle;
    QLabel *notifTitleLabel;

    // RTSP 스트리밍
    QMediaPlayer *player;
    QVideoWidget *videoWidget;

    // 알림 패널 및 설정
    NotificationPanel *notificationPanel;
    QLabel *videoSettingTitle;
    QLabel *displayTitle;
    QLabel *procTitle;
    DisplaySettingBox *displayBox;
    ProcSettingBox *procBox;
    QWidget *videoSettingLine;

    // 페이지
    QWidget *cameraPage;
    QWidget *documentPage;
    QWidget *settingsPage;

    // 로그아웃 상태
    bool m_isLogout;

private slots:
    void onCameraClicked();
    void onDocumentClicked();
    void onSettingsClicked();
    void onLogoutRequested();
};

#endif // MAINWINDOW_H

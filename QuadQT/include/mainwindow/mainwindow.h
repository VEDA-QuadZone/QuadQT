#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QCloseEvent>
#include <QShowEvent>
#include <QThread>
#include <QtMultimediaWidgets/QVideoWidget>
#include "mainwindow/topbarwidget.h"
#include "mainwindow/notificationpanel.h"
#include "mainwindow/procsettingbox.h"
#include "mainwindow/historyview.h"
#include "mainwindow/mqttmanager.h"
#include "mainwindow/rtspplayer.h"
#include "login/networkmanager.h"
#include "mainwindow/rtspthread.h"

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
    void showEvent(QShowEvent *event) override;

public:
    void setUserEmail(const QString &email);
    bool wasLogout() const;  // 로그아웃 여부 확인용

private:
    void setupUI();
    void setupFonts();
    void updateLayout();
    void setupPages();
    void showPage(PageType pageType);
    void updateCameraPageLayout();
    void forceLayoutUpdate();

    QWidget* createCameraPage();
    QWidget* createDocumentPage();
    QWidget* createSettingsPage();

    // 공통 UI 요소
    TopBarWidget *topBar;
    QStackedWidget *stackedWidget;

    // 카메라 페이지 구성 요소
    QLabel *cameraTitle;
    QLabel *notifTitleLabel;

    // 영상 설정 라벨 및 구분선
    QLabel *videoSettingTitle;
    QLabel *displayTitle;
    QLabel *procTitle;
    QWidget *videoSettingLine;

    // RTSP 스트리밍
    QLabel *rtspLabel;
    RtspThread *rtspThread;

    // 알림 패널 및 설정
    NotificationPanel *notificationPanel;
    DisplaySettingBox *displayBox;
    ProcSettingBox *procBox;

    // MQTT
    MqttManager *mqttManager;

    // 페이지들
    QWidget *cameraPage;
    QWidget *documentPage;
    QWidget *settingsPage;

    // 네트워크 매니저
    NetworkManager *networkManager;

    // 로그아웃 상태
    bool m_isLogout;

    // 기록 뷰
    HistoryView *historyView;

private slots:
    void onCameraClicked();
    void onDocumentClicked();
    void onSettingsClicked();
    void onLogoutRequested();
};

#endif // MAINWINDOW_H

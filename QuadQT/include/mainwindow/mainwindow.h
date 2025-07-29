#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QCloseEvent>

#include "mainwindow/topbarwidget.h"
#include "mainwindow/notificationpanel.h"
#include "mainwindow/procsettingbox.h"
#include "mainwindow/historyview.h"
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
    void setUserEmail(const QString &email);  // <- 🔓 public으로 변경

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

    // 카메라 페이지 UI 구성요소
    QLabel *cameraTitle;
    QLabel *notifTitleLabel;
    QLabel *videoArea;  // QLabel로 고정
    NotificationPanel *notificationPanel;
    QLabel *videoSettingTitle;
    QLabel *displayTitle;
    QLabel *procTitle;
    DisplaySettingBox *displayBox;
    ProcSettingBox *procBox;
    QWidget *videoSettingLine;

    // 페이지들
    QWidget *cameraPage;
    QWidget *documentPage;
    QWidget *settingsPage;

    // 로그아웃 플래그
    bool m_isLogout;
private:
    HistoryView *historyView;
private slots:
    void onCameraClicked();
    void onDocumentClicked();
    void onSettingsClicked();
    void onLogoutRequested();
};

#endif // MAINWINDOW_H

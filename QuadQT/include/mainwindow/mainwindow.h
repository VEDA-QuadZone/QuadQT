#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QCloseEvent>
#include "mainwindow/topbarwidget.h"  // TopBarWidget 사용

class QLabel;
class QWidget;
class DisplaySettingBox;  // displayBox의 실제 타입이 이거라면 미리 선언

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
    
    void setUserEmail(const QString &email);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onCameraClicked();
    void onDocumentClicked();
    void onSettingsClicked();
    void onLogoutRequested();

private:
    void setupUI();
    void setupFonts();
    void updateLayout();
    void setupPages();
    void showPage(PageType pageType);
    void updateCameraPageLayout();
    
    // 페이지 생성 함수들
    QWidget* createCameraPage();
    QWidget* createDocumentPage();
    QWidget* createSettingsPage();

    TopBarWidget *topBar;
    QStackedWidget *stackedWidget;
    
    // 카메라 페이지 구성 요소들
    QLabel *cameraTitle;
    QLabel *notifTitleLabel;
    QLabel *videoArea;  // QWidget에서 QLabel로 변경
    QWidget *notificationPanel;
    QLabel *videoSettingTitle;
    QLabel *displayTitle;
    QLabel *procTitle;
    DisplaySettingBox *displayBox;
    QWidget *procBox;
    QWidget *videoSettingLine;
    
    // 페이지들
    QWidget *cameraPage;
    QWidget *documentPage;
    QWidget *settingsPage;
    
    // 로그아웃 플래그
    bool m_isLogout;
};

#endif // MAINWINDOW_H

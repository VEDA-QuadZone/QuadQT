#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();         // 위젯 생성 (1회)
    void updateLayout();    // 리사이즈 시 배치 업데이트 ← 이 줄 추가됨

    // ───── 상단 바 구성 요소 ─────
    QLabel *logoLabel;
    QLabel *cameraIcon;
    QLabel *docIcon;
    QLabel *settingIcon;
    QLabel *loginStatus;
    QWidget *topLine;

    // ───── 영상 및 알림 제목 ─────
    QLabel *cameraTitle;       // "역삼 초등학교 앞 CCTV"
    QLabel *notifTitleLabel;   // "Notifications"

    // ───── 메인 콘텐츠 영역 ─────
    QWidget *videoArea;        // CCTV 영상 표시 영역
    QWidget *notificationPanel; // 알림 패널 영역

    // ───── 하단 제어 패널 ─────
    QWidget *bottomControlBar;
    QWidget *bottomLeftPanel;
    QWidget *bottomRightPanel;
};

#endif // MAINWINDOW_H

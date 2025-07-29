#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mainwindow/topbarwidget.h"  // TopBarWidget 사용
#include "mainwindow/notificationpanel.h"
#include "mainwindow/procsettingbox.h"
#include "mainwindow/videowidget.h"
#include "mainwindow/videoplayer.h"

class QLabel;
class QWidget;
class DisplaySettingBox;  // displayBox의 실제 타입이 이거라면 미리 선언


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void updateLayout();

    TopBarWidget *topBar;

    QLabel *cameraTitle;
    QLabel *notifTitleLabel;

    // ───── RTSP 스트리밍 ─────
    VideoWidget *videoArea;
    VideoPlayer *player;

    // ───── Notification ─────
    NotificationPanel *notificationPanel;

    // ───── 영상 설정 관련 구성 요소 ─────
    QLabel *videoSettingTitle;   // "영상 설정"
    QLabel *displayTitle;        // "화면 표시"
    QLabel *procTitle;           // "영상 처리"
    DisplaySettingBox *displayBox; // 화면 표시용 박스
    ProcSettingBox *procBox;          // 영상 처리용 박스
    QWidget *videoSettingLine;   // 영상 설정 아래 수평선
};

#endif // MAINWINDOW_H

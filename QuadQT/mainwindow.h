// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "topbarwidget.h"  // ✅ 직접 include 추가

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
private slots:
    void onLoginSuccessful();

private:
    void setupUI();
    void updateLayout();

    TopBarWidget *topBar;  // ✅ 정의에 문제 없음

    QLabel *cameraTitle;
    QLabel *notifTitleLabel;

    QWidget *videoArea;
    QWidget *notificationPanel;

    // ───── 영상 설정 관련 구성 요소 ─────
    QLabel *videoSettingTitle;  // "영상 설정"
    QLabel *displayTitle;       // "화면 표시"
    QLabel *procTitle;          // "영상 처리"
    QWidget *displayBox;        // 화면 표시용 박스
    QWidget *procBox;           // 영상 처리용 박스
    QWidget *videoSettingLine; // 영상 설정 아래 수평선
};

#endif // MAINWINDOW_H

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QImage>

class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void displayFrame(const QImage &frame);   // 프레임 표시 함수

protected:
    void resizeEvent(QResizeEvent *event) override; // 크기 변경 시 처리

private:
    QLabel *videoLabel;    // 실제 영상이 출력될 QLabel
    QLabel *statusLabel;    // 상태 안내문구 QLabel
    bool firstFrameReceived = false;
};

#endif // VIDEOWIDGET_H

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QImage>
#include <QTimer>
#include <opencv2/opencv.hpp>

class VideoPlayer : public QObject {
    Q_OBJECT
public:
    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();

    void startStream(const std::string &url);   // RTSP 시작
    void stopStream();                         // 스트림 종료

signals:
    void frameReady(const QImage &frame);      // 새 프레임이 준비되면 시그널로 전달

private slots:
    void updateFrame();                        // QTimer로 호출되는 프레임 갱신

private:
    cv::VideoCapture cap;                      // OpenCV 캡처 객체
    QTimer *timer;                             // 주기적으로 프레임 갱신
};

#endif // VIDEOPLAYER_H

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QImage>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <chrono>

class VideoPlayer : public QObject {
    Q_OBJECT
public:
    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();

    void startStream(const std::string &url);
    void stopStream();

signals:
    void frameReady(const QImage &frame);

private slots:
    void updateFrame();

private:
    cv::VideoCapture cap;
    QTimer *timer;

    // FPS 계산용 멤버 변수
    int frameCount = 0;
    std::chrono::steady_clock::time_point lastTime;
};

#endif // VIDEOPLAYER_H

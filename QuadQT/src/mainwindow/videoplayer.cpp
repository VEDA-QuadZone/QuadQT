#include "mainwindow/videoplayer.h"
#include <QDebug>

VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent), timer(new QTimer(this)) {
    connect(timer, &QTimer::timeout, this, &VideoPlayer::updateFrame);
}

VideoPlayer::~VideoPlayer() {
    stopStream();
}

void VideoPlayer::startStream(const std::string &url) {
    // RTSP 스트림 열기
    cap.open(url, cv::CAP_FFMPEG);

    // 안정적 FPS를 위한 옵션
    cap.set(cv::CAP_PROP_BUFFERSIZE, 2);            // 버퍼 최소화 (FPS 안정성)
    cap.set(cv::CAP_PROP_FPS, 30);                  // 30fps 힌트
    cap.set(cv::CAP_PROP_OPEN_TIMEOUT_MSEC, 5000);  // 타임아웃 설정

    if (!cap.isOpened()) {
        qWarning() << "RTSP stream open failed!";
        return;
    }

    timer->start(33); // 30fps (1000ms / 30 ≈ 33ms)
}

void VideoPlayer::stopStream() {
    timer->stop();
    if (cap.isOpened()) cap.release();
}

void VideoPlayer::updateFrame() {
    cv::Mat frame;
    if (cap.read(frame)) {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        emit frameReady(img.copy());
    }
}

#include "mainwindow/videoplayer.h"
#include <QDebug>
#include <chrono>

using namespace std::chrono;

VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent),
    timer(new QTimer(this))
{
    connect(timer, &QTimer::timeout, this, &VideoPlayer::updateFrame);
}

VideoPlayer::~VideoPlayer() {
    stopStream();
}

void VideoPlayer::startStream(const std::string &url) {
    qDebug() << "[VideoPlayer] startStream() 진입";
    cap.open(url, cv::CAP_FFMPEG);

    // FPS 안정성 옵션
    cap.set(cv::CAP_PROP_BUFFERSIZE, 2);
    cap.set(cv::CAP_PROP_FPS, 30);
    cap.set(cv::CAP_PROP_OPEN_TIMEOUT_MSEC, 5000);

    if (!cap.isOpened()) {
        qWarning() << "[VideoPlayer] RTSP 스트림 열기 실패";
        return;
    }

    frameCount = 0;
    lastTime = steady_clock::now();

    qDebug() << "[VideoPlayer] RTSP 스트림 연결 성공";
    timer->start(33); // ~30fps
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

        // ===== FPS 디버그 출력 =====
        frameCount++;
        auto now = steady_clock::now();
        auto elapsed = duration_cast<seconds>(now - lastTime).count();
        if (elapsed >= 1) {
            qDebug() << "[VideoPlayer] 현재 FPS:" << frameCount;
            frameCount = 0;
            lastTime = now;
        }
    }
}

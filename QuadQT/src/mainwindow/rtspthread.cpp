#include "mainwindow/rtspthread.h"
#include <opencv2/opencv.hpp>
#include <QDebug>

RtspThread::RtspThread(const QString& url, QObject *parent)
    : QThread(parent), m_url(url), m_running(true)
{
}

RtspThread::~RtspThread()
{
    stop();
    wait();  // 쓰레드 종료 대기
}

void RtspThread::stop()
{
    m_running = false;
}

void RtspThread::run()
{
    cv::VideoCapture cap;

    // RTSP 지연 최소화 설정
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    if (!cap.open(m_url.toStdString(), cv::CAP_FFMPEG)) {
        qWarning() << "❌ RTSP 스트림 열기 실패:" << m_url;
        return;
    }

    qDebug() << "✅ RTSP 스트림 연결 성공:" << m_url;

    cv::Mat frame;
    while (m_running) {
        cap >> frame;
        if (frame.empty()) {
            msleep(10);  // 다음 프레임 대기
            continue;
        }

        // BGR → RGB → QImage 변환
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        emit frameReady(image.copy());  // emit 전에 copy() 필수

        msleep(10);  // 너무 빠른 루프 방지
    }

    cap.release();
    qDebug() << "🛑 RTSP 쓰레드 종료됨";
}

#include "mainwindow/rtspthread.h"
#include <opencv2/opencv.hpp>
#include <QDebug>

RtspThread::RtspThread(const QString& url, QObject *parent)
    : QThread(parent), m_url(url), m_running(true)
{
    m_refreshTimer.start();  // 타이머 시작
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

    while (m_running) {
        // RTSP 지연 최소화 설정
        cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

        if (!cap.open(m_url.toStdString(), cv::CAP_FFMPEG)) {
            qWarning() << "❌ RTSP 스트림 열기 실패:" << m_url;
            msleep(1000);  // 1초 대기 후 재시도
            continue;
        }

        qDebug() << "✅ RTSP 스트림 연결 성공:" << m_url;
        m_refreshTimer.restart();  // 연결 성공 시 타이머 재시작

        cv::Mat frame;
        while (m_running) {
            // 7초마다 스트림 새로고침
            if (m_refreshTimer.elapsed() >= REFRESH_INTERVAL_MS) {
                qDebug() << "🔄 RTSP 스트림 7초 새로고침";
                cap.release();
                break;  // 내부 루프 종료하여 재연결
            }

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
    }

    qDebug() << "🛑 RTSP 쓰레드 종료됨";
}

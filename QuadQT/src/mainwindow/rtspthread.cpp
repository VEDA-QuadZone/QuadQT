#include "mainwindow/rtspthread.h"
#include <opencv2/opencv.hpp>
#include <QDebug>

RtspThread::RtspThread(const QString& url, QObject *parent)
    : QThread(parent), m_url(url), m_running(true)
{
    m_refreshTimer.start();  // 새로고침 타이머 시작
}

RtspThread::~RtspThread()
{
    stop();
    wait();  // 스레드 종료 대기
}

void RtspThread::stop()
{
    m_running = false;
}

void RtspThread::run()
{
    cv::VideoCapture cap;

    while (m_running) {
        // RTSP 지연 최소화를 위한 버퍼 크기 설정
        cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
        
        // RTSPS (RTSP over SSL) 연결을 위한 SSL 인증서 환경 변수 설정
        if (m_url.startsWith("rtsps://")) {
            qDebug() << "[RTSP] RTSPS 보안 연결을 위한 SSL 설정";
            qputenv("SSL_CERT_FILE", "ca.cert.pem");
            qputenv("SSL_CERT_DIR", ".");
            // 클라이언트 인증서 설정 (상호 인증 필요 시)
            qputenv("SSL_CLIENT_CERT_FILE", "client.cert.pem");
            qputenv("SSL_CLIENT_KEY_FILE", "client.key.pem");
        }

        // RTSP 스트림 연결 시도
        if (!cap.open(m_url.toStdString(), cv::CAP_FFMPEG)) {
            qWarning() << "[RTSP] 스트림 연결 실패:" << m_url;
            msleep(1000);  // 1초 대기 후 재시도
            continue;
        }

        qDebug() << "[RTSP] 스트림 연결 성공:" << m_url;
        m_refreshTimer.restart();  // 연결 성공 시 타이머 재시작

        cv::Mat frame;
        while (m_running) {
            // 7초마다 스트림 새로고침 (연결 안정성 향상)
            if (m_refreshTimer.elapsed() >= REFRESH_INTERVAL_MS) {
                qDebug() << "[RTSP] 스트림 새로고침 (7초 주기)";
                cap.release();
                break;  // 내부 루프 종료하여 재연결
            }

            // 프레임 읽기
            cap >> frame;
            if (frame.empty()) {
                msleep(10);  // 다음 프레임 대기
                continue;
            }

            // OpenCV BGR 형식을 Qt RGB 형식으로 변환
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            emit frameReady(image.copy());  // 프레임 준비 완료 시그널 발생

            msleep(10);  // CPU 사용률 조절
        }

        cap.release();
    }

    qDebug() << "[RTSP] 스레드 종료";
}

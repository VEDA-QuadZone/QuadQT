#include "mainwindow/rtspthread.h"
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QElapsedTimer>

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

        // RTSPS (RTSP over SSL)를 위한 SSL 인증서 경로 설정
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

        qDebug() << "✅ RTSP 스트림 연결 성공:" << m_url;
        m_refreshTimer.restart();

        cv::Mat frame;
        int emptyFrameCount = 0;

        while (m_running) {
            cap >> frame;

            if (frame.empty()) {
                emptyFrameCount++;
                if (emptyFrameCount > 30) {  // 약 300ms 이상 연속 실패
                    qWarning() << "⚠️ 연속 프레임 수신 실패, 스트림 재연결 시도";
                    cap.release();
                    break;
                }
                msleep(10);  // 프레임 없을 때만 쉬기
                continue;
            }

            emptyFrameCount = 0;  // 정상 프레임 수신 시 초기화

            // BGR → RGB → QImage 변환
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            emit frameReady(image.copy());  // emit 전에 copy() 필수
        }

        cap.release();
    }

    qDebug() << "[RTSP] 스레드 종료";
}

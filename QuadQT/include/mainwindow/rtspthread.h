#pragma once

#include <QThread>
#include <QImage>
#include <QString>
#include <QMutex>
#include <QElapsedTimer>
#include <atomic>

class RtspThread : public QThread
{
    Q_OBJECT

public:
    explicit RtspThread(const QString& url, QObject *parent = nullptr);
    ~RtspThread();

    void stop();

signals:
    void frameReady(const QImage &frame);  // 새로운 프레임 도착 시 시그널

protected:
    void run() override;  // QThread 메인 루프

private:
    QString m_url;
    std::atomic<bool> m_running;
    QMutex m_mutex;
    QElapsedTimer m_refreshTimer;  // 5초마다 새로고침을 위한 타이머
    static const int REFRESH_INTERVAL_MS = 5000;  // 5초
};

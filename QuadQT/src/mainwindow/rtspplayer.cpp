#include "RtspPlayer.h"
#include <QDebug>
#include <QDateTime>
#include <QImage>

RtspPlayer::RtspPlayer(QObject* parent)
    : QObject(parent),
    player(new QMediaPlayer(this)),
    sink(new QVideoSink(this))
{
    player->setVideoSink(sink);

    // 프레임 수신 시그널 → 신호만 방출
    connect(sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame){
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        emit frameReceived(frame, now);
    });

    qDebug() << "[RtspPlayer] 초기화 완료 (UI 의존 제거)";
}

RtspPlayer::~RtspPlayer()
{
    stop();
    qDebug() << "[RtspPlayer] 해제";
}

void RtspPlayer::setUrl(const QString& url)
{
    rtspUrl = url;
    qDebug() << "[RtspPlayer] RTSP URL 설정:" << rtspUrl;
}

void RtspPlayer::start()
{
    if (rtspUrl.isEmpty()) {
        qWarning() << "[RtspPlayer] URL이 비어있어 재생할 수 없음";
        return;
    }
    player->setSource(QUrl(rtspUrl));
    player->play();
    qDebug() << "[RtspPlayer] 재생 시작";
}

void RtspPlayer::stop()
{
    player->stop();
    qDebug() << "[RtspPlayer] 재생 중지";
}

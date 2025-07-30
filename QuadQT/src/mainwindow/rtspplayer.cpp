#include "RtspPlayer.h"
#include <QDebug>

RtspPlayer::RtspPlayer(QVideoWidget* outputWidget, QObject* parent)
    : QObject(parent),
    player(new QMediaPlayer(this)),
    videoWidget(outputWidget)
{
    if (videoWidget) {
        player->setVideoOutput(videoWidget);
    }
    qDebug() << "[RtspPlayer] 초기화 완료";
}

RtspPlayer::~RtspPlayer()
{
    player->stop();
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

#ifndef RTSPPLAYER_H
#define RTSPPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QString>

class RtspPlayer : public QObject
{
    Q_OBJECT
public:
    explicit RtspPlayer(QObject* parent = nullptr);
    ~RtspPlayer();

    void setUrl(const QString &url);

public slots:
    void start();
    void stop();

signals:
    void frameReceived(const QVideoFrame &frame, qint64 recvTime);

private:
    QMediaPlayer *player;
    QVideoSink *sink;
    QString rtspUrl;
};

#endif // RTSPPLAYER_H

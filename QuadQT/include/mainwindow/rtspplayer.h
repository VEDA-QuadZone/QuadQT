#ifndef RTSPPLAYER_H
#define RTSPPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QLabel>
#include <QString>

class RtspPlayer : public QObject
{
    Q_OBJECT
public:
    explicit RtspPlayer(QLabel* outputLabel, QObject* parent = nullptr);
    ~RtspPlayer();

    void setUrl(const QString& url);
    void start();
    void stop();

signals:
    void frameReceived(const QVideoFrame &frame, qint64 recvTime);

private:
    QMediaPlayer* player;
    QVideoSink* sink;
    QLabel* videoLabel;
    QString rtspUrl;
};

#endif // RTSPPLAYER_H

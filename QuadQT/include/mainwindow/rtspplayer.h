#ifndef RTSPPLAYER_H
#define RTSPPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QString>

class RtspPlayer : public QObject
{
    Q_OBJECT
public:
    explicit RtspPlayer(QVideoWidget* outputWidget, QObject* parent = nullptr);
    ~RtspPlayer();

    void setUrl(const QString& url);
    void start();
    void stop();

private:
    QMediaPlayer* player;
    QVideoWidget* videoWidget;
    QString rtspUrl;
};

#endif // RTSPPLAYER_H

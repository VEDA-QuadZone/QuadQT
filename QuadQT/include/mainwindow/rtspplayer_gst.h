#ifndef RTSPPLAYER_GST_H
#define RTSPPLAYER_GST_H

#include <QObject>
#include <QWidget>
#include <QImage>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

class RtspPlayerGst : public QObject
{
    Q_OBJECT
public:
    explicit RtspPlayerGst(QWidget* outputWidget, QObject* parent = nullptr);
    ~RtspPlayerGst();

    void start(const QString& url);
    void stop();

signals:
    void frameReceived(const QImage &img, qint64 recvTime);

private:
    QWidget* videoWidget;
    GstElement* pipeline;
    QString rtspUrl;
};

#endif // RTSPPLAYER_GST_H

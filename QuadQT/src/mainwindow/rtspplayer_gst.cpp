#include "mainwindow/rtspplayer_gst.h"
#include <QDebug>
#include <QDateTime>

// RtspPlayerGst::RtspPlayerGst(QLabel* outputLabel, QObject* parent)
//     : QObject(parent), videoLabel(outputLabel), pipeline(nullptr)
// {
//     gst_init(nullptr, nullptr);
//     qDebug() << "[RTSP] GStreamer initialized";
// }

RtspPlayerGst::RtspPlayerGst(QWidget* outputWidget, QObject* parent)
    : QObject(parent), videoWidget(outputWidget), pipeline(nullptr)
{
    gst_init(nullptr, nullptr);
    qDebug() << "[RTSP] GStreamer initialized";
}

RtspPlayerGst::~RtspPlayerGst()
{
    stop();
}

void RtspPlayerGst::start(const QString& url)
{
    rtspUrl = url;
    QString pipelineStr = QString(
                              "rtspsrc location=%1 latency=50 protocols=tcp tls-validation-flags=0 "
                              "! rtph264depay "
                              "! h264parse config-interval=1 disable-passthrough=true "
                              "! avdec_h264 "
                              "! videoconvert "
                              "! queue max-size-buffers=1 leaky=downstream "
                              "! glimagesink sync=false").arg(rtspUrl);

    GError* err = nullptr;
    pipeline = gst_parse_launch(pipelineStr.toUtf8().constData(), &err);
    if (!pipeline) {
        qWarning() << "[RTSP] Pipeline creation failed:" << err->message;
        g_error_free(err);
        return;
    }

    // QWidget 핸들 연결
    // if (videoLabel) {
    //     WId winId = videoLabel->winId();
    //     GstElement* sink = gst_bin_get_by_interface(GST_BIN(pipeline), GST_TYPE_VIDEO_OVERLAY);
    //     if (sink) {
    //         gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), (guintptr)winId);
    //         qDebug() << "[RTSP] Video sink connected to QLabel";
    //         gst_object_unref(sink);
    //     }
    // }


    WId winId = videoWidget->winId();
    qDebug() << "[RTSP] Video sink connected to QWidget, winId =" << winId;

    GstElement *sink = gst_bin_get_by_interface(GST_BIN(pipeline), GST_TYPE_VIDEO_OVERLAY);
    if (sink) {
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), (guintptr)winId);
        gst_object_unref(sink);
    } else {
        qWarning() << "[RTSP] No video sink found!";
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    qDebug() << "[RTSP] Pipeline started:" << url;
}

void RtspPlayerGst::stop()
{
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
        qDebug() << "[RTSP] Pipeline stopped";
    }
}

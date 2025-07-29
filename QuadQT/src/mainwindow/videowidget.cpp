#include "mainwindow/videowidget.h"
#include <QPixmap>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent),
    videoLabel(new QLabel(this)),
    statusLabel(new QLabel("스트리밍 연결중입니다...", this))
{
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("background-color: black;");
    videoLabel->setGeometry(0, 0, width(), height());

    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold; background-color:  transparent;");
    statusLabel->setGeometry(0, 0, width(), height());
}

void VideoWidget::displayFrame(const QImage &frame) {
    videoLabel->setPixmap(QPixmap::fromImage(frame).scaled(
        videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    if (!firstFrameReceived) {
        statusLabel->hide();           // 첫 프레임 수신 시 안내문구 숨김
        firstFrameReceived = true;
    }
}

void VideoWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    videoLabel->setGeometry(0, 0, width(), height());
    statusLabel->setGeometry(0, 0, width(), height());

    QPixmap pixmap = videoLabel->pixmap();
    if (!pixmap.isNull()) {
        videoLabel->setPixmap(pixmap.scaled(
            videoLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }
}

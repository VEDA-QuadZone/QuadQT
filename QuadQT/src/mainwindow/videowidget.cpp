#include "mainwindow/videowidget.h"
#include <QPixmap>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent),
    videoLabel(new QLabel(this))
{
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("background-color: black;");
    videoLabel->setGeometry(0, 0, width(), height());
}

void VideoWidget::displayFrame(const QImage &frame) {
    videoLabel->setPixmap(QPixmap::fromImage(frame).scaled(
        videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));
}

void VideoWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    videoLabel->setGeometry(0, 0, width(), height());

    QPixmap pixmap = videoLabel->pixmap();
    if (!pixmap.isNull()) {
        videoLabel->setPixmap(pixmap.scaled(
            videoLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }
}

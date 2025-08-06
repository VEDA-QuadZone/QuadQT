#include "mainwindow/overlaywidget.h"
#include <QPainter>
#include <QColor>

OverlayWidget::OverlayWidget(QWidget *parent)
    : QWidget(parent), opacity_(128) // 50% 투명도
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setStyleSheet("background: transparent;");
}

void OverlayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 반투명 검은색 오버레이
    QColor overlayColor(0, 0, 0, opacity_);
    painter.fillRect(rect(), overlayColor);
}
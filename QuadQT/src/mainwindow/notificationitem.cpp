#include "mainwindow/notificationitem.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

NotificationItem::NotificationItem(int eventType, const QString &date, QWidget *parent)
    : QWidget(parent), eventType(eventType)
{
    setFixedHeight(92);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleSheet("background-color: orange; border-radius: 8px;");

    QHBoxLayout *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(12, 12, 12, 12);
    outerLayout->setSpacing(8);

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setSpacing(4);

    messageLabel = new QLabel("알림 내용", this);
    dateLabel = new QLabel(date, this);

    messageLabel->setStyleSheet("font-weight: bold; font-size: 14px; background: transparent;");
    dateLabel->setStyleSheet("color: #555; font-size: 12px; background: transparent;");

    textLayout->addWidget(messageLabel);
    textLayout->addWidget(dateLabel);

    outerLayout->addLayout(textLayout);
}

NotificationItem::~NotificationItem() {}

void NotificationItem::onRemoveClicked()
{
    emit removeRequested(this);
}

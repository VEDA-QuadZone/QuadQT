#include "mainwindow/notificationpanel.h"
#include "mainwindow/notificationitem.h"
#include <QVBoxLayout>

NotificationPanel::NotificationPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 12, 0, 12);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignTop);

    auto *item = new NotificationItem(0, "2025-07-28 16:00", this);
    layout->addWidget(item);

    connect(item, &NotificationItem::removeRequested, this, &NotificationPanel::removeNotification);

    mainLayout = layout;
}

NotificationPanel::~NotificationPanel() {}

void NotificationPanel::removeNotification(NotificationItem *item)
{
    mainLayout->removeWidget(item);
    item->deleteLater();
}

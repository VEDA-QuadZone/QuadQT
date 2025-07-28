#include "mainwindow/notificationpanel.h"
#include "mainwindow/notificationitem.h"
#include <QVBoxLayout>

NotificationPanel::NotificationPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 12, 0, 12);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);

    auto *item = new NotificationItem(0, "2025-07-28 16:00", this);
    layout->addWidget(item);

    connect(item, &NotificationItem::removeRequested, this, &NotificationPanel::removeNotification);

    // dummy data
    layout->addWidget(new NotificationItem(2, "2025-07-28 16:00", this)); // 어린이
    layout->addWidget(new NotificationItem(1, "2025-07-28 15:55", this)); // 과속 차량
    layout->addWidget(new NotificationItem(0, "2025-07-28 15:50", this)); // 불법 주정차

    mainLayout = layout;
}

NotificationPanel::~NotificationPanel() {}

void NotificationPanel::removeNotification(NotificationItem *item)
{
    mainLayout->removeWidget(item);
    item->deleteLater();
}

#include "mainwindow/notificationpanel.h"
#include "mainwindow/notificationitem.h"
#include <QVBoxLayout>
#include <QLabel>

NotificationPanel::NotificationPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 12, 0, 12);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);

    mainLayout = layout;

    // ✅ 알림 없음 라벨 생성 (위쪽 정렬)
    emptyLabel = new QLabel("현재 알림이 없습니다", this);
    emptyLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop); // 왼쪽 위 정렬

    // 기본 마진/패딩 제거
    emptyLabel->setContentsMargins(0,0,0,0);
    emptyLabel->setStyleSheet("color: #666; font-size: 14px; margin: 0; padding: 0;");
    layout->addWidget(emptyLabel);

    // ✅ 더미 데이터 추가
    addNotification(0, "2025-07-28 16:00");
    addNotification(2, "2025-07-28 16:00");
    addNotification(1, "2025-07-28 15:55");
    addNotification(0, "2025-07-28 15:50");
}

NotificationPanel::~NotificationPanel() {}

void NotificationPanel::addNotification(int eventType, const QString &date)
{
    if (emptyLabel) emptyLabel->hide(); // ✅ 알림이 생기면 숨김

    auto *item = new NotificationItem(eventType, date, this);
    connect(item, &NotificationItem::removeRequested,
            this, &NotificationPanel::removeNotification);

    mainLayout->addWidget(item);
}

void NotificationPanel::removeNotification(NotificationItem *item)
{
    mainLayout->removeWidget(item);
    item->deleteLater();

    // ✅ 남은 알림이 있는지 확인
    bool hasItems = false;
    for (int i = 0; i < mainLayout->count(); ++i) {
        if (qobject_cast<NotificationItem*>(mainLayout->itemAt(i)->widget())) {
            hasItems = true;
            break;
        }
    }

    // ✅ 없으면 emptyLabel 다시 표시
    if (!hasItems && emptyLabel) {
        emptyLabel->show();
    }
}

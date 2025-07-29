#include "mainwindow/notificationpanel.h"
#include "mainwindow/notificationitem.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

NotificationPanel::NotificationPanel(QWidget *parent)
    : QWidget(parent)
{
    // 🔧 외곽선 제거 및 전체 배경 설정 (패널 자체)
    this->setStyleSheet("background-color: transparent; border: none;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);   // 🔧 여백 제거
    layout->setSpacing(0);                   // 🔧 간격 제거
    layout->setAlignment(Qt::AlignTop);

    mainLayout = layout;

    // 알림 없음 라벨
    emptyLabel = new QLabel("현재 알림이 없습니다", this);
    emptyLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    emptyLabel->setContentsMargins(0, 0, 0, 0);
    emptyLabel->setStyleSheet("color: #666; font-size: 14px; margin: 0; padding: 0;");
    layout->addWidget(emptyLabel);

    // 더미 데이터
    // addNotification(0, "2025-07-28 16:00");
    // addNotification(2, "2025-07-28 16:00");
    // addNotification(1, "2025-07-28 15:55");
    // addNotification(0, "2025-07-28 15:50");
}

NotificationPanel::~NotificationPanel() {}

void NotificationPanel::handleMqttMessage(const QByteArray &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    int eventType = obj.value("event").toInt(-1);
    QString timestamp = obj.value("timestamp").toString();

    // 시간 포맷 변환 (ISO8601 → yyyy-MM-dd HH:mm)
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
    QString formattedDate = dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm") : timestamp;

    addNotification(eventType, formattedDate);
}

void NotificationPanel::addNotification(int eventType, const QString &date)
{
    if (emptyLabel) emptyLabel->hide();

    auto *item = new NotificationItem(eventType, date, this);

    // ❌ 스타일 지정 안 함 → 내부 배경색 유지됨

    connect(item, &NotificationItem::removeRequested,
            this, &NotificationPanel::removeNotification);

    mainLayout->addWidget(item);
}

void NotificationPanel::removeNotification(NotificationItem *item)
{
    mainLayout->removeWidget(item);
    item->deleteLater();

    bool hasItems = false;
    for (int i = 0; i < mainLayout->count(); ++i) {
        if (qobject_cast<NotificationItem*>(mainLayout->itemAt(i)->widget())) {
            hasItems = true;
            break;
        }
    }

    if (!hasItems && emptyLabel) {
        emptyLabel->show();
    }
}

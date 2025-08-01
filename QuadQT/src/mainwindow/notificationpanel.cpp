#include "mainwindow/notificationpanel.h"
#include "mainwindow/notificationitem.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

NotificationPanel::NotificationPanel(QWidget *parent)
    : QWidget(parent)
{
    // 패널 자체 스타일 설정
    this->setStyleSheet("background-color: transparent; border: none;");

    // 메인 레이아웃 (패널 전체)
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // 스크롤 영역 생성
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background-color: #f0f0f0;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: #c0c0c0;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #a0a0a0;
        }
    )");

    // 스크롤 내부 컨테이너
    container = new QWidget();
    container->setStyleSheet("background-color: transparent; border: none;");
    
    // 컨테이너 내부 레이아웃
    mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2); // 알림 간 간격
    mainLayout->setAlignment(Qt::AlignTop);

    // 알림 없음 라벨
    emptyLabel = new QLabel("현재 알림이 없습니다", container);
    emptyLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    emptyLabel->setContentsMargins(8, 8, 8, 8);
    emptyLabel->setStyleSheet("color: #666; font-size: 14px;");
    mainLayout->addWidget(emptyLabel);

    // 스크롤 영역에 컨테이너 설정
    scrollArea->setWidget(container);
    outerLayout->addWidget(scrollArea);
}

NotificationPanel::~NotificationPanel() {}

void NotificationPanel::handleMqttMessage(const QByteArray &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    int eventType = obj.value("event").toInt(-1);

    // test 메시지 등 특정 eventType 무시
    if (eventType == -1)  // 99는 예시, 실제 테스트 메시지 event 번호 넣으세요
        return;

    QString timestamp = obj.value("timestamp").toString();

    // 시간 포맷 변환 (ISO8601 → yyyy-MM-dd HH:mm)
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
    QString formattedDate = dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm") : timestamp;

    addNotification(eventType, formattedDate);
}
void NotificationPanel::addNotification(int eventType, const QString &date)
{
    if (emptyLabel) emptyLabel->hide();

    auto *item = new NotificationItem(eventType, date, container);

    connect(item, &NotificationItem::removeRequested,
            this, &NotificationPanel::removeNotification);

    // 새 알림을 맨 위에 추가 (index 0에 삽입)
    mainLayout->insertWidget(0, item);
    
    // 스크롤을 맨 위로 이동 (새 알림이 보이도록)
    scrollArea->verticalScrollBar()->setValue(0);
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

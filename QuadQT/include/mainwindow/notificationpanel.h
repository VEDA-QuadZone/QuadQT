#ifndef NOTIFICATIONPANEL_H
#define NOTIFICATIONPANEL_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class QScrollArea;
class NotificationItem;  // 알림 항목 클래스 전방 선언

class NotificationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationPanel(QWidget *parent = nullptr);
    ~NotificationPanel();

    void addNotification(int eventType, const QString &date);

public slots:
    void handleMqttMessage(const QByteArray &message);

private slots:
    void removeNotification(NotificationItem *item);

private:
    QScrollArea *scrollArea;
    QWidget *container;
    QVBoxLayout *mainLayout;
    QLabel *emptyLabel;
};

#endif // NOTIFICATIONPANEL_H

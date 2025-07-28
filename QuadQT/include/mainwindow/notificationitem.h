#ifndef NOTIFICATIONITEM_H
#define NOTIFICATIONITEM_H

#include <QWidget>

class QLabel;
class QPushButton;

class NotificationItem : public QWidget
{
    Q_OBJECT

public:
    NotificationItem(int eventType, const QString &date, QWidget *parent = nullptr);
    ~NotificationItem();

signals:
    void removeRequested(NotificationItem *item);

private slots:
    //void onRemoveClicked();

private:
    QLabel *messageLabel;
    QLabel *dateLabel;
    QPushButton *removeBtn;
    int eventType;
};

#endif // NOTIFICATIONITEM_H

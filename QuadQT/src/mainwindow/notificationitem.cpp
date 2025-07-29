#include "mainwindow/notificationitem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>

NotificationItem::NotificationItem(int eventType, const QString &date, QWidget *parent)
    : QWidget(parent), eventType(eventType)
{
    setFixedHeight(72);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_StyledBackground, true);

    QString bgColor;
    QString lineColor;
    QString message;
    QString iconPath;

    switch (eventType) {
    case 0:
        bgColor = "rgba(241, 194, 27, 0.1)";  // 연한 노랑
        lineColor = "#F1C21B";
        message = "불법 주정차 차량 인식";
        iconPath = ":/images/images/caution.png";
        break;
    case 1:
        bgColor = "rgba(218, 30, 40, 0.1)";   // 연한 빨강
        lineColor = "#DA1E28";
        message = "과속 차량 인식";
        iconPath = ":/images/images/prohibition.png";
        break;
    case 2:
        bgColor = "rgba(2, 68, 207, 0.1)";    // 연한 파랑
        lineColor = "#0244CF";
        message = "어린이 인식";
        iconPath = ":/images/images/info.png";
        break;
    default:
        bgColor = "rgba(224, 224, 224, 0.8)";
        lineColor = "#CCCCCC";
        message = "알 수 없는 이벤트";
        iconPath = "";
        break;
    }

    setStyleSheet(QString(
                      "background-color: %1;"
                      "border: none;"
                      "border-radius: 0;"
                      ).arg(bgColor));

    QHBoxLayout *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(8);

    QFrame *leftLine = new QFrame(this);
    leftLine->setFixedWidth(6);
    leftLine->setStyleSheet(QString("background-color: %1;").arg(lineColor));
    outerLayout->addWidget(leftLine);

    // 아이콘 이미지 로드
    QLabel *iconLabel = new QLabel(this);
    QPixmap iconPixmap(iconPath);
    iconLabel->setPixmap(iconPixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background: transparent;");

    QWidget *iconContainer = new QWidget(this);
    iconContainer->setStyleSheet("background: transparent;");
    iconContainer->setFixedWidth(50);  // 이미지 사이즈 고려

    QVBoxLayout *iconLayout = new QVBoxLayout(iconContainer);
    iconLayout->setContentsMargins(0, 12, 0, 0);
    iconLayout->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignHCenter);

    outerLayout->addWidget(iconContainer);

    QWidget *textContainer = new QWidget(this);
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 8, 0, 0);
    textLayout->setSpacing(4);
    textLayout->setAlignment(Qt::AlignTop);
    textContainer->setStyleSheet("background: transparent;");

    messageLabel = new QLabel(message, this);
    messageLabel->setStyleSheet("font-weight: bold; font-size: 14px; background: transparent;");

    dateLabel = new QLabel(date, this);
    dateLabel->setStyleSheet("font-size: 14px; margin: 0; padding: 0; background: transparent; color: #333;");

    textLayout->addWidget(messageLabel, 0, Qt::AlignTop);
    textLayout->addWidget(dateLabel, 0, Qt::AlignTop);

    textContainer->setLayout(textLayout);
    outerLayout->addWidget(textContainer, 1);

    QPushButton *removeBtn = new QPushButton("✕", this);
    removeBtn->setFixedSize(20, 20);
    removeBtn->setStyleSheet(
        "QPushButton { border:none; background:transparent; font-size:14px; }"
        );

    connect(removeBtn, &QPushButton::clicked, this, [this]() {
        emit removeRequested(this);
    });

    QWidget *btnContainer = new QWidget(this);
    btnContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *btnLayout = new QVBoxLayout(btnContainer);
    btnLayout->setContentsMargins(0, 8, 8, 0);
    btnLayout->addWidget(removeBtn, 0, Qt::AlignTop);

    outerLayout->addWidget(btnContainer);
}

NotificationItem::~NotificationItem() {}

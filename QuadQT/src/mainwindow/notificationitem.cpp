#include "mainwindow/notificationitem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

NotificationItem::NotificationItem(int eventType, const QString &date, QWidget *parent)
    : QWidget(parent), eventType(eventType)
{
    setFixedHeight(92);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_StyledBackground, true);

    QString bgColor;
    QString lineColor;
    QString message;
    QString iconSymbol;

    switch (eventType) {
    case 0:
        bgColor = "rgba(241, 194, 27, 0.8)";
        lineColor = "#F1C21B";
        message = "불법 주정차 차량 인식";
        iconSymbol = "⚠";
        break;
    case 1:
        bgColor = "rgba(243, 115, 34, 0.8)";
        lineColor = "#F37322";
        message = "과속 차량 인식";
        iconSymbol = "🚫";
        break;
    case 2:
        bgColor = "rgba(251, 181, 132, 0.8)";
        lineColor = "#FBB584";
        message = "어린이 인식";
        iconSymbol = "ℹ";
        break;
    default:
        bgColor = "rgba(224, 224, 224, 0.8)";
        lineColor = "#CCCCCC";
        message = "알 수 없는 이벤트";
        iconSymbol = "?";
        break;
    }

    setStyleSheet(QString(
                      "background-color: %1;"
                      "border-radius: 0;"
                      ).arg(bgColor));

    // === 전체 수평 레이아웃 ===
    QHBoxLayout *outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(8);

    // 1) 좌측 굵은 컬러 라인
    QFrame *leftLine = new QFrame(this);
    leftLine->setFixedWidth(6);  // 굵은 라인
    leftLine->setStyleSheet(QString("background-color: %1;").arg(lineColor));
    outerLayout->addWidget(leftLine);

    // 2) 아이콘
    QLabel *iconLabel = new QLabel(iconSymbol, this);
    iconLabel->setStyleSheet("font-size: 20px; color: #333; background: transparent;");

    // 아이콘 컨테이너
    QWidget *iconContainer = new QWidget(this);
    iconContainer->setStyleSheet("background: transparent;");
    iconContainer->setFixedWidth(40);   // ✅ width 고정 (예: 40px)

    QVBoxLayout *iconLayout = new QVBoxLayout(iconContainer);
    iconLayout->setContentsMargins(0, 12, 0, 0);         // ✅ 상단 마진 제거
    iconLayout->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignHCenter);


    outerLayout->addWidget(iconContainer);

    // 3) 텍스트 영역 (VBox)
    QWidget *textContainer = new QWidget(this);
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 12, 0, 0);
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
    outerLayout->addWidget(textContainer, 1); // 가변 폭

    // X 버튼
    QPushButton *removeBtn = new QPushButton("✕", this);
    removeBtn->setFixedSize(20, 20);
    removeBtn->setStyleSheet(
        "QPushButton { border:none; background:transparent; font-size:14px; }"
        );

    // 컨테이너
    QWidget *btnContainer = new QWidget(this);
    btnContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *btnLayout = new QVBoxLayout(btnContainer);
    btnLayout->setContentsMargins(0, 12, 8, 0);  // ✅ 오른쪽 8px, 위쪽 12px 마진
    btnLayout->addWidget(removeBtn, 0, Qt::AlignTop);

    outerLayout->addWidget(btnContainer);
}

NotificationItem::~NotificationItem() {}

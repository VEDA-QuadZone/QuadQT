#include "mainwindow/displaysettingbox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QPixmap>

DisplaySettingBox::DisplaySettingBox(QWidget *parent)
    : QWidget(parent), objectBoxOn(false), timestampOn(false)
{
    this->setStyleSheet("border: 1px solid #ccc; background-color: transparent;");
    setupUI();
}

void DisplaySettingBox::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 객체 박스 위젯
    objectBoxWidget = new QWidget(this);
    objectBoxWidget->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *objectLayout = new QVBoxLayout(objectBoxWidget);
    objectLayout->setContentsMargins(8, 8, 8, 8);
    objectLayout->setSpacing(4);

    objectIcon = new QLabel(objectBoxWidget);
    objectIcon->setAlignment(Qt::AlignCenter);
    objectIcon->setStyleSheet("border: none; background: transparent;");

    objectLabel = new QLabel("객체 박스", objectBoxWidget);
    objectLabel->setAlignment(Qt::AlignCenter);
    objectLabel->setStyleSheet("font-size: 14px; color: #aaa; border: none; background: transparent;");

    objectLayout->addStretch();
    objectLayout->addWidget(objectIcon);
    objectLayout->addStretch();
    objectLayout->addWidget(objectLabel);

    // 타임스탬프 위젯
    timestampWidget = new QWidget(this);
    timestampWidget->setStyleSheet("border-left: 1px solid #ccc;");
    QVBoxLayout *timestampLayout = new QVBoxLayout(timestampWidget);
    timestampLayout->setContentsMargins(8, 8, 8, 8);
    timestampLayout->setSpacing(4);

    timestampIcon = new QLabel(timestampWidget);
    timestampIcon->setAlignment(Qt::AlignCenter);
    timestampIcon->setStyleSheet("border: none; background: transparent;");

    timestampLabel = new QLabel("타임 스탬프", timestampWidget);
    timestampLabel->setAlignment(Qt::AlignCenter);
    timestampLabel->setStyleSheet("font-size: 14px; color: #aaa; border: none; background: transparent;");

    timestampLayout->addStretch();
    timestampLayout->addWidget(timestampIcon);
    timestampLayout->addStretch();
    timestampLayout->addWidget(timestampLabel);

    mainLayout->addWidget(objectBoxWidget);
    mainLayout->addWidget(timestampWidget);

    objectBoxWidget->installEventFilter(this);
    timestampWidget->installEventFilter(this);

    updateObjectBoxUI();
    updateTimestampUI();
}

bool DisplaySettingBox::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == objectBoxWidget) {
            objectBoxOn = !objectBoxOn;
            updateObjectBoxUI();

            // 객체 박스 상태 변경 → 명령 전송
            QString cmd = QString("CHANGE_FRAME 0 %1").arg(objectBoxOn ? 1 : 0);
            emit requestCommand(cmd);

            return true;
        } else if (watched == timestampWidget) {
            timestampOn = !timestampOn;
            updateTimestampUI();

            // 타임스탬프 상태 변경 → 명령 전송
            QString cmd = QString("CHANGE_FRAME 1 %1").arg(timestampOn ? 1 : 0);
            emit requestCommand(cmd);

            return true;
        }
    } else if (event->type() == QEvent::Enter) {
        if (watched == objectBoxWidget || watched == timestampWidget) {
            static_cast<QWidget *>(watched)->setStyleSheet("background-color: #FCE8D9; border: 1px solid #ccc;");
        }
    } else if (event->type() == QEvent::Leave) {
        if (watched == objectBoxWidget || watched == timestampWidget) {
            QString borderStyle = (watched == objectBoxWidget)
            ? "border-right: 1px solid #ccc;"
            : "border-left: 1px solid #ccc;";
            static_cast<QWidget *>(watched)->setStyleSheet("background-color: transparent; " + borderStyle);
        }
    }

    return QWidget::eventFilter(watched, event);
}


void DisplaySettingBox::updateObjectBoxUI()
{
    QString iconPath = objectBoxOn ? ":/images/objectbox_orange.png" : ":/images/objectbox_gray.png";
    objectIcon->setPixmap(QPixmap(iconPath).scaled(130, 130, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    objectIcon->setStyleSheet("border: none; background: transparent;");
    objectLabel->setStyleSheet(QString("font-size: 14px; color: %1; border: none; background: transparent;")
                                   .arg(objectBoxOn ? "#F37321" : "#aaa"));
}

void DisplaySettingBox::updateTimestampUI()
{
    QString iconPath = timestampOn ? ":/images/timestamp_orange.png" : ":/images/timestamp_gray.png";
    timestampIcon->setPixmap(QPixmap(iconPath));
    timestampLabel->setStyleSheet(QString("font-size: 14px; color: %1; border: none; background: transparent;")
                                      .arg(timestampOn ? "#F37321" : "#aaa"));
}

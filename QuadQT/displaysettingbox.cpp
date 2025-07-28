#include "displaysettingbox.h"
#include <QHBoxLayout>
#include <QLabel>

DisplaySettingBox::DisplaySettingBox(QWidget *parent)
    : QWidget(parent)
{
    // 전체 박스 테두리
    this->setStyleSheet("border: 1px solid #ccc; background-color: transparent;");
    setupUI();
}

void DisplaySettingBox::setupUI()
{
    QLabel *left = new QLabel("🚗\n객체 박스", this);
    left->setAlignment(Qt::AlignCenter);
    left->setStyleSheet(
        "font-size: 32px; color: #aaa; line-height: 120%; "
        "border-right: none;");

    QLabel *right = new QLabel("⏱️\n타임 스탬프", this);
    right->setAlignment(Qt::AlignCenter);
    right->setStyleSheet(
        "font-size: 32px; color: #aaa; line-height: 120%; "
        "border-left: none;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(left);
    layout->addWidget(right);

    this->setStyleSheet("border: 1px solid #ccc;");
}


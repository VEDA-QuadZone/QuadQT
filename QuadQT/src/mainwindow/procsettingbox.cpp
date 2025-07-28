#include "mainwindow/procsettingbox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSlider>
#include <QPushButton>

ProcSettingBox::ProcSettingBox(QWidget *parent)
    : QWidget(parent)
{
    this->setStyleSheet("border: 1px solid #ccc; background-color: transparent;");
    setupUI();
}

void ProcSettingBox::setupUI()
{
    QButtonGroup *group = new QButtonGroup(this);

    // ───── 공통 라디오 버튼 스타일 ─────
    const QString radioStyle = R"(
        QRadioButton {
            background: transparent;
            outline: none;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
            background: transparent;
            border: none;
        }
    )";

    // ───── 주간 모드 박스 ─────
    QWidget *dayBox = new QWidget(this);
    dayBox->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *dayLayout = new QVBoxLayout(dayBox);
    dayLayout->setContentsMargins(10, 10, 10, 10);
    dayLayout->setSpacing(5);
    dayLayout->setAlignment(Qt::AlignTop);

    QRadioButton *dayRadio = new QRadioButton(this);
    dayRadio->setStyleSheet(radioStyle);
    group->addButton(dayRadio);

    QHBoxLayout *dayRadioRow = new QHBoxLayout;
    dayRadioRow->addWidget(dayRadio);
    dayRadioRow->addStretch();

    QLabel *dayIcon = new QLabel("🌞", this);
    dayIcon->setAlignment(Qt::AlignCenter);
    dayIcon->setStyleSheet("font-size: 32px; background: transparent; border: none;");

    QLabel *dayLabel = new QLabel("주간 모드", this);
    dayLabel->setAlignment(Qt::AlignCenter);
    dayLabel->setStyleSheet("color: orange; font-size: 14px; background: transparent; border: none;");

    dayLayout->addLayout(dayRadioRow);
    dayLayout->addWidget(dayIcon);
    dayLayout->addWidget(dayLabel);

    // ───── 야간 모드 박스 ─────
    QWidget *nightBox = new QWidget(this);
    nightBox->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *nightLayout = new QVBoxLayout(nightBox);
    nightLayout->setContentsMargins(10, 10, 10, 10);
    nightLayout->setSpacing(5);
    nightLayout->setAlignment(Qt::AlignTop);

    QRadioButton *nightRadio = new QRadioButton(this);
    nightRadio->setStyleSheet(radioStyle);
    group->addButton(nightRadio);

    QHBoxLayout *nightRadioRow = new QHBoxLayout;
    nightRadioRow->addWidget(nightRadio);
    nightRadioRow->addStretch();

    QLabel *nightIcon = new QLabel("🌙", this);
    nightIcon->setAlignment(Qt::AlignCenter);
    nightIcon->setStyleSheet("font-size: 32px; background: transparent; border: none;");

    QLabel *nightLabel = new QLabel("야간 모드", this);
    nightLabel->setAlignment(Qt::AlignCenter);
    nightLabel->setStyleSheet("color: gray; font-size: 14px; background: transparent; border: none;");

    nightLayout->addLayout(nightRadioRow);
    nightLayout->addWidget(nightIcon);
    nightLayout->addWidget(nightLabel);

    // ───── 선명도 박스 ─────
    QWidget *sharpBox = new QWidget(this);
    QVBoxLayout *sharpLayout = new QVBoxLayout(sharpBox);
    sharpLayout->setContentsMargins(10, 10, 10, 10);
    sharpLayout->setSpacing(5);
    sharpLayout->setAlignment(Qt::AlignTop);

    QRadioButton *sharpRadio = new QRadioButton(this);
    sharpRadio->setStyleSheet(radioStyle);
    group->addButton(sharpRadio);
    sharpRadio->setChecked(true);  // ✅ 기본 선택

    QPushButton *minusBtn = new QPushButton("-");
    QPushButton *plusBtn = new QPushButton("+");
    minusBtn->setFixedWidth(30);
    plusBtn->setFixedWidth(30);
    minusBtn->setStyleSheet("border: none; background: transparent;");
    plusBtn->setStyleSheet("border: none; background: transparent;");

    QSlider *sharpSlider = new QSlider(Qt::Horizontal);
    sharpSlider->setFixedWidth(120);
    sharpSlider->setRange(0, 100);
    sharpSlider->setValue(0);
    sharpSlider->setEnabled(true);  // 기본 활성화

    QLabel *valueLabel = new QLabel("0", this);
    valueLabel->setFixedWidth(30);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("font-weight: bold; background: transparent; border: none;");

    QLabel *sharpLabel = new QLabel("선명도 (%)", this);
    sharpLabel->setAlignment(Qt::AlignCenter);
    sharpLabel->setStyleSheet("color: black; font-size: 14px; background: transparent; border: none;");

    QHBoxLayout *sharpTopRow = new QHBoxLayout;
    sharpTopRow->addWidget(sharpRadio);
    sharpTopRow->addStretch();

    QHBoxLayout *sliderRow = new QHBoxLayout;
    sliderRow->setAlignment(Qt::AlignCenter);
    sliderRow->setSpacing(5);
    sliderRow->addWidget(minusBtn);
    sliderRow->addWidget(sharpSlider);
    sliderRow->addWidget(plusBtn);
    sliderRow->addWidget(valueLabel);

    sharpLayout->addLayout(sharpTopRow);
    sharpLayout->addLayout(sliderRow);
    sharpLayout->addWidget(sharpLabel);

    // ───── 전체 레이아웃 구성 ─────
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(dayBox);
    mainLayout->addWidget(nightBox);
    mainLayout->addWidget(sharpBox);
    mainLayout->setStretch(0, 2);
    mainLayout->setStretch(1, 2);
    mainLayout->setStretch(2, 6);

    // ───── 동작 연결 ─────
    connect(sharpRadio, &QRadioButton::toggled, this, [=](bool checked){
        minusBtn->setEnabled(checked);
        plusBtn->setEnabled(checked);
        sharpSlider->setEnabled(checked);
        valueLabel->setEnabled(checked);
        sharpLabel->setStyleSheet(QString("color: %1; font-size: 14px; background: transparent; border: none;")
                                      .arg(checked ? "black" : "gray"));
    });

    connect(sharpSlider, &QSlider::valueChanged, valueLabel, [=](int value){
        valueLabel->setText(QString::number(value));
    });

    connect(minusBtn, &QPushButton::clicked, this, [=](){
        int val = sharpSlider->value();
        if (val > 0) sharpSlider->setValue(val - 1);
    });

    connect(plusBtn, &QPushButton::clicked, this, [=](){
        int val = sharpSlider->value();
        if (val < 100) sharpSlider->setValue(val + 1);
    });
}

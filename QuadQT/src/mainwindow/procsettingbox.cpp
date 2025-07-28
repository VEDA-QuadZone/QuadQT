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

    // ───── 주간 모드 박스 ─────
    QWidget *dayBox = new QWidget(this);
    dayBox->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *dayLayout = new QVBoxLayout(dayBox);
    dayLayout->setAlignment(Qt::AlignCenter);
    dayLayout->setContentsMargins(10, 10, 10, 10);
    dayLayout->setSpacing(5);

    QRadioButton *dayRadio = new QRadioButton(this);
    group->addButton(dayRadio);
    QLabel *dayIcon = new QLabel("🌞", this);
    dayIcon->setAlignment(Qt::AlignCenter);
    dayIcon->setStyleSheet("font-size: 32px;");
    QLabel *dayLabel = new QLabel("주간 모드", this);
    dayLabel->setAlignment(Qt::AlignCenter);
    dayLabel->setStyleSheet("color: orange; font-size: 14px;");

    dayLayout->addWidget(dayRadio, 0, Qt::AlignLeft);
    dayLayout->addWidget(dayIcon);
    dayLayout->addWidget(dayLabel);

    // ───── 야간 모드 박스 ─────
    QWidget *nightBox = new QWidget(this);
    nightBox->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *nightLayout = new QVBoxLayout(nightBox);
    nightLayout->setAlignment(Qt::AlignCenter);
    nightLayout->setContentsMargins(10, 10, 10, 10);
    nightLayout->setSpacing(5);

    QRadioButton *nightRadio = new QRadioButton(this);
    group->addButton(nightRadio);
    QLabel *nightIcon = new QLabel("🌙", this);
    nightIcon->setAlignment(Qt::AlignCenter);
    nightIcon->setStyleSheet("font-size: 32px;");
    QLabel *nightLabel = new QLabel("야간 모드", this);
    nightLabel->setAlignment(Qt::AlignCenter);
    nightLabel->setStyleSheet("color: gray; font-size: 14px;");

    nightLayout->addWidget(nightRadio, 0, Qt::AlignLeft);
    nightLayout->addWidget(nightIcon);
    nightLayout->addWidget(nightLabel);

    // ───── 선명도 박스 ─────
    QWidget *sharpBox = new QWidget(this);
    QVBoxLayout *sharpLayout = new QVBoxLayout(sharpBox);
    sharpLayout->setAlignment(Qt::AlignCenter);
    sharpLayout->setContentsMargins(10, 10, 10, 10);
    sharpLayout->setSpacing(5);

    QRadioButton *sharpRadio = new QRadioButton(this);
    group->addButton(sharpRadio);

    QPushButton *minusBtn = new QPushButton("-");
    QPushButton *plusBtn = new QPushButton("+");
    QSlider *sharpSlider = new QSlider(Qt::Horizontal);
    QLabel *valueLabel = new QLabel("0", this);
    QLabel *sharpLabel = new QLabel("선명도 (%)", this);

    minusBtn->setFixedWidth(30);
    plusBtn->setFixedWidth(30);
    sharpSlider->setFixedWidth(120);
    sharpSlider->setRange(0, 100);
    sharpSlider->setValue(0);
    sharpSlider->setEnabled(false);
    valueLabel->setFixedWidth(30);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("font-weight: bold;");
    sharpLabel->setAlignment(Qt::AlignCenter);
    sharpLabel->setStyleSheet("color: gray; font-size: 14px;");

    // 슬라이더 한 줄 구성
    QHBoxLayout *sliderRow = new QHBoxLayout;
    sliderRow->setAlignment(Qt::AlignCenter);
    sliderRow->setSpacing(5);
    sliderRow->addWidget(sharpRadio);
    sliderRow->addWidget(minusBtn);
    sliderRow->addWidget(sharpSlider);
    sliderRow->addWidget(plusBtn);
    sliderRow->addWidget(valueLabel);

    sharpLayout->addLayout(sliderRow);
    sharpLayout->addWidget(sharpLabel);

    // 동작 연결
    connect(sharpRadio, &QRadioButton::toggled, this, [=](bool checked){
        minusBtn->setEnabled(checked);
        plusBtn->setEnabled(checked);
        sharpSlider->setEnabled(checked);
        valueLabel->setEnabled(checked);
        sharpLabel->setStyleSheet(QString("color: %1; font-size: 14px;").arg(checked ? "black" : "gray"));
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

    // ───── 전체 레이아웃 구성 ─────
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(dayBox);
    mainLayout->addWidget(nightBox);
    mainLayout->addWidget(sharpBox);
}

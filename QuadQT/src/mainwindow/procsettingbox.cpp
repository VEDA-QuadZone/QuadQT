#include "mainwindow/procsettingbox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSlider>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPixmap>
#include <QDebug>
#include <algorithm>

ProcSettingBox::ProcSettingBox(QWidget *parent)
    : QWidget(parent)
{
    // 원래 기본 테두리 스타일
    this->setStyleSheet("border: 1px solid #ccc; background-color: transparent;");
    setupUI();
    setupConnections();
    updateModeUI(); // 초기 UI 상태 설정
}

void ProcSettingBox::setupUI()
{
    QString radioStyle = "margin: 4px 0 0 4px; background: transparent; border: none;";
    sliderStyleGray = R"(
        QSlider::groove:horizontal { background: #ccc; height: 8px; border-radius: 4px; }
        QSlider::handle:horizontal {
            background: #ccc; width: 18px; height: 18px;
            margin: -6px 0; border-radius: 9px;
        }
        QSlider::sub-page:horizontal { background: #ccc; border-radius: 4px; }
        QSlider::add-page:horizontal { background: #eee; border-radius: 4px; }
    )";
    sliderStyleOrange = R"(
        QSlider::groove:horizontal { background: #ddd; height: 8px; border-radius: 4px; }
        QSlider::handle:horizontal {
            background: #F37321; width: 18px; height: 18px;
            margin: -6px 0; border-radius: 9px;
        }
        QSlider::sub-page:horizontal { background: #F37321; border-radius: 4px; }
        QSlider::add-page:horizontal { background: #eee; border-radius: 4px; }
    )";

    textStyleGray = "font-size: 14px; color: gray; padding-bottom: 4px; border: none; background: transparent;";
    textStyleOrange = "font-size: 14px; color: #F37321; padding-bottom: 4px; border: none; background: transparent;";

    modeGroup = new QButtonGroup(this);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);

    // Day Mode
    dayBox = new QWidget(this);
    dayBox->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *dayLayout = new QVBoxLayout(dayBox);
    dayLayout->setContentsMargins(4, 4, 4, 4);
    dayLayout->setSpacing(0);

    dayRadio = new QRadioButton(dayBox);
    dayRadio->setStyleSheet(radioStyle);
    modeGroup->addButton(dayRadio);

    dayIcon = new QLabel(dayBox);
    dayIcon->setStyleSheet("border: none; background: transparent;");
    dayIcon->setAlignment(Qt::AlignCenter);

    dayLabel = new QLabel("주간 모드", dayBox);
    dayLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    dayLabel->setStyleSheet(textStyleGray);

    dayLayout->addWidget(dayRadio, 0, Qt::AlignLeft | Qt::AlignTop);
    dayLayout->addStretch();
    dayLayout->addWidget(dayIcon);
    dayLayout->addStretch();
    dayLayout->addWidget(dayLabel);
    mainLayout->addWidget(dayBox, 2);

    // Night Mode
    nightBox = new QWidget(this);
    nightBox->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *nightLayout = new QVBoxLayout(nightBox);
    nightLayout->setContentsMargins(4, 4, 4, 4);
    nightLayout->setSpacing(0);

    nightRadio = new QRadioButton(nightBox);
    nightRadio->setStyleSheet(radioStyle);
    modeGroup->addButton(nightRadio);

    nightIcon = new QLabel(nightBox);
    nightIcon->setStyleSheet("border: none; background: transparent;");
    nightIcon->setAlignment(Qt::AlignCenter);

    nightLabel = new QLabel("야간 모드", nightBox);
    nightLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    nightLabel->setStyleSheet(textStyleGray);

    nightLayout->addWidget(nightRadio, 0, Qt::AlignLeft | Qt::AlignTop);
    nightLayout->addStretch();
    nightLayout->addWidget(nightIcon);
    nightLayout->addStretch();
    nightLayout->addWidget(nightLabel);
    mainLayout->addWidget(nightBox, 2);

    // Sharpness Mode
    sharpnessBox = new QWidget(this);
    QVBoxLayout *sharpnessLayout = new QVBoxLayout(sharpnessBox);
    sharpnessLayout->setContentsMargins(4, 4, 4, 4);
    sharpnessLayout->setSpacing(0);

    sharpnessRadio = new QRadioButton(sharpnessBox);
    sharpnessRadio->setStyleSheet(radioStyle);
    modeGroup->addButton(sharpnessRadio);

    QWidget *controlWidget = new QWidget(sharpnessBox);
    controlWidget->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setContentsMargins(8, 12, 8, 0);
    controlLayout->setSpacing(6);

    minusButton = new QPushButton(controlWidget);
    minusButton->setFixedSize(24, 24);
    minusButton->setStyleSheet("border: none; background: transparent;");

    sharpnessSlider = new QSlider(Qt::Horizontal, controlWidget);
    sharpnessSlider->setMinimum(0);
    sharpnessSlider->setMaximum(100);
    sharpnessSlider->setValue(50);

    plusButton = new QPushButton(controlWidget);
    plusButton->setFixedSize(24, 24);
    plusButton->setStyleSheet("border: none; background: transparent;");

    sharpnessEdit = new QLineEdit("50", controlWidget);
    sharpnessEdit->setFixedWidth(36);
    sharpnessEdit->setAlignment(Qt::AlignCenter);
    sharpnessEdit->setStyleSheet("border: 1px solid #ccc; padding: 2px; border-radius: 4px;");

    controlLayout->addWidget(minusButton);
    controlLayout->addWidget(sharpnessSlider, 1);
    controlLayout->addWidget(plusButton);
    controlLayout->addWidget(sharpnessEdit);

    sharpnessLabel = new QLabel("선명도 (%)", sharpnessBox);
    sharpnessLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    sharpnessLabel->setStyleSheet(textStyleGray);

    sharpnessLayout->addWidget(sharpnessRadio, 0, Qt::AlignLeft | Qt::AlignTop);
    sharpnessLayout->addStretch();
    sharpnessLayout->addWidget(controlWidget);
    sharpnessLayout->addStretch();
    sharpnessLayout->addWidget(sharpnessLabel);
    mainLayout->addWidget(sharpnessBox, 5);
}

void ProcSettingBox::setupConnections()
{
    connect(modeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, [=](QAbstractButton *) { updateModeUI(); });

    connect(sharpnessEdit, &QLineEdit::returnPressed, this, [=]() {
        sharpnessEdit->clearFocus();
        bool ok;
        int val = sharpnessEdit->text().toInt(&ok);
        if (ok) {
            val = std::clamp(val, 0, 100);
            sharpnessSlider->setValue(val);
        }
    });

    connect(sharpnessSlider, &QSlider::valueChanged, this, [=](int val) {
        sharpnessEdit->setText(QString::number(val));
    });
}

void ProcSettingBox::updateModeUI()
{
    bool isDay = dayRadio->isChecked();
    bool isNight = nightRadio->isChecked();
    bool isSharpness = sharpnessRadio->isChecked();

    // 버튼 아이콘 설정
    QString minusIcon = isSharpness ? ":/images/images/minus_orange.png" : ":/images/images/minus_gray.png";
    QString plusIcon = isSharpness ? ":/images/images/plus_orange.png" : ":/images/images/plus_gray.png";
    minusButton->setIcon(QIcon(minusIcon));
    minusButton->setIconSize(QSize(20, 20));
    plusButton->setIcon(QIcon(plusIcon));
    plusButton->setIconSize(QSize(20, 20));

    // 아이콘 이미지 설정
    dayIcon->setPixmap(QPixmap(isDay ? ":/images/images/sun_orange.png" : ":/images/images/sun_gray.png").scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    nightIcon->setPixmap(QPixmap(isNight ? ":/images/images/moon_orange.png" : ":/images/images/moon_gray.png").scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 텍스트 색상 변경
    dayLabel->setStyleSheet(isDay ? textStyleOrange : textStyleGray);
    nightLabel->setStyleSheet(isNight ? textStyleOrange : textStyleGray);
    sharpnessLabel->setStyleSheet(isSharpness ? textStyleOrange : textStyleGray);

    // 슬라이더 활성화 및 색상 변경
    sharpnessSlider->setEnabled(isSharpness);
    sharpnessSlider->setStyleSheet(isSharpness ? sliderStyleOrange : sliderStyleGray);
}

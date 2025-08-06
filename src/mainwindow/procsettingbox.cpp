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
#include <QEvent>

ProcSettingBox::ProcSettingBox(QWidget *parent)
    : QWidget(parent)
{
    this->setStyleSheet("border: 1px solid #ccc; background-color: transparent;");
    setupUI();
    setupConnections();
    updateModeUI();
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
        QSlider::add-page:horizontal { background: #FFFFFF; border-radius: 4px; }
    )";
    sliderStyleOrange = R"(
        QSlider::groove:horizontal { background: #ddd; height: 8px; border-radius: 4px; }
        QSlider::handle:horizontal {
            background: #F37321; width: 18px; height: 18px;
            margin: -6px 0; border-radius: 9px;
        }
        QSlider::sub-page:horizontal { background: #F37321; border-radius: 4px; }
        QSlider::add-page:horizontal { background: #FFFFFF; border-radius: 4px; }
    )";

    textStyleGray = "font-size: 14px; color: gray; padding-bottom: 4px; border: none; background: transparent;";
    textStyleOrange = "font-size: 14px; color: #F37321; padding-bottom: 4px; border: none; background: transparent;";

    modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(false);  // allow manual exclusive behavior

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);

    // Day Box
    dayBox = createModeBox("주간 모드", dayRadio, dayIcon, dayLabel);
    mainLayout->addWidget(dayBox, 2);

    // Night Box
    nightBox = createModeBox("야간 모드", nightRadio, nightIcon, nightLabel);
    mainLayout->addWidget(nightBox, 2);

    // Sharpness Box
    sharpnessBox = createSharpnessBox();
    mainLayout->addWidget(sharpnessBox, 5);

    dayBox->installEventFilter(this);
    nightBox->installEventFilter(this);
    sharpnessBox->installEventFilter(this);
}

QWidget* ProcSettingBox::createModeBox(const QString &text, QRadioButton *&radio, QLabel *&icon, QLabel *&label)
{
    QWidget *box = new QWidget(this);
    box->setStyleSheet("border-right: 1px solid #ccc;");
    QVBoxLayout *layout = new QVBoxLayout(box);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(0);

    radio = new QRadioButton(box);
    radio->setStyleSheet("margin: 4px 0 0 4px; background: transparent; border: none;");
    modeGroup->addButton(radio);

    icon = new QLabel(box);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("border: none; background: transparent;");

    label = new QLabel(text, box);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    label->setStyleSheet(textStyleGray);

    layout->addWidget(radio, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addStretch();
    layout->addWidget(icon);
    layout->addStretch();
    layout->addWidget(label);

    return box;
}

QWidget* ProcSettingBox::createSharpnessBox()
{
    QWidget *box = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(box);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(0);

    sharpnessRadio = new QRadioButton(box);
    sharpnessRadio->setStyleSheet("margin: 4px 0 0 4px; background: transparent; border: none;");
    modeGroup->addButton(sharpnessRadio);

    QWidget *controlWidget = new QWidget(box);
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

    sharpnessLabel = new QLabel("선명도 (%)", box);
    sharpnessLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    sharpnessLabel->setStyleSheet(textStyleGray);

    layout->addWidget(sharpnessRadio, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addStretch();
    layout->addWidget(controlWidget);
    layout->addStretch();
    layout->addWidget(sharpnessLabel);

    return box;
}

void ProcSettingBox::setupConnections()
{
    connect(sharpnessEdit, &QLineEdit::returnPressed, this, [=]() {
        sharpnessEdit->clearFocus();
        bool ok;
        int val = sharpnessEdit->text().toInt(&ok);
        if (ok) {
            val = std::clamp(val, 0, 100);
            sharpnessSlider->setValue(val);

            // ✅ 선명도 라디오가 선택된 상태에서만 전송
            if (sharpnessRadio->isChecked()) {
                QString command = QString("CHANGE_FRAME 2 sharp %1").arg(val);
                emit requestCommand(command);
            }
        }
    });

    connect(sharpnessSlider, &QSlider::valueChanged, this, [=](int val) {
        sharpnessEdit->setText(QString::number(val));  // ✅ 실시간으로 QLineEdit 업데이트
    });

    connect(sharpnessSlider, &QSlider::sliderReleased, this, [=]() {
        if (sharpnessRadio->isChecked()) {
            int val = sharpnessSlider->value();
            QString command = QString("CHANGE_FRAME 2 sharp %1").arg(val);
            emit requestCommand(command);
        }
    });

    connect(minusButton, &QPushButton::clicked, this, [=]() {
        if (sharpnessRadio->isChecked()) {
            int val = sharpnessSlider->value();
            sharpnessSlider->setValue(std::max(0, val - 1));
        }
    });

    connect(plusButton, &QPushButton::clicked, this, [=]() {
        if (sharpnessRadio->isChecked()) {
            int val = sharpnessSlider->value();
            sharpnessSlider->setValue(std::min(100, val + 1));
        }
    });
}

void ProcSettingBox::updateModeUI()
{
    bool isDay = dayRadio->isChecked();
    bool isNight = nightRadio->isChecked();
    bool isSharpness = sharpnessRadio->isChecked();

    QString minusIcon = isSharpness ? ":/images/minus_orange.png" : ":/images/minus_gray.png";
    QString plusIcon = isSharpness ? ":/images/plus_orange.png" : ":/images/plus_gray.png";
    minusButton->setIcon(QIcon(minusIcon));
    minusButton->setIconSize(QSize(20, 20));
    plusButton->setIcon(QIcon(plusIcon));
    plusButton->setIconSize(QSize(20, 20));

    // 주간모드 아이콘 (80% 크기로 조정)
    QPixmap dayPixmap(isDay ? ":/images/sun_orange.png" : ":/images/sun_gray.png");
    int dayScaledWidth = dayPixmap.width() * 0.6;
    int dayScaledHeight = dayPixmap.height() * 0.6;
    dayIcon->setPixmap(dayPixmap.scaled(dayScaledWidth, dayScaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // 야간모드 아이콘 (80% 크기로 조정)
    QPixmap nightPixmap(isNight ? ":/images/moon_orange.png" : ":/images/moon_gray.png");
    int nightScaledWidth = nightPixmap.width() * 0.6;
    int nightScaledHeight = nightPixmap.height() * 0.6;
    nightIcon->setPixmap(nightPixmap.scaled(nightScaledWidth, nightScaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    dayLabel->setStyleSheet(isDay ? textStyleOrange : textStyleGray);
    nightLabel->setStyleSheet(isNight ? textStyleOrange : textStyleGray);
    sharpnessLabel->setStyleSheet(isSharpness ? textStyleOrange : textStyleGray);

    sharpnessSlider->setEnabled(isSharpness);
    sharpnessSlider->setStyleSheet(isSharpness ? sliderStyleOrange : sliderStyleGray);
}

bool ProcSettingBox::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == dayBox) {
            if (dayRadio->isChecked()) {
                dayRadio->setChecked(false);
            } else {
                dayRadio->setChecked(true);
                nightRadio->setChecked(false);
                sharpnessRadio->setChecked(false);
            }
        } else if (watched == nightBox) {
            if (nightRadio->isChecked()) {
                nightRadio->setChecked(false);
            } else {
                nightRadio->setChecked(true);
                dayRadio->setChecked(false);
                sharpnessRadio->setChecked(false);
            }
        } else if (watched == sharpnessBox) {
            if (sharpnessRadio->isChecked()) {
                sharpnessRadio->setChecked(false);
            } else {
                sharpnessRadio->setChecked(true);
                dayRadio->setChecked(false);
                nightRadio->setChecked(false);
            }
        }
        updateModeUI();

        // 🔽 모드에 따라 TCP 명령 전송
        QString mode;
        if (dayRadio->isChecked()) {
            mode = "day";
        } else if (nightRadio->isChecked()) {
            mode = "night";
        } else if (sharpnessRadio->isChecked()) {
            mode = "sharp";
        }

        if (!mode.isEmpty()) {
            QString command;
            if (mode == "sharp") {
                int level = sharpnessSlider->value();
                command = QString("CHANGE_FRAME 2 sharp %1").arg(level);
            } else {
                command = QString("CHANGE_FRAME 2 %1").arg(mode);
            }
            emit requestCommand(command);
        } else {
            emit requestCommand("CHANGE_FRAME 2 original");
        }
        return true;
    }

    if (event->type() == QEvent::Enter) {
        if (watched == dayBox || watched == nightBox || watched == sharpnessBox) {
            static_cast<QWidget*>(watched)->setStyleSheet("background-color: #FCE8D9; border: 1px solid #ccc;");
        }
    } else if (event->type() == QEvent::Leave) {
        if (watched == dayBox || watched == nightBox || watched == sharpnessBox) {
            static_cast<QWidget*>(watched)->setStyleSheet("background-color: transparent; border: 1px solid #ccc;");
        }
    }

    return QWidget::eventFilter(watched, event);
}

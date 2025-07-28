#ifndef PROCSETTINGBOX_H
#define PROCSETTINGBOX_H

#include <QWidget>
#include <QButtonGroup>

class QRadioButton;
class QLabel;
class QSlider;
class QLineEdit;
class QPushButton;

class ProcSettingBox : public QWidget
{
    Q_OBJECT

public:
    explicit ProcSettingBox(QWidget *parent = nullptr);

private:
    void setupUI();
    void setupConnections();
    void updateModeUI();

    // 모드 라디오 버튼
    QRadioButton *dayRadio;
    QRadioButton *nightRadio;
    QRadioButton *sharpnessRadio;

    // 텍스트 라벨
    QLabel *dayLabel;
    QLabel *nightLabel;
    QLabel *sharpnessLabel;

    // 아이콘 라벨
    QLabel *dayIcon;
    QLabel *nightIcon;

    // 슬라이더/입력 UI
    QSlider *sharpnessSlider;
    QLineEdit *sharpnessEdit;
    QPushButton *plusButton;
    QPushButton *minusButton;

    // 모드 그룹
    QButtonGroup *modeGroup;

    // 동적 스타일 문자열
    QString textStyleGray;
    QString textStyleOrange;
    QString sliderStyleGray;
    QString sliderStyleOrange;
};

#endif // PROCSETTINGBOX_H

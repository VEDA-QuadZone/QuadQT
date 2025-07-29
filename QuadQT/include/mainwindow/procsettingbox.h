#ifndef PROCSETTINGBOX_H
#define PROCSETTINGBOX_H

#include <QWidget>

class QLabel;
class QRadioButton;
class QSlider;
class QPushButton;
class QLineEdit;
class QButtonGroup;

class ProcSettingBox : public QWidget
{
    Q_OBJECT

public:
    explicit ProcSettingBox(QWidget *parent = nullptr);

private:
    void setupUI();
    void setupConnections();
    void updateModeUI();

    QWidget *dayBox;
    QWidget *nightBox;
    QWidget *sharpnessBox;

    QRadioButton *dayRadio;
    QRadioButton *nightRadio;
    QRadioButton *sharpnessRadio;

    QLabel *dayIcon;
    QLabel *nightIcon;
    QLabel *dayLabel;
    QLabel *nightLabel;
    QLabel *sharpnessLabel;

    QPushButton *minusButton;
    QPushButton *plusButton;
    QSlider *sharpnessSlider;
    QLineEdit *sharpnessEdit;

    QButtonGroup *modeGroup;

    QString textStyleGray;
    QString textStyleOrange;
    QString sliderStyleGray;
    QString sliderStyleOrange;
};

#endif // PROCSETTINGBOX_H

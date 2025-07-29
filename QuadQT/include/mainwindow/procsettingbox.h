#ifndef PROCSETTINGBOX_H
#define PROCSETTINGBOX_H

#include <QWidget>
#include <QRadioButton>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>

class ProcSettingBox : public QWidget
{
    Q_OBJECT

public:
    explicit ProcSettingBox(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void setupConnections();
    void updateModeUI();

    QWidget* createModeBox(const QString &text, QRadioButton *&radio, QLabel *&icon, QLabel *&label);
    QWidget* createSharpnessBox();

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

    QSlider *sharpnessSlider;
    QLineEdit *sharpnessEdit;
    QPushButton *minusButton;
    QPushButton *plusButton;

    QButtonGroup *modeGroup;

    QString sliderStyleGray;
    QString sliderStyleOrange;
    QString textStyleGray;
    QString textStyleOrange;
};

#endif // PROCSETTINGBOX_H

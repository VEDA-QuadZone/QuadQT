#ifndef DISPLAYSETTINGBOX_H
#define DISPLAYSETTINGBOX_H

#include <QWidget>

class QLabel;
class QWidget;

class DisplaySettingBox : public QWidget
{
    Q_OBJECT

public:
    explicit DisplaySettingBox(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void updateObjectBoxUI();
    void updateTimestampUI();

    // 상태 플래그
    bool objectBoxOn;
    bool timestampOn;

    // 객체 박스 UI 구성요소
    QWidget *objectBoxWidget;
    QLabel *objectIcon;
    QLabel *objectLabel;

    // 타임스탬프 UI 구성요소
    QWidget *timestampWidget;
    QLabel *timestampIcon;
    QLabel *timestampLabel;
};

#endif // DISPLAYSETTINGBOX_H

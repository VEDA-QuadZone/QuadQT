#ifndef TOPBARWIDGET_H
#define TOPBARWIDGET_H

#include <QWidget>

class QLabel;

class TopBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TopBarWidget(QWidget *parent = nullptr);
    void updateLayout(int parentWidth, int parentHeight);

    QLabel *logoLabel;
    QLabel *cameraIcon;
    QLabel *docIcon;
    QLabel *settingIcon;
    QLabel *loginStatus;
    QWidget *topLine;
};

#endif // TOPBARWIDGET_H

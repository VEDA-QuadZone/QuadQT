#include "topbarwidget.h"
#include <QLabel>
#include <QFont>

TopBarWidget::TopBarWidget(QWidget *parent)
    : QWidget(parent)
{
    logoLabel = new QLabel("QuardZone", this);
    QFont logoFont;
    logoFont.setBold(true);
    logoLabel->setFont(logoFont);
    logoLabel->setStyleSheet("color: orange;");

    cameraIcon = new QLabel("CAM", this);
    docIcon = new QLabel("DOC", this);
    settingIcon = new QLabel("SET", this);
    loginStatus = new QLabel("USER", this);

    QList<QLabel*> icons = {cameraIcon, docIcon, settingIcon, loginStatus};
    for (auto icon : icons) {
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet("font-size: 14px; font-weight: bold; background-color: #eee; border: 1px solid #ccc;");
    }

    topLine = new QWidget(this);
    topLine->setStyleSheet("background-color: #ccc;");
}

void TopBarWidget::updateLayout(int w, int h)
{
    double w_unit = w / 24.0;
    double h_unit = h / 24.0;

    logoLabel->setGeometry(w_unit * 1, h_unit * 1, w_unit * 4, h_unit * 2);
    QFont logoFont = logoLabel->font();
    logoFont.setPointSizeF(h_unit * 0.65);
    logoLabel->setFont(logoFont);

    topLine->setGeometry(w_unit * 1, h_unit * 3 - 1, w_unit * 22, 1);

    cameraIcon->setGeometry(w_unit * 9, h_unit * 1, w_unit * 2, h_unit * 2);
    docIcon->setGeometry(w_unit * 11, h_unit * 1, w_unit * 2, h_unit * 2);
    settingIcon->setGeometry(w_unit * 13, h_unit * 1, w_unit * 2, h_unit * 2);
    loginStatus->setGeometry(w_unit * 21.0, h_unit * 1, w_unit * 2, h_unit * 2);
}

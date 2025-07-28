#ifndef DISPLAYSETTINGBOX_H
#define DISPLAYSETTINGBOX_H

#include <QWidget>

class DisplaySettingBox : public QWidget
{
    Q_OBJECT
public:
    explicit DisplaySettingBox(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // DISPLAYSETTINGBOX_H

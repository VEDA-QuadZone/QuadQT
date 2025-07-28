#ifndef PROCSETTINGBOX_H
#define PROCSETTINGBOX_H

#include <QWidget>

class ProcSettingBox : public QWidget
{
    Q_OBJECT

public:
    explicit ProcSettingBox(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // PROCSETTINGBOX_H

#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CustomMessageBox : public QDialog
{
    Q_OBJECT

public:
    explicit CustomMessageBox(const QString &title,
                              const QString &message,
                              QWidget *parent = nullptr);  // iconKey 제거

    static void showMessage(QWidget *parent,
                            const QString &title,
                            const QString &message);

    static bool showConfirm(QWidget *parent,
                            const QString &title,
                            const QString &message);

private:
    QLabel *m_titleLabel;
    QLabel *m_messageLabel;
    QPushButton *m_okButton;
    QVBoxLayout *m_mainLayout;
};

#endif // CUSTOMMESSAGEBOX_H

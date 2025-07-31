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
                              const QString &iconKey = "",
                              QWidget *parent = nullptr);

    static void showMessage(QWidget *parent,
                            const QString &title,
                            const QString &message,
                            const QString &iconKey = "");

    static bool showConfirm(QWidget *parent,
                            const QString &title,
                            const QString &message,
                            const QString &iconKey = "");

private:
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_messageLabel;
    QPushButton *m_okButton;
    QVBoxLayout *m_mainLayout;
};

#endif // CUSTOMMESSAGEBOX_H

#ifndef TOPBARWIDGET_H
#define TOPBARWIDGET_H

#include <QWidget>

class QLabel;
class QMenu;

enum class TopBarButton {
    Camera,
    Document,
    Settings
};

class TopBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TopBarWidget(QWidget *parent = nullptr);
    void updateLayout(int parentWidth, int parentHeight);
    void setActiveButton(TopBarButton button);
    void setUserEmail(const QString &email);
    void clearUserData();

signals:
    void cameraClicked();
    void documentClicked();
    void settingsClicked();
    void logoutRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onUserButtonClicked();

private:
    void setupIcons();
    void updateButtonStates();
    void showUserMenu();
    QRect getCameraRect() const;
    QRect getDocumentRect() const;
    QRect getSettingsRect() const;
    QRect getUserRect() const;

    QLabel *logoLabel;
    QLabel *cameraIcon;
    QLabel *docIcon;
    QLabel *settingIcon;
    QLabel *loginStatus;
    QWidget *topLine;
    
    TopBarButton m_activeButton;
    int m_parentWidth;
    int m_parentHeight;
    QString m_userEmail;
};

#endif // TOPBARWIDGET_H

#include "login/custommessagebox.h"
#include <QPixmap>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

CustomMessageBox::CustomMessageBox(const QString &title,
                                   const QString &message,
                                   const QString &iconKey,
                                   QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QString iconPath;
    if (iconKey == "info")
        iconPath = ":/images/info.png";
    else if (iconKey == "warning")
        iconPath = ":/images/caution.png";
    else if (iconKey == "error")
        iconPath = ":/images/prohibition.png";

    // 아이콘
    m_iconLabel = new QLabel();
    if (!iconPath.isEmpty()) {
        QPixmap icon(iconPath);
        m_iconLabel->setPixmap(icon.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_iconLabel->hide();
    }
    m_iconLabel->setStyleSheet("background: transparent; border: none;");

    // 제목
    m_titleLabel = new QLabel(title);
    m_titleLabel->setStyleSheet(
        "font-size: 16pt; font-weight: bold; color: #111;"
        "background: transparent; border: none; margin: 0;"
        );

    // 메시지
    m_messageLabel = new QLabel(message);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setStyleSheet(
        "font-size: 12pt; color: #333;"
        "background: transparent; border: none; padding: 0; margin: 0;"
        );

    // 기본 OK 버튼
    m_okButton = new QPushButton("확인");
    m_okButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #ff6a00;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #e55a00; }"
        );
    connect(m_okButton, &QPushButton::clicked, this, &CustomMessageBox::accept);

    // [아이콘 + 제목] 수평 정렬
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(m_iconLabel);
    topLayout->addSpacing(8);
    topLayout->addWidget(m_titleLabel);
    topLayout->setAlignment(Qt::AlignHCenter);

    // 콘텐츠 박스
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setStyleSheet(
        "background-color: white;"
        "border: 1px solid #bbb;"
        "border-radius: 0px;"
        );

    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(12);
    contentLayout->addLayout(topLayout);
    contentLayout->addWidget(m_messageLabel);
    contentLayout->addSpacing(16);
    contentLayout->addWidget(m_okButton, 0, Qt::AlignCenter);

    // 그림자 효과
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setOffset(0, 4);
    contentWidget->setGraphicsEffect(shadow);

    // 전체 레이아웃
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(contentWidget);

    setFixedSize(400, 220);
}

// ✔ 정보/오류 메시지용 단순 팝업
void CustomMessageBox::showMessage(QWidget *parent,
                                   const QString &title,
                                   const QString &message,
                                   const QString &iconKey)
{
    CustomMessageBox box(title, message, iconKey, parent);
    box.exec();
}

// ✔ 예/아니오 팝업
bool CustomMessageBox::showConfirm(QWidget *parent,
                                   const QString &title,
                                   const QString &message,
                                   const QString &iconKey)
{
    CustomMessageBox box(title, message, iconKey, parent);

    // 기존 OK 버튼 숨김
    box.m_okButton->hide();

    // 새 버튼 생성
    QPushButton *yesButton = new QPushButton("예");
    QPushButton *noButton  = new QPushButton("아니오");

    QString btnStyle =
        "QPushButton {"
        "  background-color: #ff6a00;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #e55a00; }";

    yesButton->setStyleSheet(btnStyle);
    noButton->setStyleSheet(btnStyle);

    QObject::connect(yesButton, &QPushButton::clicked, &box, &CustomMessageBox::accept);
    QObject::connect(noButton,  &QPushButton::clicked, &box, &CustomMessageBox::reject);

    // 버튼 박스 (하얀 contentWidget 안에 넣기 위해 새로 생성)
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(yesButton);
    btnLayout->addSpacing(20);
    btnLayout->addWidget(noButton);
    btnLayout->addStretch();

    // ✅ 버튼 박스를 contentWidget 내부에 추가 (하얀 박스 안에 포함)
    QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout *>(box.m_mainLayout->itemAt(0)->widget()->layout());
    if (contentLayout) {
        contentLayout->addSpacing(8);
        contentLayout->addLayout(btnLayout);
    }

    return box.exec() == QDialog::Accepted;
}


#include "login/custommessagebox.h"
#include "mainwindow/overlaywidget.h"
#include <QGraphicsDropShadowEffect>
#include <QApplication>

CustomMessageBox::CustomMessageBox(const QString &title,
                                   const QString &message,
                                   QWidget *parent)
    : QDialog(parent), overlay_(nullptr)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

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

    // OK 버튼
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

    // 제목 수평 정렬
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(m_titleLabel);
    topLayout->addStretch();
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
                                   const QString &message)
{
    CustomMessageBox box(title, message, parent);
    box.exec();
}

// ✔ 예/아니오 팝업
bool CustomMessageBox::showConfirm(QWidget *parent,
                                   const QString &title,
                                   const QString &message)
{
    CustomMessageBox box(title, message, parent);
    box.m_okButton->hide();

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

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(yesButton);
    btnLayout->addSpacing(20);
    btnLayout->addWidget(noButton);
    btnLayout->addStretch();

    QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout *>(box.m_mainLayout->itemAt(0)->widget()->layout());
    if (contentLayout) {
        contentLayout->addSpacing(8);
        contentLayout->addLayout(btnLayout);
    }

    return box.exec() == QDialog::Accepted;
}

void CustomMessageBox::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    
    // 최상위 윈도우를 찾아서 오버레이 생성
    QWidget* topLevelWidget = this;
    while (topLevelWidget->parentWidget()) {
        topLevelWidget = topLevelWidget->parentWidget();
    }
    
    if (topLevelWidget && topLevelWidget != this) {
        overlay_ = new OverlayWidget(topLevelWidget);
        overlay_->resize(topLevelWidget->size());
        overlay_->show();
        
        // 다이얼로그를 최상위로
        raise();
    }
}

void CustomMessageBox::hideEvent(QHideEvent* event) {
    // 오버레이 제거
    if (overlay_) {
        overlay_->deleteLater();
        overlay_ = nullptr;
    }
    
    QDialog::hideEvent(event);
}

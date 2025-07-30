#include "mainwindow/getimageview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QPixmap>

GetImageView::GetImageView(const QString& path, const QString& timestamp, QWidget* parent)
    : QDialog(parent)
{
    this->setStyleSheet("background-color: #FFFFFF;");
    QVBoxLayout* layout = new QVBoxLayout(this);

    // 1. 이미지 영역
    imageLabel_ = new QLabel(this);
    imageLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel_);

    // 2. 아래 정보(파일명+일시)를 수평배치
    QHBoxLayout* infoLayout = new QHBoxLayout;

    // 파일명: "파일 이름\n파일이름.jpg"
    QFileInfo fi(path);
    filenameLabel_ = new QLabel(this);
    filenameLabel_->setText(QString("파일 이름\n%1").arg(fi.fileName()));
    filenameLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    filenameLabel_->setWordWrap(true); // 줄바꿈 허용
    infoLayout->addWidget(filenameLabel_, /*stretch*/2);

    // 일시: "일시\n2025-07-25 14:48:08"
    timeLabel_ = new QLabel(this);
    timeLabel_->setText(QString("일시\n%1").arg(timestamp));
    timeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    infoLayout->addWidget(timeLabel_, /*stretch*/2);

    layout->addLayout(infoLayout);
    setLayout(layout);

    setMinimumSize(500, 400);
}

void GetImageView::setImageData(const QByteArray& data) {
    QPixmap pix;
    bool ok = pix.loadFromData(data, "JPEG"); // PNG/JPEG 둘 다 가능하면 둘 다 시도해도 됨
    if (!ok) {
        ok = pix.loadFromData(data, "PNG");
    }
    if (!ok) return;
    imageLabel_->setPixmap(pix.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

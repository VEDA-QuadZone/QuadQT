#include "mainwindow/getimageview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>

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
    qDebug() << "[GetImageView] setImageData 호출됨, 데이터 크기:" << data.size() << "bytes";
    
    if (data.isEmpty()) {
        qDebug() << "[GetImageView] 이미지 데이터가 비어있음";
        return;
    }
    
    QPixmap pix;
    bool ok = pix.loadFromData(data, "JPEG");
    qDebug() << "[GetImageView] JPEG 로드 시도 결과:" << ok;
    
    if (!ok) {
        ok = pix.loadFromData(data, "PNG");
        qDebug() << "[GetImageView] PNG 로드 시도 결과:" << ok;
    }
    
    if (!ok) {
        qDebug() << "[GetImageView] 이미지 로드 실패 - 지원되지 않는 형식이거나 손상된 데이터";
        // 데이터의 첫 몇 바이트를 확인해보자
        if (data.size() > 10) {
            QByteArray header = data.left(10);
            qDebug() << "[GetImageView] 데이터 헤더 (hex):" << header.toHex();
        }
        return;
    }
    
    qDebug() << "[GetImageView] 이미지 로드 성공, 원본 크기:" << pix.size();
    QPixmap scaledPix = pix.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    qDebug() << "[GetImageView] 스케일된 크기:" << scaledPix.size();
    
    imageLabel_->setPixmap(scaledPix);
    qDebug() << "[GetImageView] 이미지 표시 완료";
}

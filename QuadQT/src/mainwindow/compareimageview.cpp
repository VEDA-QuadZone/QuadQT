#include "mainwindow/compareimageview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QSpacerItem>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>

constexpr int IMAGE_WIDTH = 427;
constexpr int IMAGE_HEIGHT = 240;
constexpr int TOTAL_WIDTH = IMAGE_WIDTH * 2 + 60; // 두 이미지 + 화살표 + 여백

CompareImageView::CompareImageView(const QString& event, const QString& plate,
                                   const QString& datetime, const QString& startFilename, 
                                   const QString& endFilename, QWidget* parent)
    : QDialog(parent), startFilename_(startFilename), endFilename_(endFilename)
{
    setWindowTitle("불법 주정차 증명 자료");
    setStyleSheet("background:#fff;");

    // --- 전체 테두리 Frame ---
    QFrame* border = new QFrame(this);
    border->setFrameShape(QFrame::Box);
    border->setLineWidth(1);
    border->setStyleSheet("QFrame { border: 1px solid #222; border-radius: 3px; background:#fff; }");
    border->setFixedWidth(TOTAL_WIDTH);
    QVBoxLayout* borderLayout = new QVBoxLayout(border);
    borderLayout->setSpacing(10);
    borderLayout->setContentsMargins(10, 10, 10, 10);

    // --- 1. 정보 테이블 부분 (QLabel로 구현) ---
    QHBoxLayout* tableLayout = new QHBoxLayout;
    tableLayout->setSpacing(40); // 테이블 간격을 화살표와 맞춤
    
    // 왼쪽 테이블 (정차 시작)
    QWidget* leftTableWidget = new QWidget(this);
    leftTableWidget->setFixedSize(IMAGE_WIDTH, 96); // 3행 * 32px
    leftTableWidget->setStyleSheet("border: 1px solid #888; background: #fff;");
    
    QVBoxLayout* leftTableLayout = new QVBoxLayout(leftTableWidget);
    leftTableLayout->setSpacing(0);
    leftTableLayout->setContentsMargins(0, 0, 0, 0);
    
    // 헤더 (주황색)
    QLabel* leftHeader = new QLabel("정차 시작", leftTableWidget);
    leftHeader->setFixedHeight(32);
    leftHeader->setAlignment(Qt::AlignCenter);
    leftHeader->setStyleSheet("background: #FBD5B8; border-bottom: 1px solid #888; font-weight: normal;");
    leftTableLayout->addWidget(leftHeader);
    
    // 번호판 행
    QWidget* leftPlateRow = new QWidget(leftTableWidget);
    leftPlateRow->setFixedHeight(32);
    leftPlateRow->setStyleSheet("background: #fff; border-bottom: 1px solid #888;");
    QHBoxLayout* leftPlateLayout = new QHBoxLayout(leftPlateRow);
    leftPlateLayout->setSpacing(0);
    leftPlateLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* leftPlateLabel = new QLabel("번호판", leftPlateRow);
    leftPlateLabel->setFixedWidth(120);
    leftPlateLabel->setAlignment(Qt::AlignCenter);
    leftPlateLabel->setStyleSheet("border-right: 1px solid #888;");
    
    QLabel* leftPlateVal = new QLabel(plate, leftPlateRow);
    leftPlateVal->setAlignment(Qt::AlignCenter);
    
    leftPlateLayout->addWidget(leftPlateLabel);
    leftPlateLayout->addWidget(leftPlateVal);
    leftTableLayout->addWidget(leftPlateRow);
    
    // 일시 행
    QWidget* leftTimeRow = new QWidget(leftTableWidget);
    leftTimeRow->setFixedHeight(32);
    leftTimeRow->setStyleSheet("background: #fff;");
    QHBoxLayout* leftTimeLayout = new QHBoxLayout(leftTimeRow);
    leftTimeLayout->setSpacing(0);
    leftTimeLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* leftTimeLabel = new QLabel("일시", leftTimeRow);
    leftTimeLabel->setFixedWidth(120);
    leftTimeLabel->setAlignment(Qt::AlignCenter);
    leftTimeLabel->setStyleSheet("border-right: 1px solid #888;");
    
    QLabel* leftTimeVal = new QLabel(datetime, leftTimeRow);
    leftTimeVal->setAlignment(Qt::AlignCenter);
    leftTimeVal->setStyleSheet("font-weight: bold;");
    
    leftTimeLayout->addWidget(leftTimeLabel);
    leftTimeLayout->addWidget(leftTimeVal);
    leftTableLayout->addWidget(leftTimeRow);
    
    // 오른쪽 테이블 (정차 후 1분 경과)
    QWidget* rightTableWidget = new QWidget(this);
    rightTableWidget->setFixedSize(IMAGE_WIDTH, 96); // 3행 * 32px
    rightTableWidget->setStyleSheet("border: 1px solid #888; background: #fff;");
    
    QVBoxLayout* rightTableLayout = new QVBoxLayout(rightTableWidget);
    rightTableLayout->setSpacing(0);
    rightTableLayout->setContentsMargins(0, 0, 0, 0);
    
    // 헤더 (주황색)
    QLabel* rightHeader = new QLabel("정차 후 1분 경과", rightTableWidget);
    rightHeader->setFixedHeight(32);
    rightHeader->setAlignment(Qt::AlignCenter);
    rightHeader->setStyleSheet("background: #FBD5B8; border-bottom: 1px solid #888; font-weight: normal;");
    rightTableLayout->addWidget(rightHeader);
    
    // 번호판 행
    QWidget* rightPlateRow = new QWidget(rightTableWidget);
    rightPlateRow->setFixedHeight(32);
    rightPlateRow->setStyleSheet("background: #fff; border-bottom: 1px solid #888;");
    QHBoxLayout* rightPlateLayout = new QHBoxLayout(rightPlateRow);
    rightPlateLayout->setSpacing(0);
    rightPlateLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* rightPlateLabel = new QLabel("번호판", rightPlateRow);
    rightPlateLabel->setFixedWidth(120);
    rightPlateLabel->setAlignment(Qt::AlignCenter);
    rightPlateLabel->setStyleSheet("border-right: 1px solid #888;");
    
    QLabel* rightPlateVal = new QLabel(plate, rightPlateRow);
    rightPlateVal->setAlignment(Qt::AlignCenter);
    
    rightPlateLayout->addWidget(rightPlateLabel);
    rightPlateLayout->addWidget(rightPlateVal);
    rightTableLayout->addWidget(rightPlateRow);
    
    // 일시 행
    QWidget* rightTimeRow = new QWidget(rightTableWidget);
    rightTimeRow->setFixedHeight(32);
    rightTimeRow->setStyleSheet("background: #fff;");
    QHBoxLayout* rightTimeLayout = new QHBoxLayout(rightTimeRow);
    rightTimeLayout->setSpacing(0);
    rightTimeLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* rightTimeLabel = new QLabel("일시", rightTimeRow);
    rightTimeLabel->setFixedWidth(120);
    rightTimeLabel->setAlignment(Qt::AlignCenter);
    rightTimeLabel->setStyleSheet("border-right: 1px solid #888;");
    
    QLabel* rightTimeVal = new QLabel(datetime, rightTimeRow);
    rightTimeVal->setAlignment(Qt::AlignCenter);
    rightTimeVal->setStyleSheet("font-weight: bold;");
    
    rightTimeLayout->addWidget(rightTimeLabel);
    rightTimeLayout->addWidget(rightTimeVal);
    rightTableLayout->addWidget(rightTimeRow);

    tableLayout->addWidget(leftTableWidget);
    tableLayout->addWidget(rightTableWidget);
    
    borderLayout->addLayout(tableLayout);

    // --- 2. 이미지 영역 (두 이미지를 나란히 + 화살표) ---
    QHBoxLayout* imageLayout = new QHBoxLayout;
    imageLayout->setSpacing(0); // 간격을 0으로 설정하고 수동으로 조정
    
    // 왼쪽 이미지 (정차 시작)
    startImageLabel_ = new QLabel(this);
    startImageLabel_->setFixedSize(IMAGE_WIDTH, IMAGE_HEIGHT);
    startImageLabel_->setAlignment(Qt::AlignCenter);
    startImageLabel_->setStyleSheet("background:#ccc; border:none; color:#888; font-size:18px;");
    startImageLabel_->setText("이미지");
    
    // 화살표
    QLabel* arrowLabel = new QLabel("→", this);
    arrowLabel->setAlignment(Qt::AlignCenter);
    arrowLabel->setStyleSheet("font-size:24px; font-weight:bold; color:#333; border:none; background:transparent;");
    arrowLabel->setFixedWidth(40);
    
    // 오른쪽 이미지 (1분 경과)
    endImageLabel_ = new QLabel(this);
    endImageLabel_->setFixedSize(IMAGE_WIDTH, IMAGE_HEIGHT);
    endImageLabel_->setAlignment(Qt::AlignCenter);
    endImageLabel_->setStyleSheet("background:#ccc; border:none; color:#888; font-size:18px;");
    endImageLabel_->setText("이미지");
    
    imageLayout->addWidget(startImageLabel_);
    imageLayout->addWidget(arrowLabel);
    imageLayout->addWidget(endImageLabel_);
    
    borderLayout->addLayout(imageLayout);

    // --- 3. 파일이름 행들 ---
    // 왼쪽 파일명
    QHBoxLayout* leftFileRow = new QHBoxLayout;
    leftFileRow->setContentsMargins(8, 0, 8, 0);
    
    QLabel* leftFileKey = new QLabel("파일 이름", this);
    leftFileKey->setMinimumWidth(64);
    leftFileKey->setStyleSheet("border:none; background:transparent; font-size:15px; color:#222;");
    leftFileRow->addWidget(leftFileKey);
    
    startFilenameLabel_ = new QLabel(
        QString("<a href=\"#\" style=\"color:#1976D2;text-decoration:underline;\">%1</a>").arg(startFilename), this);
    startFilenameLabel_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    startFilenameLabel_->setOpenExternalLinks(false);
    startFilenameLabel_->setStyleSheet("border:none; background:transparent; color:#1976D2; font-size:15px;");
    connect(startFilenameLabel_, &QLabel::linkActivated, [=](const QString&) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(startFilename));
    });
    leftFileRow->addWidget(startFilenameLabel_);
    leftFileRow->addStretch();
    
    downloadButton1_ = new QPushButton("⬇", this);
    downloadButton1_->setFixedWidth(28);
    downloadButton1_->setStyleSheet("border:none; background:transparent; font-size:15px; padding:0 2px;");
    leftFileRow->addWidget(downloadButton1_);
    
    // 오른쪽 파일명
    QHBoxLayout* rightFileRow = new QHBoxLayout;
    rightFileRow->setContentsMargins(8, 0, 8, 0);
    
    QLabel* rightFileKey = new QLabel("파일 이름", this);
    rightFileKey->setMinimumWidth(64);
    rightFileKey->setStyleSheet("border:none; background:transparent; font-size:15px; color:#222;");
    rightFileRow->addWidget(rightFileKey);
    
    endFilenameLabel_ = new QLabel(
        QString("<a href=\"#\" style=\"color:#1976D2;text-decoration:underline;\">%1</a>").arg(endFilename), this);
    endFilenameLabel_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    endFilenameLabel_->setOpenExternalLinks(false);
    endFilenameLabel_->setStyleSheet("border:none; background:transparent; color:#1976D2; font-size:15px;");
    connect(endFilenameLabel_, &QLabel::linkActivated, [=](const QString&) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(endFilename));
    });
    rightFileRow->addWidget(endFilenameLabel_);
    rightFileRow->addStretch();
    
    downloadButton2_ = new QPushButton("⬇", this);
    downloadButton2_->setFixedWidth(28);
    downloadButton2_->setStyleSheet("border:none; background:transparent; font-size:15px; padding:0 2px;");
    rightFileRow->addWidget(downloadButton2_);
    
    // 파일명 행들을 나란히 배치
    QHBoxLayout* fileRowsLayout = new QHBoxLayout;
    fileRowsLayout->setSpacing(0); // 간격을 0으로 설정하고 수동으로 조정
    
    // 왼쪽 파일명을 위한 컨테이너
    QWidget* leftFileContainer = new QWidget(this);
    leftFileContainer->setFixedWidth(IMAGE_WIDTH);
    leftFileContainer->setLayout(leftFileRow);
    
    // 화살표 공간
    QWidget* arrowSpace = new QWidget(this);
    arrowSpace->setFixedWidth(40);
    
    // 오른쪽 파일명을 위한 컨테이너
    QWidget* rightFileContainer = new QWidget(this);
    rightFileContainer->setFixedWidth(IMAGE_WIDTH);
    rightFileContainer->setLayout(rightFileRow);
    
    fileRowsLayout->addWidget(leftFileContainer);
    fileRowsLayout->addWidget(arrowSpace);
    fileRowsLayout->addWidget(rightFileContainer);
    
    borderLayout->addLayout(fileRowsLayout);

    // --- 전체를 수직으로 정렬 ---
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->addWidget(border, 1);

    // --- 4. 인쇄/닫기 버튼 (중앙정렬) ---
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    printButton_ = new QPushButton("인쇄", this);
    printButton_->setFixedWidth(72);
    printButton_->setStyleSheet("background:#F37321; color:white; font-weight:bold;");
    closeButton_ = new QPushButton("닫기", this);
    closeButton_->setFixedWidth(72);
    closeButton_->setStyleSheet("background:#FBB584; color:#333;");
    btnLayout->addWidget(printButton_);
    btnLayout->addSpacing(20);
    btnLayout->addWidget(closeButton_);
    btnLayout->addStretch();
    vLayout->addSpacing(8);
    vLayout->addLayout(btnLayout);

    setFixedWidth(TOTAL_WIDTH + 32); // 좌우 마진 여유 포함
    setLayout(vLayout);

    // 버튼 연결
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(downloadButton1_, &QPushButton::clicked, this, &CompareImageView::downloadStartImage);
    connect(downloadButton2_, &QPushButton::clicked, this, &CompareImageView::downloadEndImage);
    connect(printButton_, &QPushButton::clicked, this, &CompareImageView::printToPdf);
}

void CompareImageView::setStartImageData(const QByteArray& data) {
    startImageData_ = data; // 이미지 데이터 저장
    if (data.isEmpty()) {
        startImageLabel_->setText("이미지 없음");
        return;
    }
    QPixmap pix;
    if (!pix.loadFromData(data, "JPEG") && !pix.loadFromData(data, "PNG")) {
        startImageLabel_->setText("이미지 오류");
        return;
    }
    QPixmap scaledPix = pix.scaled(IMAGE_WIDTH, IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    startImageLabel_->setPixmap(scaledPix);
    startImageLabel_->setAlignment(Qt::AlignCenter);
}

void CompareImageView::setEndImageData(const QByteArray& data) {
    endImageData_ = data; // 이미지 데이터 저장
    if (data.isEmpty()) {
        endImageLabel_->setText("이미지 없음");
        return;
    }
    QPixmap pix;
    if (!pix.loadFromData(data, "JPEG") && !pix.loadFromData(data, "PNG")) {
        endImageLabel_->setText("이미지 오류");
        return;
    }
    QPixmap scaledPix = pix.scaled(IMAGE_WIDTH, IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    endImageLabel_->setPixmap(scaledPix);
    endImageLabel_->setAlignment(Qt::AlignCenter);
}

void CompareImageView::downloadStartImage() {
    if (startImageData_.isEmpty()) {
        QMessageBox::warning(this, "경고", "다운로드할 이미지가 없습니다.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "정차 시작 이미지 저장", 
        QString("start_image_%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JPEG Files (*.jpg);;PNG Files (*.png);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(startImageData_);
            file.close();
            QMessageBox::information(this, "완료", "정차 시작 이미지가 저장되었습니다.");
        } else {
            QMessageBox::warning(this, "오류", "파일을 저장할 수 없습니다.");
        }
    }
}

void CompareImageView::downloadEndImage() {
    if (endImageData_.isEmpty()) {
        QMessageBox::warning(this, "경고", "다운로드할 이미지가 없습니다.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "1분 경과 이미지 저장", 
        QString("end_image_%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "JPEG Files (*.jpg);;PNG Files (*.png);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(endImageData_);
            file.close();
            QMessageBox::information(this, "완료", "1분 경과 이미지가 저장되었습니다.");
        } else {
            QMessageBox::warning(this, "오류", "파일을 저장할 수 없습니다.");
        }
    }
}

void CompareImageView::printToPdf() {
    QString fileName = QFileDialog::getSaveFileName(this, 
        "PDF로 저장", 
        QString("parking_violation_report_%1.pdf").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "PDF Files (*.pdf)");
    
    if (!fileName.isEmpty()) {
        QPdfWriter pdfWriter(fileName);
        pdfWriter.setPageSize(QPageSize::A4);
        pdfWriter.setPageMargins(QMarginsF(20, 20, 20, 20));
        
        QPainter painter(&pdfWriter);
        
        // 현재 창을 PDF로 렌더링
        QPixmap windowPixmap = this->grab();
        
        // PDF 페이지 크기에 맞게 조정
        QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());
        QPixmap scaledPixmap = windowPixmap.scaled(pageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        // 중앙에 배치
        int x = (pageRect.width() - scaledPixmap.width()) / 2;
        int y = (pageRect.height() - scaledPixmap.height()) / 2;
        
        painter.drawPixmap(x, y, scaledPixmap);
        painter.end();
        
        QMessageBox::information(this, "완료", "불법 주정차 증명 자료 PDF가 저장되었습니다.");
    }
}
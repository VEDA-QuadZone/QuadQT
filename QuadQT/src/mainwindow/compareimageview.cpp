#include "mainwindow/compareimageview.h"
#include "mainwindow/filenameutils.h"
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
#include <QMouseEvent>
#include <QIcon>
#include <QRegularExpression>

constexpr int IMAGE_WIDTH = 427;
constexpr int IMAGE_HEIGHT = 240;
constexpr int TOTAL_WIDTH = IMAGE_WIDTH * 2 + 60; // 두 이미지 + 화살표 + 여백

CompareImageView::CompareImageView(const QString& event, const QString& plate,
                                   const QString& datetime, const QString& startFilename, 
                                   const QString& endFilename, QWidget* parent)
    : QDialog(parent), startFilename_(startFilename), endFilename_(endFilename), dragging_(false)
{
    // 이벤트 타입을 멤버 변수로 저장
    eventType_ = event;
    setStyleSheet("background:#fff;");
    
    // 프레임리스 창으로 설정하고 커스텀 타이틀바 생성
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    // --- 메인 레이아웃 ---
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1, 1, 1, 1); // 테두리를 위한 여백
    mainLayout->setSpacing(0);

    // --- 내용 컨테이너 (테두리 적용) ---
    QWidget* container = new QWidget(this);
    container->setObjectName("mainContainer");
    container->setFixedWidth(TOTAL_WIDTH);
    container->setStyleSheet("#mainContainer { background:#fff; border: 1px solid #ccc; }");
    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(10);
    containerLayout->setContentsMargins(10, 10, 10, 10);

    mainLayout->addWidget(container);

    // 커스텀 타이틀바 생성
    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(35);
    titleBar->setStyleSheet("background:#fff;");
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(8, 2, 8, 2);
    
    // 아이콘 (선택사항)
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/images/hanwha_icon.png").pixmap(20, 20));
    titleLayout->addWidget(iconLabel);
    
    // 제목 라벨 (가운데 정렬, 볼드 제거)
    QLabel* titleLabel = new QLabel("불법 주정차 증명 자료", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size:16px; color:#333; font-weight:bold; ");
    titleLayout->addWidget(titleLabel, 1);
    
    // 닫기 버튼
    QPushButton* closeBtn = new QPushButton("×", this);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setStyleSheet("QPushButton { border:none; background:transparent; font-size:18px; color:#666; } QPushButton:hover { background:#f0f0f0; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    titleLayout->addWidget(closeBtn);
    
    containerLayout->addWidget(titleBar);

    // --- 1. 정보 테이블 부분 (QLabel로 구현) ---
    QHBoxLayout* tableLayout = new QHBoxLayout;
    tableLayout->setSpacing(40); // 테이블 간격을 화살표와 맞춤
    
    // 왼쪽 테이블 (정차 시작)
    QWidget* leftTableWidget = new QWidget(this);
    leftTableWidget->setFixedSize(IMAGE_WIDTH, 96); // 3행 * 32px
    leftTableWidget->setStyleSheet("border: none; background: #fff;");
    
    QVBoxLayout* leftTableLayout = new QVBoxLayout(leftTableWidget);
    leftTableLayout->setSpacing(0);
    leftTableLayout->setContentsMargins(0, 0, 0, 0);
    
    // 헤더 (주황색)
    QLabel* leftHeader = new QLabel("정차 시작", leftTableWidget);
    leftHeader->setFixedHeight(32);
    leftHeader->setAlignment(Qt::AlignCenter);
    leftHeader->setStyleSheet("background: #FBB584; border-bottom: 1px solid #888; font-weight: normal;");
    leftTableLayout->addWidget(leftHeader);
    
    // 번호판 행
    QWidget* leftPlateRow = new QWidget(leftTableWidget);
    leftPlateRow->setFixedHeight(32);
    leftPlateRow->setStyleSheet("background: #fff; border-bottom: 1px solid #888;");
    QHBoxLayout* leftPlateLayout = new QHBoxLayout(leftPlateRow);
    leftPlateLayout->setContentsMargins(0, 0, 0, 0);
    leftPlateLayout->setSpacing(0);
    
    QLabel* leftPlateLabel = new QLabel("번호판", leftPlateRow);
    leftPlateLabel->setFixedWidth(80);
    leftPlateLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    leftPlateLabel->setStyleSheet("border: none; background: transparent; padding-left: 8px;");
    
    QLabel* leftPlateVal = new QLabel(plate, leftPlateRow);
    leftPlateVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    leftPlateVal->setStyleSheet("border: none; background: transparent; padding-left: 8px;");
    
    leftPlateLayout->addWidget(leftPlateLabel);
    leftPlateLayout->addWidget(leftPlateVal);
    leftTableLayout->addWidget(leftPlateRow);
    
    // 일시 행
    QWidget* leftTimeRow = new QWidget(leftTableWidget);
    leftTimeRow->setFixedHeight(32);
    leftTimeRow->setStyleSheet("background: #fff;");
    QHBoxLayout* leftTimeLayout = new QHBoxLayout(leftTimeRow);
    leftTimeLayout->setContentsMargins(0, 0, 0, 0);
    leftTimeLayout->setSpacing(0);
    
    QLabel* leftTimeLabel = new QLabel("일시", leftTimeRow);
    leftTimeLabel->setFixedWidth(80);
    leftTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    leftTimeLabel->setStyleSheet("border: none; background: transparent; padding-left: 8px;");
    
    QLabel* leftTimeVal = new QLabel(datetime, leftTimeRow);
    leftTimeVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    leftTimeVal->setStyleSheet("font-weight: bold; border: none; background: transparent; padding-left: 8px;");
    
    leftTimeLayout->addWidget(leftTimeLabel);
    leftTimeLayout->addWidget(leftTimeVal);
    leftTableLayout->addWidget(leftTimeRow);
    
    // 오른쪽 테이블 (정차 후 1분 경과)
    QWidget* rightTableWidget = new QWidget(this);
    rightTableWidget->setFixedSize(IMAGE_WIDTH, 96); // 3행 * 32px
    rightTableWidget->setStyleSheet("border: none; background: #fff;");
    
    QVBoxLayout* rightTableLayout = new QVBoxLayout(rightTableWidget);
    rightTableLayout->setSpacing(0);
    rightTableLayout->setContentsMargins(0, 0, 0, 0);
    
    // 헤더 (주황색)
    QLabel* rightHeader = new QLabel("정차 후 1분 경과", rightTableWidget);
    rightHeader->setFixedHeight(32);
    rightHeader->setAlignment(Qt::AlignCenter);
    rightHeader->setStyleSheet("background: #FBB584; border-bottom: 1px solid #888; font-weight: normal;");
    rightTableLayout->addWidget(rightHeader);
    
    // 번호판 행
    QWidget* rightPlateRow = new QWidget(rightTableWidget);
    rightPlateRow->setFixedHeight(32);
    rightPlateRow->setStyleSheet("background: #fff; border-bottom: 1px solid #888;");
    QHBoxLayout* rightPlateLayout = new QHBoxLayout(rightPlateRow);
    rightPlateLayout->setContentsMargins(0, 0, 0, 0);
    rightPlateLayout->setSpacing(0);
    
    QLabel* rightPlateLabel = new QLabel("번호판", rightPlateRow);
    rightPlateLabel->setFixedWidth(80);
    rightPlateLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    rightPlateLabel->setStyleSheet("border: none; background: transparent; padding-left: 8px;");
    
    QLabel* rightPlateVal = new QLabel(plate, rightPlateRow);
    rightPlateVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    rightPlateVal->setStyleSheet("border: none; background: transparent; padding-left: 8px;");
    
    rightPlateLayout->addWidget(rightPlateLabel);
    rightPlateLayout->addWidget(rightPlateVal);
    rightTableLayout->addWidget(rightPlateRow);
    
    // 일시 행
    QWidget* rightTimeRow = new QWidget(rightTableWidget);
    rightTimeRow->setFixedHeight(32);
    rightTimeRow->setStyleSheet("background: #fff;");
    QHBoxLayout* rightTimeLayout = new QHBoxLayout(rightTimeRow);
    rightTimeLayout->setContentsMargins(0, 0, 0, 0);
    rightTimeLayout->setSpacing(0);
    
    QLabel* rightTimeLabel = new QLabel("일시", rightTimeRow);
    rightTimeLabel->setFixedWidth(80);
    rightTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    rightTimeLabel->setStyleSheet("border: none; background: transparent; padding-left: 8px;");
    
    QLabel* rightTimeVal = new QLabel(datetime, rightTimeRow);
    rightTimeVal->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    rightTimeVal->setStyleSheet("font-weight: bold; border: none; background: transparent; padding-left: 8px;");
    
    rightTimeLayout->addWidget(rightTimeLabel);
    rightTimeLayout->addWidget(rightTimeVal);
    rightTableLayout->addWidget(rightTimeRow);

    tableLayout->addWidget(leftTableWidget);
    tableLayout->addWidget(rightTableWidget);
    
    containerLayout->addLayout(tableLayout);

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
    
    containerLayout->addLayout(imageLayout);

    // --- 3. 파일이름 행들 ---
    // 왼쪽 파일명
    QHBoxLayout* leftFileRow = new QHBoxLayout;
    leftFileRow->setContentsMargins(8, 0, 8, 0);
    
    QLabel* leftFileKey = new QLabel("파일 이름", this);
    leftFileKey->setMinimumWidth(64);
    leftFileKey->setStyleSheet("border:none; background:transparent; font-size:15px; color:#222;");
    leftFileRow->addWidget(leftFileKey);
    
    QString displayStartFilename = convertFilename(startFilename, event);
    startFilenameLabel_ = new QLabel(
        QString("<a href=\"#\" style=\"color:#1976D2;text-decoration:underline;\">%1</a>").arg(displayStartFilename), this);
    startFilenameLabel_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    startFilenameLabel_->setOpenExternalLinks(false);
    startFilenameLabel_->setStyleSheet("border:none; background:transparent; color:#1976D2; font-size:15px;");
    connect(startFilenameLabel_, &QLabel::linkActivated, [=](const QString&) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(startFilename));
    });
    leftFileRow->addWidget(startFilenameLabel_);
    leftFileRow->addStretch();
    
    downloadButton1_ = new QPushButton(this);
    downloadButton1_->setIcon(QIcon(":/images/download.png"));
    downloadButton1_->setFixedSize(24, 24);
    downloadButton1_->setStyleSheet("border:none; background:transparent; padding:0;");
    leftFileRow->addWidget(downloadButton1_);
    
    // 오른쪽 파일명
    QHBoxLayout* rightFileRow = new QHBoxLayout;
    rightFileRow->setContentsMargins(8, 0, 8, 0);
    
    QLabel* rightFileKey = new QLabel("파일 이름", this);
    rightFileKey->setMinimumWidth(64);
    rightFileKey->setStyleSheet("border:none; background:transparent; font-size:15px; color:#222;");
    rightFileRow->addWidget(rightFileKey);
    
    QString displayEndFilename = convertFilename(endFilename, event);
    endFilenameLabel_ = new QLabel(
        QString("<a href=\"#\" style=\"color:#1976D2;text-decoration:underline;\">%1</a>").arg(displayEndFilename), this);
    endFilenameLabel_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    endFilenameLabel_->setOpenExternalLinks(false);
    endFilenameLabel_->setStyleSheet("border:none; background:transparent; color:#1976D2; font-size:15px;");
    connect(endFilenameLabel_, &QLabel::linkActivated, [=](const QString&) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(endFilename));
    });
    rightFileRow->addWidget(endFilenameLabel_);
    rightFileRow->addStretch();
    
    downloadButton2_ = new QPushButton(this);
    downloadButton2_->setIcon(QIcon(":/images/download.png"));
    downloadButton2_->setFixedSize(24, 24);
    downloadButton2_->setStyleSheet("border:none; background:transparent; padding:0;");
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
    
    containerLayout->addLayout(fileRowsLayout);

    // --- 4. 인쇄/닫기 버튼 (중앙정렬) ---
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    printButton_ = new QPushButton("인쇄", this);
    printButton_->setFixedSize(72, 24);
    printButton_->setStyleSheet("background:#F37321; color:black; font-weight:bold; border:none;");
    closeButton_ = new QPushButton("닫기", this);
    closeButton_->setFixedSize(72, 24);
    closeButton_->setStyleSheet("background:#FBB584; color:black; border:none;");
    btnLayout->addWidget(printButton_);
    btnLayout->addSpacing(20);
    btnLayout->addWidget(closeButton_);
    btnLayout->addStretch();
    
    containerLayout->addSpacing(8);
    containerLayout->addLayout(btnLayout);

    setFixedWidth(TOTAL_WIDTH + 4); // 테두리 여백만 포함

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
    
    // 변환된 파일명을 기본 파일명으로 사용
    QString displayFilename = startFilenameLabel_->text();
    // HTML 태그 제거
    displayFilename = displayFilename.remove(QRegularExpression("<[^>]*>"));
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "정차 시작 이미지 저장", 
        QString("%1.jpg").arg(displayFilename),
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
    
    // 변환된 파일명을 기본 파일명으로 사용
    QString displayFilename = endFilenameLabel_->text();
    // HTML 태그 제거
    displayFilename = displayFilename.remove(QRegularExpression("<[^>]*>"));
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "1분 경과 이미지 저장", 
        QString("%1.jpg").arg(displayFilename),
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
void CompareImageView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragPosition_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
        dragging_ = true;
        event->accept();
    }
}

void CompareImageView::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton && dragging_) {
        move(event->globalPosition().toPoint() - dragPosition_);
        event->accept();
    }
}

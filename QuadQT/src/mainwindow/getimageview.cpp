#include "mainwindow/getimageview.h"
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

GetImageView::GetImageView(const QString& event, const QString& plate,
                           const QString& datetime, const QString& filename, QWidget* parent)
    : QDialog(parent), dragging_(false)
{
    // 이벤트 타입을 멤버 변수로 저장
    eventType_ = event;
    setStyleSheet("QDialog { background:#fff; border: 1px solid #ccc; }");
    
    // 프레임리스 창으로 설정하고 커스텀 타이틀바 생성
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    // --- 메인 레이아웃 ---
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8); // 상하좌우 패딩 추가
    mainLayout->setSpacing(0);

    // --- 내용 컨테이너 (테두리 적용) ---
    QWidget* container = new QWidget(this);
    container->setObjectName("mainContainer");
    container->setFixedWidth(IMAGE_WIDTH);
    container->setStyleSheet("#mainContainer { background:#fff; border: none; }");
    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(4,4,4,4); // 컨테이너 내부 패딩 추가

    mainLayout->addWidget(container);

    // 커스텀 타이틀바 생성
    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(35);
    titleBar->setStyleSheet("background:#fff; border:none;");
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(8, 2, 8, 2);
    
    // 아이콘 (선택사항)
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/images/hanwha_icon.png").pixmap(20, 20));
    titleLayout->addWidget(iconLabel);
    
    // 왼쪽 스페이서 (아이콘과 닫기 버튼 크기 차이를 보정)
    titleLayout->addSpacing(20);
    
    // 제목 라벨 (가운데 정렬)
    QLabel* titleLabel = new QLabel("    이미지 상세정보", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size:16px; color:#333; font-weight:bold;");
    titleLayout->addWidget(titleLabel, 1);
    
    // 닫기 버튼
    QPushButton* closeBtn = new QPushButton("×", this);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setStyleSheet("QPushButton { border:none; background:transparent; font-size:18px; color:#666; } QPushButton:hover { background:#f0f0f0; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    titleLayout->addWidget(closeBtn);
    
    containerLayout->addWidget(titleBar);

    // --- 1. 정보 테이블 부분 ---
    int rowCount = (plate != "-" && !plate.trimmed().isEmpty()) ? 3 : 2;
    QTableWidget* table = new QTableWidget(rowCount, 2, this);
    table->setFixedHeight(rowCount * 32);
    table->setFixedWidth(IMAGE_WIDTH);
    table->setColumnWidth(0, 45);  // 레이블 컬럼 너비를 70px로 설정하여 값들이 더 가운데 오도록 조정
    table->setColumnWidth(1, IMAGE_WIDTH - 75);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setShowGrid(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setStyleSheet(
        "QTableWidget, QHeaderView::section { background: #fff; border: none; }"
        "QTableWidget::item { border-bottom: 1px solid #888; border-left: none; border-right: none; border-top: none; }"
        );

    // 이벤트 유형
    auto* typeItem = new QTableWidgetItem("유형");
    QFont labelBoldFont = typeItem->font();
    labelBoldFont.setBold(true);
    typeItem->setFont(labelBoldFont);
    auto* typeVal  = new QTableWidgetItem(event);
    typeVal->setTextAlignment(Qt::AlignCenter);
    table->setItem(0, 0, typeItem);
    table->setItem(0, 1, typeVal);

    int curRow = 1;
    // 번호판(필요시)
    if (plate != "-" && !plate.trimmed().isEmpty()) {
        auto* plateItem = new QTableWidgetItem("번호판");
        plateItem->setFont(labelBoldFont);
        auto* plateVal  = new QTableWidgetItem(plate);
        plateVal->setTextAlignment(Qt::AlignCenter);
        table->setItem(curRow, 0, plateItem);
        table->setItem(curRow, 1, plateVal);
        ++curRow;
    }
    // 일시 (레이블만 bold)
    auto* timeItem = new QTableWidgetItem("일시");
    timeItem->setFont(labelBoldFont);
    auto* timeVal  = new QTableWidgetItem(datetime);
    timeVal->setTextAlignment(Qt::AlignCenter);
    table->setItem(curRow, 0, timeItem);
    table->setItem(curRow, 1, timeVal);

    containerLayout->addWidget(table);

    // --- 2. 이미지 영역 ---
    imageLabel_ = new QLabel(this);
    imageLabel_->setFixedSize(IMAGE_WIDTH, IMAGE_HEIGHT);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setStyleSheet("background:#ccc; border:none; color:#888; font-size:18px;");
    imageLabel_->setText("이미지");
    containerLayout->addWidget(imageLabel_);

    // --- 3. 파일이름 행 ---
    QHBoxLayout* fileRow = new QHBoxLayout;
    fileRow->setContentsMargins(8, 0, 8, 0);

    // "파일 이름" 라벨 (볼드, 작은 크기)
    QLabel* fileKey = new QLabel("파일 이름", this);
    fileKey->setMinimumWidth(64);
    fileKey->setStyleSheet("border:none; background:transparent; font-size:12px; color:#222; font-weight:bold;");
    fileRow->addWidget(fileKey);

    // 파일명(하이퍼링크처럼) - 변환된 파일명 사용 (이벤트 타입 전달)
    QString displayFilename = convertFilename(filename, event);
    filenameLabel_ = new QLabel(
        QString("<a href=\"#\" style=\"color:#1976D2;text-decoration:underline;font-weight:bold;\">%1</a>").arg(displayFilename), this);
    filenameLabel_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    filenameLabel_->setOpenExternalLinks(false);
    filenameLabel_->setAlignment(Qt::AlignCenter);
    filenameLabel_->setStyleSheet("border:none; background:transparent; color:#1976D2; font-size:12px; font-weight:bold; text-align:center;");
    connect(filenameLabel_, &QLabel::linkActivated, [=](const QString&) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    });
    fileRow->addWidget(filenameLabel_, 1, Qt::AlignCenter);

    // Stretch for right-align download button
    fileRow->addStretch();

    // 다운로드 버튼 - 리소스 이미지 사용
    downloadButton_ = new QPushButton(this);
    downloadButton_->setIcon(QIcon(":/images/download.png"));
    downloadButton_->setFixedSize(24, 24);
    downloadButton_->setStyleSheet("border:none; background:transparent; padding:0;");
    fileRow->addWidget(downloadButton_);

    containerLayout->addLayout(fileRow);

    // --- 4. 인쇄/닫기 버튼 (중앙정렬) ---
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    printButton_ = new QPushButton("인쇄", this);
    printButton_->setFixedSize(72, 24);
    printButton_->setStyleSheet("background:#F37321; color:black; border:none;");
    closeButton_ = new QPushButton("닫기", this);
    closeButton_->setFixedSize(72, 24);
    closeButton_->setStyleSheet("background:#FBB584; color:black; border:none;");
    btnLayout->addWidget(printButton_);
    btnLayout->addSpacing(20);
    btnLayout->addWidget(closeButton_);
    btnLayout->addStretch();
    
    containerLayout->addSpacing(8);
    containerLayout->addLayout(btnLayout);

    setFixedWidth(IMAGE_WIDTH + 18); // 패딩과 테두리 여백 포함 (8*2 + 2)

    // 버튼 연결
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(downloadButton_, &QPushButton::clicked, this, &GetImageView::downloadImage);
    connect(printButton_, &QPushButton::clicked, this, &GetImageView::printToPdf);
}

void GetImageView::setImageData(const QByteArray& data) {
    imageData_ = data; // 이미지 데이터 저장
    if (data.isEmpty()) {
        imageLabel_->setText("이미지 없음");
        return;
    }
    QPixmap pix;
    if (!pix.loadFromData(data, "JPEG") && !pix.loadFromData(data, "PNG")) {
        imageLabel_->setText("이미지 오류");
        return;
    }
    // 427x240에 원본 비율 유지하며 최대 크기로 맞춤
    QPixmap scaledPix = pix.scaled(IMAGE_WIDTH, IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel_->setPixmap(scaledPix);
    imageLabel_->setAlignment(Qt::AlignCenter);
}

void GetImageView::downloadImage() {
    if (imageData_.isEmpty()) {
        QMessageBox::warning(this, "경고", "다운로드할 이미지가 없습니다.");
        return;
    }
    
    // 변환된 파일명을 기본 파일명으로 사용
    QString displayFilename = filenameLabel_->text();
    // HTML 태그 제거
    displayFilename = displayFilename.remove(QRegularExpression("<[^>]*>"));
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "이미지 저장", 
        QString("%1.jpg").arg(displayFilename),
        "JPEG Files (*.jpg);;PNG Files (*.png);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(imageData_);
            file.close();
            QMessageBox::information(this, "완료", "이미지가 저장되었습니다.");
        } else {
            QMessageBox::warning(this, "오류", "파일을 저장할 수 없습니다.");
        }
    }
}

void GetImageView::printToPdf() {
    QString fileName = QFileDialog::getSaveFileName(this, 
        "PDF로 저장", 
        QString("report_%1.pdf").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
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
        
        QMessageBox::information(this, "완료", "PDF가 저장되었습니다.");
    }
}

void GetImageView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragPosition_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
        dragging_ = true;
        event->accept();
    }
}

void GetImageView::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton && dragging_) {
        move(event->globalPosition().toPoint() - dragPosition_);
        event->accept();
    }
}
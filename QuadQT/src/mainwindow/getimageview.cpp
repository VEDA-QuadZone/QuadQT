#include "mainwindow/getimageview.h"
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

GetImageView::GetImageView(const QString& event, const QString& plate,
                           const QString& datetime, const QString& filename, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("상세정보");
    setStyleSheet("background:#fff;");

    // --- 전체 테두리 Frame ---
    QFrame* border = new QFrame(this);
    border->setFrameShape(QFrame::Box);
    border->setLineWidth(1);
    border->setStyleSheet("QFrame { border: 1px solid #222; border-radius: 3px; background:#fff; }");
    border->setFixedWidth(IMAGE_WIDTH);
    QVBoxLayout* borderLayout = new QVBoxLayout(border);
    borderLayout->setSpacing(0);
    borderLayout->setContentsMargins(0,0,0,0);

    // --- 1. 정보 테이블 부분 ---
    int rowCount = (plate != "-" && !plate.trimmed().isEmpty()) ? 3 : 2;
    QTableWidget* table = new QTableWidget(rowCount, 2, this);
    table->setFixedHeight(rowCount * 32);
    table->setFixedWidth(IMAGE_WIDTH);
    table->setColumnWidth(0, 120);
    table->setColumnWidth(1, IMAGE_WIDTH - 120 - 2);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setShowGrid(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setStyleSheet(
        "QTableWidget, QHeaderView::section { background: #fff; border: none; }"
        "QTableWidget::item { border-bottom: 1px solid #888; }"
        );

    // 이벤트 유형
    auto* typeItem = new QTableWidgetItem("이벤트 유형");
    auto* typeVal  = new QTableWidgetItem(event);
    typeVal->setTextAlignment(Qt::AlignCenter);
    table->setItem(0, 0, typeItem);
    table->setItem(0, 1, typeVal);

    int curRow = 1;
    // 번호판(필요시)
    if (plate != "-" && !plate.trimmed().isEmpty()) {
        auto* plateItem = new QTableWidgetItem("번호판");
        auto* plateVal  = new QTableWidgetItem(plate);
        plateVal->setTextAlignment(Qt::AlignCenter);
        table->setItem(curRow, 0, plateItem);
        table->setItem(curRow, 1, plateVal);
        ++curRow;
    }
    // 일시 (값 bold)
    auto* timeItem = new QTableWidgetItem("일시");
    auto* timeVal  = new QTableWidgetItem(datetime);
    QFont boldFont = timeVal->font();
    boldFont.setBold(true);
    timeVal->setFont(boldFont);
    timeVal->setTextAlignment(Qt::AlignCenter);
    table->setItem(curRow, 0, timeItem);
    table->setItem(curRow, 1, timeVal);

    borderLayout->addWidget(table);

    // --- 2. 이미지 영역 ---
    imageLabel_ = new QLabel(this);
    imageLabel_->setFixedSize(IMAGE_WIDTH, IMAGE_HEIGHT);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setStyleSheet("background:#ccc; border:none; color:#888; font-size:18px;");
    imageLabel_->setText("이미지");
    borderLayout->addWidget(imageLabel_);

    // --- 3. 파일이름 행 ---
    QHBoxLayout* fileRow = new QHBoxLayout;
    fileRow->setContentsMargins(8, 0, 8, 0);

    // "파일 이름" 라벨 (border, background 제거)
    QLabel* fileKey = new QLabel("파일 이름", this);
    fileKey->setMinimumWidth(64);
    fileKey->setStyleSheet("border:none; background:transparent; font-size:15px; color:#222;");
    fileRow->addWidget(fileKey);

    // 파일명(하이퍼링크처럼)
    filenameLabel_ = new QLabel(
        QString("<a href=\"#\" style=\"color:#1976D2;text-decoration:underline;\">%1</a>").arg(filename), this);
    filenameLabel_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    filenameLabel_->setOpenExternalLinks(false);
    filenameLabel_->setStyleSheet("border:none; background:transparent; color:#1976D2; font-size:15px;");
    connect(filenameLabel_, &QLabel::linkActivated, [=](const QString&) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    });
    fileRow->addWidget(filenameLabel_);

    // Stretch for right-align download button
    fileRow->addStretch();

    // 다운로드 버튼 (border, background 제거)
    downloadButton_ = new QPushButton("⬇", this);
    downloadButton_->setFixedWidth(28);
    downloadButton_->setStyleSheet("border:none; background:transparent; font-size:15px; padding:0 2px;");
    fileRow->addWidget(downloadButton_);

    borderLayout->addLayout(fileRow);

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

    setFixedWidth(IMAGE_WIDTH + 32); // 좌우 마진 여유 포함
    setLayout(vLayout);

    // 닫기
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);
}

void GetImageView::setImageData(const QByteArray& data) {
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

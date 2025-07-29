// historyview.cpp
#include "mainwindow/historyview.h"
#include "mainwindow/tcphistoryhandler.h"

#include <QResizeEvent>
#include <QDate>
#include <QHeaderView>
#include <QMenu>
#include <QHBoxLayout>
#include <QLabel>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>

static constexpr int PAGE_SIZE = 16;

HistoryView::HistoryView(QWidget *parent)
    : QWidget(parent),
    calendarForStart(false),
    currentPage(0)
{
    this->setStyleSheet("background-color: #FFFFFF;");  // 원하는 배경색으로 변경
    // 1) 제목
    titleLabel = new QLabel("히스토리", this);
    titleLabel->setStyleSheet("font-size:16px;font-weight:bold;");
    titleLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    // 2) 테이블 설정
    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(8);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setShowGrid(false);
    tableWidget->setHorizontalHeaderLabels({
        "", "날짜", "유형", "이미지",
        "번호판", "속도", "정차 시작 이미지", "1분 경과 이미지"
    });
    tableWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { background:#FBB584; padding:4px; border:1px solid #E0A070; }"
        );
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableWidget->setStyleSheet(
        "QTableWidget::item { border-bottom:1px solid #D3D3D3; }"
        "QTableWidget::item:selected { background-color:#B3B3B3; }"
        );
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(tableWidget, &QTableWidget::cellClicked,
            this, &HistoryView::onImageCellClicked);

    // 3) 헤더 체크박스
    headerCheck = new QCheckBox(tableWidget->horizontalHeader());
    headerCheck->setChecked(false);
    connect(headerCheck, &QCheckBox::toggled, this, [this](bool on){
        // 페이지 내 모든 행 체크/언체크
        for(int r=0; r<tableWidget->rowCount(); ++r){
            auto* cell = tableWidget->cellWidget(r,0);
            if(!cell) continue;
            auto* cb = cell->findChild<QCheckBox*>();
            if(!cb) continue;
            cb->blockSignals(true);
            cb->setChecked(on);
            cb->blockSignals(false);
            int id = tableWidget->item(r,1)->data(Qt::UserRole).toInt();
            QModelIndex idx = tableWidget->model()->index(r,0);
            if(on){
                selectedRecordIds.insert(id);
                tableWidget->selectionModel()->select(idx,
                                                      QItemSelectionModel::Select|QItemSelectionModel::Rows);
            } else {
                selectedRecordIds.remove(id);
                tableWidget->selectionModel()->select(idx,
                                                      QItemSelectionModel::Deselect|QItemSelectionModel::Rows);
            }
        }
    });

    // 4) 날짜 버튼
    startDateButton = new QPushButton(this);
    startDateButton->setText("시작일 선택하기");
    startDateButton->setIcon(QIcon(":/images/images/calendar.png"));
    startDateButton->setIconSize(QSize(16,16));
    startDateButton->setStyleSheet(
        "QPushButton{padding:4px;background:transparent;border:none;text-align:right;}"
        );
    arrowLabel = new QLabel(this);
    arrowLabel->setPixmap(QPixmap(":/images/images/sign.png").scaled(16,16,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    arrowLabel->setAlignment(Qt::AlignCenter);
    endDateButton = new QPushButton(this);
    endDateButton->setText("종료일 선택하기");
    endDateButton->setIcon(QIcon(":/images/images/calendar.png"));
    endDateButton->setIconSize(QSize(16,16));
    endDateButton->setStyleSheet(
        "QPushButton{padding:4px;background:transparent;border:none;text-align:left;}"
        );
    connect(startDateButton,&QPushButton::clicked,this,&HistoryView::openStartDatePicker);
    connect(endDateButton,  &QPushButton::clicked,this,&HistoryView::openEndDatePicker);

    // 5) 필터 버튼
    filterButton = new QToolButton(this);
    filterButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    filterButton->setLayoutDirection(Qt::RightToLeft);
    filterButton->setText("유형");
    filterButton->setIcon(QIcon(":/images/images/below.png"));
    filterButton->setPopupMode(QToolButton::InstantPopup);
    filterButton->setStyleSheet(R"(
        QToolButton{background:#FBB584;border:none;padding:4px;text-align:center;}
        QToolButton::menu-indicator{image:none;}
    )");
    QMenu* fm = new QMenu(this);
    fm->setStyleSheet(R"(
        QMenu{background:#FBB584;border:none;}
        QMenu::item{background:#FBB584;padding:4px 20px;}
        QMenu::item:selected{background:#E0A070;}
    )");
    auto addF=[&](auto txt){
        QAction* a=fm->addAction(txt);
        connect(a,&QAction::triggered,this,[this,txt](){
            currentFilter=txt;
            filterButton->setText(txt);
            currentPage=0;
            requestPage();
        });
    };
    addF("주정차감지"); addF("어린이감지"); addF("과속감지");
    filterButton->setMenu(fm);

    // 6) 다운로드 버튼
    downloadButton = new QPushButton(this);
    downloadButton->setIcon(QIcon(":/images/images/download.png"));
    downloadButton->setFlat(true);
    connect(downloadButton,&QPushButton::clicked,this,&HistoryView::exportCsv);

    // 7) 캘린더
    calendarWidget = new QCalendarWidget(this);
    calendarWidget->setWindowFlags(Qt::Popup);
    calendarWidget->hide();
    connect(calendarWidget, &QCalendarWidget::activated,
            this, &HistoryView::dateSelected);

    // TCP 핸들러
    tcpHandler_      = new TcpHistoryHandler(this);
    tcpImageHandler_ = new TcpImageHandler(this);
    connect(tcpHandler_, &TcpHistoryHandler::historyDataReady, this, &HistoryView::onHistoryData);
    connect(tcpHandler_, &TcpHistoryHandler::errorOccurred,   this, &HistoryView::onHistoryError);
    connect(tcpImageHandler_, &TcpImageHandler::imageDataReady,
            this, &HistoryView::onImageDataReady);
    connect(tcpImageHandler_, &TcpImageHandler::errorOccurred,
            this, [this](auto e){ QMessageBox::warning(this, "Image Error", e); });
    tcpHandler_->connectToServer("192.168.0.10", 8080);
    // 페이징
    setupPaginationUI();

    // 최초 요청
    requestPage();
}
QString HistoryView::parseTimestampFromPath(const QString& path) {
    QRegularExpression re(R"(20\d{6}_\d{6})");
    QRegularExpressionMatch match = re.match(path);
    if (!match.hasMatch()) return "";

    QString raw = match.captured(); // 예: "20250725_114251"
    if (raw.length() != 15) return "";

    QString date = raw.mid(0, 8);  // 20250725
    QString time = raw.mid(9, 6);  // 114251

    return QString("%1-%2-%3 %4:%5:%6")
        .arg(date.mid(0, 4))
        .arg(date.mid(4, 2))
        .arg(date.mid(6, 2))
        .arg(time.mid(0, 2))
        .arg(time.mid(2, 2))
        .arg(time.mid(4, 2));
}
void HistoryView::onImageCellClicked(int row, int col) {
    if (col != 3 && col != 6 && col != 7) return;
    auto* it = tableWidget->item(row, col);
    if (!it) return;
    QString path = it->text();
    if (path.isEmpty()) return;

    // 📌 타임스탬프 추출
    QString timestamp = parseTimestampFromPath(path);

    // 뷰어 생성
    currentImageView_ = new GetImageView(path, timestamp, this);
    currentImageView_->show();

    tcpImageHandler_->connectToServerThenRequestImage("192.168.0.10", 8080, path);
}


void HistoryView::onImageDataReady(const QString& path, const QByteArray& data) {
    if (currentImageView_) {
        currentImageView_->setImageData(data);
    }
}

void HistoryView::exportCsv()
{
    QString path = QFileDialog::getSaveFileName(this, tr("CSV로 저장"), QString(), tr("CSV Files (*.csv)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) return;

    file.write("\xEF\xBB\xBF"); // BOM
    QStringList headers = {
        "날짜","유형","이미지","번호판","속도","정차 시작 이미지","1분 경과 이미지"
    };
    file.write(headers.join(',').toUtf8() + "\r\n");

    static const QStringList typeNames = {"주정차감지","과속감지","어린이감지"};
    for (int id : selectedRecordIds) {
        auto obj = recordDataMap.value(id);
        if (obj.isEmpty()) continue;
        QStringList cols;
        cols << obj.value("date").toString();
        int et = obj.value("event_type").toInt();
        cols << (et>=0&&et<typeNames.size()?typeNames[et]:QString::number(et));
        cols << obj.value("image_path").toString();
        cols << obj.value("plate_number").toString();
        QJsonValue sp = obj.value("speed");
        cols << (sp.isDouble()?QString::number(sp.toDouble(),'f',2):QString("-"));
        cols << obj.value("start_snapshot").toString();
        cols << obj.value("end_snapshot").toString();
        file.write(cols.join(',').toUtf8() + "\r\n");
    }
    file.close();
}

void HistoryView::requestPage()
{
    int offset = currentPage * PAGE_SIZE;
    static const QMap<QString,int> filterMap = {
        {"주정차감지",0},{"과속감지",1},{"어린이감지",2}
    };

    if (currentFilter.isEmpty()) {
        if (!startDate.isEmpty() && !endDate.isEmpty())
            tcpHandler_->getHistoryByDateRange(currentEmail,startDate,endDate,PAGE_SIZE,offset);
        else
            tcpHandler_->getHistory(currentEmail,PAGE_SIZE,offset);
    } else {
        int et = filterMap.value(currentFilter,-1);
        if (et<0) return;
        if (!startDate.isEmpty() && !endDate.isEmpty())
            tcpHandler_->getHistoryByEventTypeAndDateRange(currentEmail,et,startDate,endDate,PAGE_SIZE,offset);
        else
            tcpHandler_->getHistoryByEventType(currentEmail,et,PAGE_SIZE,offset);
    }
}

void HistoryView::onHistoryData(const QJsonObject &resp)
{
    tableWidget->clearContents();
    QJsonArray arr = resp.value("data").toArray();
    int count = arr.size();
    tableWidget->setRowCount(count);

    static const QStringList typeNames = {"주정차감지","과속감지","어린이감지"};
    for (int i = 0; i < count; ++i) {
        auto obj = arr.at(i).toObject();
        int id = obj.value("id").toInt();
        recordDataMap[id] = obj;

        // 체크박스
        QWidget* cell = new QWidget(this);
        auto* cb = new QCheckBox(cell);
        cb->setChecked(selectedRecordIds.contains(id));
        auto* lay = new QHBoxLayout(cell);
        lay->addWidget(cb);
        lay->setAlignment(Qt::AlignCenter);
        lay->setContentsMargins(0,0,0,0);
        cell->setLayout(lay);
        tableWidget->setCellWidget(i,0,cell);

        connect(cb, &QCheckBox::toggled, this, [this,id,i](bool on){
            if (on) {
                selectedRecordIds.insert(id);
                // row 하이라이트
                QModelIndex idx = tableWidget->model()->index(i, 0);
                tableWidget->selectionModel()->select(
                    idx,
                    QItemSelectionModel::Select | QItemSelectionModel::Rows
                    );
            } else {
                selectedRecordIds.remove(id);
                QModelIndex idx = tableWidget->model()->index(i, 0);
                tableWidget->selectionModel()->select(
                    idx,
                    QItemSelectionModel::Deselect | QItemSelectionModel::Rows
                    );
            }
            // header 상태 동기화
            bool all = true;
            for (int r = 0; r < tableWidget->rowCount(); ++r) {
                auto* c = tableWidget->cellWidget(r,0);
                if (auto* box = c?c->findChild<QCheckBox*>():nullptr) {
                    if (!box->isChecked()) { all = false; break; }
                }
            }
            headerCheck->blockSignals(true);
            headerCheck->setChecked(all);
            headerCheck->blockSignals(false);
        });

        // 날짜 (ID 저장)
        // 날짜 (ID 저장) - 이미지 파일명에서 타임스탬프 추출
        QString imagePath = obj.value("image_path").toString();
        QString parsedTime = parseTimestampFromPath(imagePath);
        QTableWidgetItem* dateIt = new QTableWidgetItem(parsedTime.isEmpty() ? obj.value("date").toString() : parsedTime);
        dateIt->setTextAlignment(Qt::AlignCenter);
        dateIt->setData(Qt::UserRole, id);
        tableWidget->setItem(i,1,dateIt);

        // 유형
        int et = obj.value("event_type").toInt();
        QString ts = (et>=0&&et<typeNames.size()?typeNames[et]:QString::number(et));
        QWidget* tc = new QWidget(this);
        QLabel* lb = new QLabel(ts,tc);
        lb->setStyleSheet("background-color:#B3B3B3;border-radius:8px;padding:2px 6px;");
        lb->setAlignment(Qt::AlignCenter);
        auto* tl = new QHBoxLayout(tc);
        tl->addWidget(lb);
        tl->setAlignment(Qt::AlignCenter);
        tl->setContentsMargins(0,0,0,0);
        tc->setLayout(tl);
        tableWidget->setCellWidget(i,2,tc);

        // 나머지 열
        tableWidget->setItem(i,3,new QTableWidgetItem(obj.value("image_path").toString()));
        QTableWidgetItem* pi = new QTableWidgetItem(obj.value("plate_number").toString());
        pi->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(i,4,pi);
        QJsonValue sp = obj.value("speed");
        QTableWidgetItem* si = new QTableWidgetItem(
            sp.isDouble()?QString::number(sp.toDouble(),'f',2):QString("-"));
        si->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(i,5,si);
        tableWidget->setItem(i,6,new QTableWidgetItem(obj.value("start_snapshot").toString()));
        tableWidget->setItem(i,7,new QTableWidgetItem(obj.value("end_snapshot").toString()));
    }

    // 페이징 업데이트
    prevButton->setEnabled(currentPage > 0);
    nextButton->setEnabled(count == PAGE_SIZE);
    pageLabel->setText(QString::number(currentPage + 1));

    // 헤더 체크박스 리셋 & 동기화
    headerCheck->blockSignals(true);
    bool allChecked = true;
    for (int r = 0; r < tableWidget->rowCount(); ++r) {
        auto* c = tableWidget->cellWidget(r,0);
        if (auto* box = c?c->findChild<QCheckBox*>():nullptr) {
            if (!box->isChecked()) { allChecked = false; break; }
        }
    }
    headerCheck->setChecked(allChecked);
    headerCheck->blockSignals(false);

    // 테이블 높이 동적 조정 (행 높이 고정)
    // rowCount 만큼만 공간 할당, row 높이는 Fixed 모드로 uH로 설정됨
    resizeEvent(nullptr);
}

void HistoryView::onHistoryError(const QString &err)
{
    QMessageBox::warning(this, tr("통신 오류"), err);
}

void HistoryView::prevPage()
{
    if (currentPage > 0) { --currentPage; requestPage(); }
}

void HistoryView::nextPage()
{
    ++currentPage; requestPage();
}

void HistoryView::setupPaginationUI()
{
    prevButton = new QPushButton("< 이전", this);
    nextButton = new QPushButton("다음 >", this);
    pageLabel  = new QLabel("1", this);
    pageLabel->setAlignment(Qt::AlignCenter);
    prevButton->setEnabled(false);
    connect(prevButton, &QPushButton::clicked, this, &HistoryView::prevPage);
    connect(nextButton, &QPushButton::clicked, this, &HistoryView::nextPage);
}

void HistoryView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    int w = width();
    int h = height();
    double wu = w / 24.0;
    double hu = h / 21.0;
    int uW = int(wu);
    int uH = int(hu);

    int yOffset = hu * 3;  // 올릴 높이

    titleLabel->setGeometry(wu*1, hu*3 - yOffset, wu*1, uH);

    // 테이블 높이: header + 실제 row 개수
    int rows = tableWidget->rowCount();
    int tableH = uH * (1 + rows); // 헤더 1행 + 데이터 rows
    tableWidget->setGeometry(wu*1, hu*4 - yOffset, wu*22, tableH);

    // 각 열 너비, 행 높이 고정
    static constexpr int cw[8] = {1,3,2,4,2,2,4,4};
    for (int c = 0; c < 8; ++c)
        tableWidget->setColumnWidth(c, cw[c] * uW);
    tableWidget->verticalHeader()->setDefaultSectionSize(uH);
    tableWidget->horizontalHeader()->setFixedHeight(uH);

    // header checkbox 위치
    QHeaderView* hh = tableWidget->horizontalHeader();
    int x0 = hh->sectionPosition(0);
    int w0 = hh->sectionSize(0);
    int hh_h = hh->height();
    int cbSize = hh_h - 4;
    headerCheck->setGeometry(
        x0 + (w0 - cbSize)/2,
        (hh_h - cbSize)/2 + tableWidget->y(), // headerCheck는 tableWidget 안에 있으므로 y도 같이 위로 이동됨
        cbSize, cbSize
        );

    startDateButton ->setGeometry(wu*13+wu*0.5, hu*3 - yOffset, wu*2.5, uH);
    arrowLabel      ->setGeometry(wu*16, hu*3 - yOffset, wu*0.5, uH);
    endDateButton   ->setGeometry(wu*16+wu*0.5, hu*3 - yOffset, wu*2.5, uH);
    filterButton    ->setGeometry(wu*19, hu*3 - yOffset, wu*3, uH);
    downloadButton  ->setGeometry(wu*22, hu*3 - yOffset, wu*1, uH);

    // 페이징: 가로 8칸, 중앙, row22
    int navX = int(wu*8);
    int navY = int(hu*22 - yOffset);
    int navW = int(wu*8);
    int navH = uH;
    int btnW = navW / 3;
    prevButton->setGeometry(navX,           navY, btnW, navH);
    pageLabel ->setGeometry(navX + btnW,    navY, btnW, navH);
    nextButton->setGeometry(navX + btnW*2,  navY, btnW, navH);
}


void HistoryView::openStartDatePicker()
{
    calendarForStart = true;
    calendarWidget->show();
}

void HistoryView::openEndDatePicker()
{
    calendarForStart = false;
    calendarWidget->show();
}

void HistoryView::dateSelected()
{
    QString txt = calendarWidget->selectedDate().toString("yyyy-MM-dd");
    (calendarForStart ? startDateButton : endDateButton)->setText(txt);
    calendarWidget->hide();
    if (!startDateButton->text().isEmpty() && !endDateButton->text().isEmpty()) {
        startDate = startDateButton->text();
        endDate   = endDateButton->text();
        currentPage = 0;
        requestPage();
    }
}

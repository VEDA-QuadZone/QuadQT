// historyview.cpp
#include "mainwindow/historyview.h"
#include "mainwindow/tcphistoryhandler.h"

#include <QResizeEvent>
#include <QDate>
#include <QHeaderView>
#include <QMenu>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QTimer>

static constexpr int PAGE_SIZE = 16;

HistoryView::HistoryView(QWidget *parent)
    : QWidget(parent),
    calendarForStart(false),
    currentPage(0)
{
    this->setStyleSheet("background-color: #FFFFFF;");  // 원하는 배경색으로 변경
    // 1) 제목
    titleLabel = new QLabel("히스토리", this);
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
    // 테이블 헤더 스타일은 resizeEvent에서 동적으로 설정
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableWidget->setStyleSheet(
        "QTableWidget { border: none; }"
        "QTableWidget::item { border-bottom:1px solid #D3D3D3; border-left: none; border-right: none; border-top: none; }"
        "QTableWidget::item:selected { background-color:#B3B3B3; }"
        );
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(tableWidget, &QTableWidget::cellClicked,
            this, &HistoryView::onImageCellClicked);
    
    // 행 선택 변경 시 유형 열 배경색 업데이트
    connect(tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &HistoryView::updateTypeColumnBackground);

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
    startDateButton->setIcon(QIcon(":/images/calendar.png"));
    // 스타일과 아이콘 크기는 resizeEvent에서 동적으로 설정
    arrowLabel = new QLabel(this);
    // 화살표 이미지 크기는 resizeEvent에서 동적으로 설정
    arrowLabel->setAlignment(Qt::AlignCenter);
    endDateButton = new QPushButton(this);
    endDateButton->setText("종료일 선택하기");
    endDateButton->setIcon(QIcon(":/images/calendar.png"));
    // 스타일과 아이콘 크기는 resizeEvent에서 동적으로 설정
    connect(startDateButton,&QPushButton::clicked,this,&HistoryView::openStartDatePicker);
    connect(endDateButton,  &QPushButton::clicked,this,&HistoryView::openEndDatePicker);

    // 5) 필터 버튼
    filterButton = new QToolButton(this);
    filterButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    filterButton->setLayoutDirection(Qt::LeftToRight);
    filterButton->setText("유형");
    filterButton->setIcon(QIcon(":/images/below.png"));
    filterButton->setPopupMode(QToolButton::InstantPopup);
    // filterButton 스타일은 resizeEvent에서 동적으로 설정
    QMenu* fm = new QMenu(this);
    fm->setStyleSheet(R"(
        QMenu{background:#F5D5B8;border:none;}
        QMenu::item{background:#F5D5B8;padding:4px 20px;}
        QMenu::item:selected{background:#E8C4A0;}
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
    downloadButton->setIcon(QIcon(":/images/download.png"));
    downloadButton->setFlat(true);
    connect(downloadButton,&QPushButton::clicked,this,&HistoryView::exportCsv);

    // 7) 캘린더 컨테이너
    calendarContainer = new QWidget(this);
    calendarContainer->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    calendarContainer->hide();
    // 캘린더 컨테이너 스타일은 resizeEvent에서 동적으로 설정
    
    // 캘린더 위젯을 레이아웃으로 배치
    QVBoxLayout* calendarLayout = new QVBoxLayout(calendarContainer);
    // 캘린더 마진은 resizeEvent에서 동적으로 설정
    calendarLayout->setSpacing(0);
    
    calendarWidget = new QCalendarWidget();
    calendarLayout->addWidget(calendarWidget);
    
    // 달력 스타일 설정
    calendarWidget->setStyleSheet(R"(
        QCalendarWidget {
            background-color: transparent;
            border: none;
            outline: none;
        }
        QCalendarWidget * {
            border: none;
            outline: none;
        }
        QCalendarWidget QToolButton {
            background-color: transparent;
            color: black;
            border: none;
            border-radius: 4px;
            padding: 4px;
            font-weight: bold;
            min-width: 20px;
            min-height: 20px;
        }
        QCalendarWidget QToolButton:hover {
            background-color: #E8C4A0;
        }
        QCalendarWidget QToolButton::menu-indicator {
            image: none;
            width: 0px;
            height: 0px;
        }
        QCalendarWidget QToolButton#qt_calendar_prevmonth {
            qproperty-icon: url(:/images/left.png);
            background-color: transparent;
        }
        QCalendarWidget QToolButton#qt_calendar_nextmonth {
            qproperty-icon: url(:/images/right.png);
            background-color: transparent;
        }
        QCalendarWidget QToolButton#qt_calendar_prevmonth:hover,
        QCalendarWidget QToolButton#qt_calendar_nextmonth:hover {
            background-color: #E8C4A0;
            border-radius: 4px;
        }
        QCalendarWidget QMenu {
            background-color: white;
            border: 1px solid #F5D5B8;
        }
        QCalendarWidget QSpinBox {
            background-color: white;
            border: 1px solid #F5D5B8;
            color: black;
            font-weight: bold;
        }
        QCalendarWidget QSpinBox::down-button,
        QCalendarWidget QSpinBox::up-button {
            width: 0px;
            height: 0px;
            border: none;
        }
        QCalendarWidget QAbstractItemView {
            background-color: white;
            selection-background-color: #F5D5B8;
            color: black;
        }
        QCalendarWidget QAbstractItemView:enabled {
            color: black;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background-color: #F5D5B8;
        }
    )");
    
    connect(calendarWidget, &QCalendarWidget::activated,
            this, &HistoryView::dateSelected);
    
    // 달력 크기는 resizeEvent에서 동적으로 설정
    calendarContainer->setAttribute(Qt::WA_StyledBackground, true);
    
    // 달력 밖 클릭 시 숨기기 위한 이벤트 필터 설치
    this->installEventFilter(this);

    // TCP 핸들러
    tcpHandler_      = new TcpHistoryHandler(this);
    tcpImageHandler_ = new TcpImageHandler(this);
    connect(tcpHandler_, &TcpHistoryHandler::historyDataReady, this, &HistoryView::onHistoryData);
    connect(tcpHandler_, &TcpHistoryHandler::errorOccurred,   this, &HistoryView::onHistoryError);
    connect(tcpImageHandler_, &TcpImageHandler::imageDataReady,
            this, &HistoryView::onImageDataReady);
    connect(tcpImageHandler_, &TcpImageHandler::errorOccurred,
            this, [this](auto e){ 
                qDebug() << "HistoryView: TCP 이미지 핸들러 에러:" << e;
                QMessageBox::warning(this, "Image Error", e); 
            });
    
    // 연결 완료 시 최초 요청하도록 시그널 연결
    connect(tcpHandler_, &TcpHistoryHandler::connected, this, [this]() {
        qDebug() << "HistoryView: TCP 연결 완료, 최초 데이터 요청";
        requestPage();
    });
    
    // 연결 실패 시 더미 데이터 로드
    connect(tcpHandler_, &TcpHistoryHandler::connectionFailed, this, [this]() {
        qDebug() << "HistoryView: TCP 연결 실패, 더미 데이터 로드";
        loadDummyData();
    });
    
    // config.ini에서 TCP 설정 읽기
    QString configPath = findConfigFile();
    if (!configPath.isEmpty()) {
        QSettings settings(configPath, QSettings::IniFormat);
        QString tcpHost = settings.value("tcp/ip").toString();
        int tcpPort = settings.value("tcp/port").toInt();
        
        qDebug() << "HistoryView TCP 설정 - Host:" << tcpHost << "Port:" << tcpPort;
        tcpHandler_->connectToServer(tcpHost, tcpPort);
        
        // 연결 타임아웃 타이머 (5초 후 더미 데이터 로드)
        QTimer::singleShot(5000, this, [this]() {
            if (tableWidget->rowCount() == 0) { // 아직 데이터가 없으면
                qDebug() << "HistoryView: 연결 타임아웃, 더미 데이터 로드";
                loadDummyData();
            }
        });
    } else {
        qDebug() << "HistoryView: config.ini 파일을 찾을 수 없습니다. 더미 데이터 로드";
        loadDummyData();
    }
    // 페이징
    setupPaginationUI();
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
    qDebug() << "HistoryView::onImageCellClicked - row:" << row << "col:" << col;
    
    if (col != 3 && col != 6 && col != 7) {
        qDebug() << "HistoryView: 이미지 열이 아님, 클릭 무시";
        return;
    }
    
    auto* it = tableWidget->item(row, col);
    if (!it) {
        qDebug() << "HistoryView: 테이블 아이템이 null";
        return;
    }
    
    QString path = it->text();
    if (path.isEmpty()) {
        qDebug() << "HistoryView: 이미지 경로가 비어있음";
        return;
    }
    
    qDebug() << "HistoryView: 이미지 경로:" << path;

    // 📌 타임스탬프 추출
    QString timestamp = parseTimestampFromPath(path);
    qDebug() << "HistoryView: 추출된 타임스탬프:" << timestamp;

    // 뷰어 생성
    qDebug() << "HistoryView: GetImageView 생성 중...";
    currentImageView_ = new GetImageView(path, timestamp, this);
    currentImageView_->show();
    qDebug() << "HistoryView: GetImageView 표시됨";

    // config.ini에서 TCP 설정 읽기
    QString configPath = findConfigFile();
    if (configPath.isEmpty()) {
        qDebug() << "HistoryView: config.ini 파일을 찾을 수 없음";
        return;
    }
    
    QSettings settings(configPath, QSettings::IniFormat);
    QString tcpHost = settings.value("tcp/ip").toString();
    int tcpPort = settings.value("tcp/port").toInt();
    
    qDebug() << "HistoryView: TCP 이미지 서버 연결 시도 - Host:" << tcpHost << "Port:" << tcpPort;
    qDebug() << "HistoryView: 요청할 이미지 경로:" << path;
    
    if (tcpImageHandler_) {
        tcpImageHandler_->connectToServerThenRequestImage(tcpHost, tcpPort, path);
    } else {
        qDebug() << "HistoryView: tcpImageHandler_가 null";
    }
}


void HistoryView::onImageDataReady(const QString& path, const QByteArray& data) {
    qDebug() << "HistoryView::onImageDataReady - 경로:" << path << "데이터 크기:" << data.size() << "bytes";
    
    if (currentImageView_) {
        qDebug() << "HistoryView: currentImageView_에 이미지 데이터 설정 중...";
        currentImageView_->setImageData(data);
        qDebug() << "HistoryView: 이미지 데이터 설정 완료";
    } else {
        qDebug() << "HistoryView: currentImageView_가 null, 이미지를 표시할 수 없음";
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
    // TCP 연결 상태 확인
    if (!tcpHandler_ || !tcpHandler_->isConnected()) {
        qDebug() << "HistoryView: TCP 연결되지 않음, 더미 데이터 사용";
        loadDummyData();
        return;
    }
    
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
        lb->setStyleSheet("background-color:#E0E0E0;border-radius:8px;padding:2px 6px;");
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

void HistoryView::updateTypeColumnBackground()
{
    // 모든 행의 유형 열 배경색을 기본값으로 설정
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QWidget* typeCell = tableWidget->cellWidget(row, 2);
        if (typeCell) {
            typeCell->setStyleSheet("background-color: transparent;");
        }
    }
    
    // 선택된 행들의 유형 열 배경색을 회색으로 설정
    QModelIndexList selectedIndexes = tableWidget->selectionModel()->selectedRows();
    for (const QModelIndex& index : selectedIndexes) {
        int row = index.row();
        QWidget* typeCell = tableWidget->cellWidget(row, 2);
        if (typeCell) {
            typeCell->setStyleSheet("background-color: #B3B3B3;");
        }
    }
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
    titleLabel->setStyleSheet(QString("font-family: 'HanwhaGothicB', 'Malgun Gothic', Arial; font-size:%1px;").arg(int(hu*0.4)));

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
    tableWidget->horizontalHeader()->setStyleSheet(QString(
        "QHeaderView::section { background:#FBB584; padding:4px; border:none; font-size:%1px; }"
        ).arg(int(hu*0.5)));

    // header checkbox 위치
    QHeaderView* hh = tableWidget->horizontalHeader();
    int x0 = hh->sectionPosition(0);
    int w0 = hh->sectionSize(0);
    int hh_h = hh->height();
    int cbSize = hh_h - int(hu*0.2);
    headerCheck->setGeometry(
        x0 + (w0 - cbSize)/2,
        (hh_h - cbSize)/2 + tableWidget->y(),
        cbSize, cbSize
        );

    startDateButton->setGeometry(wu*13+wu*0.5, hu*3 - yOffset, wu*2.5, uH);
    startDateButton->setIconSize(QSize(int(hu*0.6), int(hu*0.6)));
    startDateButton->setStyleSheet(QString(
        "QPushButton {"
        "    padding: 4px;"
        "    background: transparent;"
        "    border: none;"
        "    text-align: right;"
        "    font-size: %1px;"
        "    min-width: %2px;"
        "    max-width: %2px;"
        "    width: %2px;"
        "}"
    ).arg(int(hu*0.5)).arg(int(wu*2.5)));
    
    arrowLabel->setGeometry(wu*16, hu*3 - yOffset, wu*0.5, uH);
    arrowLabel->setPixmap(QPixmap(":/images/sign.png").scaled(int(hu*0.6), int(hu*0.6), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    endDateButton->setGeometry(wu*16+wu*0.5, hu*3 - yOffset, wu*2.5, uH);
    endDateButton->setIconSize(QSize(int(hu*0.6), int(hu*0.6)));
    endDateButton->setStyleSheet(QString(
        "QPushButton {"
        "    padding: 4px;"
        "    background: transparent;"
        "    border: none;"
        "    text-align: left;"
        "    font-size: %1px;"
        "    min-width: %2px;"
        "    max-width: %2px;"
        "    width: %2px;"
        "}"
    ).arg(int(hu*0.5)).arg(int(wu*2.5)));
    
    filterButton    ->setGeometry(wu*19, hu*3 - yOffset, wu*2.5, uH);
    filterButton->setStyleSheet(QString(R"(
        QToolButton{
            background:#F5D5B8 url(:/images/menu.png) no-repeat left center;
            background-size:%4px %4px;
            border:none;
            padding-left:%5px;
            padding-top:%2px;
            padding-bottom:%2px;
            padding-right:%2px;
            text-align:center;
            font-size:%1px;
            qproperty-toolButtonStyle: ToolButtonTextOnly;
        }
        QToolButton::menu-indicator{
            image:url(:/images/below.png);
            subcontrol-position:right center;
            subcontrol-origin:padding;
            width:%3px;
            height:%3px;
        }
    )").arg(int(hu*0.5)).arg(int(hu*0.2)).arg(int(hu*0.8)).arg(int(hu*0.6)).arg(int(hu*1.0)));
    downloadButton  ->setGeometry(wu*21.5, hu*3 - yOffset, wu*1.5, uH);
    downloadButton->setIconSize(QSize(int(hu*0.7), int(hu*0.7)));

    // 캘린더 컨테이너 스타일 설정
    calendarContainer->setStyleSheet(QString(R"(
        QWidget {
            background-color: white;
            border: %1px solid #FF6B35;
            border-radius: %2px;
        }
    )").arg(int(hu*0.1)).arg(int(hu*0.3)));

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
    
    // 화면 크기에 따른 달력 크기 설정
    double hu = height() / 21.0;
    int calendarW = int(hu * 12);
    int calendarH = int(hu * 8);
    calendarWidget->setFixedSize(calendarW, calendarH);
    calendarContainer->setFixedSize(calendarW + int(hu*0.3), calendarH + int(hu*0.3));
    
    // 캘린더 마진 설정
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(calendarContainer->layout());
    if (layout) {
        layout->setContentsMargins(int(hu*0.15), int(hu*0.15), int(hu*0.15), int(hu*0.15));
    }
    
    // 버튼 바로 아래에 달력 컨테이너 위치 설정
    int x = startDateButton->x();
    int y = startDateButton->y() + startDateButton->height() + int(hu*0.2);
    
    // 화면 경계 체크
    if (x + calendarContainer->width() > this->width()) {
        x = this->width() - calendarContainer->width() - int(hu*0.4);
    }
    if (y + calendarContainer->height() > this->height()) {
        y = startDateButton->y() - calendarContainer->height() - int(hu*0.2);
    }
    
    calendarContainer->move(x, y);
    calendarContainer->show();
    calendarContainer->raise();
    calendarContainer->activateWindow();
}

void HistoryView::openEndDatePicker()
{
    calendarForStart = false;
    
    // 화면 크기에 따른 달력 크기 설정
    double hu = height() / 21.0;
    int calendarW = int(hu * 12);
    int calendarH = int(hu * 8);
    calendarWidget->setFixedSize(calendarW, calendarH);
    calendarContainer->setFixedSize(calendarW + int(hu*0.3), calendarH + int(hu*0.3));
    
    // 캘린더 마진 설정
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(calendarContainer->layout());
    if (layout) {
        layout->setContentsMargins(int(hu*0.15), int(hu*0.15), int(hu*0.15), int(hu*0.15));
    }
    
    // 버튼 바로 아래에 달력 컨테이너 위치 설정
    int x = endDateButton->x();
    int y = endDateButton->y() + endDateButton->height() + int(hu*0.2);
    
    // 화면 경계 체크
    if (x + calendarContainer->width() > this->width()) {
        x = this->width() - calendarContainer->width() - int(hu*0.4);
    }
    if (y + calendarContainer->height() > this->height()) {
        y = endDateButton->y() - calendarContainer->height() - int(hu*0.2);
    }
    
    calendarContainer->move(x, y);
    calendarContainer->show();
    calendarContainer->raise();
    calendarContainer->activateWindow();
}

void HistoryView::dateSelected()
{
    QString txt = calendarWidget->selectedDate().toString("yyyy-MM-dd");
    QPushButton* targetButton = calendarForStart ? startDateButton : endDateButton;
    
    // 버튼 텍스트 변경
    targetButton->setText(txt);
    
    // 버튼 크기와 위치를 강제로 유지 (resizeEvent에서 설정한 값 그대로)
    double wu = width() / 24.0;
    double hu = height() / 21.0;
    int uH = int(hu);
    int yOffset = hu * 3;
    
    if (calendarForStart) {
        startDateButton->setGeometry(wu*13+wu*0.5, hu*3 - yOffset, wu*2.5, uH);
    } else {
        endDateButton->setGeometry(wu*16+wu*0.5, hu*3 - yOffset, wu*2.5, uH);
    }
    
    // 버튼 스타일 재적용하여 크기 고정
    QString buttonStyle = QString(
        "QPushButton {"
        "    padding: 4px;"
        "    background: transparent;"
        "    border: none;"
        "    text-align: %1;"
        "    font-size: %2px;"
        "    min-width: %3px;"
        "    max-width: %3px;"
        "    width: %3px;"
        "}"
    ).arg(calendarForStart ? "right" : "left")
     .arg(int(hu*0.5))
     .arg(int(wu*2.5));
    
    targetButton->setStyleSheet(buttonStyle);
    
    // 아이콘 크기도 다시 설정
    targetButton->setIconSize(QSize(int(hu*0.6), int(hu*0.6)));
    
    // 강제로 업데이트
    targetButton->updateGeometry();
    targetButton->update();
    
    calendarContainer->hide();
    
    if (!startDateButton->text().isEmpty() && !endDateButton->text().isEmpty() &&
        startDateButton->text() != "시작일 선택하기" && endDateButton->text() != "종료일 선택하기") {
        startDate = startDateButton->text();
        endDate   = endDateButton->text();
        currentPage = 0;
        requestPage();
    }
}
bool HistoryView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        
        // 달력이 보이는 상태에서 달력 밖을 클릭했을 때
        if (calendarContainer->isVisible()) {
            QRect calendarRect = calendarContainer->geometry();
            QPoint clickPos = mouseEvent->pos();
            
            // 클릭 위치가 달력 영역 밖이면 달력 숨기기
            if (!calendarRect.contains(clickPos)) {
                calendarContainer->hide();
            }
        }
    }
    
    return QWidget::eventFilter(obj, event);
}

QString HistoryView::findConfigFile()
{
    // config.ini 파일을 여러 경로에서 찾아봅니다
    QStringList searchPaths = {
        "config.ini",                    // 현재 디렉토리
        "./config.ini",                  // 명시적 현재 디렉토리
        "../config.ini",                 // 상위 디렉토리
        "../../config.ini",              // 상위의 상위 디렉토리
        QCoreApplication::applicationDirPath() + "/config.ini",  // 실행 파일 디렉토리
        QDir::currentPath() + "/config.ini"  // 현재 작업 디렉토리
    };
    
    for (const QString &path : searchPaths) {
        QFile file(path);
        if (file.exists()) {
            qDebug() << "HistoryView 설정 파일 발견:" << path;
            return path;
        }
    }
    
    qDebug() << "HistoryView: 다음 경로들에서 config.ini 파일을 찾을 수 없습니다:";
    for (const QString &path : searchPaths) {
        qDebug() << "  -" << path;
    }
    
    return QString(); // 빈 문자열 반환
}

void HistoryView::loadDummyData()
{
    qDebug() << "히스토리 서버 연결 실패 - 더미 데이터 로드";
    QJsonObject dummyResponse = createDummyHistoryResponse();
    onHistoryData(dummyResponse);
}

QJsonObject HistoryView::createDummyHistoryResponse()
{
    QJsonArray dataArray;
    
    // 30개의 더미 데이터 생성
    QStringList plateNumbers = {
        "12가3456", "34나5678", "56다7890", "78라1234", "90마5678",
        "11바9012", "22사3456", "33아7890", "44자1234", "55차5678",
        "66카9012", "77타3456", "88파7890", "99하1234", "00호5678",
        "96저0587", "123가456", "789나012", "345다678", "901라234"
    };
    
    QStringList eventTypeNames = {"주정차감지", "과속감지", "어린이감지"};
    
    for (int i = 0; i < 30; ++i) {
        QJsonObject item;
        
        // ID (역순으로 최신이 위에 오도록)
        item["id"] = 1000 + (29 - i);
        
        // 이벤트 타입 (0: 주정차, 1: 과속, 2: 어린이)
        int eventType = i % 3;
        item["event_type"] = eventType;
        
        // 날짜 (최근 7일 내 랜덤)
        QDateTime baseTime = QDateTime::currentDateTime().addDays(-(i / 4));
        QDateTime randomTime = baseTime.addSecs(-(i * 300 + (i % 7) * 60)); // 5분씩 간격
        item["date"] = randomTime.toString("yyyy-MM-dd hh:mm:ss");
        
        // 이미지 경로
        QString imagePrefix;
        switch (eventType) {
        case 0: imagePrefix = "shm_snapshot"; break;
        case 1: imagePrefix = "speed"; break;
        case 2: imagePrefix = "person"; break;
        }
        
        QString timestamp = randomTime.toString("yyyyMMdd_hhmmss");
        item["image_path"] = QString("images/%1_%2_%3.jpg")
                                .arg(imagePrefix)
                                .arg(1000 + i)
                                .arg(timestamp);
        
        // 번호판 (주정차와 과속감지만)
        if (eventType == 0 || eventType == 1) {
            item["plate_number"] = plateNumbers[i % plateNumbers.size()];
        } else {
            item["plate_number"] = "-";
        }
        
        // 속도 (과속감지만)
        if (eventType == 1) {
            item["speed"] = 35.0 + (i % 20); // 35~54 km/h
        } else {
            item["speed"] = QJsonValue(); // null
        }
        
        // 시작/종료 스냅샷 (주정차감지만)
        if (eventType == 0) {
            item["start_snapshot"] = QString("images/shm_startshot_%1_%2.jpg")
                                        .arg(1000 + i)
                                        .arg(timestamp);
            item["end_snapshot"] = QString("images/shm_endshot_%1_%2.jpg")
                                      .arg(1000 + i)
                                      .arg(timestamp);
        } else {
            item["start_snapshot"] = "";
            item["end_snapshot"] = "";
        }
        
        dataArray.append(item);
    }
    
    // 페이징을 위해 현재 페이지에 해당하는 데이터만 반환
    QJsonArray pageData;
    int startIndex = currentPage * PAGE_SIZE;
    int endIndex = qMin(startIndex + PAGE_SIZE, dataArray.size());
    
    for (int i = startIndex; i < endIndex; ++i) {
        pageData.append(dataArray[i]);
    }
    
    QJsonObject response;
    response["status"] = "success";
    response["code"] = 200;
    response["message"] = "Dummy history data loaded";
    response["data"] = pageData;
    
    return response;
}



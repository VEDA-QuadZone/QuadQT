// historyview.cpp
#include "mainwindow/historyview.h"
#include "mainwindow/tcphistoryhandler.h"


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
#include <QSizePolicy>

static constexpr int PAGE_SIZE = 16;

HistoryView::HistoryView(QWidget *parent)
    : QWidget(parent),
    calendarForStart(false),
    currentPage(0)
{
    setStyleSheet("background-color: #FFFFFF;");
    titleLabel = new QLabel("히스토리", this);
    titleLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(10);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget->setShowGrid(false);
    tableWidget->setHorizontalHeaderLabels({
        "", "날짜", "", "유형", "", "이미지",
        "번호판", "속도(km/h)", "정차 시작 이미지", "1분 경과 이미지"
    });
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableWidget->setStyleSheet(
        "QTableWidget { border: none; selection-background-color: transparent; }"
        "QTableWidget::item { border-bottom:1px solid #D3D3D3; border-left: none; border-right: none; border-top: none; }"
        "QTableWidget::item:selected { background-color: transparent; }"
        "QTableWidget::item:focus { background-color: transparent; outline: none; }"
        "QTableWidget::item:hover { background-color: transparent; }"
        );
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(tableWidget, &QTableWidget::cellClicked,
            this, [this](int row, int col) {
                if (calendarContainer->isVisible())
                    calendarContainer->hide();
                onImageCellClicked(row, col);
            });

    headerCheck = new QCheckBox(tableWidget->horizontalHeader());
    headerCheck->setChecked(false);
    connect(headerCheck, &QCheckBox::toggled, this, [this](bool on){
        for(int r=0; r<tableWidget->rowCount(); ++r){
            auto* cell = tableWidget->cellWidget(r,0);
            if(!cell) continue;
            auto* cb = cell->findChild<QCheckBox*>();
            if(!cb) continue;
            cb->blockSignals(true);
            cb->setChecked(on);
            cb->blockSignals(false);
            int id = tableWidget->item(r,1)->data(Qt::UserRole).toInt();
            if(on){
                selectedRecordIds.insert(id);
            } else {
                selectedRecordIds.remove(id);
            }
        }
        updateTypeColumnBackground();
    });

    startDateButton = new QPushButton(this);
    startDateButton->setText("시작일 선택하기");
    startDateButton->setIcon(QIcon(":/images/calendar.png"));
    arrowLabel = new QLabel(this);
    arrowLabel->setAlignment(Qt::AlignCenter);
    endDateButton = new QPushButton(this);
    endDateButton->setText("종료일 선택하기");
    endDateButton->setIcon(QIcon(":/images/calendar.png"));
    connect(startDateButton,&QPushButton::clicked,this,&HistoryView::openStartDatePicker);
    connect(endDateButton,  &QPushButton::clicked,this,&HistoryView::openEndDatePicker);

    filterButton = new QToolButton(this);
    filterButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    filterButton->setLayoutDirection(Qt::LeftToRight);
    filterButton->setText("전체보기");
    filterButton->setIcon(QIcon(":/images/below.png"));
    filterButton->setPopupMode(QToolButton::InstantPopup);
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
    addF("전체보기"); addF("주정차감지"); addF("어린이감지"); addF("과속감지");
    filterButton->setMenu(fm);

    downloadButton = new QPushButton(this);
    downloadButton->setIcon(QIcon(":/images/download.png"));
    downloadButton->setFlat(true);
    connect(downloadButton,&QPushButton::clicked,this,&HistoryView::exportCsv);

    refreshButton = new QPushButton(this);
    refreshButton->setIcon(QIcon(":/images/refresh.png"));
    refreshButton->setFlat(true);
    connect(refreshButton,&QPushButton::clicked,this,[this](){
        currentFilter = "전체보기";
        filterButton->setText("전체보기");
        currentPage = 0;
        requestPage();
    });

    calendarContainer = new QWidget(this);
    calendarContainer->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    calendarContainer->hide();
    QVBoxLayout* calendarLayout = new QVBoxLayout(calendarContainer);
    calendarLayout->setSpacing(0);
    calendarWidget = new QCalendarWidget();
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendarLayout->addWidget(calendarWidget);
    calendarWidget->setStyleSheet(R"( ... )"); // 기존 스타일시트 그대로

    connect(calendarWidget, &QCalendarWidget::activated,
            this, &HistoryView::dateSelected);
    calendarContainer->setAttribute(Qt::WA_StyledBackground, true);
    this->installEventFilter(this);

    tcpHandler_      = new TcpHistoryHandler(this);
    tcpImageHandler_ = new TcpImageHandler(this);
    connect(tcpHandler_, &TcpHistoryHandler::historyDataReady, this, &HistoryView::onHistoryData);
    connect(tcpHandler_, &TcpHistoryHandler::errorOccurred,   this, &HistoryView::onHistoryError);
    connect(tcpImageHandler_, &TcpImageHandler::imageDataReady,
            this, &HistoryView::onImageDataReady);
    connect(tcpImageHandler_, &TcpImageHandler::errorOccurred,
            this, [this](auto e){ });

    connect(tcpHandler_, &TcpHistoryHandler::connected, this, [this]() {
        requestPage();
    });
    connect(tcpHandler_, &TcpHistoryHandler::connectionFailed, this, [this]() {
        loadDummyData();
    });

    QString configPath = findConfigFile();
    if (!configPath.isEmpty()) {
        QSettings settings(configPath, QSettings::IniFormat);
        QString tcpHost = settings.value("tcp/ip").toString();
        int tcpPort = settings.value("tcp/port").toInt();
        tcpHandler_->connectToServer(tcpHost, tcpPort);
        QTimer::singleShot(5000, this, [this]() {
            if (tableWidget->rowCount() == 0) loadDummyData();
        });
    } else {
        loadDummyData();
    }
    setupPaginationUI();
    QTimer::singleShot(100, this, [this]() {
        if (tableWidget->rowCount() == 0)
            loadDummyData();
    });
}

QString HistoryView::parseTimestampFromPath(const QString& path) {
    QRegularExpression re(R"(20\d{6}_\d{6})");
    QRegularExpressionMatch match = re.match(path);
    if (!match.hasMatch()) return "";
    QString raw = match.captured();
    if (raw.length() != 15) return "";
    QString date = raw.mid(0, 8);
    QString time = raw.mid(9, 6);
    return QString("%1-%2-%3 %4:%5:%6")
        .arg(date.mid(0, 4))
        .arg(date.mid(4, 2))
        .arg(date.mid(6, 2))
        .arg(time.mid(0, 2))
        .arg(time.mid(2, 2))
        .arg(time.mid(4, 2));
}

void HistoryView::onImageCellClicked(int row, int col) {
    if (col != 5 && col != 8 && col != 9) return;

    QString path;
    QTableWidgetItem* item = tableWidget->item(row, col);
    if (item && item->data(Qt::UserRole).isValid()) {
        path = item->data(Qt::UserRole).toString();
    }
    if (path.isEmpty()) {
        QWidget* cell = tableWidget->cellWidget(row, col);
        if (cell && cell->property("imagePath").isValid()) {
            path = cell->property("imagePath").toString();
        }
    }
    if (path.isEmpty()) path = "-";

    QString timestamp = parseTimestampFromPath(path);
    
    // 유형 정보 가져오기 (3번 열에서)
    QString eventType = "";
    QWidget* typeCell = tableWidget->cellWidget(row, 3);
    if (typeCell && typeCell->layout() && typeCell->layout()->itemAt(0)) {
        QLabel* typeLabel = qobject_cast<QLabel*>(typeCell->layout()->itemAt(0)->widget());
        if (typeLabel) {
            eventType = typeLabel->text();
        }
    }
    
    // 번호판 정보 가져오기 (6번 열에서)
    QString plate = "";
    QWidget* plateCell = tableWidget->cellWidget(row, 6);
    if (plateCell && plateCell->layout() && plateCell->layout()->itemAt(0)) {
        QLabel* plateLabel = qobject_cast<QLabel*>(plateCell->layout()->itemAt(0)->widget());
        if (plateLabel) {
            plate = plateLabel->text();
        }
    }
    
    QString filename = path;

    currentImageView_ = new GetImageView(eventType, plate, timestamp, filename, this);
    currentImageView_->show();

    QString configPath = findConfigFile();
    if (configPath.isEmpty()) return;
    QSettings settings(configPath, QSettings::IniFormat);
    QString tcpHost = settings.value("tcp/ip").toString();
    int tcpPort = settings.value("tcp/port").toInt();
    if (tcpImageHandler_) {
        tcpImageHandler_->connectToServerThenRequestImage(tcpHost, tcpPort, path);
    }
}

void HistoryView::onImageDataReady(const QString& path, const QByteArray& data) {
    if (currentImageView_) {
        currentImageView_->setImageData(data);
    }
}

void HistoryView::exportCsv() {
    QString path = QFileDialog::getSaveFileName(this, tr("CSV로 저장"), QString(), tr("CSV Files (*.csv)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) return;
    file.write("\xEF\xBB\xBF");
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

    if (currentFilter.isEmpty() || currentFilter == "전체보기") {
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
            } else {
                selectedRecordIds.remove(id);
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
            
            // 배경색 업데이트 (체크박스 상태에 따라)
            updateTypeColumnBackground();
        });

        // 날짜 (ID 저장) - 커스텀 위젯으로 변경
        QString imagePath = obj.value("image_path").toString();
        QString parsedTime = parseTimestampFromPath(imagePath);
        QString dateText = parsedTime.isEmpty() ? obj.value("date").toString() : parsedTime;
        
        QWidget* dateCell = new QWidget(this);
        QLabel* dateLabel = new QLabel(dateText, dateCell);
        dateLabel->setAlignment(Qt::AlignCenter);
        
        QHBoxLayout* dateLayout = new QHBoxLayout(dateCell);
        dateLayout->addWidget(dateLabel);
        dateLayout->setAlignment(Qt::AlignCenter);
        dateLayout->setContentsMargins(0, 0, 0, 0);
        dateCell->setLayout(dateLayout);
        
        tableWidget->setCellWidget(i, 1, dateCell);
        
        // 빈 아이템도 설정 (ID 저장용)
        QTableWidgetItem* emptyDateItem = new QTableWidgetItem();
        emptyDateItem->setData(Qt::UserRole, id);
        tableWidget->setItem(i, 1, emptyDateItem);

        // 2번 열은 투명 패딩 (빈 공간) - 커스텀 위젯으로 변경
        QWidget* paddingCell2 = new QWidget(this);
        tableWidget->setCellWidget(i, 2, paddingCell2);

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
        tableWidget->setCellWidget(i,3,tc);

        // 4번 열은 투명 패딩 (빈 공간) - 커스텀 위젯으로 변경
        QWidget* paddingCell4 = new QWidget(this);
        tableWidget->setCellWidget(i, 4, paddingCell4);
        
        // 나머지 열 (인덱스 +2 이동)
        // 이미지 열 (5번) - 아이콘 또는 "-" 표시
        if (!imagePath.isEmpty()) {
            // 커스텀 위젯으로 아이콘을 중앙에 배치
            QWidget* imageCell = new QWidget(this);
            QLabel* imageLabel = new QLabel(imageCell);
            imageLabel->setPixmap(QIcon(":/images/image.png").pixmap(16, 16));
            imageLabel->setAlignment(Qt::AlignCenter);

            QHBoxLayout* imageLayout = new QHBoxLayout(imageCell);
            imageLayout->addWidget(imageLabel);
            imageLayout->setAlignment(Qt::AlignCenter);
            imageLayout->setContentsMargins(0, 0, 0, 0);
            imageCell->setLayout(imageLayout);

            // 경로 정보를 위젯의 property로 저장
            imageCell->setProperty("imagePath", imagePath);

            tableWidget->setCellWidget(i, 5, imageCell);

            // 빈 아이템도 설정 (클릭 이벤트를 위해)
            QTableWidgetItem* emptyItem = new QTableWidgetItem();
            emptyItem->setData(Qt::UserRole, imagePath);
            tableWidget->setItem(i, 5, emptyItem);
        } else {
            // "-" 텍스트도 커스텀 위젯으로 변경
            QWidget* imageCell = new QWidget(this);
            QLabel* imageLabel = new QLabel("-", imageCell);
            imageLabel->setAlignment(Qt::AlignCenter);

            QHBoxLayout* imageLayout = new QHBoxLayout(imageCell);
            imageLayout->addWidget(imageLabel);
            imageLayout->setAlignment(Qt::AlignCenter);
            imageLayout->setContentsMargins(0, 0, 0, 0);
            imageCell->setLayout(imageLayout);

            tableWidget->setCellWidget(i, 5, imageCell);
        }

        // 번호판 - 커스텀 위젯으로 변경
        QString plateText = obj.value("plate_number").toString();
        QWidget* plateCell = new QWidget(this);
        QLabel* plateLabel = new QLabel(plateText, plateCell);
        plateLabel->setAlignment(Qt::AlignCenter);

        QHBoxLayout* plateLayout = new QHBoxLayout(plateCell);
        plateLayout->addWidget(plateLabel);
        plateLayout->setAlignment(Qt::AlignCenter);
        plateLayout->setContentsMargins(0, 0, 0, 0);
        plateCell->setLayout(plateLayout);

        tableWidget->setCellWidget(i, 6, plateCell);
        // 속도 - 커스텀 위젯으로 변경
        QJsonValue sp = obj.value("speed");
        QString speedText = sp.isDouble() ? QString::number(sp.toDouble(),'f',2) : QString("-");

        QWidget* speedCell = new QWidget(this);
        QLabel* speedLabel = new QLabel(speedText, speedCell);
        speedLabel->setAlignment(Qt::AlignCenter);

        QHBoxLayout* speedLayout = new QHBoxLayout(speedCell);
        speedLayout->addWidget(speedLabel);
        speedLayout->setAlignment(Qt::AlignCenter);
        speedLayout->setContentsMargins(0, 0, 0, 0);
        speedCell->setLayout(speedLayout);

        tableWidget->setCellWidget(i, 7, speedCell);

        // 정차 시작 이미지 열 (8번) - 아이콘 또는 "-" 표시
        QString startSnapshot = obj.value("start_snapshot").toString();
        if (!startSnapshot.isEmpty()) {
            // 커스텀 위젯으로 아이콘을 중앙에 배치
            QWidget* startCell = new QWidget(this);
            QLabel* startLabel = new QLabel(startCell);
            startLabel->setPixmap(QIcon(":/images/image.png").pixmap(16, 16));
            startLabel->setAlignment(Qt::AlignCenter);

            QHBoxLayout* startLayout = new QHBoxLayout(startCell);
            startLayout->addWidget(startLabel);
            startLayout->setAlignment(Qt::AlignCenter);
            startLayout->setContentsMargins(0, 0, 0, 0);
            startCell->setLayout(startLayout);

            // 경로 정보를 위젯의 property로 저장
            startCell->setProperty("imagePath", startSnapshot);
            qDebug() << "row:" << i << "startSnapshot:" << startSnapshot;
            tableWidget->setCellWidget(i, 8, startCell);

            // 빈 아이템도 설정 (클릭 이벤트를 위해)
            QTableWidgetItem* emptyItem = new QTableWidgetItem();
            emptyItem->setData(Qt::UserRole, startSnapshot);
            tableWidget->setItem(i, 8, emptyItem);
        } else {
            // "-" 텍스트도 커스텀 위젯으로 변경
            QWidget* startCell = new QWidget(this);
            QLabel* startLabel = new QLabel("-", startCell);
            startLabel->setAlignment(Qt::AlignCenter);

            QHBoxLayout* startLayout = new QHBoxLayout(startCell);
            startLayout->addWidget(startLabel);
            startLayout->setAlignment(Qt::AlignCenter);
            startLayout->setContentsMargins(0, 0, 0, 0);
            startCell->setLayout(startLayout);

            tableWidget->setCellWidget(i, 8, startCell);
        }

        // 1분 경과 이미지 열 (9번) - 아이콘 또는 "-" 표시
        QString endSnapshot = obj.value("end_snapshot").toString();
        if (!endSnapshot.isEmpty()) {
            // 커스텀 위젯으로 아이콘을 중앙에 배치
            QWidget* endCell = new QWidget(this);
            QLabel* endLabel = new QLabel(endCell);
            endLabel->setPixmap(QIcon(":/images/image.png").pixmap(16, 16));
            endLabel->setAlignment(Qt::AlignCenter);

            QHBoxLayout* endLayout = new QHBoxLayout(endCell);
            endLayout->addWidget(endLabel);
            endLayout->setAlignment(Qt::AlignCenter);
            endLayout->setContentsMargins(0, 0, 0, 0);
            endCell->setLayout(endLayout);

            // 경로 정보를 위젯의 property로 저장
            endCell->setProperty("imagePath", endSnapshot);

            tableWidget->setCellWidget(i, 9, endCell);

            // 빈 아이템도 설정 (클릭 이벤트를 위해)
            QTableWidgetItem* emptyItem = new QTableWidgetItem();
            emptyItem->setData(Qt::UserRole, endSnapshot);
            tableWidget->setItem(i, 9, emptyItem);
        } else {
            // "-" 텍스트도 커스텀 위젯으로 변경
            QWidget* endCell = new QWidget(this);
            QLabel* endLabel = new QLabel("-", endCell);
            endLabel->setAlignment(Qt::AlignCenter);

            QHBoxLayout* endLayout = new QHBoxLayout(endCell);
            endLayout->addWidget(endLabel);
            endLayout->setAlignment(Qt::AlignCenter);
            endLayout->setContentsMargins(0, 0, 0, 0);
            endCell->setLayout(endLayout);

            tableWidget->setCellWidget(i, 9, endCell);
        }
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
    // 동적 스타일시트 생성
    QString dynamicStyle = 
        "QTableWidget { border: none; selection-background-color: transparent; }"
        "QTableWidget::item { border-bottom:1px solid #D3D3D3; border-left: none; border-right: none; border-top: none; }"
        "QTableWidget::item:selected { background-color: transparent; }"
        "QTableWidget::item:focus { background-color: transparent; outline: none; }"
        "QTableWidget::item:hover { background-color: transparent; }";
    
    // 모든 행을 체크박스 상태에 따라 배경색 설정
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // 체크박스 상태 확인
        QWidget* checkCell = tableWidget->cellWidget(row, 0);
        bool isChecked = false;
        if (checkCell) {
            QCheckBox* checkBox = checkCell->findChild<QCheckBox*>();
            if (checkBox) {
                isChecked = checkBox->isChecked();
            }
        }
        
        // 배경색 설정 (체크된 경우 회색, 아닌 경우 투명)
        QString backgroundColor = isChecked ? "background-color: #B3B3B3;" : "background-color: transparent;";
        
        // 모든 커스텀 위젯 영역에 배경색 설정
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            QWidget* cellWidget = tableWidget->cellWidget(row, col);
            if (cellWidget) {
                cellWidget->setStyleSheet(backgroundColor);
            }
        }
        
        // 체크된 행에 대해 동적 스타일 추가
        if (isChecked) {
            // 각 아이템에 속성 설정하여 스타일시트에서 선택할 수 있도록 함
            for (int col = 0; col < tableWidget->columnCount(); ++col) {
                QTableWidgetItem* item = tableWidget->item(row, col);
                if (item) {
                    item->setBackground(QBrush(QColor("#B3B3B3")));
                    // 강제로 배경색 적용
                    item->setData(Qt::BackgroundRole, QColor("#B3B3B3"));
                }
            }
        } else {
            for (int col = 0; col < tableWidget->columnCount(); ++col) {
                QTableWidgetItem* item = tableWidget->item(row, col);
                if (item) {
                    item->setBackground(QBrush());
                    item->setData(Qt::BackgroundRole, QVariant());
                }
            }
        }
    }
    
    // 테이블을 강제로 다시 그리기
    tableWidget->viewport()->update();
    tableWidget->repaint();
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
    prevButton = new QPushButton(this);
    prevButton->setIcon(QIcon(":/images/left.png"));
    // 텍스트 제거 - 아이콘만 표시
    
    nextButton = new QPushButton(this);
    nextButton->setIcon(QIcon(":/images/right.png"));
    // 텍스트 제거 - 아이콘만 표시
    
    pageLabel = new QLabel("1", this);
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

    // 각 열 너비, 행 높이 고정 (2번, 4번 인덱스에 투명 패딩 열 추가)
    static constexpr double cw[10] = {2,3,0.5,2,0.5,2,3,3,2,2};
    for (int c = 0; c < 10; ++c)
        tableWidget->setColumnWidth(c, int(cw[c] * uW));
    tableWidget->verticalHeader()->setDefaultSectionSize(uH);
    tableWidget->horizontalHeader()->setFixedHeight(uH);
    tableWidget->horizontalHeader()->setStyleSheet(QString(
        "QHeaderView::section { background:#FBB584; padding:4px; border:none; font-size:%1px; }"
        "QHeaderView::section:nth-child(3) { background:transparent; }"
        "QHeaderView::section:nth-child(5) { background:transparent; }"
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

    startDateButton->setGeometry(wu*12, hu*3 - yOffset, wu*3, uH);
    startDateButton->setIconSize(QSize(int(hu*0.5), int(hu*0.5)));
    startDateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    startDateButton->setFixedSize(wu*3, uH);
    startDateButton->setStyleSheet(QString(
        "QPushButton {"
        "    padding: 2px;"
        "    background: transparent;"
        "    border: none;"
        "    text-align: right;"
        "    font-size: %1px;"
        "    min-width: %2px;"
        "    max-width: %2px;"
        "    width: %2px;"
        "    qproperty-iconSize: %3px %3px;"
        "}"
    ).arg(int(hu*0.35)).arg(int(wu*3)).arg(int(hu*0.5)));
    
    arrowLabel->setGeometry(wu*15, hu*3 - yOffset, wu*0.5, uH);
    arrowLabel->setPixmap(QPixmap(":/images/sign.png").scaled(int(hu*0.4), int(hu*0.4), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    endDateButton->setGeometry(wu*15.5, hu*3 - yOffset, wu*3, uH);
    endDateButton->setIconSize(QSize(int(hu*0.5), int(hu*0.5)));
    endDateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    endDateButton->setFixedSize(wu*3, uH);
    endDateButton->setStyleSheet(QString(
        "QPushButton {"
        "    padding: 2px;"
        "    background: transparent;"
        "    border: none;"
        "    text-align: left;"
        "    font-size: %1px;"
        "    min-width: %2px;"
        "    max-width: %2px;"
        "    width: %2px;"
        "    qproperty-iconSize: %3px %3px;"
        "}"
    ).arg(int(hu*0.35)).arg(int(wu*3)).arg(int(hu*0.5)));
    
    filterButton    ->setGeometry(wu*18, hu*3 - yOffset, wu*3, uH);
    filterButton->setStyleSheet(QString(R"(
        QToolButton{
            background:white url(:/images/menu.png) no-repeat left center;
            background-size:%4px %4px;
            border:1px solid #D3D3D3;
            border-radius:4px;
            padding-left:%5px;
            padding-top:%2px;
            padding-bottom:%2px;
            padding-right:%2px;
            text-align:center;
            font-size:%1px;
            qproperty-toolButtonStyle: ToolButtonTextOnly;
        }
        QToolButton:hover{
            background:white url(:/images/menu.png) no-repeat left center;
            background-size:%4px %4px;
            border:1px solid #B0B0B0;
        }
        QToolButton::menu-indicator{
            image:url(:/images/below.png);
            subcontrol-position:right center;
            subcontrol-origin:padding;
            width:%3px;
            height:%3px;
        }
    )").arg(int(hu*0.35)).arg(int(hu*0.2)).arg(int(hu*0.8)).arg(int(hu*0.6)).arg(int(hu*1.0)));
    downloadButton  ->setGeometry(wu*22.5 - uH - wu*0.2, hu*3 - yOffset, uH, uH);
    downloadButton->setIconSize(QSize(int(hu*0.7), int(hu*0.7)));
    
    refreshButton   ->setGeometry(wu*22.5, hu*3 - yOffset, uH, uH);
    refreshButton->setIconSize(QSize(int(hu*0.7), int(hu*0.7)));

    // 캘린더 컨테이너 스타일 설정
    calendarContainer->setStyleSheet(QString(R"(
        QWidget {
            background-color: white;
            border: %1px solid #FF6B35;
            border-radius: 0px;
        }
    )").arg(int(hu*0.1)));

    // 페이징: 가로 8칸, 중앙, row22
    int navX = int(wu*8);
    int navY = int(hu*22 - yOffset);
    int navH = uH;
    int btnSize = navH; // 정사각형으로 만들기 위해 높이와 동일하게 설정
    int totalWidth = btnSize * 2 + int(wu*2); // 버튼 2개 + 라벨 너비
    int startX = navX + (int(wu*8) - totalWidth) / 2; // 중앙 정렬
    
    // 페이징 버튼 geometry 설정 (정사각형)
    prevButton->setGeometry(startX,                    navY, btnSize, navH);
    pageLabel ->setGeometry(startX + btnSize,          navY, int(wu*2), navH);
    nextButton->setGeometry(startX + btnSize + int(wu*2), navY, btnSize, navH);
    
    // 페이징 버튼 스타일 설정 (글씨 크기 키움)
    int fontSize = int(hu * 0.6); // 글씨 크기를 키움
    int iconSize = int(hu * 0.7); // 아이콘 크기
    
    QString buttonStyle = QString(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: none;"
        "    padding: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(232, 196, 160, 0.3);"
        "    border-radius: 4px;"
        "}"
        "QPushButton:disabled {"
        "    background-color: transparent;"
        "    opacity: 0.3;"
        "}"
    );
    
    prevButton->setStyleSheet(buttonStyle);
    nextButton->setStyleSheet(buttonStyle);
    
    // 아이콘 크기 설정
    prevButton->setIconSize(QSize(iconSize, iconSize));
    nextButton->setIconSize(QSize(iconSize, iconSize));
    
    // 페이지 라벨 스타일 설정 (테두리 제거)
    pageLabel->setStyleSheet(QString(
        "QLabel {"
        "    font-size: %1px;"
        "    font-weight: bold;"
        "    background-color: transparent;"
        "    border: none;"
        "}"
    ).arg(fontSize));
}


void HistoryView::openStartDatePicker()
{
    calendarForStart = true;
    
    // 화면 크기에 따른 달력 크기 설정
    double hu = height() / 21.0;
    int calendarW = int(hu * 6);
    int calendarH = int(hu * 5);
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
    int calendarW = int(hu * 6);
    int calendarH = int(hu * 5);
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
    
    // 날짜 유효성 검사
    QDate selectedDate = QDate::fromString(txt, "yyyy-MM-dd");
    QString currentStartDate = startDateButton->text();
    QString currentEndDate = endDateButton->text();
    
    // 시작일을 선택하는 경우
    if (calendarForStart) {
        // 종료일이 이미 설정되어 있고, 선택한 시작일이 종료일보다 늦은 경우
        if (!currentEndDate.isEmpty() && currentEndDate != "시작일 선택하기") {
            QDate endDate = QDate::fromString(currentEndDate, "yyyy-MM-dd");
            if (selectedDate > endDate) {
                QMessageBox::warning(this, "날짜 오류", 
                    QString("시작날짜가 종료날짜보다 늦을 수 없습니다.\n\n"
                           "선택한 시작날짜: %1\n"
                           "현재 종료날짜: %2\n\n"
                           "올바른 범위를 선택해 주세요.")
                           .arg(txt).arg(currentEndDate));
                calendarContainer->hide();
                return; // 날짜 설정하지 않고 종료
            }
        }
    }
    // 종료일을 선택하는 경우
    else {
        // 시작일이 이미 설정되어 있고, 선택한 종료일이 시작일보다 이른 경우
        if (!currentStartDate.isEmpty() && currentStartDate != "종료일 선택하기") {
            QDate startDate = QDate::fromString(currentStartDate, "yyyy-MM-dd");
            if (selectedDate < startDate) {
                QMessageBox::warning(this, "날짜 오류", 
                    QString("종료날짜가 시작날짜보다 이를 수 없습니다.\n\n"
                           "현재 시작날짜: %1\n"
                           "선택한 종료날짜: %2\n\n"
                           "올바른 범위를 선택해 주세요.")
                           .arg(currentStartDate).arg(txt));
                calendarContainer->hide();
                return; // 날짜 설정하지 않고 종료
            }
        }
    }
    
    // 유효성 검사를 통과한 경우에만 버튼 텍스트 변경
    targetButton->setText(txt);
    
    // 버튼 크기와 위치를 강제로 유지 (resizeEvent에서 설정한 값 그대로)
    double wu = width() / 24.0;
    double hu = height() / 21.0;
    int uH = int(hu);
    int yOffset = hu * 3;
    
    // 버튼 크기와 위치를 완전히 고정
    int fixedWidth = int(wu*3);
    int fixedHeight = uH;
    
    if (calendarForStart) {
        startDateButton->setGeometry(wu*12, hu*3 - yOffset, fixedWidth, fixedHeight);
        startDateButton->setFixedSize(fixedWidth, fixedHeight);
        startDateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    } else {
        endDateButton->setGeometry(wu*15.5, hu*3 - yOffset, fixedWidth, fixedHeight);
        endDateButton->setFixedSize(fixedWidth, fixedHeight);
        endDateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    
    // 버튼 스타일 재적용하여 크기 고정 (더 작은 글자 크기)
    QString buttonStyle = QString(
        "QPushButton {"
        "    padding: 2px;"
        "    background: transparent;"
        "    border: none;"
        "    text-align: %1;"
        "    font-size: %2px;"
        "    min-width: %3px;"
        "    max-width: %3px;"
        "    width: %3px;"
        "    height: %4px;"
        "    qproperty-iconSize: %5px %5px;"
        "}"
    ).arg(calendarForStart ? "right" : "left")
     .arg(int(hu*0.35))  // 글자 크기 줄임
     .arg(fixedWidth)
     .arg(fixedHeight)
     .arg(int(hu*0.5));  // 아이콘 크기
    
    targetButton->setStyleSheet(buttonStyle);
    
    // 아이콘 크기도 다시 설정
    targetButton->setIconSize(QSize(int(hu*0.5), int(hu*0.5)));
    
    // 강제로 업데이트 - 여러 번 호출하여 확실히 적용
    targetButton->updateGeometry();
    targetButton->update();
    targetButton->repaint();
    
    // 약간의 지연 후 다시 한 번 크기 고정
    QTimer::singleShot(10, this, [this, targetButton, fixedWidth, fixedHeight]() {
        targetButton->setFixedSize(fixedWidth, fixedHeight);
        targetButton->updateGeometry();
        targetButton->update();
    });
    
    calendarContainer->hide();
    
    // 두 날짜가 모두 선택되었는지 확인
    if (!startDateButton->text().isEmpty() && !endDateButton->text().isEmpty() &&
        startDateButton->text() != "시작일 선택하기" && endDateButton->text() != "종료일 선택하기") {
        
        // 날짜 문자열을 QDate로 변환하여 비교
        QDate startDateObj = QDate::fromString(startDateButton->text(), "yyyy-MM-dd");
        QDate endDateObj = QDate::fromString(endDateButton->text(), "yyyy-MM-dd");
        
        // 시작날짜가 종료날짜보다 늦은지 확인
        if (startDateObj > endDateObj) {
            QMessageBox::warning(this, "날짜 오류", 
                QString("시작날짜가 종료날짜보다 늦을 수 없습니다.\n\n"
                       "현재 선택된 날짜:\n"
                       "시작날짜: %1\n"
                       "종료날짜: %2\n\n"
                       "올바른 날짜 범위를 선택해주세요.")
                .arg(startDateButton->text())
                .arg(endDateButton->text()));
            return;
        }
        
        // 날짜가 유효하면 검색 실행
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
            
            // 날짜 버튼을 클릭한 경우는 달력을 숨기지 않음
            QRect startButtonRect = startDateButton->geometry();
            QRect endButtonRect = endDateButton->geometry();
            
            if (startButtonRect.contains(clickPos) || endButtonRect.contains(clickPos)) {
                return QWidget::eventFilter(obj, event);
            }
            
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
    currentPage = 0; // 페이지를 0으로 초기화
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



// historyview.h
#ifndef HISTORYVIEW_H
#define HISTORYVIEW_H
#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QCalendarWidget>
#include <QTableWidget>
#include <QActionGroup>
#include <QCheckBox>
#include "tcphistoryhandler.h"
#include <QSet>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include "getimageview.h"
#include "compareimageview.h"
#include "tcpimagehandler.h"
#include <QByteArray>
class HistoryView : public QWidget {
    Q_OBJECT
public:
    explicit HistoryView(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void openStartDatePicker();
    void openEndDatePicker();
    void dateSelected();
private slots:
    void exportCsv();
    // 서버 응답
    void onHistoryData(const QJsonObject &resp);
    void onHistoryError(const QString &err);

    // 페이지 전환
    void prevPage();
    void nextPage();
private slots:
    // (기존 슬롯 아래에)
    void onImageCellClicked(int row, int column);
    void onImageDataReady(const QString& imagePath, const QByteArray& data);
    void updateTypeColumnBackground();
private:
    QLabel*          titleLabel;
    QTableWidget*    tableWidget;
    QCheckBox*       headerCheck;
    QPushButton*     startDateButton;
    QLabel*          arrowLabel;
    QPushButton*     endDateButton;
    QToolButton*     filterButton;
    QActionGroup*    filterGroup;
    QAction*         actionParking;
    QAction*         actionChildren;
    QAction*         actionSpeed;
    QPushButton*     downloadButton;
    QPushButton*     refreshButton;
    QWidget*         separatorLine;
    QWidget*         calendarContainer;
    QCalendarWidget* calendarWidget;
    bool             calendarForStart;
    QLabel*          monthYearLabel;
    QPushButton*     prevMonthButton;
    QPushButton*     nextMonthButton;
    QString          currentFilter;
    QPushButton    *prevButton;
    QLabel         *pageLabel;
    QPushButton    *nextButton;

    // TCP handler
    TcpHistoryHandler *tcpHandler_;

    // 상태
    QString   currentEmail = "aa@naver.com";
    QString   startDate   = "";
    QString   endDate     = "";
    int       currentPage = 0;
    static constexpr int PAGE_SIZE = 16;
    
    // 클라이언트 사이드 필터링을 위한 전체 데이터 저장
    QJsonArray allHistoryData;

    void requestPage();
    void setupPaginationUI();
private:
    QSet<int>            selectedRecordIds;  // 선택된 레코드 ID 저장
    QMap<int, QJsonObject> recordDataMap;     // ID → JSON 데이터 맵
private:
    // ...기존 변수들...
    TcpImageHandler*   tcpImageHandler_;
    GetImageView*      currentImageView_;
    CompareImageView*  currentCompareView_;
    QString            pendingStartImagePath_;
    QString            pendingEndImagePath_;
    QByteArray         startImageData_;
    QByteArray         endImageData_;
private:
    QString parseTimestampFromPath(const QString& path, const QString& eventType = "");
    QString findConfigFile();
    void loadDummyData(); // 더미 데이터 로드 함수
    QJsonObject createDummyHistoryResponse(); // 더미 히스토리 응답 생성
    
    // 파일명에서 날짜 추출하는 함수
    QDate extractDateFromFilename(const QString& filename);
    
    // 클라이언트 사이드 날짜 필터링 함수
    bool isDateInRange(const QDate& date, const QString& startDateStr, const QString& endDateStr);
};

#endif // HISTORYVIEW_H

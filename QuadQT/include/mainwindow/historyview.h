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
#include "getimageview.h"
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
    QWidget*         calendarContainer;
    QCalendarWidget* calendarWidget;
    bool             calendarForStart;
    QString          currentFilter;
    QPushButton    *prevButton;
    QLabel         *pageLabel;
    QPushButton    *nextButton;

    // TCP handler
    TcpHistoryHandler *tcpHandler_;

    // 상태
    QString   currentEmail = "yuna@cctv.com";
    QString   startDate   = "";
    QString   endDate     = "";
    int       currentPage = 0;
    static constexpr int PAGE_SIZE = 16;

    void requestPage();
    void setupPaginationUI();
private:
    QSet<int>            selectedRecordIds;  // 선택된 레코드 ID 저장
    QMap<int, QJsonObject> recordDataMap;     // ID → JSON 데이터 맵
private:
    // ...기존 변수들...
    TcpImageHandler*   tcpImageHandler_;
    GetImageView*      currentImageView_;
private:
    QString parseTimestampFromPath(const QString& path);
};

#endif // HISTORYVIEW_H

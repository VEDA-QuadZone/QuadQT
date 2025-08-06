#include "mainwindow/filenameutils.h"
#include <QRegularExpression>
#include <QDateTime>

QString convertFilename(const QString& originalPath, const QString& eventType) {
    // "images/" 제거
    QString filename = originalPath;
    if (filename.startsWith("images/")) {
        filename = filename.mid(7); // "images/" 제거
    }
    
    QString timeStr = "";
    
    // 과속감지일 경우 뒷쪽 타임스탬프 우선 시도 (YYYY-MM-DD_HH-MM-SS 형식)
    if (eventType == "과속감지") {
        QRegularExpression backTimestampRegex("(\\d{4})-(\\d{2})-(\\d{2})_(\\d{2})-(\\d{2})-(\\d{2})");
        QRegularExpressionMatch backMatch = backTimestampRegex.match(filename);
        if (backMatch.hasMatch()) {
            QString year = backMatch.captured(1);
            QString month = backMatch.captured(2);
            QString day = backMatch.captured(3);
            QString hour = backMatch.captured(4);
            QString minute = backMatch.captured(5);
            QString second = backMatch.captured(6);
            timeStr = QString("%1%2%3_%4%5%6").arg(year, month, day, hour, minute, second);
        }
    }
    
    // 뒷쪽 시간이 없거나 과속감지가 아닌 경우 앞쪽 타임스탬프 사용 (YYYYMMDD_HHMMSS 형식)
    if (timeStr.isEmpty()) {
        QRegularExpression frontTimestampRegex("(\\d{8})_(\\d{6})");
        QRegularExpressionMatch frontMatch = frontTimestampRegex.match(filename);
        if (frontMatch.hasMatch()) {
            QString dateStr = frontMatch.captured(1); // YYYYMMDD
            QString timeOnlyStr = frontMatch.captured(2); // HHMMSS
            timeStr = QString("%1_%2").arg(dateStr, timeOnlyStr);
        }
    }
    
    // person으로 시작하는 경우
    if (filename.startsWith("person_")) {
        if (!timeStr.isEmpty()) {
            return QString("보행자감지_%1").arg(timeStr);
        }
        return "보행자감지";
    }
    
    // speed로 시작하는 경우
    if (filename.startsWith("speed_")) {
        if (!timeStr.isEmpty()) {
            return QString("과속감지_%1").arg(timeStr);
        }
        return "과속감지";
    }
    
    // shm_startshot으로 시작하는 경우
    if (filename.startsWith("shm_startshot_")) {
        if (!timeStr.isEmpty()) {
            return QString("주정차시작_%1").arg(timeStr);
        }
        return "주정차시작";
    }
    
    // shm_endshot으로 시작하는 경우
    if (filename.startsWith("shm_endshot_")) {
        if (!timeStr.isEmpty()) {
            return QString("주정차종료_%1").arg(timeStr);
        }
        return "주정차종료";
    }
    
    // shm_snapshot으로 시작하는 경우
    if (filename.startsWith("shm_snapshot_")) {
        // 이벤트 타입이 "주정차감지"인 경우 illegalparking으로 처리
        if (eventType == "주정차감지") {
            if (!timeStr.isEmpty()) {
                return QString("주정차감지_%1").arg(timeStr);
            }
            return "주정차감지";
        } else {
            // 그 외의 경우 speeding으로 처리
            if (!timeStr.isEmpty()) {
                return QString("과속감지_%1").arg(timeStr);
            }
            return "과속감지";
        }
    }
    
    // 매칭되지 않는 경우 원본 반환
    return filename;
}
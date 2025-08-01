#include "mainwindow/filenameutils.h"
#include <QRegularExpression>
#include <QDateTime>

QString convertFilename(const QString& originalPath, const QString& eventType) {
    // "images/" 제거
    QString filename = originalPath;
    if (filename.startsWith("images/")) {
        filename = filename.mid(7); // "images/" 제거
    }
    
    // 파일명에서 타임스탬프 추출 (YYYYMMDD_HHMMSS 형식)
    QRegularExpression timestampRegex("(\\d{8})_(\\d{6})");
    QRegularExpressionMatch timestampMatch = timestampRegex.match(filename);
    QString timeStr = "";
    if (timestampMatch.hasMatch()) {
        QString dateStr = timestampMatch.captured(1); // YYYYMMDD
        QString timeOnlyStr = timestampMatch.captured(2); // HHMMSS
        timeStr = QString("%1_%2").arg(dateStr, timeOnlyStr);
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
#include "mainwindow/filenameutils.h"
#include <QRegularExpression>

QString convertFilename(const QString& originalPath, const QString& eventType) {
    // "images/" 제거
    QString filename = originalPath;
    if (filename.startsWith("images/")) {
        filename = filename.mid(7); // "images/" 제거
    }
    
    // person으로 시작하는 경우
    if (filename.startsWith("person_")) {
        QRegularExpression personRegex("person_(\\d+)_");
        QRegularExpressionMatch match = personRegex.match(filename);
        if (match.hasMatch()) {
            return QString("person_%1").arg(match.captured(1));
        }
    }
    
    // shm_startshot으로 시작하는 경우
    if (filename.startsWith("shm_startshot_")) {
        QRegularExpression startshotRegex("shm_startshot_(\\d+)_");
        QRegularExpressionMatch match = startshotRegex.match(filename);
        if (match.hasMatch()) {
            return QString("illegalparking_start_%1").arg(match.captured(1));
        }
    }
    
    // shm_endshot으로 시작하는 경우
    if (filename.startsWith("shm_endshot_")) {
        QRegularExpression endshotRegex("shm_endshot_(\\d+)_");
        QRegularExpressionMatch match = endshotRegex.match(filename);
        if (match.hasMatch()) {
            return QString("illegalparking_end_%1").arg(match.captured(1));
        }
    }
    
    // shm_snapshot으로 시작하는 경우
    if (filename.startsWith("shm_snapshot_")) {
        QRegularExpression snapshotRegex("shm_snapshot_(\\d+)_(\\d+)_");
        QRegularExpressionMatch match = snapshotRegex.match(filename);
        if (match.hasMatch()) {
            // 이벤트 타입이 "주정차감지"인 경우 illegalparking으로 처리
            if (eventType == "주정차감지") {
                return QString("illegalparking_%1_%2").arg(match.captured(1), match.captured(2));
            } else {
                // 그 외의 경우 speeding으로 처리
                return QString("speeding_%1_%2").arg(match.captured(1), match.captured(2));
            }
        }
    }
    
    // 매칭되지 않는 경우 원본 반환
    return filename;
}
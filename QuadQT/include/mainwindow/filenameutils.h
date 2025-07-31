#ifndef FILENAMEUTILS_H
#define FILENAMEUTILS_H

#include <QString>

// 파일명 변환 함수 (이벤트 타입 정보 포함)
QString convertFilename(const QString& originalPath, const QString& eventType = "");

#endif // FILENAMEUTILS_H
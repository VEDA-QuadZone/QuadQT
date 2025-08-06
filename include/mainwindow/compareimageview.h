#pragma once
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QDateTime>
#include <QMessageBox>
#include <QFile>
#include <QIODevice>
#include <QPdfWriter>
#include <QPageSize>
#include <QPainter>

class OverlayWidget;

class CompareImageView : public QDialog {
    Q_OBJECT
public:
    CompareImageView(const QString& event, const QString& plate, const QString& datetime,
                     const QString& startFilename, const QString& endFilename, QWidget* parent=nullptr);

    void setStartImageData(const QByteArray& data);
    void setEndImageData(const QByteArray& data);

signals:
    void downloadRequested();
    void printRequested();

private slots:
    void downloadStartImage();
    void downloadEndImage();
    void printToPdf();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    QLabel* startImageLabel_;
    QLabel* endImageLabel_;
    QLabel* startFilenameLabel_;
    QLabel* endFilenameLabel_;
    QPushButton* downloadButton1_;
    QPushButton* downloadButton2_;
    QPushButton* printButton_;
    QPushButton* closeButton_;
    
    QString startFilename_;
    QString endFilename_;
    QString eventType_;       // 이벤트 타입 저장
    QByteArray startImageData_;
    QByteArray endImageData_;
    
    // 창 드래그를 위한 변수
    QPoint dragPosition_;
    bool dragging_;
    
    // 오버레이 효과
    OverlayWidget* overlay_;
    
    // 파일명에서 시간 파싱하는 함수
    QString parseEndFilenameTimestamp(const QString& filename);
};
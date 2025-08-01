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

class GetImageView : public QDialog {
    Q_OBJECT
public:
    GetImageView(const QString& event, const QString& plate, const QString& datetime,
                 const QString& filename, QWidget* parent=nullptr);

    void setImageData(const QByteArray& data);

signals:
    void downloadRequested();
    void printRequested();

private slots:
    void downloadImage();
    void printToPdf();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    QLabel* eventLabel_;
    QLabel* plateLabel_;
    QLabel* dateLabel_;
    QLabel* imageLabel_;
    QLabel* filenameLabel_;   // 파일명(하이퍼링크)
    QPushButton* downloadButton_;
    QPushButton* printButton_;
    QPushButton* closeButton_;
    QByteArray imageData_;    // 이미지 데이터 저장
    QString eventType_;       // 이벤트 타입 저장
    
    // 창 드래그를 위한 변수
    QPoint dragPosition_;
    bool dragging_;
    
    // 오버레이 효과
    OverlayWidget* overlay_;
};

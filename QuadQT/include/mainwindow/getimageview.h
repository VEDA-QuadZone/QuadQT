#pragma once
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>

class GetImageView : public QDialog {
    Q_OBJECT
public:
    GetImageView(const QString& event, const QString& plate, const QString& datetime,
                 const QString& filename, QWidget* parent=nullptr);

    void setImageData(const QByteArray& data);

signals:
    void downloadRequested();
    void printRequested();

private:
    QLabel* eventLabel_;
    QLabel* plateLabel_;
    QLabel* dateLabel_;
    QLabel* imageLabel_;
    QLabel* filenameLabel_;   // 파일명(하이퍼링크)
    QPushButton* downloadButton_;
    QPushButton* printButton_;
    QPushButton* closeButton_;
};

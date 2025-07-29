#pragma once
#include <QDialog>
#include <QLabel>

class GetImageView : public QDialog {
    Q_OBJECT
public:
    GetImageView(const QString& path, const QString& timestamp, QWidget* parent=nullptr);

    void setImageData(const QByteArray& data);

private:
    QLabel* imageLabel_;
    QLabel* filenameLabel_;
    QLabel* timeLabel_;
};

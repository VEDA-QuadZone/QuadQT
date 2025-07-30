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
    QByteArray startImageData_;
    QByteArray endImageData_;
};
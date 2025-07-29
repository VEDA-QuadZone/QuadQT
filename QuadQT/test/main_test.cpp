#include <QApplication>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "🚀 테스트 애플리케이션 시작...";
    qDebug() << "🏠 메인 윈도우 직접 생성 및 표시";
    
    MainWindow w;
    w.show();
    
    qDebug() << "🎯 메인 윈도우 표시됨, 메인 이벤트 루프 진입";
    return app.exec();
}
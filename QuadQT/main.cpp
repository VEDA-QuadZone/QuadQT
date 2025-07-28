#include <QApplication>
#include "loginpage.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginPage login;
    login.setFixedSize(800, 600);

    if (login.exec() == QDialog::Accepted) {
        MainWindow w;        // 굳이 new 안 써도 됩니다 (수명은 이벤트 루프 동안 유지)
        w.show();
        return app.exec();
    }
    return 0;
}

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

# Qt의 내장 SSL 지원 사용 (OpenSSL 라이브러리 직접 링크 제거)
# Windows에서는 Qt가 OpenSSL을 동적으로 로드합니다

# ====== INCLUDE PATHS ======
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/include/login \
    $$PWD/include/mainwindow

# ====== SOURCES (.cpp) ======
SOURCES += \
    main.cpp \  # 또는 main_test.cpp 사용 시 교체
    src/login/loginpage.cpp \
    src/login/networkmanager.cpp \
    src/mainwindow/displaysettingbox.cpp \
    src/mainwindow/mainwindow.cpp \
    src/mainwindow/notificationitem.cpp \
    src/mainwindow/notificationpanel.cpp \
    src/mainwindow/procsettingbox.cpp \
    src/mainwindow/topbarwidget.cpp

# ====== HEADERS (.h) ======
HEADERS += \
    include/login/loginpage.h \
    include/login/networkmanager.h \
    include/mainwindow/displaysettingbox.h \
    include/mainwindow/mainwindow.h \
    include/mainwindow/notificationitem.h \
    include/mainwindow/notificationpanel.h \
    include/mainwindow/procsettingbox.h \
    include/mainwindow/topbarwidget.h

# ====== FORMS (.ui) ======
FORMS += \
    loginpage.ui \
    mainwindow.ui

# ====== RESOURCES (.qrc) ======
RESOURCES += \
    resources/images.qrc \
    resources/style.qrc
    resource.qrc

# ====== Default install rules (optional) ======
qnx: target.path = /tmp/$${TARGET}/bin
unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ca.cert.pem \
    client.cert.pem \
    client.key.pem \
    config.ini

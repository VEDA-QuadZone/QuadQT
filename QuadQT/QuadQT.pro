QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

<<<<<<< Updated upstream
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
=======
# Qt의 내장 SSL 지원 사용 (OpenSSL 라이브러리 직접 링크 제거)
# Windows에서는 Qt가 OpenSSL을 동적으로 로드합니다

# ====== INCLUDE PATHS ======
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/include/login \
    $$PWD/include/mainwindow
>>>>>>> Stashed changes

SOURCES += \
<<<<<<< Updated upstream
    main.cpp \
    mainwindow.cpp \
    loginpage.cpp \
    networkmanager.cpp
=======
    main.cpp \  # 또는 test/main_test.cpp 사용 시 교체
    src/login/loginpage.cpp \
    src/login/networkmanager.cpp \
    src/mainwindow/displaysettingbox.cpp \
    src/mainwindow/mainwindow.cpp \
    src/mainwindow/procsettingbox.cpp \
    src/mainwindow/topbarwidget.cpp
>>>>>>> Stashed changes

HEADERS += \
    mainwindow.h \
    loginpage.h \
    networkmanager.h

FORMS += \
    mainwindow.ui \
    loginpage.ui

<<<<<<< Updated upstream
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
=======
# ====== RESOURCES (.qrc) ======
RESOURCES += \
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
>>>>>>> Stashed changes

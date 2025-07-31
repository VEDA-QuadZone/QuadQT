QT       += core gui network widgets mqtt multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

# Qt의 내장 SSL 지원 사용 (OpenSSL 라이브러리 직접 링크 제거)
# Windows에서는 Qt가 OpenSSL을 동적으로 로드합니다

# ====== INCLUDE PATHS ======
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/include/login \
    $$PWD/include/mainwindow \
    C:/opencv/prebuilt_mingw/include

# GStreamer 헤더 경로
INCLUDEPATH += "C:/Program Files/gstreamer/1.0/mingw_x86_64/include/gstreamer-1.0"
INCLUDEPATH += "C:/Program Files/gstreamer/1.0/mingw_x86_64/include"

# glib 관련 추가 경로
INCLUDEPATH += "C:/Program Files/gstreamer/1.0/mingw_x86_64/include/glib-2.0"
INCLUDEPATH += "C:/Program Files/gstreamer/1.0/mingw_x86_64/lib/glib-2.0/include"

# GStreamer 라이브러리 링크 (경로는 반드시 따옴표)
LIBS += -L"C:/Program Files/gstreamer/1.0/mingw_x86_64/lib" \
        -lgstreamer-1.0 \
        -lgstbase-1.0 \
        -lgstvideo-1.0 \
        -lgobject-2.0 \
        -lglib-2.0


# ====== SOURCES (.cpp) ======
SOURCES += \
    main.cpp \
    src/login/loginpage.cpp \
    src/login/networkmanager.cpp \
    src/mainwindow/displaysettingbox.cpp \
    src/mainwindow/getimageview.cpp \
    src/mainwindow/historyview.cpp \
    src/mainwindow/mainwindow.cpp \
    src/mainwindow/mqttmanager.cpp \
    src/mainwindow/notificationitem.cpp \
    src/mainwindow/notificationpanel.cpp \
    src/mainwindow/procsettingbox.cpp \
    src/mainwindow/rtspplayer_gst.cpp \
    src/mainwindow/tcphistoryhandler.cpp \
    src/mainwindow/tcpimagehandler.cpp \
    src/mainwindow/topbarwidget.cpp

# ====== HEADERS (.h) ======
HEADERS += \
    include/login/loginpage.h \
    include/login/networkmanager.h \
    include/mainwindow/displaysettingbox.h \
    include/mainwindow/getimageview.h \
    include/mainwindow/historyview.h \
    include/mainwindow/mainwindow.h \
    include/mainwindow/mqttmanager.h \
    include/mainwindow/notificationitem.h \
    include/mainwindow/notificationpanel.h \
    include/mainwindow/procsettingbox.h \
    include/mainwindow/rtspplayer_gst.h \
    include/mainwindow/tcphistoryhandler.h \
    include/mainwindow/tcpimagehandler.h \
    include/mainwindow/topbarwidget.h

# ====== FORMS (.ui) ======
FORMS += \
    loginpage.ui \
    mainwindow.ui

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

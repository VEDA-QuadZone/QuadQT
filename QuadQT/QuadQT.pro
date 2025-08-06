# ===================================
#  빌드 출력 경로 설정 (가장 중요)
# ===================================
CONFIG(release, debug|release) {
    # Release 모드일 때 적용될 설정
    DESTDIR     = $$OUT_PWD/release
    OBJECTS_DIR = $$OUT_PWD/release/.obj
} else {
    # Debug 모드일 때 적용될 설정
    DESTDIR     = $$OUT_PWD/debug
    OBJECTS_DIR = $$OUT_PWD/debug/.obj
}

# ====== INCLUDE PATHS ======
# (이하 기존 내용)



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
    C:/opencv/prebuilt_mingw/include \
    C:/opencv/OpenCV-MinGW-Build/include

LIBS += -LC:/opencv/OpenCV-MinGW-Build/x64/mingw/lib \
    -lopencv_core455 \
    -lopencv_videoio455 \
    -lopencv_imgcodecs455 \
    -lopencv_imgproc455 \
    -lopencv_highgui455

# ====== SOURCES (.cpp) ======
SOURCES += \
    main.cpp \
    src/login/custommessagebox.cpp \
    src/login/loginpage.cpp \
    src/login/networkmanager.cpp \
    src/mainwindow/displaysettingbox.cpp \
    src/mainwindow/compareimageview.cpp \
    src/mainwindow/filenameutils.cpp \
    src/mainwindow/getimageview.cpp \
    src/mainwindow/historyview.cpp \
    src/mainwindow/mainwindow.cpp \
    src/mainwindow/mqttmanager.cpp \
    src/mainwindow/notificationitem.cpp \
    src/mainwindow/notificationpanel.cpp \
    src/mainwindow/overlaywidget.cpp \
    src/mainwindow/procsettingbox.cpp \
    src/mainwindow/rtspthread.cpp \
    src/mainwindow/tcphistoryhandler.cpp \
    src/mainwindow/tcpimagehandler.cpp \
    src/mainwindow/topbarwidget.cpp

# ====== HEADERS (.h) ======
HEADERS += \
    include/login/custommessagebox.h \
    include/login/loginpage.h \
    include/login/networkmanager.h \
    include/mainwindow/compareimageview.h \
    include/mainwindow/displaysettingbox.h \
    include/mainwindow/filenameutils.h \
    include/mainwindow/getimageview.h \
    include/mainwindow/historyview.h \
    include/mainwindow/mainwindow.h \
    include/mainwindow/mqttmanager.h \
    include/mainwindow/notificationitem.h \
    include/mainwindow/notificationpanel.h \
    include/mainwindow/overlaywidget.h \
    include/mainwindow/procsettingbox.h \
    include/mainwindow/rtspthread.h \
    include/mainwindow/tcphistoryhandler.h \
    include/mainwindow/tcpimagehandler.h \
    include/mainwindow/topbarwidget.h

# ====== FORMS (.ui) ======
FORMS += \
    ui/loginpage.ui \
    ui/mainwindow.ui

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

# ====== 빌드 후 설정 파일 자동 복사 (간단한 버전) ======
win32 {
    # config.ini 복사만 (안전한 방법)
    QMAKE_POST_LINK += copy /Y $$shell_path($$PWD/config.ini) $$shell_path($$DESTDIR) $$escape_expand(\\n\\t)
}

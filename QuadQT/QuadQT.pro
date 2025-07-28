QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

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
    src/mainwindow/procsettingbox.cpp \
    src/mainwindow/topbarwidget.cpp

# ====== HEADERS (.h) ======
HEADERS += \
    include/login/loginpage.h \
    include/login/networkmanager.h \
    include/mainwindow/displaysettingbox.h \
    include/mainwindow/mainwindow.h \
    include/mainwindow/procsettingbox.h \
    include/mainwindow/topbarwidget.h

# ====== FORMS (.ui) ======
FORMS += \
    loginpage.ui \
    mainwindow.ui

# ====== RESOURCES (.qrc) ======
RESOURCES += \
    resources/source.qrc

# ====== Default install rules (optional) ======
qnx: target.path = /tmp/$${TARGET}/bin
unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.ini

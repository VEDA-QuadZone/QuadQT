QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

# ====== SOURCES (.cpp) ======
SOURCES += \
    main.cpp \
    src/mainwindow/displaysettingbox.cpp \
    src/mainwindow/mainwindow.cpp \
    src/mainwindow/procsettingbox.cpp \
    src/mainwindow/topbarwidget.cpp \
    src/login/loginpage.cpp \
    src/login/networkmanager.cpp

# ====== HEADERS (.h) ======
HEADERS += \
    include/mainwindow/displaysettingbox.h \
    include/mainwindow/mainwindow.h \
    include/mainwindow/procsettingbox.h \
    include/mainwindow/topbarwidget.h \
    include/login/loginpage.h \
    include/login/networkmanager.h

# ====== FORMS (.ui) ======
FORMS += \
    mainwindow.ui \
    loginpage.ui

# ====== RESOURCES ======
RESOURCES += \
    source.qrc

# ====== Default rules ======
qnx: target.path = /tmp/$${TARGET}/bin
unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

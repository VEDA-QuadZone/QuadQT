QT       += core gui network multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

# ====== INCLUDE PATHS ======
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/include/login \
    $$PWD/include/mainwindow \
    C:/opencv/prebuilt_mingw/include

# ====== SOURCES (.cpp) ======
SOURCES += \
    main.cpp \
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
    resources/certs.qrc \
    resources/certs/certs.qrc \
    resources/images.qrc \
    resources/style.qrc \

# ====== OpenCV (MinGW Prebuilt) ======
#LIBS += -LC:/opencv/prebuilt_mingw/x64/mingw/lib \
    # -lopencv_core455 \
    # -lopencv_imgproc455 \
    # -lopencv_highgui455 \
    # -lopencv_videoio455 \
    # -lopencv_imgcodecs455

# ====== Default install rules (optional) ======
qnx: target.path = /tmp/$${TARGET}/bin
unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.ini

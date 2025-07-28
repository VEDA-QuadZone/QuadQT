QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    displaysettingbox.cpp \
    loginpage.cpp \
    main.cpp \
    mainwindow.cpp \
    procsettingbox.cpp \
    topbarwidget.cpp \
    networkmanager.cpp

HEADERS += \
    displaysettingbox.h \
    mainwindow.h \
    loginpage.h \
    procsettingbox.h \
    topbarwidget.h \
    networkmanager.h

FORMS += \
    mainwindow.ui \
    loginpage.ui

RESOURCES += \
    resources.qrc \
    source.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    displaysettingbox.cpp \
    loginpage.cpp \
    main.cpp \
    mainwindow.cpp \
    procsettingbox.cpp \
    topbarwidget.cpp

HEADERS += \
    displaysettingbox.h \
    mainwindow.h \
    procsettingbox.h \
    topbarwidget.h
    loginpage.cpp \
    networkmanager.cpp

HEADERS += \
    mainwindow.h \
    loginpage.h \
    networkmanager.h

FORMS += \
    mainwindow.ui \
    loginpage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc \
    source.qrc

DISTFILES +=
!isEmpty(target.path): INSTALLS += target

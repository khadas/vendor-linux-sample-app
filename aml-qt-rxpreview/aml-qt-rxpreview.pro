QT       += core gui multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# add GStreamer modules
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0 gstreamer-video-1.0
INCLUDEPATH += /usr/include/gstreamer-1.0

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -L/usr/lib -ltvclient

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    QtRx.cpp \
    Page1.cpp \
    Page2.cpp \
    Page3.cpp

HEADERS += \
    mainwindow.h \
    Whiteboard.h \
    QtRx.h \
    Page1.h \
    Page2.h \
    Page3.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

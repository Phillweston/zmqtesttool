QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simplemqttclient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060000 # disables all APIs deprecated in Qt 6.0.0 and earlier

SOURCES += src/main.cpp\
        src/aboutdialog.cpp \
        src/mainwindow.cpp

HEADERS  += mainwindow.h \
    include/aboutdialog.h \
    include/mainwindow.h

FORMS    += ui/aboutdialog.ui \
    ui/mainwindow.ui


target.path = $$[QT_INSTALL_EXAMPLES]/mqtt/simpleclient
INSTALLS += target

RESOURCES += \
    resources/images.qrc

DISTFILES += \
    resources/images/add.png \
    resources/images/cleanport.png \
    resources/images/clearbytes.png \
    resources/images/edit-clear.png \
    resources/images/exit.png \
    resources/images/find.png \
    resources/images/header.bmp \
    resources/images/lcd.bmp \
    resources/images/loadfile.png \
    resources/images/logo-toggle.svg \
    resources/images/logo.png \
    resources/images/mesage.png \
    resources/images/open.png \
    resources/images/pause.png \
    resources/images/save.png \
    resources/images/send.png \
    resources/images/stop.png \
    resources/images/time.png \
    resources/images/wincom.ICO \
    resources/images/write2file.png \
    resources/myico.rc

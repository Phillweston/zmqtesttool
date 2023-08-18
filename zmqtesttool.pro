QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zmqtesttool
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

HEADERS  += include/mainwindow.h \
    include/aboutdialog.h \
    include/Publisher.hpp \
    include/SampleBase.hpp \
    include/Subscriber.hpp \
    include/aboutdialog.h \
    include/cppzmq/zmq.h \
    include/cppzmq/zmq.hpp \
    include/mainwindow.h \
    include/nzmqt/global.hpp \
    include/nzmqt/impl.hpp \
    include/nzmqt/nzmqt.hpp

FORMS    += ui/aboutdialog.ui \
    ui/mainwindow.ui

INCLUDEPATH += include

RESOURCES += \
    resources/images.qrc

RC_FILE += resources/myico.rc

win32 {
    LIBS += -L$$PWD/lib -llibzmq

    SOURCE_DIR = $$PWD

    CONFIG(release, debug|release): {
        DESTDIR += release
        LIBS += -L$$SOURCE_DIR/lib -lQt5Gui
        LIBS += -L$$SOURCE_DIR/lib -lQt5Widgets

        # Specify additional dependencies that should be built before the target is built, but after all other dependencies.
        POST_TARGETDEPS += $$SOURCE_DIR/bin/libzmq-v142-mt-4_3_5.dll
        POST_TARGETDEPS += $$SOURCE_DIR/bin/Qt5Gui.dll
        POST_TARGETDEPS += $$SOURCE_DIR/bin/Qt5Widgets.dll

        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SOURCE_DIR/bin/libzmq-v142-mt-4_3_5.dll) $$quote($$DESTDIR) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SOURCE_DIR/bin/Qt5Gui.dll) $$quote($$DESTDIR) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SOURCE_DIR/bin/Qt5Widgets.dll) $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    } else {
        DESTDIR += debug
        LIBS += -L$$SOURCE_DIR/lib -lQt5Guid
        LIBS += -L$$SOURCE_DIR/lib -lQt5Widgetsd

        # Specify additional dependencies that should be built before the target is built, but after all other dependencies.
        POST_TARGETDEPS += $$SOURCE_DIR/bin/libzmq-v142-mt-4_3_5.dll
        POST_TARGETDEPS += $$SOURCE_DIR/bin/Qt5Guid.dll
        POST_TARGETDEPS += $$SOURCE_DIR/bin/Qt5Widgetsd.dll

        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SOURCE_DIR/bin/libzmq-v142-mt-4_3_5.dll) $$quote($$DESTDIR) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SOURCE_DIR/bin/Qt5Guid.dll) $$quote($$DESTDIR) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SOURCE_DIR/bin/Qt5Widgetsd.dll) $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    }
}

// Copyright (C) 2023 The JOUAV Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#define SHOW_DEBUG 1
#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QApplication>
#else
#include <QtGui/QApplication>
#endif
#include "mainwindow.h"
#include <QTextCodec>

MainWindow *mainWindowInstance = nullptr;

void DebugMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    // Format the message as per your requirement.
    QString message;
    switch (type)
    {
        case QtDebugMsg:
            message = QString("Debug: %1").arg(msg);
            break;
        case QtInfoMsg:
            message = QString("Info: %1").arg(msg);
            break;
        case QtWarningMsg:
            message = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            message = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            message = QString("Fatal: %1").arg(msg);
            break;
    }

    mainWindowInstance->logMessage(message);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
#endif
    MainWindow w;
    // mainWindowInstance should be a global or static pointer to the MainWindow instance
    mainWindowInstance = &w;
#if SHOW_DEBUG == 1
    // Install the debug message handler, show in the text box
    qInstallMessageHandler(DebugMessageHandler);
#endif

    w.show();
    return a.exec();
}

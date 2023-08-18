// Copyright 2011-2014 Johann Duscher (a.k.a. Jonny Dee). All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this list of
//       conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice, this list
//       of conditions and the following disclaimer in the documentation and/or other materials
//       provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY JOHANN DUSCHER ''AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those of the
// authors and should not be interpreted as representing official policies, either expressed
// or implied, of Johann Duscher.

#ifndef NZMQT_SAMPLEBASE_H
#define NZMQT_SAMPLEBASE_H

#include "nzmqt/nzmqt.hpp"

#include <QDebug>
#include <QEventLoop>
#include <QThread>
#include <QDateTime>
#include <QElapsedTimer>


namespace nzmqt
{

namespace samples
{

class SampleBase : public QObject
{
    Q_OBJECT
    typedef QObject super;

signals:
    void finished();
    void failure(const QString& what);
    void signal_log(int type, const QString& message);

public slots:
    void start();
    void stop();

protected:
    SampleBase(QObject* parent);

    virtual void startImpl() = 0;

    static void sleep(unsigned long msecs);

    virtual QString getCurrentTime();

private:
    class ThreadTools : private QThread
    {
    public:
        using QThread::msleep;

    private:
        ThreadTools() {}
    };
};

inline SampleBase::SampleBase(QObject* parent)
    : super(parent)
{
}

inline void SampleBase::start()
{
    try
    {
        startImpl();
    }
    catch (const nzmqt::ZMQException& ex)
    {
        qWarning() << Q_FUNC_INFO << "Exception:" << ex.what();
        QString info = QString("Exception: %1").arg(ex.what());
        emit failure(ex.what());
        emit finished();
        emit signal_log(1, info);
    }
}

inline void SampleBase::stop()
{
    qDebug() << Q_FUNC_INFO << "Info: Stopping";
    emit finished();
}

inline void SampleBase::sleep(unsigned long msecs)
{
    ThreadTools::msleep(msecs);
}

inline QString SampleBase::getCurrentTime()
{
    // Get the current time with millisecond precision
    QDateTime now = QDateTime::currentDateTime();

    // Start an elapsed timer immediately after fetching the current time
    QElapsedTimer timer;
    timer.start();

    // Use the elapsed microseconds since the last millisecond boundary to adjust the time
    qint64 elapsedMicroseconds = timer.nsecsElapsed() / 1000;
    now = now.addMSecs(elapsedMicroseconds / 1000.0);

    // Format the time string to include the microseconds
    QString format = "yyyy-MM-dd hh:mm:ss.zzz";
    return now.toString(format);
}

}

}

#endif // NZMQT_SAMPLEBASE_H

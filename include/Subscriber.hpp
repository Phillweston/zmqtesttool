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

#ifndef NZMQT_PUBSUBCLIENT_H
#define NZMQT_PUBSUBCLIENT_H

#include "SampleBase.hpp"

#include "nzmqt/nzmqt.hpp"

#include <QByteArray>
#include <QList>


namespace nzmqt
{

namespace samples
{

namespace pubsub
{

class Subscriber : public SampleBase
{
    Q_OBJECT
    typedef SampleBase super;

public:
    explicit Subscriber(ZMQContext& context, const QString& address, const QString& topic, const bool& useHex, QObject *parent = 0)
        : super(parent)
        , address_(address), topic_(topic), useHex_(useHex)
        , socket_(0)
    {
        socket_ = context.createSocket(ZMQSocket::TYP_SUB, this);
        socket_->setObjectName("Subscriber.Socket.socket(SUB)");
        connect(socket_, SIGNAL(messageReceived(const QList<QByteArray>&)), SLOT(subMessageReceived(const QList<QByteArray>&)));
    }

signals:
    void messageReceived(const QString& timeStamp, const QList<QByteArray>& message);

protected:
    void startImpl()
    {
        socket_->subscribeTo(topic_);
        socket_->connectTo(address_);
    }

protected slots:
    void subMessageReceived(const QList<QByteArray>& msg)
    {
        QList<QByteArray> hexMsg;
        QString currentTime = getCurrentTime();

        if (useHex_)
        {
            for (const QByteArray& ba : msg)
            {
                hexMsg.append(ba.toHex());
            }
            qDebug() << "Subscriber> " << hexMsg << ", Timestamp: " << currentTime;
            emit messageReceived(currentTime, hexMsg);
        }
        else
        {
            qDebug() << "Subscriber> " << msg << ", Timestamp: " << currentTime;
            emit messageReceived(currentTime, msg);
        }
    }

private:
    QString address_;
    QString topic_;
    QString message_;
    bool useHex_;
    ZMQSocket* socket_;
};

}

}

}

#endif // NZMQT_PUBSUBCLIENT_H

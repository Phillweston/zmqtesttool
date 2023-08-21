// Copyright (C) 2023 The JOUAV Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Subscriber.hpp"
#include "Publisher.hpp"

#include <cppzmq/zmq.hpp>
#include <QMessageBox>

using namespace nzmqt;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->buttonPublishStop->setEnabled(false);
    ui->buttonSubscribeStop->setEnabled(false);
    ui->buttonPublishStart->setEnabled(true);
    ui->buttonSubscribeStart->setEnabled(true);
    ui->spinBoxFrequency->setEnabled(false);
    ui->textViewPublish->setReadOnly(true);
    ui->textViewSubscribe->setReadOnly(true);
    ui->decPublish->setChecked(true);
    ui->decSubscribe->setChecked(true);
    ui->statusBar->showMessage(tr("Welcome to use ZeroMQ Test Tool!"));
    ui->jouav->setText(tr("<a href=\"http://www.jouav.com\">www.jouav.com</a>"));
}


/**
 * @brief Show message on the text browser.
 * @param None
 * @return None
 */
void MainWindow::showMessage(const QString &msg)
{
    // Assume that ui->textEdit is your QTextEdit control.
    ui->logMessage->append(msg);
}


/**
 * @brief Creates a new execution thread for the given sample.
 * @param sample The sample to run in the new thread.
 * @return A pointer to the new thread.
 */
QThread* MainWindow::makeExecutionThread(nzmqt::samples::SampleBase& sample) const
{
    QThread* thread = new QThread;
    sample.moveToThread(thread);

    bool connected = false;
    connected = connect(thread, SIGNAL(started()), &sample, SLOT(start()));         Q_ASSERT(connected);
    connected = connect(&sample, SIGNAL(finished()), thread, SLOT(quit()));         Q_ASSERT(connected);
    connected = connect(&sample, SIGNAL(finished()), &sample, SLOT(deleteLater())); Q_ASSERT(connected);
    connected = connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));   Q_ASSERT(connected);

    return thread;
}


/*
Publisher Context:
The publisher context encompasses creating and handling the publisher side of a publish-subscribe pattern using the nzmqt library. The details of the context are as follows:
ZMQContext: This is the core part of the ZMQ (ZeroMQ) communication. It represents the context in which the sockets run. It must be created first and is used for the entire lifetime of your application.
Publisher Creation: A publisher is created within the context, and signals are connected to track the status of messages sent, any failures, and whether the publishing has finished.
Execution Thread: The publisher's functionality is set to run in a separate thread, providing asynchronous operation.
Test Start: The context is started, followed by the publisher thread. A QTimer is used to stop the publisher after 6 seconds.
Postconditions Check: After the test is finished, conditions are checked to verify the behavior of the publisher (e.g., number of pings sent, failures, etc.).
*/
void MainWindow::publish(QString ipAddress, int port, QString topic, QString message, int frequency, int maxPublishTime, bool useHex)
{
    try
    {
        QScopedPointer<ZMQContext> context(nzmqt::createDefaultContext());

        // Create the connection string using the given IP address and port
        QString connectionString = QString("tcp://%1:%2").arg(ipAddress).arg(port);

        // Create publisher with the connection string
        samples::pubsub::Publisher* publisher = new samples::pubsub::Publisher(*context, connectionString, topic, message, frequency, useHex, this);
        connect(publisher, SIGNAL(messageSent(const QString&, const QList<QByteArray>&)), SLOT(messageSent(const QString&, const QList<QByteArray>&)));
        connect(publisher, SIGNAL(finished()), SLOT(messagePublishFinished()));
        connect(publisher, SIGNAL(signal_log(int, const QString&)), SLOT(handleLogMessage(int, const QString&)));
        // Stop publishing after user clicked the stop button, only available when in loop publish mode
        connect(ui->buttonPublishStop, &QPushButton::clicked, publisher, &samples::pubsub::Publisher::stop);

        // QSignalSpy spyPublisherMessageSent(publisher, SIGNAL(messageSent(const QString&, const QList<QByteArray>&)));
        // QSignalSpy spyPublisherFailure(publisher, SIGNAL(failure(const QString&)));
        // QSignalSpy spyPublisherFinished(publisher, SIGNAL(finished()));

        // Create publisher execution thread.
        QThread* publisherThread = makeExecutionThread(*publisher);
        // QSignalSpy spyPublisherThreadFinished(publisherThread, SIGNAL(finished()));

        // START TEST
        context->start();
        publisherThread->start();

        // Note: Adjust waiting time based on your application's requirement.
        QTimer::singleShot(maxPublishTime * 1000, publisher, SLOT(stop()));
        QTest::qWait(maxPublishTime * 1000 + 100);

        // CHECK POSTCONDITIONS
        // QCOMPARE(spyPublisherFailure.size(), 0);
        // QVERIFY2(spyPublisherMessageSent.size() > 0, "Server didn't send any messages.");
        // QCOMPARE(spyPublisherFinished.size(), 1);
        // QCOMPARE(spyPublisherThreadFinished.size(), 1);
    }
    catch (std::exception& ex)
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not publish message"));
        ui->statusBar->showMessage(tr("Error: Could not publish message"));
        QFAIL(ex.what());
    }
}


void MainWindow::messagePublishFinished()
{
    // TODO:
    ui->buttonPublishStop->setEnabled(false);
    ui->buttonPublishStart->setEnabled(true);
}


/**
 * @brief Handles log messages and displays them in a message box.
 * @param type The type of the log message. 0 for information, 1 for error, and any other value for unknown error.
 * @param message The log message to be displayed.
 */
void MainWindow::handleLogMessage(int type, const QString& message)
{
    if (type == 0)
    {
        QMessageBox::information(this, tr("Information"), message, QMessageBox::Ok);
        ui->statusBar->showMessage(tr("Information") + message);
    }
    else if (type == 1)
    {
        QMessageBox::critical(this, tr("Error"), message, QMessageBox::Ok);
        ui->statusBar->showMessage(tr("Error") + message);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Unknown error"), QMessageBox::Ok);
        ui->statusBar->showMessage(tr("Unknown error") + message);
    }
}


/**
 * @brief Stops the publisher thread and disables the "Stop" button while enabling the "Start" button.
 * @param None
 * @return None
 */
void MainWindow::on_buttonPublishStop_clicked()
{
    ui->buttonPublishStop->setEnabled(false);
    ui->buttonPublishStart->setEnabled(true);
    ui->statusBar->showMessage(tr("Publish stopped"));
}


/*
Subscriber Context:
The subscriber context refers to creating and handling the subscriber side of a publish-subscribe pattern. It is symmetrical to the publisher context with some details:
ZMQContext: Just like the publisher, the subscriber needs a ZMQ context for its operation.
Subscriber Creation: A subscriber is created within the same context, listening to the same endpoint that the publisher is using to publish the messages.
Execution Thread: Similar to the publisher, the subscriber's functionality runs in a separate thread.
Test Start: The subscriber thread starts with a slight delay (500 ms) and is stopped after 6 seconds, similar to the publisher.
Postconditions Check: After the test is finished, conditions are checked to verify the behavior of the subscriber (e.g., number of pings received, failures, etc.).
*/
void MainWindow::subscribe(QString ipAddress, int port, QString topic, int maxSubscribeTime, bool useHex)
{
    try
    {
        QScopedPointer<ZMQContext> context(nzmqt::createDefaultContext());

        // Create the connection string using the given IP address and port
        QString connectionString = QString("tcp://%1:%2").arg(ipAddress).arg(port);

        // Create subscriber with the connection string and the specified topic
        samples::pubsub::Subscriber* subscriber = new samples::pubsub::Subscriber(*context, connectionString, topic, useHex);
        connect(subscriber, SIGNAL(messageReceived(const QString&, const QList<QByteArray>&)), SLOT(messageReceived(const QString&, const QList<QByteArray>&)));
        connect(subscriber, SIGNAL(finished()), SLOT(messageSubscribeFinished()));
        connect(subscriber, SIGNAL(signal_log(int, const QString&)), SLOT(handleLogMessage(int, const QString&)));

        // Stop subscribing after user clicked the stop button
        connect(ui->buttonSubscribeStop, &QPushButton::clicked, subscriber, &samples::pubsub::Subscriber::stop);

        // QSignalSpy spySubscriberMessageReceived(subscriber, SIGNAL(messageReceived(const QString&, const QList<QByteArray>&)));
        // QSignalSpy spySubscriberFailure(subscriber, SIGNAL(failure(const QString&)));
        // QSignalSpy spySubscriberFinished(subscriber, SIGNAL(finished()));

        // Create subscriber execution thread.
        QThread* subscriberThread = makeExecutionThread(*subscriber);
        // QSignalSpy spySubscriberThreadFinished(subscriberThread, SIGNAL(finished()));

        // Start subscriber thread
        context->start();
        subscriberThread->start();

        // Note: Adjust waiting time based on your application's requirement.
        QTimer::singleShot(maxSubscribeTime * 1000, subscriber, SLOT(stop()));
        QTest::qWait(maxSubscribeTime * 1000 + 100);

        // CHECK POSTCONDITIONS
        // qDebug() << "Subscriber messages received:" << spySubscriberMessageReceived.size();
        // QCOMPARE(spySubscriberFailure.size(), 0);
        // QVERIFY2(spySubscriberMessageReceived.size() > 0, "Client didn't receive any messages.");
        // QCOMPARE(spySubscriberFinished.size(), 1);
        // QCOMPARE(spySubscriberThreadFinished.size(), 1);
    }
    catch (std::exception& ex)
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not subscribe message"));
        ui->statusBar->showMessage(tr("Error: Could not subscribe message"));
        QFAIL(ex.what());
    }
}


void MainWindow::messageSubscribeFinished()
{
    ui->buttonSubscribeStop->setEnabled(false);
    ui->buttonSubscribeStart->setEnabled(true);
    ui->statusBar->showMessage(tr("Message subscription expired"));
}


/**
 * @brief Appends the given timestamp and message list to the publish text view.
 * @param timeStamp The timestamp to be appended to the text view.
 * @param messageList The list of messages to be appended to the text view.
 * @return None
 */
void MainWindow::messageSent(const QString& timeStamp, const QList<QByteArray>& messageList)
{
    ui->lcdNumberPublish->display(ui->lcdNumberPublish->value() + 1);
    ui->textViewPublish->append("TimeStamp: ");
    ui->textViewPublish->append(timeStamp);
    ui->textViewPublish->append(" Message: ");
    foreach(const QByteArray& message, messageList)
    {
        ui->textViewPublish->append(QString::fromUtf8(message));
    }
    ui->textViewPublish->append("\n");
}


/**
 * @brief This function is called when a message is received. It appends the timestamp and message to the subscribe text view.
 * @param timeStamp The timestamp of the received message.
 * @param messageList The list of messages received.
 * @return None
 */
void MainWindow::messageReceived(const QString& timeStamp, const QList<QByteArray>& messageList)
{
    ui->lcdNumberSubscribe->display(ui->lcdNumberSubscribe->value() + 1);
    ui->textViewSubscribe->append("TimeStamp: ");
    ui->textViewSubscribe->append(timeStamp);
    ui->textViewSubscribe->append(" Message: ");
    foreach(const QByteArray& message, messageList)
    {
        ui->textViewSubscribe->append(QString::fromUtf8(message));
    }
    ui->textViewSubscribe->append("\n");
}


/**
 * @brief Slot function.
 * @param None
 * @return None
 */
void MainWindow::on_buttonSubscribeStop_clicked()
{
    ui->buttonSubscribeStop->setEnabled(false);
    ui->buttonSubscribeStart->setEnabled(true);
    ui->statusBar->showMessage(tr("Subscribe stopped"));
}


MainWindow::~MainWindow()
{
    delete ui;
}


/**
 * @brief Resets the values of the host and ports to their default values.
 * @param None
 * @return None
 */
void MainWindow::on_buttonDefault_clicked()
{
    ui->lineEditHost->setText("127.0.0.1");
    ui->spinBoxPortSubscribe->setValue(9446);
    ui->spinBoxPortPublish->setValue(9445);
    ui->statusBar->showMessage(tr("Parameter set to default value"));
}


void MainWindow::on_buttonQuit_clicked()
{
    this->close();
}


/**
 * @brief Handles the click event of the "Publish Start" button.
 *        It retrieves the IP address, port, topic, message, and frequency from the UI elements.
 *        Validates the IP address, port, topic, and message.
 *        Calls the publish function with the retrieved parameters.
 *        Disables the "Publish Start" button and enables the "Publish Stop" button.
 * @param None
 * @return None
 */
void MainWindow::on_buttonPublishStart_clicked()
{
    QString ipAddress = ui->lineEditHost->text();
    int port = ui->spinBoxPortPublish->value();
    QString topic = ui->lineEditPublishTopic->text();
    int maxTime = ui->spinBoxMaxPublishTime->value();
    bool useHex = false;

    int frequency = 0;
    if (ui->checkBoxLoop->isChecked())
    {
        frequency = ui->spinBoxFrequency->value();
    }

    QString contents = ui->lineEditPublishMessage->text();

    if (!isValidIPv4(ipAddress))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid IP address"));
        return;
    }

    if (port < 0 || port > 65535)
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid port"));
        return;
    }

    if (!isValidString(topic))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid topic"));
        ui->statusBar->showMessage(tr("Please enter a valid topic"));
        return;
    }

    if (!isValidString(contents))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid message"));
        ui->statusBar->showMessage(tr("Please enter a valid message"));
        return;
    }

    if (ui->hexPublish->isChecked())
        useHex = true;
    else
        useHex = false;

    ui->buttonPublishStart->setEnabled(false);
    ui->buttonPublishStop->setEnabled(true);
    ui->lcdNumberPublish->display(0);
    ui->statusBar->showMessage(tr("Publishing message ..."));

    publish(ipAddress, port, topic, contents, frequency, maxTime, useHex);
}


/**
 * @brief This function is called when the user clicks on the "Subscribe Start" button. It retrieves the IP address, port, and topic from the UI, validates them, and then calls the subscribe function to start the subscription.
 * @param None
 * @return None
 */
void MainWindow::on_buttonSubscribeStart_clicked()
{
    QString ipAddress = ui->lineEditHost->text();
    int port = ui->spinBoxPortSubscribe->value();
    QString topic = ui->lineEditSubscribeTopic->text();
    int maxTime = ui->spinBoxMaxSubscribeTime->value();
    bool useHex = false;

    if (!isValidIPv4(ipAddress))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid IPv4 address"));
        ui->statusBar->showMessage(tr("Please enter a valid IPv4 address"));
        return;
    }

    if (port < 0 || port > 65535)
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid port"));
        ui->statusBar->showMessage(tr("Please enter a valid port"));
        return;
    }

    if (!isValidString(topic))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid topic"));
        ui->statusBar->showMessage(tr("Please enter a valid topic"));
        return;
    }

    if (ui->hexSubscribe->isChecked())
        useHex = true;
    else
        useHex = false;

    ui->buttonSubscribeStart->setEnabled(false);
    ui->buttonSubscribeStop->setEnabled(true);
    ui->lcdNumberSubscribe->display(0);
    ui->statusBar->showMessage(tr("Subscribing message ..."));

    subscribe(ipAddress, port, topic, maxTime, useHex);
}


/**
 * @brief Clears all input fields in the publish section of the UI.
 * @param None
 * @return None
 */
void MainWindow::on_buttonPublishClearAll_clicked()
{
    ui->textViewPublish->clear();
    ui->lineEditPublishTopic->clear();
    ui->lineEditPublishMessage->clear();
    ui->lcdNumberPublish->display(0);
    ui->statusBar->showMessage(tr("Information cleared"));
}


/**
 * @brief Clears the text view and line edit for the subscribe topic.
 * @param None
 * @return None
 */
void MainWindow::on_buttonSubscribeClearAll_clicked()
{
    ui->textViewSubscribe->clear();
    ui->lineEditSubscribeTopic->clear();
    ui->lcdNumberSubscribe->display(0);
    ui->statusBar->showMessage(tr("Information cleared"));
}


/**
 * @brief Slot function that is called when the state of the "Loop" checkbox is changed.
 *        Enables or disables the "Frequency" spin box based on the state of the checkbox.
 * @param arg1 The new state of the checkbox (Qt::Checked or Qt::Unchecked).
 * @return void
 */
void MainWindow::on_checkBoxLoop_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        ui->spinBoxFrequency->setEnabled(true);
        ui->buttonPublishStop->setEnabled(false);
    }
    else
    {
        ui->spinBoxFrequency->setEnabled(false);
        ui->buttonPublishStop->setEnabled(true);
    }
}


/**
 * @brief Displays the about dialog in the center of the main window.
 * @param None
 * @return None
 */
void MainWindow::on_actionAbout_triggered()
{
    aboutdlg.show();
    // Display in front of the main window
    int x = this->x() + (this->width() - aboutdlg.width()) / 2;
    int y = this->y() + (this->height() - aboutdlg.height()) / 2;
    aboutdlg.move(x, y);
}


void MainWindow::on_actionQuit_triggered()
{
    this->close();
}


/**
 * @brief Overrides the changeEvent function to handle language change events.
 * @param e A pointer to the QEvent object.
 * @return void
 * @note This function is called whenever a language change event occurs. It retranslated the UI to the new language.
 */
void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


/**
 * @brief Checks if the given string contains only alphanumeric characters.
 * @param str The string to be checked.
 * @return True if the string contains only alphanumeric characters, false otherwise.
 */
bool MainWindow::isValidString(const QString &str)
{
    if (str.isEmpty())
        return false;

    QRegExp regex("^[a-zA-Z0-9/]*$");
    return regex.exactMatch(str);
}


/**
 * @brief Checks if a given string is a valid IPv4 address.
 *
 * @param ip The string to be checked.
 * @return True if the string is a valid IPv4 address, false otherwise.
 */
bool MainWindow::isValidIPv4(const QString &ip)
{
    if (ip.isEmpty())
        return false;

    // Define the regular expression for an IPv4 address
    QRegExp regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    // If the regex matches, it's a valid IPv4 address
    return regex.exactMatch(ip);
}

// Copyright (C) 2023 The JOUAV Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cppzmq/zmq.hpp>
#include <QMessageBox>

using namespace nzmqt;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Create the timer but don't start it yet, it will be started once the buttonStart is clicked
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateTextEdit);

    updateLogTimer = new QTimer(this);
    // Starts the log stimer to update every 100ms
    updateLogTimer->start(100);
    connect(updateLogTimer, &QTimer::timeout, this, &MainWindow::showMessage);

    initTable();

    ui->tableViewTopics->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->buttonSend->setEnabled(false);
    ui->buttonStop->setEnabled(false);
    ui->buttonStart->setEnabled(true);
    ui->publishFrequency->setEnabled(false);
    ui->textView->setReadOnly(true);
    ui->decDisplay->setChecked(true);
    ui->checkBoxLoop->setChecked(false);
    ui->statusBar->showMessage(tr("Welcome to use ZeroMQ Test Tool!"));
    ui->jouav->setText(tr("<a href=\"http://www.jouav.com\">www.jouav.com</a>"));
}


MainWindow::~MainWindow()
{
    delete updateTimer;
    delete updateLogTimer;
    delete ui;
}


/**
 * @brief Initializes the table view for subscribing to topics.
 * @param None
 * @return None
 */
void MainWindow::initTable()
{
    QStandardItemModel *subscribeModel = new QStandardItemModel();
    subscribeModel->setHorizontalHeaderLabels(QStringList() << tr("Enable") << tr("Topic"));
    ui->tableViewTopics->setModel(subscribeModel);
}


/**
 * @brief Show message on the text browser.
 * @param None
 * @return None
 */
void MainWindow::showMessage()
{
    QMutexLocker locker(&bufferedLogMessagesMutex);
    if (!bufferedLogMessages.isEmpty())
    {
        ui->logMessage->append(bufferedLogMessages.join("\n"));
        bufferedLogMessages.clear();
    }
}


/**
 * @brief Logs a message to the buffered log messages.
 * @param msg The message to be logged.
 * @return None
 */
void MainWindow::logMessage(const QString &msg)
{
    QMutexLocker locker(&bufferedLogMessagesMutex);
    bufferedLogMessages.append(msg);
}


/**
 * @brief Creates a new execution thread for the given sample.
 * @param sample The sample to run in the new thread.
 * @return A pointer to the new thread.
 */
QThread* MainWindow::makeExecutionThread(nzmqt::samples::SampleBase& sample) const
{
    QThread* thread = new QThread;

    // Reparent before moving an object to a different thread
    sample.setParent(nullptr);
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
void MainWindow::publishInit(QString ipAddress, int port, bool useHex)
{
    try
    {
        QSharedPointer<ZMQContext> context(nzmqt::createDefaultContext());

        // Create the connection string using the given IP address and port
        QString connectionString = QString("tcp://%1:%2").arg(ipAddress).arg(port);

        // Create publisher with the connection string
        samples::pubsub::Publisher* publisher = new samples::pubsub::Publisher(*context, connectionString, useHex, this);
        connect(publisher, SIGNAL(messageSent(const QString&, const QList<QByteArray>&)), SLOT(messageSent(const QString&, const QList<QByteArray>&)));
        connect(publisher, SIGNAL(finished()), SLOT(messageFinished()));
        connect(publisher, SIGNAL(signal_log(int, const QString&)), SLOT(handleLogMessage(int, const QString&)));
        
        // Start subscriber after user clicked the add button (startAction), and stop after user clicked the stop button when the frequency is not equivalent to 0 (stopAction)
        // Note: Since we don't have a direct reference to the lambda to use in a disconnect call,
        // we need to utilize the QMetaObject::Connection object returned by the connect function to be able to disconnect it.
        // Connect your button once and use the state to decide the action
        // We need to capture the isSendMode variable by reference in the lambda so that changes to its value persist across multiple invocations of the lambda.
        QMetaObject::Connection publishMessageConnection = connect(ui->buttonSend, &QPushButton::clicked, this, [this, publisher]() {
            if (isSendMode) 
            {
                int frequency = 0;
                if (ui->checkBoxLoop->isChecked())
                {
                    frequency = ui->publishFrequency->value();
                }
                publisher->setFrequency(frequency);

                // This block handles the "Send" behavior
                bool res = publishMessage(publisher);

                if (frequency != 0 && res) 
                {
                    ui->buttonSend->setText(tr("Stop"));
                    ui->buttonSend->setIcon(QIcon(":/images/stop.png"));
                    isSendMode = false;  // Switch to "Stop" mode
                }
            }
            else
            {
                // This block handles the "Stop" behavior (stopAction)
                publisher->stopAction();
                ui->buttonSend->setText(tr("Send"));
                ui->buttonSend->setIcon(QIcon(":/images/send.png"));
                isSendMode = true;  // Switch back to "Send" mode
            }
        });
        
        // Stop publishing after user clicked the stop button
        connect(ui->buttonStop, &QPushButton::clicked, publisher, &samples::pubsub::Publisher::stop);
        // Disconnect the ui->buttonSend when ui->buttonStop is clicked
        connect(ui->buttonStop, &QPushButton::clicked, this, [this, publishMessageConnection](){
            disconnect(publishMessageConnection);
        });

        // QSignalSpy spyPublisherMessageSent(publisher, SIGNAL(messageSent(const QString&, const QList<QByteArray>&)));
        // QSignalSpy spyPublisherFailure(publisher, SIGNAL(failure(const QString&)));
        // QSignalSpy spyPublisherFinished(publisher, SIGNAL(finished()));

        // Create publisher execution thread.
        QThread* publisherThread = makeExecutionThread(*publisher);
        connect(publisherThread, &QThread::finished, [context](){}); // Keeps the context alive until the thread finishes
        
        // QSignalSpy spyPublisherThreadFinished(publisherThread, SIGNAL(finished()));

        // START TEST
        context->start();
        publisherThread->start();

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


/**
 * @brief Publishes a message to a given publisher.
 * @param publisher A pointer to the publisher object.
 * @return bool True if the message was published successfully, false otherwise.
 * @note This function retrieves the topic and contents of the message from the UI view. 
 *       If either of them is invalid, an error message is displayed and the function returns. 
 *       Otherwise, the topic and contents are added to a QStringList and passed to the publisher's startAction() function. 
 *       Finally, a status message is displayed on the UI view.
 */
bool MainWindow::publishMessage(samples::pubsub::Publisher* publisher)
{
    // Get the topic from the ui view
    QString topic = ui->lineEditPublishTopic->text();
    if (!isValidString(topic))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid topic"));
        ui->statusBar->showMessage(tr("Please enter a valid topic"));
        return false;
    }

    QString contents = ui->lineEditPublishMessage->text();
    if (!isValidString(contents))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid message"));
        ui->statusBar->showMessage(tr("Please enter a valid message"));
        return false;
    }

    QStringList messages;
    messages.append(topic);
    messages.append(contents);

    publisher->startAction(messages);

    ui->statusBar->showMessage(tr("Sending message ..."));
    return true;
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
 * @brief Clears all messages from the text view and displays a status message.
 * @param None
 * @return None
 */
void MainWindow::on_buttonClearAll_clicked()
{
    ui->textView->clear();
    ui->statusBar->showMessage(tr("Cleared all messages"));
}


/**
 * @brief Stops the publisher thread and disables the "Stop" button while enabling the "Start" button.
 * @param None
 * @return None
 */
void MainWindow::on_buttonStop_clicked()
{
    updateTimer->stop();

    ui->buttonSend->setEnabled(false);
    ui->buttonStop->setEnabled(false);
    ui->buttonStart->setEnabled(true);
    ui->buttonDefault->setEnabled(true);
    ui->lineEditHost->setEnabled(true);
    ui->spinBoxPortPublish->setEnabled(true);
    ui->spinBoxPortSubscribe->setEnabled(true);
    
    ui->buttonSend->setText(tr("Send"));
    ui->buttonSend->setIcon(QIcon(":/images/send.png"));

    ui->statusBar->showMessage(tr("Stopped"));
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
void MainWindow::subscribeInit(QString ipAddress, int port, bool useHex)
{
    try
    {
        QSharedPointer<ZMQContext> context(nzmqt::createDefaultContext());

        // Create the connection string using the given IP address and port
        QString connectionString = QString("tcp://%1:%2").arg(ipAddress).arg(port);

        // Create subscriber with the connection string and the specified topic
        samples::pubsub::Subscriber* subscriber = new samples::pubsub::Subscriber(*context, connectionString, useHex, this);
        connect(subscriber, SIGNAL(messageReceived(const QString&, const QList<QByteArray>&)), SLOT(messageReceived(const QString&, const QList<QByteArray>&)));
        connect(subscriber, SIGNAL(finished()), SLOT(messageFinished()));
        connect(subscriber, SIGNAL(signal_log(int, const QString&)), SLOT(handleLogMessage(int, const QString&)));
        
        // Subscribe topics from the table view if the table view is not empty
        QStringList topics;
        // Get the topic from the table view
        QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableViewTopics->model());
        int rows = model->rowCount();
        for (int i = 0; i < rows; i++)
        {
            QStandardItem *item = model->item(i, 0);
            if (item->checkState() == Qt::Checked)
            {
                // Second column contains the topic
                QString topic = model->item(i, 1)->text();
                topics.append(topic);
            }
        }
        if (!topics.isEmpty())
        {
            subscribeMessage(subscriber, topics);
        }

        // Start subscriber after user clicked the add button (startAction)
        // Note: Since we don't have a direct reference to the lambda to use in a disconnect call,
        // we need to utilize the QMetaObject::Connection object returned by the connect function to be able to disconnect it.
        QMetaObject::Connection subscribeMessageConnection = connect(ui->buttonAddTopic, &QPushButton::clicked, this, [this, subscriber]() {
            subscribeMessage(subscriber);
        });

        // Stop subscribing after user clicked the stop button (stopAction)
        QMetaObject::Connection unsubscribeMessageConnection = connect(ui->buttonRemoveTopic, &QPushButton::clicked, this, [this, subscriber]() {
            unsubscribeMessage(subscriber);
        });

        // Stop subscribing after user clicked the stop button
        connect(ui->buttonStop, &QPushButton::clicked, subscriber, &samples::pubsub::Subscriber::stop);
        // Disconnect the ui->buttonAddTopic and ui->buttonRemoveTopic when ui->buttonStop is clicked
        connect(ui->buttonStop, &QPushButton::clicked, this,
            [this, subscribeMessageConnection, unsubscribeMessageConnection]() {
            disconnect(subscribeMessageConnection);
            disconnect(unsubscribeMessageConnection);
        });

        // QSignalSpy spySubscriberMessageReceived(subscriber, SIGNAL(messageReceived(const QString&, const QList<QByteArray>&)));
        // QSignalSpy spySubscriberFailure(subscriber, SIGNAL(failure(const QString&)));
        // QSignalSpy spySubscriberFinished(subscriber, SIGNAL(finished()));

        // Create subscriber execution thread.
        QThread* subscriberThread = makeExecutionThread(*subscriber);
        connect(subscriberThread, &QThread::finished, [context](){}); // Keeps the context alive until the thread finishes
        
        // QSignalSpy spySubscriberThreadFinished(subscriberThread, SIGNAL(finished()));

        // Start subscriber thread
        context->start();
        subscriberThread->start();

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


/**
 * @brief Subscribes to the given topics using the provided subscriber object.
 * @param subscriber A pointer to the subscriber object.
 * @param topics A QStringList containing the topics to subscribe to.
 */
void MainWindow::subscribeMessage(samples::pubsub::Subscriber* subscriber, const QStringList& topics)
{
    subscriber->startAction(topics);
}


/**
 * @brief Subscribes to a new topic and starts the subscriber action.
 * @param subscriber A pointer to the subscriber object.
 * @return void
 * @note Both of this slot function and on_buttonAddTopic_clicked() slot function
 *       are connected with the same signal, so they will be called with the order of connection.
 */
void MainWindow::subscribeMessage(samples::pubsub::Subscriber* subscriber)
{
    QMutexLocker locker(&slotMutex);

    if (!subscribeFlag)
        return;

    QString newTopic = ui->lineEditSubscribeTopic->text();

    QStringList topics;
    topics.append(newTopic);

    subscriber->startAction(topics);
    qDebug() << "Subscribe topic: " << newTopic;
    ui->statusBar->showMessage(tr("Topic added"));
}


/**
 * @brief Adds a new topic to the table view.
 * @param None
 * @return None
 * @note Both of this slot function and subscribeMessage() slot function
 *       are connected with the same signal, so they will be called with the order of connection.
 */
void MainWindow::on_buttonAddTopic_clicked()
{
    QMutexLocker locker(&slotMutex);

    // Get the model from the table view
    QStandardItemModel *itemModel = qobject_cast<QStandardItemModel*>(ui->tableViewTopics->model());

    // Get the new topic from the line edit
    QString newTopic = ui->lineEditSubscribeTopic->text();
    if (!isValidString(newTopic))
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid topic"));
        ui->statusBar->showMessage(tr("Please enter a valid topic"));
        subscribeFlag = false;
        return;
    }

    // Check for duplicates
    for (int i = 0; i < itemModel->rowCount(); ++i) 
    {
        // Topic is in column 1
        QString existingTopic = itemModel->item(i, 1)->text();
        if (existingTopic == newTopic) 
        {
            QMessageBox::warning(this, tr("Duplicate Topic"), tr("The topic already exists in the table!"));
            subscribeFlag = false;
            return;
        }
    }

    QList<QStandardItem *> items;
    QStandardItem *checkboxItem = new QStandardItem();
    checkboxItem->setCheckable(true);
    checkboxItem->setCheckState(Qt::Checked);
    items.append(checkboxItem);
    items.append(new QStandardItem(newTopic));

    itemModel->appendRow(items);
    ui->tableViewTopics->setModel(itemModel);

    subscribeFlag = true;
}


/**
 * @brief Unsubscribes a message from a subscriber.
 * @param subscriber A pointer to the subscriber object.
 * @return void
 * @note Both of this slot function and on_buttonRemoveTopic_clicked() slot function
 *       are connected with the same signal, so they will be called with the order of connection.
 */
void MainWindow::unsubscribeMessage(samples::pubsub::Subscriber* subscriber)
{
    QMutexLocker locker(&slotMutex);

    QString newTopic = ui->lineEditSubscribeTopic->text();

    QStringList topics;
    topics.append(newTopic);

    subscriber->stopAction(topics);
    qDebug() << "Unsubscribe topic: " << newTopic;
    ui->statusBar->showMessage(tr("Topic removed"));
}


/**
 * @brief Removes a topic from the table view.
 * @param None
 * @return None
 * @note Both of this slot function and unsubscribeMessage() slot function
 *       are connected with the same signal, so they will be called with the order of connection.
 */
void MainWindow::on_buttonRemoveTopic_clicked()
{
    QMutexLocker locker(&slotMutex);

    // Remove topic from table
    QStandardItemModel *itemModel = qobject_cast<QStandardItemModel*>(ui->tableViewTopics->model());
    
    // Get the current index from the table view
    QModelIndex index = ui->tableViewTopics->currentIndex();

    // Grant the second string item from the selected row
    QString newTopic = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();

    ui->lineEditSubscribeTopic->setText(newTopic);

    // Remove the row from the model
    itemModel->removeRow(index.row());
}


/**
 * @brief Enables the start and default buttons, and the host and port input fields, and shows a message in the status bar.
 * @param None
 * @return None
 */
void MainWindow::messageFinished()
{
    ui->buttonStop->setEnabled(false);
    ui->buttonStart->setEnabled(true);
    ui->buttonDefault->setEnabled(true);
    ui->lineEditHost->setEnabled(true);
    ui->spinBoxPortPublish->setEnabled(true);
    ui->spinBoxPortSubscribe->setEnabled(true);
    ui->statusBar->showMessage(tr("Message Finished"));
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

    QStringList localBuffer;
    localBuffer.append("Publish:");
    localBuffer.append("TimeStamp: ");
    localBuffer.append(timeStamp);
    localBuffer.append("Topic and Message: ");
    foreach(const QByteArray& message, messageList)
    {
        localBuffer.append(QString::fromUtf8(message));
    }
    localBuffer.append("\n");

    QMutexLocker locker(&bufferedSendMessagesMutex);
    bufferedSendMessages.append(localBuffer.join("\n"));
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

    QStringList localBuffer;
    localBuffer.append("Subscribe:");
    localBuffer.append("TimeStamp: ");
    localBuffer.append(timeStamp);
    localBuffer.append("Topic and Message: ");

    // TODO: Filter out the topic from the message list
    foreach(const QByteArray& message, messageList)
    {
        localBuffer.append(QString::fromUtf8(message));
    }
    localBuffer.append("\n");

    QMutexLocker locker(&bufferedReceiveMessagesMutex);
    bufferedReceiveMessages.append(localBuffer);
}


/**
 * @brief Updates the text edit with the buffered messages.
 * @param None
 * @return None
 */
void MainWindow::updateTextEdit()
{
    QMutexLocker locker(&bufferedSendMessagesMutex);
    if (!bufferedSendMessages.isEmpty())
    {
        ui->textView->append(bufferedSendMessages.join("\n"));
        bufferedSendMessages.clear();
    }

    QMutexLocker locker2(&bufferedReceiveMessagesMutex);
    if (!bufferedReceiveMessages.isEmpty())
    {
        ui->textView->append(bufferedReceiveMessages.join("\n"));
        bufferedReceiveMessages.clear();
    }
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
 * @brief This function is called when the user clicks on the "Start" button. It retrieves the IP address, port, and topic from the UI, validates them, and then calls the subscribe function to start the subscription.
 * @param None
 * @return None
 */
void MainWindow::on_buttonStart_clicked()
{
    int updateFrequency = ui->spinBoxFrequency->value();
    if (updateFrequency < 1 || updateFrequency > 200)
    {
        QMessageBox::critical(this, tr("Error"), tr("Please enter a valid update frequency"));
        return;
    }

    // Starts the timer to update every 100ms
    updateTimer->start((int)(1000 / updateFrequency));

    QString ipAddress = ui->lineEditHost->text();
    int port = ui->spinBoxPortSubscribe->value();

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

    if (ui->hexDisplay->isChecked())
        useHex = true;
    else
        useHex = false;

    ui->buttonSend->setEnabled(true);
    ui->buttonStart->setEnabled(false);
    ui->buttonStop->setEnabled(true);
    ui->buttonDefault->setEnabled(false);
    ui->lcdNumberSubscribe->display(0);
    ui->lcdNumberPublish->display(0);

    ui->lineEditHost->setEnabled(false);
    ui->spinBoxPortPublish->setEnabled(false);
    ui->spinBoxPortSubscribe->setEnabled(false);

    ui->statusBar->showMessage(tr("Started ..."));

    subscribeInit(ipAddress, port, useHex);
    publishInit(ipAddress, port, useHex);
}


/**
 * @brief Clears all input fields in the publish section of the UI.
 * @param None
 * @return None
 */
void MainWindow::on_buttonPublishClearAll_clicked()
{
    ui->textView->clear();
    ui->lineEditPublishTopic->clear();
    ui->lineEditPublishMessage->clear();
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
        ui->publishFrequency->setEnabled(true);
    }
    else
    {
        ui->publishFrequency->setEnabled(false);
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

// Copyright (C) 2023 The JOUAV Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QDateTime>
#include <QStandardItemModel>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>
#include "SampleBase.hpp"
#include "aboutdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief Show message on the text browser.
     * @param None
     * @return None
     */
    void showMessage(const QString &msg);

protected:
    /**
     * @brief Creates a new execution thread for the given sample.
     * @param sample The sample to run in the new thread.
     * @return A pointer to the new thread.
     */
    QThread* makeExecutionThread(nzmqt::samples::SampleBase& sample) const;

private:
    /*
    Publisher Context:
    The publisher context encompasses creating and handling the publisher side of a publish-subscribe pattern using the nzmqt library. The details of the context are as follows:
    ZMQContext: This is the core part of the ZMQ (ZeroMQ) communication. It represents the context in which the sockets run. It must be created first and is used for the entire lifetime of your application.
    Publisher Creation: A publisher is created within the context, and signals are connected to track the status of messages sent, any failures, and whether the publishing has finished.
    Execution Thread: The publisher's functionality is set to run in a separate thread, providing asynchronous operation.
    Test Start: The context is started, followed by the publisher thread. A QTimer is used to stop the publisher after 6 seconds.
    Postconditions Check: After the test is finished, conditions are checked to verify the behavior of the publisher (e.g., number of pings sent, failures, etc.).
    */
    void publish(QString ipAddress, int port, QString topic, QString message, int frequency, int maxPublishTime,bool useHex = false);

    /*
    Subscriber Context:
    The subscriber context refers to creating and handling the subscriber side of a publish-subscribe pattern. It is symmetrical to the publisher context with some details:
    ZMQContext: Just like the publisher, the subscriber needs a ZMQ context for its operation.
    Subscriber Creation: A subscriber is created within the same context, listening to the same endpoint that the publisher is using to publish the messages.
    Execution Thread: Similar to the publisher, the subscriber's functionality runs in a separate thread.
    Test Start: The subscriber thread starts with a slight delay (500 ms) and is stopped after 6 seconds, similar to the publisher.
    Postconditions Check: After the test is finished, conditions are checked to verify the behavior of the subscriber (e.g., number of pings received, failures, etc.).
    */
    void subscribe(QString ipAddress, int port, QString topic, int maxSubscribeTime, bool useHex = false);

private slots:

    void messagePublishFinished();

    void messageSubscribeFinished();

    void handleLogMessage(int type, const QString& message);

    /**
     * @brief Appends the given timestamp and message list to the publish text view.
     * @param timeStamp The timestamp to be appended to the text view.
     * @param messageList The list of messages to be appended to the text view.
     * @return None
     */
    void messageSent(const QString& timeStamp, const QList<QByteArray>& messageList);

    /**
     * @brief This function is called when a message is received. It appends the timestamp and message to the subscribe text view.
     * @param timeStamp The timestamp of the received message.
     * @param messageList The list of messages received.
     * @return None
     */
    void messageReceived(const QString& timeStamp, const QList<QByteArray>& messageList);

    /**
     * @brief Resets the values of the host and ports to their default values.
     * @param None
     * @return None
     */
    void on_buttonDefault_clicked();

    void on_buttonQuit_clicked();

    /**
     * @brief Handles the click event of the "Publish Start" button.
     *        It retrieves the IP address, port, topic, message, and frequency from the UI elements.
     *        Validates the IP address, port, topic, and message.
     *        Calls the publish function with the retrieved parameters.
     *        Disables the "Publish Start" button and enables the "Publish Stop" button.
     * @param None
     * @return None
     */
    void on_buttonPublishStart_clicked();

    /**
     * @brief This function is called when the user clicks on the "Subscribe Start" button. It retrieves the IP address, port, and topic from the UI, validates them, and then calls the subscribe function to start the subscription.
     * @param None
     * @return None
     */
    void on_buttonSubscribeStart_clicked();

    /**
     * @brief Stops the publisher thread and disables the "Stop" button while enabling the "Start" button.
     * @param None
     * @return None
     */
    void on_buttonPublishStop_clicked();

    /**
     * @brief Slot function for stopping the subscriber thread.
     * @param None
     * @return None
     */
    void on_buttonSubscribeStop_clicked();

    void on_buttonPublishClearAll_clicked();

    void on_buttonSubscribeClearAll_clicked();

    /**
     * @brief Slot function that is called when the state of the "Loop" checkbox is changed.
     *        Enables or disables the "Frequency" spin box based on the state of the checkbox.
     * @param arg1 The new state of the checkbox (Qt::Checked or Qt::Unchecked).
     * @return void
     */
    void on_checkBoxLoop_stateChanged(int arg1);

    /**
     * @brief Overrides the changeEvent function to handle language change events.
     * @param e A pointer to the QEvent object.
     * @return void
     * @note This function is called whenever a language change event occurs. It retranslated the UI to the new language.
     */
    void changeEvent(QEvent *e);

    /**
     * @brief Displays the about dialog in the center of the main window.
     * @param None
     * @return None
     */
    void on_actionAbout_triggered();

    void on_actionQuit_triggered();

private:
    Ui::MainWindow *ui;
    // QThread* subscriberThread = nullptr;
    // QThread* publisherThread = nullptr;
    AboutDialog aboutdlg;

    /**
     * @brief Checks if the given string contains only alphanumeric characters.
     * @param str The string to be checked.
     * @return True if the string contains only alphanumeric characters, false otherwise.
     */
    bool isValidString(const QString &str);

    /**
     * @brief Checks if a given string is a valid IPv4 address.
     *
     * @param ip The string to be checked.
     * @return True if the string is a valid IPv4 address, false otherwise.
     */
    bool isValidIPv4(const QString &ip);
};

#endif // MAINWINDOW_H

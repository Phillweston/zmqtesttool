// Copyright (C) 2023 The JOUAV Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui 
{
    class AboutDialog;
}

class AboutDialog : public QDialog 
{
    Q_OBJECT
public:
    AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

protected:
    /**
     * @brief Overrides the changeEvent function to handle language change events.
     * @param e A pointer to the QEvent object.
     * @return void
     * @note This function is called whenever a language change event occurs. It retranslates the UI to the new language.
     */
    void changeEvent(QEvent *e);

private:
    Ui::AboutDialog *ui;

private slots:
    /**
     * @brief Exit the program.
     * @param None
     * @return None
     */
    void on_closeBtn_clicked();

};

#endif // ABOUTDIALOG_H

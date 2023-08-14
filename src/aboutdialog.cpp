// Copyright (C) 2023 The JOUAV Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

/**
 * @brief Overrides the changeEvent function to handle language change events.
 * @param e A pointer to the QEvent object.
 * @return void
 * @note This function is called whenever a language change event occurs. It retranslated the UI to the new language.
 */
void AboutDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
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
 * @brief Exit the program.
 * @param None
 * @return None
 */
void AboutDialog::on_closeBtn_clicked()
{
    this->close();
}


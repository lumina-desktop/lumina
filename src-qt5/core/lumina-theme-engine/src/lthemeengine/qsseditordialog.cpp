/*
 * Copyright (c) 2014-2017, Ilya Kotov <forkotov02@hotmail.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QFile>
#include <QSettings>
#include "qt5ct.h"
#include "qsseditordialog.h"
#include "ui_qsseditordialog.h"

QSSEditorDialog::QSSEditorDialog(const QString &filePath, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::QSSEditorDialog)
{
    m_ui->setupUi(this);
    m_filePath = filePath;

    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    m_ui->textEdit->setPlainText(QString::fromUtf8(file.readAll()));
    setWindowTitle(tr("%1 - Style Sheet Editor").arg(file.fileName()));

    QSettings settings(Qt5CT::configFile(), QSettings::IniFormat);
    restoreGeometry(settings.value("QSSEditor/geometry").toByteArray());
}

QSSEditorDialog::~QSSEditorDialog()
{
    delete m_ui;
}

void QSSEditorDialog::save()
{
    QFile file(m_filePath);
    file.open(QIODevice::WriteOnly);
    file.write(m_ui->textEdit->toPlainText().toUtf8());
}

void QSSEditorDialog::hideEvent(QHideEvent *)
{
    QSettings settings(Qt5CT::configFile(), QSettings::IniFormat);
    settings.setValue("QSSEditor/geometry", saveGeometry());
}

void QSSEditorDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton id = m_ui->buttonBox->standardButton(button);
    if(id == QDialogButtonBox::Ok)
    {
        save();
        accept();
    }
    else if(id == QDialogButtonBox::Save)
    {
        save();
    }
    else
    {
        reject();
    }
}

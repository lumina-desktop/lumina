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

#ifndef PALETTEEDITDIALOG_H
#define PALETTEEDITDIALOG_H

#include <QDialog>
#include <QPalette>

class QTableWidgetItem;
class QStyle;

namespace Ui {
class PaletteEditDialog;
}

class PaletteEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaletteEditDialog(const QPalette &palette, QStyle *currentStyle, QWidget *parent = 0);
    ~PaletteEditDialog();

    QPalette selectedPalette() const;

signals:
    void paletteChanged(const QPalette &p);

private slots:
    void on_tableWidget_itemClicked(QTableWidgetItem *item);
    void on_resetPaletteButton_clicked();
    void on_buildInactiveButton_clicked();
    void on_buildDisabledButton_clicked();

private:
    void setPalette(const QPalette &palette);
    void hideEvent(QHideEvent *);
    Ui::PaletteEditDialog *m_ui;
    QStyle *m_currentStyle;
};

#endif // PALETTEEDITDIALOG_H

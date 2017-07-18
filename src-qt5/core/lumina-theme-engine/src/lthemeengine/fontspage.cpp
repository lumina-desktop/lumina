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

#include <QSignalMapper>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QFontDialog>
#include <QDir>
#include <QFile>
#include "qt5ct.h"
#include "fontspage.h"
#include "fontconfigdialog.h"
#include "ui_fontspage.h"

FontsPage::FontsPage(QWidget *parent) :
    TabPage(parent),
    m_ui(new Ui::FontsPage)
{
    m_ui->setupUi(this);

    QSignalMapper *mapper = new QSignalMapper(this);
    mapper->setMapping(m_ui->changeGeneralFontButton, m_ui->generalFontLabel);
    mapper->setMapping(m_ui->changeFixedWidthFontButton, m_ui->fixedFontLabel);
    connect(m_ui->changeGeneralFontButton, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(m_ui->changeFixedWidthFontButton, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(mapper, SIGNAL(mapped(QWidget*)), SLOT(onFontChangeRequested(QWidget*)));

    readSettings();

    //icons
    m_ui->createFontsConfButton->setIcon(QIcon::fromTheme("document-new"));
    m_ui->removeFontsConfButton->setIcon(QIcon::fromTheme("edit-delete"));
}

FontsPage::~FontsPage()
{
    delete m_ui;
}

void FontsPage::writeSettings()
{
    QSettings settings(Qt5CT::configFile(), QSettings::IniFormat);
    settings.beginGroup("Fonts");
    settings.setValue("general", m_ui->generalFontLabel->font());
    settings.setValue("fixed", m_ui->fixedFontLabel->font());
    settings.endGroup();
}

void FontsPage::onFontChangeRequested(QWidget *widget)
{
    bool ok = false;
    QFont font = QFontDialog::getFont (&ok, widget->font(), this);
    if(ok)
    {
        widget->setFont(font);
        qobject_cast<QLabel*>(widget)->setText(font.family () + " " + QString::number(font.pointSize ()));
    }
}

void FontsPage::readSettings()
{
    QSettings settings(Qt5CT::configFile(), QSettings::IniFormat);
    settings.beginGroup("Fonts");
    loadFont(&settings, m_ui->generalFontLabel, "general");
    loadFont(&settings, m_ui->fixedFontLabel, "fixed");
    settings.endGroup();
}

void FontsPage::loadFont(QSettings *settings, QLabel *label, const QString &key)
{
    QFont font = settings->value(key, QApplication::font()).value<QFont>();
    label->setText(font.family () + " " + QString::number(font.pointSize ()));
    label->setFont(font);
}

void FontsPage::on_createFontsConfButton_clicked()
{
    FontConfigDialog d(this);
    d.exec();
}

void FontsPage::on_removeFontsConfButton_clicked()
{
    QString path = QDir::homePath() + "/.config/fontconfig/fonts.conf";


    if(QFile::exists(path))
    {
        if(QMessageBox::question(this, tr("Remove Font Configuration"),
                                 tr("Are you sure you want to delete <i>%1</i>?").arg(path),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            return;
        }

        QFile::remove(path + ".back");
        QFile::copy(path, path + ".back");
        QFile::remove(path);
    }
}

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

#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "fontconfigdialog.h"
#include "ui_fontconfigdialog.h"

FontConfigDialog::FontConfigDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::FontConfigDialog)
{
    m_ui->setupUi(this);

    m_ui->hintingStyleComboBox->addItem(tr("None"), "hintnone");
    m_ui->hintingStyleComboBox->addItem(tr("Slight"), "hintslight");
    m_ui->hintingStyleComboBox->addItem(tr("Medium"), "hintmedium");
    m_ui->hintingStyleComboBox->addItem(tr("Full"), "hintfull");

    m_ui->rgbaComboBox->addItem(tr("None"), "none");
    m_ui->rgbaComboBox->addItem("rgb", "rgb");
    m_ui->rgbaComboBox->addItem("bgr", "bgr");
    m_ui->rgbaComboBox->addItem("vrgb", "vrgb");
    m_ui->rgbaComboBox->addItem("vbgr", "vbgr");

    m_ui->lcdFilterComboBox->addItem("lcdnone");
    m_ui->lcdFilterComboBox->addItem("lcddefault");
    m_ui->lcdFilterComboBox->addItem("lcdlight");
    m_ui->lcdFilterComboBox->addItem("lcdlegacy");
}

FontConfigDialog::~FontConfigDialog()
{
    delete m_ui;
}

void FontConfigDialog::accept()
{
    QDir::home().mkpath(".config/fontconfig/");
    QString path = QDir::homePath() + "/.config/fontconfig/fonts.conf";
    qDebug("FontConfigDialog: fontconfig path: %s", qPrintable(path));


    if(QFile::exists(path))
    {
        if(QMessageBox::question(this, tr("Font Configuration"),
                                 tr("<i>%1</i> already exists. Do you want to replace it?").arg(path),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            QDialog::reject();
            return;
        }

        QFile::remove(path + ".back");
        QFile::copy(path, path + ".back");
    }

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
    {
        qWarning("FontConfigDialog: unable to open file: %s", qPrintable(file.errorString()));
        return;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);

    stream.writeStartDocument();
    stream.writeDTD("<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">");
    stream.writeStartElement("fontconfig");

    stream.writeStartElement("match");
    stream.writeAttribute("target", "font");
    writeOption(&stream, "antialias", "bool", m_ui->antialisingCheckBox->isChecked() ? "true" : "false");
    writeOption(&stream, "hinting", "bool", m_ui->hintingCheckBox->isChecked() ? "true" : "false");
    writeOption(&stream, "hintstyle", "const", m_ui->hintingStyleComboBox->currentData().toString());
    writeOption(&stream, "rgba", "const", m_ui->rgbaComboBox->currentData().toString());
    writeOption(&stream, "autohint", "bool", m_ui->autohinterCheckBox->isChecked() ? "true" : "false");
    writeOption(&stream, "lcdfilter", "const", m_ui->lcdFilterComboBox->currentText());
    writeOption(&stream, "dpi", "double", QString::number(m_ui->dpiSpinBox->value()));
    stream.writeEndElement();

    if(m_ui->disableBoldAutohintCheckBox->isChecked())
    {
        stream.writeStartElement("match");
        stream.writeAttribute("target", "font");

        stream.writeStartElement("test");
        stream.writeAttribute("name", "weight");
        stream.writeAttribute("compare", "more");
        stream.writeTextElement("const", "medium");
        stream.writeEndElement();

        writeOption(&stream, "autohint", "bool", m_ui->autohinterCheckBox->isChecked() ? "true" : "false");

        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();

    QDialog::accept();
}

void FontConfigDialog::writeOption(QXmlStreamWriter *stream, const QString &name, const QString &type, const QString &value)
{
    stream->writeStartElement("edit");
    stream->writeAttribute("name", name);
    stream->writeAttribute("mode", "assign");
    stream->writeTextElement(type, value);
    stream->writeEndElement();
}

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

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include "qt5ct.h"
#include <QTranslator>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTranslator translator;
    QString locale = Qt5CT::systemLanguageID();
    translator.load(QString(":/qt5ct_") + locale);
    app.installTranslator(&translator);

    QTranslator qt_translator;
    qt_translator.load(QLibraryInfo::location (QLibraryInfo::TranslationsPath) + "/qtbase_" + locale);
    app.installTranslator(&qt_translator);

    qDebug("Configuration path: %s", qPrintable(Qt5CT::configPath()));
    qDebug("Shared QSS path: %s", qPrintable(Qt5CT::sharedStyleSheetPath()));

    //checking environment
    QStringList errorMessages;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    if(env.contains("QT_STYLE_OVERRIDE"))
    {
        errorMessages << app.translate("main", "Please remove the <b>QT_STYLE_OVERRIDE</b> environment variable");
    }

    if(env.value("QT_QPA_PLATFORMTHEME") != "qt5ct")
    {
        errorMessages << app.translate("main", "The <b>QT_QPA_PLATFORMTHEME</b> environment "
                                               "variable is not set correctly");
    }

    if(!QStyleFactory::keys().contains("qt5ct-style"))
    {
        errorMessages << app.translate("main", "Unable to find <b>libqt5ct-style.so</b>");
    }

    if(!errorMessages.isEmpty())
    {
        QMessageBox::critical(0, app.translate("main", "Error"), errorMessages.join("<br><br>"));
        return 0;
    }

    MainWindow w;
    w.show();

    return app.exec();
}

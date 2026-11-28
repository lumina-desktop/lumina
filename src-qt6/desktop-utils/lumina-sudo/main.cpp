//===========================================
//  Originally written as qsudo by the TrueOS Project
//  Copyright (c) 2016, TrueOS
//  Relicensed with contributor consent from BSD-2 to BSD-3
//  See the LICENSE file for full details
//===========================================
#include <qtranslator.h>
#include <qlocale.h>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <unistd.h>
#include <sys/types.h>
#include "mainwindow.h"

#ifndef PREFIX
#define PREFIX QString("/usr/local")
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if ( argc <= 1 ) {
       qDebug() << "Error: Need to specify argument to run!";
       exit(1);
    }
    //NOTE: Whether a password is needed is checked in MainWindow::ProgramInit() with "sudo -n -v",
    // which only validates credentials. Do NOT probe with "sudo -n <command>": that runs the real
    // command (and could run it a second time below if it exits non-zero).

    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();
    if ( ! QFile::exists(PREFIX + "/share/lumina-desktop/i18n/lum_sudo_" + langCode + ".qm" ) )
        langCode.truncate(langCode.indexOf("_"));

    if ( QFile::exists(PREFIX + "/share/lumina-desktop/i18n/lum_sudo_" + langCode + ".qm" ) ) {
      translator.load( QString("lum_sudo_") + langCode, PREFIX + "/share/lumina-desktop/i18n/" );
      a.installTranslator( &translator );
    }
    QTextCodec::setCodecForLocale( QTextCodec::codecForName("UTF-8") ); //Force Utf-8 compliance

    MainWindow w;
    if( w.ProgramInit() ){ w.show(); } //false: command already running with cached credentials - no password prompt

    return a.exec();
}

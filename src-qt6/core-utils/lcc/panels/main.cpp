//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTranslator>
#include <QLocale>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <LuminaOS.h>
#include <LUtils.h>
#include <LuminaThemes.h>
#include <LuminaSingleApplication.h>

#include "MainWindow.h"

int main(int argc, char ** argv)
{
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lcc-panels");
    if( !a.isPrimaryProcess() ){ return 0; } //Inputs forwarded on to the primary already
    
    LuminaThemeEngine themes(&a);

    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Lumina Control Center - Panel Configuration");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add screen option for multi-monitor support
    QCommandLineOption screenOption(QStringList() << "s" << "screen",
                                   "Screen number to configure (0-based index)",
                                   "number", "0");
    parser.addOption(screenOption);
    
    parser.process(a);
    
    // Parse screen number
    int screen = 0;
    if (parser.isSet(screenOption)) {
        bool ok;
        screen = parser.value(screenOption).toInt(&ok);
        if (!ok || screen < 0) {
            QMessageBox::critical(nullptr, "Error", "Invalid screen number specified");
            return 1;
        }
        
        // Validate screen number
        if (screen >= QGuiApplication::screens().count()) {
            QMessageBox::critical(nullptr, "Error", 
                QString("Screen %1 not found. Available screens: 0-%2")
                .arg(screen).arg(QGuiApplication::screens().count()-1));
            return 1;
        }
    }

    MainWindow w(screen);
    w.show();

    return a.exec();
}

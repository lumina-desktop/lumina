//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTranslator>
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
    LSingleApplication a(argc, argv, "lcc-mouse");
    if( !a.isPrimaryProcess() ){ return 0; } //Inputs forwarded on to the primary already
    LuminaThemeEngine themes(&a);

    //Setup the command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Lumina Control Center: Mouse Settings");
    parser.addHelpOption();
    parser.addVersionOption();
    
    //Process the command line arguments
    parser.process(a);

    //Load the translator
    QTranslator translator;
    if(QLocale::system().name() != "C"){
        if(translator.load("lcc-mouse_" + QLocale::system().name(), QCoreApplication::applicationDirPath() + "/../share/lumina-desktop/i18n/")){
            a.installTranslator(&translator);
        } else if(translator.load("lcc-mouse_" + QLocale::system().name().section("_",0,0), QCoreApplication::applicationDirPath() + "/../share/lumina-desktop/i18n/")){
            a.installTranslator(&translator);
        } else if(translator.load("lcc-mouse_" + QLocale::system().name(), QCoreApplication::applicationDirPath() + "/i18n/")){
            a.installTranslator(&translator);
        } else if(translator.load("lcc-mouse_" + QLocale::system().name().section("_",0,0), QCoreApplication::applicationDirPath() + "/i18n/")){
            a.installTranslator(&translator);
        }
    }

    //Show the main window
    MainWindow w;
    w.show();

    return a.exec();
}

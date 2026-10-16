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
#include <LuminaXDG.h>

#include "MainWindow.h"

int main(int argc, char ** argv)
{
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lcc-defaultapps");
    if( !a.isPrimaryProcess() ){ return 0; } //Inputs forwarded on to the primary already
    LuminaThemeEngine themes(&a);

    //Setup the command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription( QString(QObject::tr("Lumina Desktop Default Applications Configuration Tool")) );
    parser.addHelpOption();
    parser.addVersionOption();
    
    //Parse the current command line arguments given
    parser.process(a);
    
    //Load the translator
    QTranslator translator;
    if(QLocale::system().name() != "C"){
        if(translator.load( QLocale::system(), "lumina-config", "_", LOS::LuminaShare()+"/i18n/", ".qm")){
            a.installTranslator(&translator);
        }else if(translator.load( QLocale::system(), "lumina-config", "_", QApplication::applicationDirPath()+"/../share/lumina-desktop/i18n/", ".qm")){
            a.installTranslator(&translator);
        }else{
            qDebug() << "Could not load translations:" << QLocale::system().name() << QApplication::applicationDirPath()+"/../share/lumina-desktop/i18n/lumina-config_" + QLocale::system().name() + ".qm";
        }
    }

    //Show the main window
    MainWindow w;
    w.show();

    int retCode = a.exec();
    return retCode;
}

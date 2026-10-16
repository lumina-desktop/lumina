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
    LSingleApplication a(argc, argv, "lcc");
    if( !a.isPrimaryProcess() ){ return 0; } //Inputs forwarded on to the primary already
    LuminaThemeEngine themes(&a);
    
    //Setup the command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription( QString(QObject::tr("Lumina Control Center - Main Launcher")) );
    parser.addHelpOption();
    parser.addVersionOption();
    
    //Parse the command line
    parser.process(a);
    
    //Start the application
    MainWindow w;
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(slotSingleInstance()) );
    w.show();
    
    return a.exec();
}

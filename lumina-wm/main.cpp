//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QDebug>

#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>
#include <QUrl>


#include "WMSession.h"

#include <LuminaXDG.h> //from libLuminaUtils
#include <LuminaThemes.h>
#include <LuminaSingleApplication.h>

//#define DEBUG 0

int main(int argc, char ** argv)
{
    LSingleApplication a(argc, argv, "lumina-wm");
    if(!a.isPrimaryProcess()){ return 0; } //Inputs forwarded on to the primary already
    LuminaThemeEngine themes(&a);
    
    //Setup the special settings prefix location
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
    
    WMSession w;
    w.start();
    QObject::connect(themes, SIGNAL(updateIcons()), &w, SLOT(reloadIcons()) );
    QObject::connect(a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(newInputsAvailable(QStringList)) );
    int retCode = a.exec();
    
    return retCode;
}

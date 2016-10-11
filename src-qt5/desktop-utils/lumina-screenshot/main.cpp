#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaUtils.h>
#include <LuminaSingleApplication.h>

int main(int argc, char ** argv)
{
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "l-screenshot");
    LuminaThemeEngine theme(&a);
    a.setApplicationName("Take Screenshot");
    //LUtils::LoadTranslation(&a, "l-screenshot");

    MainUI w;
    QObject::connect(&theme,SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();

    return a.exec();
}

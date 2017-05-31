#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LUtils.h>
#include <LuminaSingleApplication.h>

int main(int argc, char ** argv)
{
    //Make sure Qt5 auto-scaling is disabled for this application (need exact pixel measurements)
    unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR");
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "l-screenshot");
    //LuminaThemeEngine theme(&a);
    a.setApplicationName("Take Screenshot");

    MainUI w;
    //QObject::connect(&theme,SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();

    return a.exec();
}

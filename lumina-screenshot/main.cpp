#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaUtils.h>

int main(int argc, char ** argv)
{
    LTHEME::LoadCustomEnvSettings();
    QApplication a(argc, argv);
    LuminaThemeEngine theme(&a);
    a.setApplicationName("Take Screenshot");
    LUtils::LoadTranslation(&a, "lumina-screenshot");

    MainUI w;
    QObject::connect(&theme,SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();

    return a.exec();
}

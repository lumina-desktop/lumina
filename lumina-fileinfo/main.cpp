#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "dialog.h"
#include <LuminaUtils.h>
#include <LuminaThemes.h>

int main(int argc, char ** argv)
{
    QApplication a(argc, argv);
    LUtils::LoadTranslation(&a, "lumina-fileinfo");
    LuminaThemeEngine theme(&a);

    Dialog w;
    QObject::connect(&theme, SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    if (argc==2) {
        w.LoadDesktopFile(QString(argv[1]).simplified());
    } else if (argc==3) {
        w.Initialise(QString(argv[1]).simplified());
        w.LoadDesktopFile(QString(argv[2]).simplified());
    } else {
        w.MissingInputs();
    }
    w.show();

    int retCode = a.exec();
    return retCode;
}
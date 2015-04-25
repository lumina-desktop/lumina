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
    //qDebug() << "Init App...";
    QApplication a(argc, argv);
    //qDebug() << "Init Theme Engine...";
    LuminaThemeEngine theme(&a);
    //qDebug() << "Load Translations...";
    a.setApplicationName("Search for...");
    LUtils::LoadTranslation(&a, "lumina-search");

    MainUI w;
    QObject::connect(&theme,SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    if (argc == 3) 
		w.initialise(QString(argv[1]).simplified(), QString(argv[2]).simplified());
    if (argc == 4) 
		w.initialise(QString(argv[1]).simplified(), QString(argv[2]).simplified(), QString(argv[3]).simplified());
	
		
    w.show();

    return a.exec();
}

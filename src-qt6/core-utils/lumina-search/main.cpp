#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QFile>

#include "MainUI.h"
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LUtils.h>

//==== INPUT FORMAT ====
//  lumina-search [-no-excludes] [-dir [directory]] [-search <term>]
//    -no-excludes: Don't exclude anything from this search
//    -dir [directory]: Setup a file/dir search within the current working dir (or specified dir)
//    -search <term>: Start a search with the given term
//===================
int main(int argc, char ** argv)
{
    //qDebug() << "Init App...";
    LTHEME::LoadCustomEnvSettings();
    QApplication a(argc, argv);
    //qDebug() << "Init Theme Engine...";
    //LuminaThemeEngine theme(&a);
    //qDebug() << "Load Translations...";
    a.setApplicationName("Search for...");
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);   
    LUtils::LoadTranslation(&a, "lumina-search");

    MainUI w;
    //QObject::connect(&theme,SIGNAL(updateIcons()), &w, SLOT(setupIcons()) );
    w.show();
    if(argc>1){
      bool startsearch = false;
      for(int i=1; i<argc; i++){
	QString val = QString(argv[i]).toLower();
        if( val == "-no-excludes"){ w.disableExcludes(); }
	else if(val=="-dir"){
	  //Check the next input value for a directory path
	  QString dir;
	  if( argc>(i+1) ){ dir = QString(argv[i+1]); }
	  //Check the input for validity
	  if(dir.startsWith("-")){dir.clear(); } //not a directory, just another input
          else if(!dir.startsWith("/")){ //not an absolute path
	    dir = QDir::currentPath()+"/"+dir;
	    QFileInfo info(dir);
            if( !info.exists() || !info.isDir() ){ dir.clear(); } //invalid relative dir
	  }
		  
	  if(dir.isEmpty()){ dir= QDir::currentPath(); } //use the current directory
	  else{ i++; } //using the next input value - skip it for the next iteration
	  w.setSearchDirectory( dir );

	}else if(val=="-search"){
	  //Check the next input value for a  text string
	  QString text;
	  if( argc>(i+1) ){ text = QString(argv[i+1]); }
	  //Check the input for validity
	  if( !text.isEmpty() ){
	    i++; //using the next input value - skip it for the next iteration
	    w.setSearchTerm( text );	
	    startsearch = true;
	  }
	}
      }//end loop over inputs
      if(startsearch){
	//A CLI search was requested, go ahead and start it now
        QTimer::singleShot(10, &w, SLOT(startSearch()));
      }
    } //end check for input arguments

    return a.exec();
}

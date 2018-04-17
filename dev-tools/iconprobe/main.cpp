#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QIcon>

QString findInDir(QString dir, QString file){
  QDir _dir(dir);
  QStringList files = _dir.entryList(QStringList() << file+".*", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  if(files.isEmpty()){
    QStringList dirs = _dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QString tmp;
    for(int i=0; i<dirs.length() && tmp.isEmpty(); i++){
      tmp = findInDir( _dir.absoluteFilePath(dirs[i]), file);
    }
    return tmp;
  }else{
    return _dir.absoluteFilePath(files.first());
  }
}

QStringList themeInherits(QString dir){
  QFile file(dir+"/index.theme");
  QStringList list;
  if( file.open(QIODevice::Text | QIODevice::ReadOnly) ){
    QTextStream in(&file);
    while(!in.atEnd()){
      QString line = in.readLine();
      if(line.startsWith("Inherits=")){
        //qDebug() << "Got Inherit Line" << line;
        list = line.section("=",1,-1).split(";");
        break; //done now
      }
    }
    file.close();
  }
  return list;
}

bool isIconTheme(QString dir){
  if(!QFile::exists(dir+"/index.theme")){ return false; }
  QDir base(dir);
  QStringList dirs = base.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  dirs.removeAll("cursors");
  return (!dirs.isEmpty());
}

int main(int argc, char ** argv)
{
   QApplication a(argc, argv);

   QString icondir="/usr/local/share/icons";
   QStringList iconfiles;
   if(argc<2){ iconfiles << "folder-downloads" << "start-here-lumina" << "firefox" << "utilities-terminal"; }
   else{ iconfiles = QString(argv[1]).split(" "); }

   QDir dir(icondir);
   QStringList themes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
   //Now look through them and see which themes have this file
   QIcon::setThemeSearchPaths( QStringList() << icondir );
   for(int i=0; i<themes.length(); i++){
     QString themepath = dir.absoluteFilePath(themes[i]);
     if( !isIconTheme(themepath) ){ continue; }
     qDebug() << "Testing Theme:" << themes[i];
     QIcon::setThemeName(themes[i]);
     QStringList inherits = themeInherits(themepath);
     for(int j=0; j<inherits.length(); j++){ inherits << themeInherits(dir.absoluteFilePath(inherits[j])); }
     qDebug() << " - Inherits From Themes (in order):" << inherits;
     for(int j=0; j<iconfiles.length(); j++){
       qDebug() << " -------------------------";
       qDebug() << " - Looking for icon:" << iconfiles[j];
       qDebug() << " - Found File:" << findInDir(themepath, iconfiles[j]);
       qDebug() << " - Has Theme Icon:" << QIcon::hasThemeIcon(iconfiles[j]);
       qDebug() << " - Found Icon:" << QIcon::fromTheme(iconfiles[j]).name();
    }
     qDebug() << " ================";
   }
   return 0;
}

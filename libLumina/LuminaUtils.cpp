//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaUtils.h"

#include <QString>
#include <QFile>
#include <QStringList>
#include <QObject>
#include <QTextCodec>
#include <QDebug>
#include <QDesktopWidget>
#include <QImageReader>
#include <QRegExp>
#include <QFuture>
#include <QtConcurrent>

#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaXDG.h>

static QStringList fav;

inline QStringList ProcessRun(QString cmd, QStringList args){
  //Assemble outputs
  QStringList out; out << "1" << ""; //error code, string output
  QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc.setProcessEnvironment(env);
  proc.setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc.start(cmd);
  }else{
    proc.start(cmd,args);	  
  }
  while(!proc.waitForFinished(500)){
    if(proc.state() == QProcess::NotRunning){ break; } //somehow missed the finished signal
  }
  out[0] = QString::number(proc.exitCode());
  out[1] = QString(proc.readAllStandardOutput());
  return out;	
}
//=============
//  LUtils Functions
//=============
QString LUtils::LuminaDesktopVersion(){ 
  QString ver = "0.9.0-devel";
  #ifdef GIT_VERSION
  ver.append( QString(" (Git Revision: %1)").arg(GIT_VERSION) );
  #endif
  return ver; 
}

QString LUtils::LuminaDesktopBuildDate(){
  #ifdef BUILD_DATE
  return BUILD_DATE;
  #endif
  return "";
}

int LUtils::runCmd(QString cmd, QStringList args){
  /*QProcess proc;
  proc.setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc.start(cmd);
  }else{
    proc.start(cmd, args);
  }
  //if(!proc.waitForStarted(30000)){ return 1; } //process never started - max wait of 30 seconds
  while(!proc.waitForFinished(300)){
    if(proc.state() == QProcess::NotRunning){ break; } //somehow missed the finished signal
    QCoreApplication::processEvents();
  }
  int ret = proc.exitCode();
  return ret;*/
  QFuture<QStringList> future = QtConcurrent::run(ProcessRun, cmd, args);
  return future.result()[0].toInt(); //turn it back into an integer return code
	
}

QStringList LUtils::getCmdOutput(QString cmd, QStringList args){
  /*QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc.setProcessEnvironment(env);
  proc.setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc.start(cmd);
  }else{
    proc.start(cmd,args);	  
  }
  //if(!proc.waitForStarted(30000)){ return QStringList(); } //process never started - max wait of 30 seconds
  while(!proc.waitForFinished(300)){
    if(proc.state() == QProcess::NotRunning){ break; } //somehow missed the finished signal
    QCoreApplication::processEvents();
  }
  QStringList out = QString(proc.readAllStandardOutput()).split("\n");
  return out;*/
  QFuture<QStringList> future = QtConcurrent::run(ProcessRun, cmd, args);
  return future.result()[1].split("\n"); //Split the return message into lines
}

QStringList LUtils::readFile(QString filepath){
  QStringList out;
  QFile file(filepath);
  if(file.open(QIODevice::Text | QIODevice::ReadOnly)){
    QTextStream in(&file);
    while(!in.atEnd()){
      out << in.readLine();
    }
    file.close();
  }
  return out;
}

bool LUtils::writeFile(QString filepath, QStringList contents, bool overwrite){
  QFile file(filepath);
  if(file.exists() && !overwrite){ return false; }
  bool ok = false;
  if(contents.isEmpty()){ contents << "\n"; }
  if( file.open(QIODevice::WriteOnly | QIODevice::Truncate) ){
    QTextStream out(&file);
    out << contents.join("\n");
    if(!contents.last().isEmpty()){ out << "\n"; } //always end with a new line
    file.close();
    ok = true;
  }
  return ok;
}

bool LUtils::isValidBinary(QString& bin){
  if(!bin.startsWith("/")){
    //Relative path: search for it on the current "PATH" settings
    QStringList paths = QString(qgetenv("PATH")).split(":");
    for(int i=0; i<paths.length(); i++){
      if(QFile::exists(paths[i]+"/"+bin)){ bin = paths[i]+"/"+bin; break;}	    
    }
  }
  //bin should be the full path by now
  if(!bin.startsWith("/")){ return false; }
  QFileInfo info(bin);
  bool good = (info.exists() && info.isExecutable());
  if(good){ bin = info.absoluteFilePath(); }
  return good;
}

QString LUtils::GenerateOpenTerminalExec(QString term, QString dirpath){
  //Check the input terminal application (default/fallback - determined by calling application)
  if(!LUtils::isValidBinary(term)){
    if(term.endsWith(".desktop")){
      //Pull the binary name out of the shortcut
      bool ok = false;
      XDGDesktop DF = LXDG::loadDesktopFile(term,ok);
      if(!ok){ term = "xterm"; }
      else{ term= DF.exec.section(" ",0,0); } //only take the binary name - not any other flags
    }else{
	term = "xterm"; //fallback
    }
  }
  //Now create the calling command for the designated terminal
  // NOTE: While the "-e" routine is supposed to be universal, many terminals do not properly use it
  //  so add some special/known terminals here as necessary
  QString exec;
  if(term=="mate-terminal" || term=="lxterminal" || term=="gnome-terminal"){
    exec = term+" --working-directory=\""+dirpath+"\"";
  }else if(term=="xfce4-terminal"){
    exec = term+" --default-working-directory=\""+dirpath+"\"";
  }else if(term=="konsole"){
    exec = term+" --workdir \""+dirpath+"\"";
  }else{
    //-e is the parameter for most of the terminal appliction to execute an external command. 
    //In this case we start a shell in the selected directory
      //Need the user's shell first
      QString shell = QString(getenv("SHELL"));
      if(!LUtils::isValidBinary(shell)){ shell = "/bin/sh"; } //universal fallback for a shell
    exec = term + " -e \"cd " + dirpath + " && " + shell + " \" ";
  }
  return exec;
}

QStringList LUtils::listSubDirectories(QString dir, bool recursive){
  //This is a recursive method for returning the full paths of all subdirectories (if recursive flag is enabled)
  QDir maindir(dir);
  QStringList out;
  QStringList subs = maindir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name);
  for(int i=0; i<subs.length(); i++){
    out << maindir.absoluteFilePath(subs[i]);
    if(recursive){
      out << LUtils::listSubDirectories(maindir.absoluteFilePath(subs[i]), recursive);
    }
  }
  return out;
}

QString LUtils::PathToAbsolute(QString path){
  //Convert an input path to an absolute path (this does not check existance ot anything)
  if(path.startsWith("/")){ return path; } //already an absolute path
  if(path.startsWith("~")){ path.replace(0,1,QDir::homePath()); }
  if(!path.startsWith("/")){
    //Must be a relative path
    if(path.startsWith("./")){ path = path.remove(2); }
    path.prepend( QDir::currentPath()+"/");
  }
  return path;
}

QStringList LUtils::imageExtensions(bool wildcards){
  //Note that all the image extensions are lowercase!!
  static QStringList imgExtensions;
  if(imgExtensions.isEmpty()){
    QList<QByteArray> fmt = QImageReader::supportedImageFormats();
    for(int i=0; i<fmt.length(); i++){ 
      if(wildcards){ imgExtensions << "*."+QString::fromLocal8Bit(fmt[i]);  }
      else{ imgExtensions << QString::fromLocal8Bit(fmt[i]); }
    }
  }
  return imgExtensions;
}

 QTranslator* LUtils::LoadTranslation(QApplication *app, QString appname, QString locale, QTranslator *cTrans){
   //Get the current localization
    QString langEnc = "UTF-8"; //default value
    QString langCode = locale; //provided locale
    if(langCode.isEmpty()){ langCode = getenv("LC_ALL"); }
    if(langCode.isEmpty()){ langCode = getenv("LANG"); }
    if(langCode.isEmpty()){ langCode = "en_US.UTF-8"; } //default to US english
    //See if the encoding is included and strip it out as necessary
    if(langCode.contains(".")){
      langEnc = langCode.section(".",-1);
      langCode = langCode.section(".",0,0);
    }
    //Now verify the encoding for the locale
    if(langCode =="C" || langCode=="POSIX" || langCode.isEmpty()){
      langEnc = "System"; //use the Qt system encoding
    }
    if(app !=0){
      qDebug() << "Loading Locale:" << appname << langCode << langEnc;
      //If an existing translator was provided, remove it first (will be replaced)
      if(cTrans!=0){ app->removeTranslator(cTrans); }
      //Setup the translator
      cTrans = new QTranslator();
      //Use the shortened locale code if specific code does not have a corresponding file
      if(!QFile::exists(LOS::LuminaShare()+"i18n/"+appname+"_" + langCode + ".qm") && langCode!="en_US" ){
        langCode.truncate( langCode.indexOf("_") );
      }
      if( cTrans->load( appname+QString("_") + langCode, LOS::LuminaShare()+"i18n/" ) ){
        app->installTranslator( cTrans );
      }else{
	//Translator could not be loaded for some reason
	cTrans = 0;
	if(langCode!="en_US"){
	  qWarning() << " - Could not load Locale:" << langCode;
	}
      }
    }else{
      //Only going to set the encoding since no application given
      qDebug() << "Loading System Encoding:" << langEnc;
    }
    //Load current encoding for this locale
    QTextCodec::setCodecForLocale( QTextCodec::codecForName(langEnc.toUtf8()) ); 
    return cTrans;
}

QStringList LUtils::knownLocales(){
  QDir i18n = QDir(LOS::LuminaShare()+"i18n");
    if( !i18n.exists() ){ return QStringList(); }
  QStringList files = i18n.entryList(QStringList() << "lumina-desktop_*.qm", QDir::Files, QDir::Name);
    if(files.isEmpty()){ return QStringList(); }
  //Now strip off the filename and just leave the locale tag
  for(int i=0; i<files.length(); i++){
     files[i].chop(3); //remove the ".qm" on the end
     files[i] = files[i].section("_",1,50).simplified();
  }
  files << "en_US"; //default locale
  files.sort();
  return files;
}

void LUtils::setLocaleEnv(QString lang, QString msg, QString time, QString num,QString money,QString collate, QString ctype){
  //Adjust the current locale environment variables
  bool all = false;
  if(msg.isEmpty() && time.isEmpty() && num.isEmpty() && money.isEmpty() && collate.isEmpty() && ctype.isEmpty() ){
    if(lang.isEmpty()){ return; } //nothing to do - no changes requested
    all = true; //set everything to the "lang" value
  }
  //If no lang given, but others are given, then use the current setting
  if(lang.isEmpty()){ lang = getenv("LC_ALL"); }
  if(lang.isEmpty()){ lang = getenv("LANG"); }
  if(lang.isEmpty()){ lang = "en_US"; }
  //Now go through and set/unset the environment variables
  // - LANG & LC_ALL
  if(!lang.contains(".")){ lang.append(".UTF-8"); }
  setenv("LANG",lang.toUtf8() ,1); //overwrite setting (this is always required as the fallback)
  if(all){ setenv("LC_ALL",lang.toUtf8() ,1); }
  else{ unsetenv("LC_ALL"); } //make sure the custom settings are used
  // - LC_MESSAGES
  if(msg.isEmpty()){ unsetenv("LC_MESSAGES"); }
  else{
    if(!msg.contains(".")){ msg.append(".UTF-8"); }
    setenv("LC_MESSAGES",msg.toUtf8(),1);
  }
  // - LC_TIME
  if(time.isEmpty()){ unsetenv("LC_TIME"); }
  else{
    if(!time.contains(".")){ time.append(".UTF-8"); }
    setenv("LC_TIME",time.toUtf8(),1);
  }
    // - LC_NUMERIC
  if(num.isEmpty()){ unsetenv("LC_NUMERIC"); }
  else{
    if(!num.contains(".")){ num.append(".UTF-8"); }
    setenv("LC_NUMERIC",num.toUtf8(),1);
  }
    // - LC_MONETARY
  if(money.isEmpty()){ unsetenv("LC_MONETARY"); }
  else{
    if(!money.contains(".")){ money.append(".UTF-8"); }
    setenv("LC_MONETARY",money.toUtf8(),1);
  }
    // - LC_COLLATE
  if(collate.isEmpty()){ unsetenv("LC_COLLATE"); }
  else{
    if(!collate.contains(".")){ collate.append(".UTF-8"); }
    setenv("LC_COLLATE",collate.toUtf8(),1);
  }
    // - LC_CTYPE
  if(ctype.isEmpty()){ unsetenv("LC_CTYPE"); }
  else{
    if(!ctype.contains(".")){ ctype.append(".UTF-8"); }
    setenv("LC_CTYPE",ctype.toUtf8(),1);
  }	
}

QString LUtils::currentLocale(){
  QString curr = getenv("LC_ALL");// = QLocale::system();
  if(curr.isEmpty()){ curr = getenv("LANG"); }
  if(curr.isEmpty()){ curr = "en_US"; }
  curr = curr.section(".",0,0); //remove any encodings off the end
  return curr;
}

double LUtils::DisplaySizeToBytes(QString num){
  //qDebug() << "Convert Num to Bytes:" << num;
  num = num.toLower().simplified();
  num = num.remove(" ");
  if(num.isEmpty()){ return 0.0; }
  if(num.endsWith("b")){ num.chop(1); } //remove the "bytes" marker (if there is one)
  QString lab = "b";
  if(!num[num.size()-1].isNumber()){
    lab = num.right(1); num.chop(1);
  }
  double N = num.toDouble();
  QStringList labs; labs <<"b"<<"k"<<"m"<<"g"<<"t"<<"p"; //go up to petabytes for now
  for(int i=0; i<labs.length(); i++){
    if(lab==labs[i]){ break; }//already at the right units - break out
    N = N*1024.0; //Move to the next unit of measurement
  }
  //qDebug() << " - Done:" << QString::number(N) << lab << num;
  return N;
}

QString LUtils::BytesToDisplaySize(qint64 ibytes){
  static QStringList labs = QStringList();
  if(labs.isEmpty()){ labs << "B" << "K" << "M" << "G" << "T" << "P"; }
  //Now get the dominant unit
  int c=0;
  double bytes = ibytes; //need to keep decimel places for calculations
  while(bytes>=1000 && c<labs.length() ){
    bytes = bytes/1024;
    c++;
  } //labs[c] is the unit
  //Bytes are now
  //Now format the number (up to 3 digits, not including decimel places)
  QString num;
  if(bytes>=100){
    //No decimel places
    num = QString::number(qRound(bytes));
  }else if(bytes>=10){
    //need 1 decimel place
    num = QString::number( (qRound(bytes*10)/10.0) );
  }else if(bytes>1){
    //need 2 decimel places
    num = QString::number( (qRound(bytes*100)/100.0) );
  }else{
    //Fully decimel (3 places)
    num = "0."+QString::number(qRound(bytes*1000));
  }
  return (num+labs[c]);
}

QString LUtils::SecondsToDisplay(int secs){
  if(secs < 0){ return "??"; }
  QString rem; //remaining
  if(secs > 3600){
    int hours = secs/3600;
    rem.append( QString::number(hours)+"h ");
    secs = secs - (hours*3600);
  }
  if(secs > 60){
    int min = secs/60;
    rem.append( QString::number(min)+"m ");
    secs = secs - (min*60);
  }
  if(secs > 0){
    rem.append( QString::number(secs)+"s");
  }else{
    rem.append( "0s" );
  }
  return rem;
}

//Various function for finding valid QtQuick plugins on the system
bool LUtils::validQuickPlugin(QString ID){
  return ( !LUtils::findQuickPluginFile(ID).isEmpty() );
}

QString LUtils::findQuickPluginFile(QString ID){
  if(ID.startsWith("quick-")){ ID = ID.section("-",1,50); } //just in case
  //Give preference to any user-supplied plugins (overwrites for system plugins)
  QString path = QDir::homePath()+"/.lumina/quickplugins/quick-"+ID+".qml";
  if( QFile::exists(path) ){return path; }
  path = LOS::LuminaShare()+"quickplugins/quick-"+ID+".qml";
  if( QFile::exists(path) ){return path; }
  return ""; //could not be found
}

QStringList LUtils::listQuickPlugins(){
  QDir dir(QDir::homePath()+"/.lumina/quickplugins");
  QStringList files = dir.entryList(QStringList() << "quick-*.qml", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  dir.cd(LOS::LuminaShare()+"quickplugins");
  files << dir.entryList(QStringList() << "quick-*.qml", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  for(int i=0; i<files.length(); i++){
    files[i] = files[i].section("quick-",1,100).section(".qml",0,0); //just grab the ID out of the middle of the filename
  }
  files.removeDuplicates();
  //qDebug() << "Found Quick Plugins:" << files;
  return files;
}

QStringList LUtils::infoQuickPlugin(QString ID){ //Returns: [Name, Description, Icon]
  //qDebug() << "Find Quick Info:" << ID;
  QString path = findQuickPluginFile(ID);
  //qDebug() << " - path:" << path;
  if(path.isEmpty()){ return QStringList(); } //invalid ID
  QStringList contents = LUtils::readFile(path);
  if(contents.isEmpty()){ return QStringList(); } //invalid file (unreadable)
  contents = contents.filter("//").filter("=").filter("Plugin"); //now just grab the comments
  //qDebug() << " - Filtered Contents:" << contents;
  QStringList info; info << "" << "" << "";
  for(int i=0; i<contents.length(); i++){
    if(contents[i].contains("Plugin-Name=")){ info[0] = contents[i].section("Plugin-Name=",1,1).simplified(); }
    else if(contents[i].contains("Plugin-Description=")){ info[1] = contents[i].section("Plugin-Description=",1,1).simplified(); }
    else if(contents[i].contains("Plugin-Icon=")){ info[2] = contents[i].section("Plugin-Icon=",1,1).simplified(); }
  }
  if(info[0].isEmpty()){ info[0]=ID; }
  if(info[2].isEmpty()){ info[2]="preferences-plugin"; }
  //qDebug() << " - info:" << info;
  return info;
}

QStringList LUtils::listFavorites(){
  static QDateTime lastRead;
  QDateTime cur = QDateTime::currentDateTime();
  if(lastRead.isNull() || lastRead<QFileInfo(QDir::homePath()+"/.lumina/favorites/fav.list").lastModified()){
    fav = LUtils::readFile(QDir::homePath()+"/.lumina/favorites/fav.list");
    fav.removeAll(""); //remove any empty lines
    lastRead = cur;
    if(fav.isEmpty()){
      //Make sure the favorites dir exists, and create it if necessary
      QDir dir(QDir::homePath()+"/.lumina/favorites");
	if(!dir.exists()){ dir.mkpath(QDir::homePath()+"/.lumina/favorites"); }
    }
  }
  
  return fav;
}

bool LUtils::saveFavorites(QStringList list){
  bool ok = LUtils::writeFile(QDir::homePath()+"/.lumina/favorites/fav.list", list, true);
  if(ok){ fav = list; } //also save internally in case of rapid write/read of the file
  return ok;
}

bool LUtils::isFavorite(QString path){
  QStringList fav = LUtils::listFavorites();
  for(int i=0; i<fav.length(); i++){
    if(fav[i].endsWith("::::"+path)){ return true; }
  }
  return false;
}

bool LUtils::addFavorite(QString path, QString name){
  //Generate the type of favorite this is
  QFileInfo info(path);
  QString type;
  if(info.isDir()){ type="dir"; }
  else if(info.suffix()=="desktop"){ type="app"; }
  else{ type = LXDG::findAppMimeForFile(path); }
  //Assign a name if none given
  if(name.isEmpty()){ name = info.fileName(); }
  //Now add it to the list
  QStringList favs = LUtils::listFavorites();
  bool found = false;
  for(int i=0; i<favs.length(); i++){
    if(favs[i].endsWith("::::"+path)){ favs[i] = name+"::::"+type+"::::"+path; }
  }
  if(!found){ favs << name+"::::"+type+"::::"+path; }
  return LUtils::saveFavorites(favs);
}

void LUtils::removeFavorite(QString path){
  QStringList fav = LUtils::listFavorites();
  bool changed = false;
  for(int i=0; i<fav.length(); i++){
    if(fav[i].endsWith("::::"+path)){ fav.removeAt(i); i--; changed=true;}
  }
  if(changed){ LUtils::saveFavorites(fav); }
}

void LUtils::upgradeFavorites(int fromoldversionnumber){
  if(fromoldversionnumber <= 8004){ // < pre-0.8.4>, sym-links in the ~/.lumina/favorites dir}
    //Include 0.8.4-devel versions in this upgrade (need to distinguish b/w devel and release versions later somehow)
    QDir favdir(QDir::homePath()+"/.lumina/favorites");
    QFileInfoList symlinks = favdir.entryInfoList(QDir::Files | QDir::Dirs | QDir::System | QDir::NoDotAndDotDot);
    QStringList favfile = LUtils::listFavorites(); //just in case some already exist
    bool newentry = false;
    for(int i=0; i<symlinks.length(); i++){
      if(!symlinks[i].isSymLink()){ continue; } //not a symlink
      QString path = symlinks[i].symLinkTarget();
      QString name = symlinks[i].fileName(); //just use the name of the symlink from the old system
      QString type;
      if(symlinks[i].isDir()){ type = "dir"; }
      else if(name.endsWith(".desktop")){ type = "app"; }
      else{ type = LXDG::findAppMimeForFile(path); }
      //Put the line into the file
      favfile << name+"::::"+type+"::::"+path;
      //Now remove the symlink - obsolete format
      QFile::remove(symlinks[i].absoluteFilePath());
      newentry = true;
    }
    if(newentry){
      LUtils::saveFavorites(favfile);
    }
  } //end check for version <= 0.8.4

}  

void LUtils::LoadSystemDefaults(bool skipOS){
  //Will create the Lumina configuration files based on the current system template (if any)
  qDebug() << "Loading System Defaults";
  QStringList sysDefaults;
  if(!skipOS){ sysDefaults = LUtils::readFile(LOS::AppPrefix()+"etc/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(LOS::AppPrefix()+"etc/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty() && !skipOS) { sysDefaults = LUtils::readFile(LOS::SysPrefix()+"etc/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(LOS::SysPrefix()+"etc/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty() && !skipOS) { sysDefaults = LUtils::readFile(L_ETCDIR+"/luminaDesktop.conf"); }
  if(sysDefaults.isEmpty() && !skipOS){ sysDefaults = LUtils::readFile(L_ETCDIR+"/luminaDesktop.conf.dist"); }
  if(sysDefaults.isEmpty()){ sysDefaults = LUtils::readFile(LOS::LuminaShare()+"luminaDesktop.conf"); }
  //Find the number of the left-most desktop screen
  QString screen = "0";
  QDesktopWidget *desk =QApplication::desktop();
  QRect screenGeom;
  for(int i=0; i<desk->screenCount(); i++){
     if(desk->screenGeometry(i).x()==0){ 
	screen = QString::number(i); 
	screenGeom = desk->screenGeometry(i); 
	break; 
    }
  }
  //Now setup the default "desktopsettings.conf" and "sessionsettings.conf" files
  QStringList deskset, sesset, lopenset;
  
  // -- SESSION SETTINGS --
  QStringList tmp = sysDefaults.filter("session_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("session."); }//for backwards compat
  sesset << "[General]"; //everything is in this section
  sesset << "DesktopVersion="+LUtils::LuminaDesktopVersion();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    if(val.isEmpty()){ continue; }
    QString istrue = (val.toLower()=="true") ? "true": "false";
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   
  
     //Special handling for values which need to exist first
    if(var.endsWith("_ifexists") ){ 
      var = var.remove("_ifexists"); //remove this flag from the variable
      //Check if the value exists (absolute path only)
      if(!QFile::exists(val)){ continue; } //skip this line - value/file does not exist
    }
    //Parse/save the value
    QString loset, sset; //temporary strings
    if(var=="session_enablenumlock"){ sset = "EnableNumlock="+ istrue; }
    else if(var=="session_playloginaudio"){ sset = "PlayStartupAudio="+istrue; }
    else if(var=="session_playlogoutaudio"){ sset = "PlayLogoutAudio="+istrue; }
    else if(var=="session_default_terminal"){ sset = "default-terminal="+val; }
    else if(var=="session_default_filemanager"){ 
      sset = "default-filemanager="+val;
      loset = "directory="+val; 
    }
    else if(var=="session_default_webbrowser"){ loset = "webbrowser="+val; }
    else if(var=="session_default_email"){ loset = "email="+val; }
    //Put the line into the file (overwriting any previous assignment as necessary)
    if(!loset.isEmpty()){
      int index = lopenset.indexOf(QRegExp(loset.section("=",0,0)+"=*", Qt::CaseSensitive, QRegExp::Wildcard));
      qDebug() << "loset line:" << loset << index << lopenset;
      if(index<0){ lopenset << loset; } //new line
      else{ lopenset[index] = loset; } //overwrite the other line
    }
    if(!sset.isEmpty()){
      int index = sesset.indexOf(QRegExp(sset.section("=",0,0)+"=*", Qt::CaseSensitive, QRegExp::Wildcard));
      if(index<0){ sesset << sset; } //new line
      else{ sesset[index] = sset; } //overwrite the other line	    
    }
  }
  if(!lopenset.isEmpty()){ lopenset.prepend("[default]"); } //the session options exist within this set

  // -- DESKTOP SETTINGS --
  //(only works for the primary desktop at the moment)
  tmp = sysDefaults.filter("desktop_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("desktop."); }//for backwards compat
  if(!tmp.isEmpty()){deskset << "[desktop-"+screen+"]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    if(val.isEmpty()){ continue; }
    QString istrue = (val.toLower()=="true") ? "true": "false";
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   
    if(var=="desktop_visiblepanels"){ deskset << "panels="+val; }
    else if(var=="desktop_backgroundfiles"){ deskset << "background\\filelist="+val; }
    else if(var=="desktop_backgroundrotateminutes"){ deskset << "background\\minutesToChange="+val; }
    else if(var=="desktop_plugins"){ deskset << "pluginlist="+val; }
    else if(var=="desktop_generate_icons"){ deskset << "generateDesktopIcons="+istrue; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections

  // -- PANEL SETTINGS --
  //(only works for the primary desktop at the moment)
  for(int i=1; i<11; i++){
    QString panvar = "panel"+QString::number(i);
    tmp = sysDefaults.filter(panvar);
    if(!tmp.isEmpty()){deskset << "[panel"+screen+"."+QString::number(i-1)+"]"; }
    for(int i=0; i<tmp.length(); i++){
      if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
      QString var = tmp[i].section("=",0,0).toLower().simplified();
      QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
      if(val.isEmpty()){ continue; }
      QString istrue = (val.toLower()=="true") ? "true": "false";
      //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
      if(var.contains(".")){ var.replace(".","_"); } 
      //Now parse the variable and put the value in the proper file   
      if(var==(panvar+"_pixelsize")){
        //qDebug() << "Panel Size:" << val;
	if(val.contains("%")){
	  QString last = val.section("%",1,1).toLower(); //last character
	  val = val.section("%",0,0);
          if(last=="h"){ val = QString::number( qRound(screenGeom.height()*val.toDouble())/100 ); }//adjust value to a percentage of the height of the screen
          else if(last=="w"){ val = QString::number( qRound(screenGeom.width()*val.toDouble())/100 ); }//adjust value to a percentage of the width of the screen
        }
        //qDebug() << " -- Adjusted:" << val;
        deskset << "height="+val; 
      }
      else if(var==(panvar+"_autohide")){ deskset << "hidepanel="+istrue; }
      else if(var==(panvar+"_location")){ deskset << "location="+val.toLower(); }
      else if(var==(panvar+"_plugins")){ deskset << "pluginlist="+val; }
      else if(var==(panvar+"_pinlocation")){ deskset << "pinLocation="+val.toLower(); }
      else if(var==(panvar+"_edgepercent")){ deskset << "lengthPercent="+val; }
    }
    if(!tmp.isEmpty()){ deskset << ""; } //space between sections
  }

  // -- MENU settings --
  tmp = sysDefaults.filter("menu_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("menu."); } //backwards compat
  if(!tmp.isEmpty()){deskset << "[menu]"; }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).toLower().simplified();
    if(val.isEmpty()){ continue; }
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   
    if(var=="menu_plugins"){ deskset << "itemlist="+val; }
  }
  if(!tmp.isEmpty()){ deskset << ""; } //space between sections

  // -- FAVORITES --
  tmp = sysDefaults.filter("favorites_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("favorites."); }
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file
    qDebug() << "Favorite entry:" << var << val;
    if(var=="favorites_add_ifexists" && QFile::exists(val)){ qDebug() << " - Exists/Adding:"; LUtils::addFavorite(val); }
    else if(var=="favorites_add"){ qDebug() << " - Adding:"; LUtils::addFavorite(val); }
    else if(var=="favorites_remove"){ qDebug() << " - Removing:"; LUtils::removeFavorite(val); }
  }
  
  //Now do any theme settings
  QStringList themesettings = LTHEME::currentSettings(); 
      //List: [theme path, colorspath, iconsname, font, fontsize]
  //qDebug() << "Current Theme Color:" << themesettings[1];
  tmp = sysDefaults.filter("theme_");
  if(tmp.isEmpty()){ tmp = sysDefaults.filter("theme."); }
  bool setTheme = !tmp.isEmpty();
  for(int i=0; i<tmp.length(); i++){
    if(tmp[i].startsWith("#") || !tmp[i].contains("=") ){ continue; }
    QString var = tmp[i].section("=",0,0).toLower().simplified();
    QString val = tmp[i].section("=",1,1).section("#",0,0).simplified();
    if(val.isEmpty()){ continue; }
    //Change in 0.8.5 - use "_" instead of "." within variables names - need backwards compat for a little while
    if(var.contains(".")){ var.replace(".","_"); } 
    //Now parse the variable and put the value in the proper file   
    if(var=="theme_themefile"){ themesettings[0] = val; }
    else if(var=="theme_colorfile"){ themesettings[1] = val; }
    else if(var=="theme_iconset"){ themesettings[2] = val; }
    else if(var=="theme_font"){ themesettings[3] = val; }
    else if(var=="theme_fontsize"){ 
      if(val.endsWith("%")){ val = QString::number( (screenGeom.height()*val.section("%",0,0).toDouble())/100 )+"px"; }
      themesettings[4] = val; 
    }
  }
  //qDebug() << " - Now Color:" << themesettings[1] << setTheme;
  
  //Now double check that the custom theme/color files exist and reset it will the full path as necessary
  if(setTheme){
    QStringList systhemes = LTHEME::availableSystemThemes();
    QStringList syscolors = LTHEME::availableSystemColors();
    //theme file
    //qDebug() << "Detected Themes/colors:" << systhemes << syscolors;
    if( !themesettings[0].startsWith("/") || !QFile::exists(themesettings[0]) || !themesettings[0].endsWith(".qss.template")){
      themesettings[0] = themesettings[0].section(".qss",0,0).simplified();
      for(int i=0; i<systhemes.length(); i++){
	 if(systhemes[i].startsWith(themesettings[0]+"::::",Qt::CaseInsensitive)){
	    themesettings[0] = systhemes[i].section("::::",1,1); //Replace with the full path
	    break;
	 }
      }
    }
    //color file
    if( !themesettings[1].startsWith("/") || !QFile::exists(themesettings[1]) || !themesettings[1].endsWith(".qss.colors") ){
      //Remove any extra/invalid extension
      themesettings[1] = themesettings[1].section(".qss",0,0).simplified();
      for(int i=0; i<syscolors.length(); i++){
	 if(syscolors[i].startsWith(themesettings[1]+"::::",Qt::CaseInsensitive)){
	    themesettings[1] = syscolors[i].section("::::",1,1); //Replace with the full path
	    break;
	 }
      }
    }
  }
  //qDebug() << " - Final Theme Color:" << themesettings[1];

  //Ensure that the settings directory exists
  QString setdir = QDir::homePath()+"/.lumina/LuminaDE";
  if(!QFile::exists(setdir)){ 
    QDir dir; 
    dir.mkpath(setdir); 
  }
  //Now save the settings files
  if(setTheme){ LTHEME::setCurrentSettings( themesettings[0], themesettings[1], themesettings[2], themesettings[3], themesettings[4]); }
  LUtils::writeFile(setdir+"/sessionsettings.conf", sesset, true);
  LUtils::writeFile(setdir+"/desktopsettings.conf", deskset, true);
  LUtils::writeFile(setdir+"/lumina-open.conf", lopenset, true);
}

// =======================
//      RESIZEMENU CLASS
// =======================
ResizeMenu::ResizeMenu(QWidget *parent) : QMenu(parent){
  this->setContentsMargins(1,1,1,1);
  this->setMouseTracking(true);
  resizeSide = NONE;
  cAct = new QWidgetAction(this);
  contents = 0;
  connect(this, SIGNAL(aboutToShow()), this, SLOT(clearFlags()) );
  connect(this, SIGNAL(aboutToHide()), this, SLOT(clearFlags()) );
  connect(cAct, SIGNAL(hovered()), this, SLOT(actionHovered()) );
}

ResizeMenu::~ResizeMenu(){
	
}

void ResizeMenu::setContents(QWidget *con){
  this->clear();
  cAct->setDefaultWidget(con);
  this->addAction(cAct);
  contents = con; //save for later
  contents->setCursor(Qt::ArrowCursor);
}

void ResizeMenu::mouseMoveEvent(QMouseEvent *ev){
  QRect geom = this->geometry();
  //Note: The exact position does not matter as much as the size
  //  since the window will be moved again the next time it is shown
  // The "-2" in the sizing below accounts for the menu margins
  QPoint gpos = this->mapToGlobal(ev->pos());
  switch(resizeSide){
    case TOP:
	if(gpos.y() >= geom.bottom()-1){ break; }
	geom.setTop(gpos.y());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2)); }
        break;	    
    case BOTTOM:
	if(gpos.y() <= geom.top()+1){ break; }
	geom.setBottom( gpos.y());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2)); }
        break;	    
    case LEFT:
	if(gpos.x() >= geom.right()-1){ break; }
	geom.setLeft(gpos.x());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2)); }
        break;	    
    case RIGHT:
	if(gpos.x() <= geom.left()+1){ break; }
	geom.setRight(gpos.x());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2)); }
        break;
    default: //NONE
	//qDebug() << " - Mouse At:" << ev->pos();
	//Just adjust the mouse cursor which is shown
        if(ev->pos().x()<=1 && ev->pos().x() >= -1){ this->setCursor(Qt::SizeHorCursor); }
        else if(ev->pos().x() >= this->width()-1 && ev->pos().x() <= this->width()+1){  this->setCursor(Qt::SizeHorCursor); }
        else if(ev->pos().y()<=1 && ev->pos().y() >= -1){ this->setCursor(Qt::SizeVerCursor); }
        else if(ev->pos().y() >= this->height()-1 && ev->pos().y() <= this->height()+1){ this->setCursor(Qt::SizeVerCursor); }
	else{ this->setCursor(Qt::ArrowCursor); }
  }
  QMenu::mouseMoveEvent(ev);  //do normal processing as well
}

void ResizeMenu::mousePressEvent(QMouseEvent *ev){
  bool used = false;
  if(ev->buttons().testFlag(Qt::LeftButton) && resizeSide==NONE){
    //qDebug() << "Mouse Press Event:" <<  ev->pos() << resizeSide;
    if(ev->pos().x()<=1 && ev->pos().x() >= -1){resizeSide = LEFT; used = true;}
    else if(ev->pos().x() >= this->width()-1 && ev->pos().x() <= this->width()+1){ resizeSide = RIGHT; used = true;}
    else if(ev->pos().y()<=1 && ev->pos().y() >= -1){ resizeSide = TOP; used = true; }
    else if(ev->pos().y() >= this->height()-1 && ev->pos().y() <= this->height()+1){ resizeSide = BOTTOM; used = true; }
  }
  if(used){ ev->accept(); }
  else{ QMenu::mousePressEvent(ev); } //do normal processing
}

void ResizeMenu::mouseReleaseEvent(QMouseEvent *ev){
  if(ev->button() == Qt::LeftButton && resizeSide!=NONE ){
    //qDebug() << "Mouse Release Event:" <<  ev->pos() << resizeSide;
    resizeSide = NONE;
    emit MenuResized(contents->size());
    ev->accept();
  }else{
    QMenu::mouseReleaseEvent(ev);  //do normal processing
  }
}

//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013, Ken Moore
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

#include <LuminaOS.h>

//=============
//  LUtils Functions
//=============
int LUtils::runCmd(QString cmd, QStringList args){
  QProcess *proc = new QProcess;
  proc->setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc->start(cmd);
  }else{
    proc->start(cmd, args);
  }
  while(!proc->waitForFinished(300)){
    QCoreApplication::processEvents();
  }
  int ret = proc->exitCode();
  delete proc;
  return ret;
	
}

QStringList LUtils::getCmdOutput(QString cmd, QStringList args){
  QProcess *proc = new QProcess;
  proc->setProcessChannelMode(QProcess::MergedChannels);
  if(args.isEmpty()){
    proc->start(cmd);
  }else{
    proc->start(cmd,args);	  
  }
  while(!proc->waitForFinished(300)){
    QCoreApplication::processEvents();
  }
  QStringList out = QString(proc->readAllStandardOutput()).split("\n");
  delete proc;
  return out;	
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
  if( file.open(QIODevice::WriteOnly | QIODevice::Truncate) ){
    QTextStream out(&file);
    out << contents.join("\n");
    file.close();
    ok = true;
  }
  return ok;
}

bool LUtils::isValidBinary(QString bin){
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
  return (info.exists() && info.isExecutable());
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

void LUtils::LoadTranslation(QApplication *app, QString appname){
   //Get the current localization
    QString langEnc = "UTF-8"; //default value
    QString langCode = getenv("LANG");
    if(langCode.isEmpty()){ langCode = getenv("LC_ALL"); }
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
    qDebug() << "Loading Locale:" << appname << langCode << langEnc;
    
    //Setup the translator
    /*if(CurTranslator != 0){
      //A Translator already loaded: unload it  before loading the new one
     app->removeTranslator(CurTranslator);
    }*/
    QTranslator *CurTranslator = new QTranslator();
    //Use the shortened locale code if specific code does not have a corresponding file
    if(!QFile::exists(LOS::LuminaShare()+"i18n/"+appname+"_" + langCode + ".qm") ){
      langCode.truncate( langCode.indexOf("_") );
    }
    CurTranslator->load( appname+QString("_") + langCode, LOS::LuminaShare()+"i18n/" );
    app->installTranslator( CurTranslator );
    //Load current encoding for this locale
    QTextCodec::setCodecForLocale( QTextCodec::codecForName(langEnc.toUtf8()) ); 
}

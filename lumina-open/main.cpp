//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include <QApplication>
#include <QX11Info>
#include <QProcess>
#include <QProcessEnvironment>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <QTranslator>
#include <QLocale>
#include <QMessageBox>
#include <QSplashScreen>
#include <QDateTime>
#include <QPixmap>
#include <QColor>
#include <QFont>
#include <QTextCodec>

#include "LFileDialog.h"

#include <LuminaXDG.h>
#include <LuminaUtils.h>
#include <LuminaOS.h>

void printUsageInfo(){
  qDebug() << "lumina-open: Application launcher for the Lumina Desktop Environment";
  qDebug() << "Description: Given a file (with absolute path) or URL, this utility will try to find the appropriate application with which to open the file. If the file is a *.desktop application shortcut, it will just start the application appropriately. It can also perform a few specific system operations if given special flags.";
  qDebug() << "Usage: lumina-open [-select] <absolute file path or URL>";
  qDebug() << "           lumina-open [-volumeup, -volumedown, -brightnessup, -brightnessdown]";
  qDebug() << "  [-select] (optional) flag to bypass any default application settings and show the application selector window";
  qDebug() << "Special Flags:";
  qDebug() << " \"-volume[up/down]\" Flag to increase/decrease audio volume by 5%";
  qDebug() << " \"-brightness[up/down]\" Flag to increase/decrease screen brightness by 5%";
  exit(1);
}

void showOSD(int argc, char **argv, QString message){
  //Setup the application
  QApplication App(argc, argv);
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if(!QFile::exists(LOS::LuminaShare()+"i18n/lumina-open_" + langCode + ".qm") ){
      langCode.truncate( langCode.indexOf("_") );
    }
    translator.load( QString("lumina-open_") + langCode, LOS::LuminaShare()+"i18n/" );
    App.installTranslator( &translator );

  //Display the OSD
  QPixmap pix(":/icons/OSD.png");
  QSplashScreen splash(pix, Qt::SplashScreen | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
     splash.setWindowTitle("");
     QFont myfont;
	myfont.setBold(true);
	myfont.setPixelSize(13);
	splash.setFont(myfont);
  qDebug() << "Display OSD";
  splash.show();
  //qDebug() << " - show message";
  splash.showMessage(message, Qt::AlignCenter, Qt::white);
  //qDebug() << " - loop";
  QDateTime end = QDateTime::currentDateTime().addMSecs(800);
  while(QDateTime::currentDateTime() < end){ App.processEvents(); }
  splash.hide();
}

QString cmdFromUser(int argc, char **argv, QString inFile, QString extension, QString& path, bool showDLG=false){
    //First check to see if there is a default for this extension
    QString defApp = LFileDialog::getDefaultApp(extension);
    if( !defApp.isEmpty() && !showDLG ){
      bool ok = false;
      if(defApp.endsWith(".desktop")){
        XDGDesktop DF = LXDG::loadDesktopFile(defApp, ok);
        if(ok){
      	  QString exec = LXDG::getDesktopExec(DF);
      	  if(!exec.isEmpty()){
      	    qDebug() << "[lumina-open] Using default application:" << DF.name << "File:" << inFile;
            if(!DF.path.isEmpty()){ path = DF.path; }
            return exec;
          }
        }
      }else{
	//Only binary given
	if(LUtils::isValidBinary(defApp)){
	  qDebug() << "[lumina-open] Using default application:" << defApp << "File:" << inFile;
	  return defApp; //just use the binary
	}
      }
      //invalid default - reset it and continue on
      LFileDialog::setDefaultApp(extension, "");
    }
    //Final catch: directory given - no valid default found - use lumina-fm
    if(extension=="directory" && !showDLG){ return "lumina-fm"; }
    //No default set -- Start up the application selection dialog
    QApplication App(argc, argv);
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if(!QFile::exists(LOS::LuminaShare()+"i18n/lumina-open_" + langCode + ".qm") ){
      langCode.truncate( langCode.indexOf("_") );
    }
    translator.load( QString("lumina-open_") + langCode, LOS::LuminaShare()+"i18n/" );
    App.installTranslator( &translator );
    qDebug() << "Locale:" << langCode;

    //Load current encoding for this locale
    QTextCodec::setCodecForTr( QTextCodec::codecForLocale() ); //make sure to use the same codec
    qDebug() << "Locale Encoding:" << QTextCodec::codecForLocale()->name();

    LFileDialog w;
    if(inFile.startsWith(extension)){
      //URL
      w.setFileInfo(inFile, extension, false);
    }else{
      //File
      if(inFile.endsWith("/")){ inFile.chop(1); }
      w.setFileInfo(inFile.section("/",-1), extension, true);
    }

    w.show();

    App.exec();
    if(!w.appSelected){ exit(1); }
    //Return the run path if appropriate
    if(!w.appPath.isEmpty()){ path = w.appPath; }
    //Just do the default application registration here for now
    //  might move it to the runtime phase later after seeing that the app has successfully started
    if(w.setDefault){ 
      if(!w.appFile.isEmpty()){ LFileDialog::setDefaultApp(extension, w.appFile); }
      else{ LFileDialog::setDefaultApp(extension, w.appExec); }
    }else{ LFileDialog::setDefaultApp(extension, ""); }
    //Now return the resulting application command
    return w.appExec;
}

void getCMD(int argc, char ** argv, QString& binary, QString& args, QString& path){
  //Get the input file
  QString inFile;
  bool showDLG = false; //flag to bypass any default application setting
  if(argc > 1){
    for(int i=1; i<argc; i++){
      if(QString(argv[i]).simplified() == "-select"){
      	showDLG = true;
      }else if(QString(argv[i]).simplified() == "-volumeup"){
	int vol = LOS::audioVolume()+5; //increase 5%
	if(vol>100){ vol=100; }
	LOS::setAudioVolume(vol);
	showOSD(argc,argv, QString(QObject::tr("Audio Volume %1%")).arg(QString::number(vol)) );
	return;
      }else if(QString(argv[i]).simplified() == "-volumedown"){
	int vol = LOS::audioVolume()-5; //decrease 5%
	if(vol<0){ vol=0; }
	LOS::setAudioVolume(vol);
	showOSD(argc,argv, QString(QObject::tr("Audio Volume %1%")).arg(QString::number(vol)) );
	return;
      }else if(QString(argv[i]).simplified() == "-brightnessup"){
	int bright = LOS::ScreenBrightness();
	if(bright > 0){ //brightness control available
	  bright = bright+5; //increase 5%
	  if(bright>100){ bright = 100; }
	  LOS::setScreenBrightness(bright);
	  showOSD(argc,argv, QString(QObject::tr("Screen Brightness %1%")).arg(QString::number(bright)) );
	}
	return;
      }else if(QString(argv[i]).simplified() == "-brightnessdown"){
	int bright = LOS::ScreenBrightness();
	if(bright > 0){ //brightness control available
	  bright = bright-5; //decrease 5%
	  if(bright<0){ bright = 0; }
	  LOS::setScreenBrightness(bright);
	  showOSD(argc,argv, QString(QObject::tr("Screen Brightness %1%")).arg(QString::number(bright)) );
	}
	return;
      }else{
        inFile = argv[i];
        break;
      }
    }
  }else{
    printUsageInfo();
  }
  //Make sure that it is a valid file/URL
  bool isFile=false; bool isUrl=false;
  if(QFile::exists(inFile)){ isFile=true; }
  else if(QUrl(inFile).isValid() && !inFile.startsWith("/") ){ isUrl=true; }
  if( !isFile && !isUrl ){ qDebug() << "Error: Invalid file or URL"; return;}
  //Determing the type of file (extension)
  QString extension;
  if(isFile){
    QFileInfo info(inFile);
    extension=info.completeSuffix();
    if(info.isDir()){ extension="directory"; }
    else if(info.isExecutable() && extension.isEmpty()){ extension="binary"; }
  }else if(isUrl){ extension = inFile.section(":",0,0); }
  //if not an application  - find the right application to open the file
  QString cmd;
  bool useInputFile = false;
  if(extension=="desktop" && !showDLG){
    bool ok = false;
    XDGDesktop DF = LXDG::loadDesktopFile(inFile, ok);
    if(!ok){
      qDebug() << "[ERROR] Input *.desktop file could not be read:" << inFile;
      exit(1);
    }
    switch(DF.type){
      case XDGDesktop::APP:
        if(!DF.exec.isEmpty()){
          cmd = LXDG::getDesktopExec(DF);
          if(!DF.path.isEmpty()){ path = DF.path; }
        }else{
          qDebug() << "[ERROR] Input *.desktop application file is missing the Exec line:" << inFile;
          exit(1);
        }
        break;
      case XDGDesktop::LINK:
        if(!DF.url.isEmpty()){
          //This is a URL - so adjust the input variables appropriately
          inFile = DF.url;
          cmd.clear();
          extension = inFile.section(":",0,0);
        }else{
          qDebug() << "[ERROR] Input *.desktop link file is missing the URL line:" << inFile;
          exit(1);
        }
        break;
      case XDGDesktop::DIR:
        if(!DF.path.isEmpty()){
          //This is a directory link - adjust inputs
          inFile = DF.path;
          cmd.clear();
          extension = "directory";
        }else{
          qDebug() << "[ERROR] Input *.desktop directory file is missing the Path line:" << inFile;
          exit(1);
        }
        break;
      default:
        qDebug() << "[ERROR] Unknown *.desktop file type:" << inFile;
        exit(1);
    }
  }
  if(cmd.isEmpty()){
    if(extension=="binary"){ cmd = inFile; }
    else{
    //Find out the proper application to use this file/directory
    useInputFile=true;
    cmd = cmdFromUser(argc, argv, inFile, extension, path, showDLG);
    }
  }
  //qDebug() << "Found Command:" << cmd << "Extension:" << extension;
  //Clean up the command appropriately for output
  if(cmd.contains("%")){cmd = cmd.remove("%U").remove("%u").remove("%F").remove("%f").remove("%i").remove("%c").remove("%k").simplified(); }
  binary = cmd;
  if(useInputFile){ args = inFile; }
}

int main(int argc, char **argv){
  //Run all the actual code in a separate function to have as little memory usage
  //  as possible aside from the main application when running

  //Make sure the XDG environment variables exist first
  LXDG::setEnvironmentVars();
  //now get the command
  QString cmd, args, path;
  getCMD(argc, argv, cmd, args, path);
  //qDebug() << "Run CMD:" << cmd << args;
  //Now run the command (move to execvp() later?)
  if(cmd.isEmpty()){ return 0; } //no command to run (handled internally)
  if(!args.isEmpty()){ cmd.append(" \""+args+"\""); }
  int retcode = system( cmd.toUtf8() );
  /*
  QProcess *p = new QProcess();
  p->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
  if(!path.isEmpty() && QFile::exists(path)){ p->setWorkingDirectory(path); }
  p->start(cmd+" \""+args+"\"");
  //Check the startup procedure
  while(!p->waitForStarted(5000)){
    if(p->state() == QProcess::NotRunning){
     //bad/invalid start
     qDebug() << "[lumina-open] Application did not startup properly:"<<cmd+" "+args;
     return p->exitCode();
    }else if(p->state() == QProcess::Running){
     //This just missed the "started" signal - continue
     break;
    }
  }
  //Now check up on it once every minute until it is finished
  while(!p->waitForFinished(60000)){
    if(p->state() != QProcess::Running){ break; } //somehow missed the finished signal
  }
  int retcode = p->exitCode();*/
  if(retcode!=0){
    qDebug() << "[lumina-open] Application Error:" << retcode;
    //Setup the application
    QApplication App(argc, argv);
    QTranslator translator;
    QLocale mylocale;
    QString langCode = mylocale.name();

    if(!QFile::exists(LOS::LuminaShare()+"i18n/lumina-open_" + langCode + ".qm") ){
      langCode.truncate( langCode.indexOf("_") );
    }
    translator.load( QString("lumina-open_") + langCode, LOS::LuminaShare()+"i18n/" );
    App.installTranslator( &translator );
    QMessageBox::critical(0,QObject::tr("Application Error"), QObject::tr("The following application experienced an error and needed to close:")+"\n\n"+cmd);
  }
  return retcode;
}

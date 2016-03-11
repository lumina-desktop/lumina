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
#include <QMessageBox>
#include <QLabel>
#include <QDateTime>
#include <QPixmap>
#include <QColor>
#include <QDesktopWidget>

#include "LFileDialog.h"

#include <LuminaXDG.h>
#include <LuminaUtils.h>
#include <LuminaOS.h>
#include <LuminaThemes.h>

void printUsageInfo(){
  qDebug() << "lumina-open: Application launcher for the Lumina Desktop Environment";
  qDebug() << "Description: Given a file (with absolute path) or URL, this utility will try to find the appropriate application with which to open the file. If the file is a *.desktop application shortcut, it will just start the application appropriately. It can also perform a few specific system operations if given special flags.";
  qDebug() << "Usage: lumina-open [-select] <absolute file path or URL>";
  qDebug() << "           lumina-open [-volumeup, -volumedown, -brightnessup, -brightnessdown]";
  qDebug() << "  [-select] (optional) flag to bypass any default application settings and show the application selector window";
  qDebug() << "Special Flags:";
  qDebug() << " \"-volume[up/down]\" Flag to increase/decrease audio volume by 5%";
  qDebug() << " \"-brightness[up/down]\" Flag to increase/decrease screen brightness by 5%";
  qDebug() << " \"-autostart-apps\" Flag to launch all the various apps which are registered with XDG autostart specification";
  exit(1);
}

void ShowErrorDialog(int argc, char **argv, QString message){
    //Setup the application
    QApplication App(argc, argv);
    LuminaThemeEngine theme(&App);
	LUtils::LoadTranslation(&App,"lumina-open");
    QMessageBox dlg(QMessageBox::Critical, QObject::tr("File Error"), message );
    dlg.exec();
    exit(1);
}

void showOSD(int argc, char **argv, QString message){
  //Setup the application
  QApplication App(argc, argv);
    LUtils::LoadTranslation(&App,"lumina-open");

  //Display the OSD
  QPixmap pix(":/icons/OSD.png");
  QLabel splash(0, Qt::Window | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
     splash.setWindowTitle("");
     splash.setStyleSheet("QLabel{background: black; color: white; font-weight: bold; font-size: 13pt; margin: 1ex;}");
     splash.setAlignment(Qt::AlignCenter);


  qDebug() << "Display OSD";
  splash.setText(message);
  //Make sure it is centered on the current screen
  QPoint center = App.desktop()->screenGeometry(QCursor::pos()).center();
  splash.move(center.x()-(splash.sizeHint().width()/2), center.y()-(splash.sizeHint().height()/2));
  splash.show();
  //qDebug() << " - show message";
  //qDebug() << " - loop";
  QDateTime end = QDateTime::currentDateTime().addMSecs(800);
  while(QDateTime::currentDateTime() < end){ App.processEvents(); }
  splash.hide();
}

void LaunchAutoStart(){
  QList<XDGDesktop> xdgapps = LXDG::findAutoStartFiles();
  for(int i=0; i<xdgapps.length(); i++){
    //Generate command and clean up any stray "Exec" field codes (should not be any here)
    QString cmd = LXDG::getDesktopExec(xdgapps[i]);
    if(cmd.contains("%")){cmd = cmd.remove("%U").remove("%u").remove("%F").remove("%f").remove("%i").remove("%c").remove("%k").simplified(); }
    //Now run the command
    if(!cmd.isEmpty()){ 
      qDebug() << " - Auto-Starting File:" << xdgapps[i].filePath;
      QProcess::startDetached(cmd); 
    }
  }
}

QString cmdFromUser(int argc, char **argv, QString inFile, QString extension, QString& path, bool showDLG=false){
    //First check to see if there is a default for this extension
    QString defApp;
    if(extension=="mimetype"){
	//qDebug() << "inFile:" << inFile;
	QStringList matches = LXDG::findAppMimeForFile(inFile, true).split("::::"); //allow multiple matches
	//qDebug() << "Matches:" << matches;
	for(int i=0; i<matches.length(); i++){
	  defApp = LXDG::findDefaultAppForMime(matches[i]);
	  if(!defApp.isEmpty()){ extension = matches[i]; break; }
	  else if(i+1==matches.length()){ extension = matches[0]; }
	}
    }else{ defApp = LFileDialog::getDefaultApp(extension); }
    //qDebug() << "extension:" << extension << "defApp:" << defApp;
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
    if(extension=="inode/directory" && !showDLG){ return "lumina-fm"; }
    //No default set -- Start up the application selection dialog
    LTHEME::LoadCustomEnvSettings();
    QApplication App(argc, argv);
    LuminaThemeEngine theme(&App);
      LUtils::LoadTranslation(&App,"lumina-open");

    LFileDialog w;
    if(extension=="email" || extension.startsWith("x-scheme-handler/")){
      //URL
      w.setFileInfo(inFile, extension, false);
    }else{
      //File
      if(inFile.endsWith("/")){ inFile.chop(1); }
      w.setFileInfo(inFile.section("/",-1), extension, true);
    }

    w.show();
    App.exec();
    if(!w.appSelected){ return ""; }
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

void getCMD(int argc, char ** argv, QString& binary, QString& args, QString& path, bool& watch){
  //Get the input file
    //Make sure to load the proper system encoding first
    LUtils::LoadTranslation(0,""); //bypass application modification
  QString inFile, ActionID;
  bool showDLG = false; //flag to bypass any default application setting
  if(argc > 1){
    for(int i=1; i<argc; i++){
      if(QString(argv[i]).simplified() == "-select"){
      	showDLG = true;
      }else if(QString(argv[i]).simplified() == "-testcrash"){
	//Test the crash handler
	binary = "internalcrashtest"; watch=true;
	return;
      }else if(QString(argv[i]).simplified() == "-autostart-apps"){
	LaunchAutoStart();
	return;
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
      }else if( (QString(argv[i]).simplified() =="-action") && (argc>(i+1)) ){
        ActionID = QString(argv[i+1]);
	i++; //skip the next input
      }else{
        inFile = QString::fromLocal8Bit(argv[i]);
        break;
      }
    }
  }else{
    printUsageInfo();
  }
  //Make sure that it is a valid file/URL
  bool isFile=false; bool isUrl=false;
  //Quick check/replacement for the URL syntax of a file
  if(inFile.startsWith("file://")){ inFile.remove(0,7); }
  //Now check what type of file this is
  if(QFile::exists(inFile)){ isFile=true; }
  else if(QFile::exists(QDir::currentPath()+"/"+inFile)){isFile=true; inFile = QDir::currentPath()+"/"+inFile;} //account for relative paths
  else if(QUrl(inFile).isValid() && !inFile.startsWith("/") ){ isUrl=true; }
  if( !isFile && !isUrl ){ ShowErrorDialog( argc, argv, QString(QObject::tr("Invalid file or URL: %1")).arg(inFile) ); }
  //Determing the type of file (extension)
  QString extension;
  //qDebug() << "File Type:" << isFile << isUrl;
  if(isFile){
    QFileInfo info(inFile);
    extension=info.suffix();
    //qDebug() << " - Extension:" << extension;
    if(info.isDir()){ extension="inode/directory"; }
    else if(info.isExecutable() && extension.isEmpty()){ extension="binary"; }
    else if(extension!="desktop"){ extension="mimetype"; } //flag to check for mimetype default based on file
  }
  else if(isUrl && inFile.startsWith("mailto:")){ extension = "email"; }
  else if(isUrl && inFile.contains("://") ){ extension = "x-scheme-handler/"+inFile.section("://",0,0); }
  else if(isUrl && inFile.startsWith("www.")){ extension = "x-scheme-handler/http"; inFile.prepend("http://"); } //this catches partial (but still valid) URL's ("www.<something>" for instance)
  //qDebug() << "Input:" << inFile << isFile << isUrl << extension;
  //if not an application  - find the right application to open the file
  QString cmd;
  bool useInputFile = false;
  if(extension=="desktop" && !showDLG){
    bool ok = false;
    XDGDesktop DF = LXDG::loadDesktopFile(inFile, ok);
    if(!ok){
      ShowErrorDialog( argc, argv, QString(QObject::tr("File could not be opened: %1")).arg(inFile) );
    }
    switch(DF.type){
      case XDGDesktop::APP:
        if(!DF.exec.isEmpty()){
          cmd = LXDG::getDesktopExec(DF,ActionID);
          if(!DF.path.isEmpty()){ path = DF.path; }
	  watch = DF.startupNotify || !DF.filePath.contains("/xdg/autostart/");
        }else{
	  ShowErrorDialog( argc, argv, QString(QObject::tr("Application shortcut is missing the launching information (malformed shortcut): %1")).arg(inFile) );
        }
        break;
      case XDGDesktop::LINK:
        if(!DF.url.isEmpty()){
          //This is a URL - so adjust the input variables appropriately
          inFile = DF.url;
          cmd.clear();
          extension = inFile.section(":",0,0);
	  watch = DF.startupNotify || !DF.filePath.contains("/xdg/autostart/");
        }else{
	  ShowErrorDialog( argc, argv, QString(QObject::tr("URL shortcut is missing the URL: %1")).arg(inFile) );
        }
        break;
      case XDGDesktop::DIR:
        if(!DF.path.isEmpty()){
          //This is a directory link - adjust inputs
          inFile = DF.path;
          cmd.clear();
          extension = "directory";
	  watch = DF.startupNotify || !DF.filePath.contains("/xdg/autostart/");
        }else{
	  ShowErrorDialog( argc, argv, QString(QObject::tr("Directory shortcut is missing the path to the directory: %1")).arg(inFile) );
        }
        break;
      default:
	qDebug() << DF.type << DF.name << DF.icon << DF.exec;
	ShowErrorDialog( argc, argv, QString(QObject::tr("Unknown type of shortcut : %1")).arg(inFile) );
    }
  }
  if(cmd.isEmpty()){
    if(extension=="binary" && !showDLG){ cmd = inFile; }
    else{
    //Find out the proper application to use this file/directory
    useInputFile=true;
    cmd = cmdFromUser(argc, argv, inFile, extension, path, showDLG);
    if(cmd.isEmpty()){ return; }
    }
  }
  //Now assemble the exec string (replace file/url field codes as necessary)
  if(useInputFile){ 
    args = inFile; //just to keep them distinct internally
    // NOTE: lumina-open is only designed for a single input file,
    //    so no need to distinguish between the list codes (uppercase) 
    //    and the single-file codes (lowercase)
    //Special "inFile" format replacements for input codes
    if( (cmd.contains("%f") || cmd.contains("%F") ) ){
      //Apply any special field replacements for the desired format
      inFile.replace("%20"," ");
      if(inFile.startsWith("file://")){ inFile.remove(0,7); } //chop that URL prefix off the front (should have happened earlier - just make sure)
      //Now replace the field codes
      cmd.replace("%f","\""+inFile+"\"");
      cmd.replace("%F","\""+inFile+"\"");
    }else if( (cmd.contains("%U") || cmd.contains("%u")) ){
      //Apply any special field replacements for the desired format
      if(!inFile.contains("://")){ inFile.prepend("file://"); } //local file - add the extra flag
      inFile.replace(" ", "%20");
      //Now replace the field codes
      cmd.replace("%u","\""+inFile+"\"");
      cmd.replace("%U","\""+inFile+"\"");
    }else{
      //No field codes (or improper field codes given in the file - which is quite common)
      // - Just tack the input file on the end and let the app handle it as necessary
      cmd.append(" \""+inFile+"\"");
    }
  }
  //qDebug() << "Found Command:" << cmd << "Extension:" << extension;
  //Clean up any leftover "Exec" field codes (should have already been replaced earlier)
  if(cmd.contains("%")){cmd = cmd.remove("%U").remove("%u").remove("%F").remove("%f").remove("%i").remove("%c").remove("%k").simplified(); }
  binary = cmd; //pass this string to the calling function

}

int main(int argc, char **argv){
  //Run all the actual code in a separate function to have as little memory usage
  //  as possible aside from the main application when running

  //Make sure the XDG environment variables exist first
  LXDG::setEnvironmentVars();
  //now get the command
  QString cmd, args, path;
  bool watch = true; //enable the crash handler by default (only disabled for some *.desktop inputs)
  getCMD(argc, argv, cmd, args, path, watch);
  //qDebug() << "Run CMD:" << cmd << args;
  //Now run the command (move to execvp() later?)
  if(cmd.isEmpty()){ return 0; } //no command to run (handled internally)
  qDebug() << "[lumina-open] Running Cmd:" << cmd;
  int retcode = 0;
  
  if(!watch && path.isEmpty()){
      //Nothing special about this one - just start it detached (less overhead)
      QProcess::startDetached(cmd);
  }else{
    //Keep an eye on this process for errors and notify the user if it crashes
    QString log;
    if(cmd.contains("\\\\")){
      //Special case (generally for Wine applications)
      cmd = cmd.replace("\\\\","\\");
      retcode = system(cmd.toLocal8Bit()); //need to run it through the "system" instead of QProcess
    }else if(cmd=="internalcrashtest"){
      log = "This is a sample crash log";
      retcode = 2;
    }else{
      QProcess *p = new QProcess();
      p->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
      if(!path.isEmpty() && QFile::exists(path)){ 
        //qDebug() << " - Setting working path:" << path;
        p->setWorkingDirectory(path); 
      }
      p->start(cmd);
      
      //Now check up on it once every minute until it is finished
      while(!p->waitForFinished(60000)){
        //qDebug() << "[lumina-open] process check:" << p->state();
        if(p->state() != QProcess::Running){ break; } //somehow missed the finished signal
      }
      retcode = p->exitCode();
      if( (p->exitStatus()==QProcess::CrashExit) && retcode ==0){ retcode=1; } //so we catch it later
      log = QString(p->readAllStandardError());
      if(log.isEmpty()){ log = QString(p->readAllStandardOutput()); }
    }
    //qDebug() << "[lumina-open] Finished Cmd:" << cmd << retcode << p->exitStatus();
    if( QFile::exists("/tmp/.luminastopping") ){ watch = false; } //closing down session - ignore "crashes" (app could have been killed during cleanup)
    if( (retcode > 0) && watch && !(retcode==1 && cmd.startsWith("pc-su ")) ){ //pc-su returns 1 if the user cancelles the operation
      
      qDebug() << "[lumina-open] Application Error:" << retcode;
        //Setup the application
        QApplication App(argc, argv);
        LuminaThemeEngine theme(&App);
	  LUtils::LoadTranslation(&App,"lumina-open");
	//App.setApplicationName("LuminaOpen");
        QMessageBox dlg(QMessageBox::Critical, QObject::tr("Application Error"), QObject::tr("The following application experienced an error and needed to close:")+"\n\n"+cmd );
	  dlg.setWindowFlags(Qt::Window);
          if(!log.isEmpty()){ dlg.setDetailedText(log); }
          dlg.exec();
      }
  }
  return retcode;
}

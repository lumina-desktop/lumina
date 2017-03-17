//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PianoBarProcess.h"

#include <QTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <LUtils.h>

PianoBarProcess::PianoBarProcess(QWidget *parent) : QObject(parent){
  setupProcess();

}

PianoBarProcess::~PianoBarProcess(){

}

// ===== PUBLIC ======
//Interaction functions
bool PianoBarProcess::isSetup(); //email/password already saved for use or not
void PianoBarProcess::setLogin(QString email, QString pass);
void PianoBarProcess::closePianoBar(); //"q"

QString PianoBarProcess::currentStation(); //Re-direct for the "autostartStation()" function;
QStringList PianoBarProcess::stations();
void PianoBarProcess::setCurrentStation(QString station);
	
void PianoBarProcess::deleteCurrentStation(); //"d"
void PianoBarProcess::createNewStation(); //"c"
void PianoBarProcess::createStationFromCurrentSong(); //"v"
void PianoBarProcess::changeStation(); //"s"

//Settings Manipulation
QString PianoBarProcess::audioQuality(){			// "audio_quality" = [low, medium, high]
  
}

void PianoBarProcess::setAudioQuality(QString); 	// [low, medium, high]
QString PianoBarProcess::autostartStation();		//"autostart_station" = ID
void PianoBarProcess::setAutostartStation(QString);
QString PianoBarProcess::proxy();					//"proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
void PianoBarProcess::setProxy(QString);
QString PianoBarProcess::controlProxy();			//"control_proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
void PianoBarProcess::setControlProxy(QString);

// ====== PUBLIC SLOTS ======
void PianoBarProcess::play(); // "P"
void PianoBarProcess::pause(); //"S" 

void PianoBarProcess::volumeDown(); //"("
void PianoBarProcess::volumeUp(); //")"

void PianoBarProcess::skipSong(); //"n"	
void PianoBarProcess::loveSong(); // "+"
void PianoBarProcess::tiredSong(); // "t"
void PianoBarProcess::banSong(); //"-"
void PianoBarProcess::bookmarkSong(); //"b"

void PianoBarProcess::explainSong(); //"e"

void PianoBarProcess::requestHistory(); // "h"
void PianoBarProcess::requestSongInfo(); //"i"
void PianoBarProcess::requestUpcoming(); //"u"

// ====== PRIVATE ======
void PianoBarProcess::GenerateSettings(){
  currentSettings << "audio_quality = medium";
  currentSettings << "autoselect = 1"; //automatically select the last item in a list (station selection only)
  currentSettings << "format_list_song = %r::::%t::::%a"; //[rating, title, artist]
  currentSettings << "format_nowplaying_song = %r::::%t::::%a::::%l::::%u::::%s"; // [rating, title, artist, album, details url, station (if not quickmix)]
  currentSettings << "format_nowplaying_station = %n::::%i"; //[name, id]
}

void PianoBarProcess::loadSettings(){
  currentSettings.clear();
  QFile file(settingspath);
  if(!file.exists()){ return; }
  if(file.open(QIODevice::ReadOnly)){
    QTextStream in(&file);
    currentSettings = in.readAll().split("\n");
    file.close();
  }
}

QString PianoBarProcess::settingValue(QString var){
  for(int i=0; i<currentSettings.length(); i++){
    if(currentSettings[i].startsWith(var+" = ")){ return currentSettings[i].section(" = ", 1,-1); }
  }
  return "";
}

void PianoBarProcess::setSettingValue(QString var,QString val){
  bool changed = false;
  for(int i=0; i<currentSettings.length() && !changed; i++){
    if(currentSettings[i].startsWith(var+" = ")){ currentSettings[i] = var+" = "+val; changed = true; }
  }
  if(!changed){ currentSettings << var+" = "+val; }
}

void PianoBarProcess::saveSettingsFile(){
  //Ensure the parent directory exists first
  QDir dir(settingspath.section("/",0,-2));
  if(!dir.exists()){ dir.mkpath(dir.absolutePath()); }
  //Now save the settings
  QFile file(settingspath);
  if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
    QTextStream out(&file);
    out << currentSettings.join("\n");
    file.close();
  }
}

void PianoBarProcess::setupProcess(){
  PROC = new QProcess(this);
  //Ensure this process always points to the right configuration directory/files
  QString configdir = getenv("XDG_CONFIG_HOME");
  if(configdir.isEmpty()){ configdir = QDir::homePath()+"/.config/lumina-desktop"; }
  else{ configdir.append("/lumina-desktop"); }
  QProcessEnvironment penv;
  penv.insert("XDG_CONFIG_HOME",configdir);
  settingspath = configdir+"/pianobar/config";
  PROC->setProcessEnvironment(penv);
  //Now setup the rest of the process
  PROC->setProcessChannelMode(QProcess::MergedChannels);
  QString bin = "pianobar";
  LUtils::isValidBinary(bin); //will change "bin" to the full path
  PROC->setProgram(bin);
  connect(PROC, SIGNAL(readyRead()), this, SLOT(ProcUpdate()) );
}

// ====== PRIVATE SLOTS ======
void PianoBarProcess::ProcUpdate(){
  QStringList info = QString(PROC->readAllStandardOutput()).split("\n");
  //NOTE: Need to have a cache of info lines which can carry over between updates as needed (for questions, etc)
  qDebug() << "Got Update:" << info;
  for(int i=0; i<info.length(); i++){
    if(info[i].startsWith("|>")){
      //Now playing line (station, or song)
      QStringList data = info[i].section(">",1,-1).simplified().split("::::"); //Make sure to chop the line prefix off first
      if(data.length()==2){ //station
        cstation = data[0]; //save the name for later
        emit NowPlayingStation(data[0], data[1]);
        //Automatically save this station for autostart next time (make toggle-able later)
        if(data[1]!=autostartStation()){ setAutostartStation(data[1]); }

      }else if(data.length()==6){ //song
        emit NowPlayingSong( data[0]=="<3", data[1], data[2], data[3], data[4], data[5] );
      }
    }else if(info[i].startsWith("(i) ")){ //informational line
      emit NewInformation(info[i].section(" ",1,-1));
    }else if(info[i].startsWith("[?] ")){ //waiting for reply to question
      if(info[i].contains("Select Station:"){
        //Find the number before this line which corresponds to the cstation variable/name
        for(j=i-1; j>=0; j--){
          if(info[j].contains(")" && info[j].contains(cstation) ){
            PROC->write(info[j].section(")",0,0).simplified() + "\r\n");
            break;
          }
        }
      }

    }else if(info[i].startsWith("#")){
      //Time Stamp
        QTime stamp = QTime::fromString(info[i].section("/",0,0).section("-",1,-1), "mm:ss");
	int curS = 60*stamp.minutes() + stamp.seconds(); //time remaining
	stamp = QTime::fromString(info[i].section("/",1,-1), "mm:ss");
        int totS = 60*stamp.minutes() + stamp.sections(); //time total
        emit TimeUpdate(totS-curS, totS);
    }
  }
}

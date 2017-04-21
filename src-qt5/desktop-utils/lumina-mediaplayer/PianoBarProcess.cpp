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
#include <QApplication>
#include <LUtils.h>

PianoBarProcess::PianoBarProcess(QWidget *parent) : QObject(parent){
  setupProcess();
  saveTimer = new QTimer(this);
  saveTimer->setInterval(100); //1/10 second (just enough to change a few settings at once before dumping to disk)
  saveTimer->setSingleShot(true);
  makingStation = false;
  connect(saveTimer, SIGNAL(timeout()), this, SLOT(saveSettingsFile()) );
  if( !loadSettings() ){ GenerateSettings(); }
}

PianoBarProcess::~PianoBarProcess(){
  if(PROC->state()!=QProcess::NotRunning){
    PROC->kill();
  }
}

PianoBarProcess::State PianoBarProcess::currentState(){
  return cState;
}

// ===== PUBLIC ======
//Interaction functions
bool PianoBarProcess::isSetup(){ //email/password already saved for use or not
  return !(settingValue("user").isEmpty() || settingValue("password").isEmpty());
}

void PianoBarProcess::setLogin(QString email, QString pass){
  setSettingValue("user",email);
  setSettingValue("password",pass);
}

QString PianoBarProcess::email(){
  return settingValue("user");
}

QString PianoBarProcess::password(){
  return settingValue("password");
}

void PianoBarProcess::closePianoBar(){ //"q"
  sendToProcess("q");
}

QString PianoBarProcess::currentStation(){ return cstation.simplified(); }
QStringList PianoBarProcess::stations(){ return stationList; }
void PianoBarProcess::setCurrentStation(QString station){
  cstation = station;
  sendToProcess("s");
}

void PianoBarProcess::answerQuestion(int selection){
  QString sel;
  if(selection>=0){ sel = QString::number(selection); }

  if(makingStation && sel.isEmpty()){ makingStation = false; } //cancelled
  sendToProcess(sel, true);
  if(makingStation){ 
    //Need to prompt to list all the available stations to switch over right away
    sendToProcess("s");
  }
}

void PianoBarProcess::deleteCurrentStation(){ //"d" -> "y"
  if(cstation == "QuickMix" || cstation=="Thumbprint Radio"){ return; } //cannot delete these stations - provided by Pandora itself
  sendToProcess("d"); //delete current station
  sendToProcess("y",true); //yes, we want to delete it
  //Now need to automatically change to another station
  setCurrentStation("QuickMix"); //this is always a valid station
}

void PianoBarProcess::createNewStation(QString searchterm){ //"c" -> search term
  sendToProcess("c");
  sendToProcess(searchterm,true);
  makingStation = true;
}

void PianoBarProcess::createStationFromCurrentSong(){ //"v" -> "s"
  sendToProcess("v");
  sendToProcess("s",true);
  makingStation = true;
    //Need to prompt to list all the available stations to switch over right away
    sendToProcess("s");
}

void PianoBarProcess::createStationFromCurrentArtist(){ //"v" -> "a"
  sendToProcess("v");
  sendToProcess("a",true);
  makingStation = true;
    //Need to prompt to list all the available stations to switch over right away
    sendToProcess("s");
}

//Settings Manipulation
QString PianoBarProcess::audioQuality(){			// "audio_quality" = [low, medium, high]
  return settingValue("audio_quality");
}

void PianoBarProcess::setAudioQuality(QString val){ 	// [low, medium, high]
  setSettingValue("audio_quality",val);
}

QString PianoBarProcess::autostartStation(){		//"autostart_station" = ID
  return settingValue("autostart_station");
}

void PianoBarProcess::setAutostartStation(QString id){
  setSettingValue("autostart_station", id);
}

QString PianoBarProcess::proxy(){					//"proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
  return settingValue("proxy");
}

void PianoBarProcess::setProxy(QString url){
  setSettingValue("proxy",url);
}

QString PianoBarProcess::controlProxy(){			//"control_proxy" = URL (example: "http://USER:PASSWORD@HOST:PORT/"  )
  return settingValue("control_proxy");
}

void PianoBarProcess::setControlProxy(QString url){
  setSettingValue("control_proxy", url);
}

QString PianoBarProcess::currentAudioDriver(){
  QString driver = "auto";
  QStringList info = LUtils::readFile(QDir::homePath()+"/.libao").filter("default_driver");
  if(!info.isEmpty()){
    driver = info.last().section("=",-1).simplified();
  }
  return driver;
}

QStringList PianoBarProcess::availableAudioDrivers(){
  QStringList known;
  known << "pulse" << "alsa" << "sndio" << "oss" << "sun" << "roar" << "esd" << "nas";
  known.sort();
  known.prepend("auto"); //make sure this is always first
  return known;
}

void PianoBarProcess::setAudioDriver(QString driver){
  QStringList info = LUtils::readFile(QDir::homePath()+"/.libao");
  bool found = false;
  QString line = "default_driver="+driver;
  if(driver=="auto"){ line.clear(); } //special bypass - nothing set
  for(int i=0; i<info.length() && !found; i++){
    if(info[i].section("=",0,0).simplified() == "default_driver"){
      info[i] = line;
      found = true;
    }
  }
  if(!found && !line.isEmpty()){ info << line; }
  //Now save the file
  LUtils::writeFile(QDir::homePath()+"/.libao", info, true);
}
// ====== PUBLIC SLOTS ======
void PianoBarProcess::play(){ 
  if(PROC->state() == QProcess::NotRunning){
    PROC->start();
  }else{
    sendToProcess("P");
    cState = PianoBarProcess::Running;
    emit currentStateChanged(cState);
  }
}

void PianoBarProcess::pause(){ 
  sendToProcess("S"); 
  cState = PianoBarProcess::Paused;
  emit currentStateChanged(cState);
}

void PianoBarProcess::skipSong(){ 
  sendToProcess("n"); 
}

// ====== PRIVATE ======
void PianoBarProcess::GenerateSettings(){
  currentSettings << "audio_quality = medium";
  currentSettings << "autoselect = 1"; //automatically select the last item in a list (station selection only)
  currentSettings << "format_list_song = %r::::%t::::%a"; //[rating, title, artist]
  currentSettings << "format_nowplaying_song = %r::::%t::::%a::::%l::::%u::::%s"; // [rating, title, artist, album, details url, station (if not quickmix)]
  currentSettings << "format_nowplaying_station = %n::::%i"; //[name, id]
  saveSettingsFile(); //save this to disk *now* - needed before starting the pianobar process
}

bool PianoBarProcess::loadSettings(){
  currentSettings.clear();
  QFile file(settingsPath);
  if(!file.exists()){ return false; }
  if(file.open(QIODevice::ReadOnly)){
    QTextStream in(&file);
    currentSettings = in.readAll().split("\n");
    file.close();
    return true;
  }
  return false;
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
    if(currentSettings[i].startsWith(var+" = ")){
      if(val.isEmpty()){ currentSettings.removeAt(i); i--; }
      else{ currentSettings[i] = var+" = "+val; }
      changed = true;
    }
  }
  if(!changed && !val.isEmpty()){ currentSettings << var+" = "+val; }
  saveTimer->start(); //save this to disk in a moment
}

void PianoBarProcess::saveSettingsFile(){
  //Ensure the parent directory exists first
  QDir dir(settingsPath.section("/",0,-2));
  if(!dir.exists()){ dir.mkpath(dir.absolutePath()); }
  //Now save the settings
  QFile file(settingsPath);
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
  QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();
  penv.insert("XDG_CONFIG_HOME",configdir);
  settingsPath = configdir+"/pianobar/config";
  PROC->setProcessEnvironment(penv);
  //Now setup the rest of the process
  //PROC->setProcessChannelMode(QProcess::MergedChannels);
  QString bin = "pianobar";
  LUtils::isValidBinary(bin); //will change "bin" to the full path
  PROC->setProgram(bin);
  connect(PROC, SIGNAL(readyReadStandardOutput()), this, SLOT(ProcUpdate()) );
  connect(PROC, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(ProcStateChanged(QProcess::ProcessState)) );
  cState = PianoBarProcess::Stopped;
}

void PianoBarProcess::sendToProcess(QString txt, bool withreturn){
  if(PROC->state()==QProcess::Running){
    if(withreturn){ PROC->write( QString(txt+"\r\n").toLocal8Bit() ); }
    else{ PROC->write( QString(txt).toLocal8Bit() ); }
  }
}

// ====== PRIVATE SLOTS ======
void PianoBarProcess::ProcUpdate(){
  QString tmp = QString(PROC->readAllStandardOutput()).replace("\r","\n").remove("\u001B[2K");
  QStringList info = tmp.split("\n",QString::SkipEmptyParts);

  //NOTE: Need to have a cache of info lines which can carry over between updates as needed (for questions, etc)
  //qDebug() << "Got Update:" << info;
  for(int i=0; i<info.length(); i++){
    //First handle any pending cache of listing lines
    if((info[i].startsWith("\t")||info[i].startsWith(" ")) && info[i].contains(")")){
      if(info[i].simplified().startsWith("0) ")){ infoList.clear(); }
      infoList << info[i].section(") ",1,-1).simplified();
      continue; //done handling this line
    }else if(!info[i].startsWith("[?]") && !infoList.isEmpty()){
      //emit NewList(infoList);
      infoList.clear();
    }
    //Now parse the lines for messages/etc
    if(info[i].startsWith("|>")){
      //Now playing line (station, or song)
      QStringList data = info[i].section(">",1,-1).simplified().split("::::"); //Make sure to chop the line prefix off first
      if(data.length()==2){ //station
        cstation = data[0].simplified(); //save the name for later
        emit NowPlayingStation(data[0], data[1]);
        if(stationList.isEmpty()){
          //Need to prompt to list all the available stations
          sendToProcess("s",true);//line return cancels the prompt
          //sendToProcess("",true); //empty line - cancels the prompt
        }
        //Automatically save this station for autostart next time (make toggle-able later)
        if(data[1]!=autostartStation()){ setAutostartStation(data[1]); }

      }else if(data.length()==6){ //song
        emit NowPlayingSong( data[0]=="<3", data[1], data[2], data[3], data[4], data[5] );
      }
      //If a new song/station is detected, ensure that the state is set to "Running"
      if(cState!=PianoBarProcess::Running){
        cState = PianoBarProcess::Running;
        emit currentStateChanged(cState);
      }
    }else if(info[i].startsWith("(i) ")){ //informational line
      emit NewInformation(info[i].section(" ",1,-1));
    }else if(info[i].startsWith("[?] ")){ //waiting for reply to question
      //qDebug() << "Got Question:" << info[i] << infoList;
      if(info[i].contains("Select station:")){
        //qDebug() << "Change to Station:" << cstation;
        //Clean up the station list a bit first (remove the quickmix-status)
        for(int j=0; j<infoList.length(); j++){
          infoList[j] = infoList[j].simplified();
          if(infoList[j].startsWith("q ")){ infoList[j] = infoList[j].section("q ",1,-1); }
          if(infoList[j].startsWith("Q ")){ infoList[j] = infoList[j].section("Q ",1,-1); }
        }
        if(makingStation){
          //Compare the new list to the previous list and switch to the new one automatically
          for(int j=0; j<infoList.length(); j++){
            if(!stationList.contains(infoList[j])){ cstation = infoList[j]; break; }
          }
          makingStation = false; //done changing the current station
        }
        stationList = infoList; //save this list for later
        infoList.clear();
        emit StationListChanged(stationList);
        //Find the station number which corresponds to the cstation variable/name
        for(int j=0; j<stationList.length(); j++){
          if(stationList[j].endsWith(cstation) ){
            //qDebug() << "Activate Station:" << stationList[i];
            sendToProcess(QString::number(j), true); 
            break;
          }else if(j==stationList.length()-1){
            //qDebug() << "Activate Last Station:" << stationList[i];
            sendToProcess(QString::number(stationList.length()-1), true);
          }
        }
      }else if( !infoList.isEmpty() ){
        qDebug() << "Got Question with List:" << info[i] << infoList;
        emit NewQuestion(info[i].section("[?]",1,-1).simplified(), infoList);
      }else if(info[i].contains(" Select ") ){
        qDebug() << "Got Question without List:" << info[i];
        //Got a prompt without a list of answers - just cancel the prompt
        sendToProcess("",true);
        if(makingStation){
          emit showError(tr("Could not find any matches. Please try a different search term"));
          makingStation = false; //make sure this flag is reset
        }
      }
      infoList.clear(); //done with question - make sure this is cleared

    }else if(info[i].startsWith("#")){
      //Time Stamp
        QTime stamp = QTime::fromString(info[i].section("/",0,0).section("-",1,-1), "mm:ss");
	int curS = 60*stamp.minute() + stamp.second(); //time remaining
	stamp = QTime::fromString(info[i].section("/",1,-1), "mm:ss");
        int totS = 60*stamp.minute() + stamp.second(); //time total
        emit TimeUpdate(totS-curS, totS);
    }
  }
}

void PianoBarProcess::ProcStateChanged(QProcess::ProcessState stat){
  if(stat == QProcess::NotRunning){ cState = PianoBarProcess::Stopped; }
  else{ cState = PianoBarProcess::Paused; }
  emit currentStateChanged(cState);
}

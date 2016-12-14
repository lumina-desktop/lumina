//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "7ZBackend.h"
#include <QFile>
#include <QDebug>

ZBackend::ZBackend(QObject *parent) : QObject(parent){
  //Setup the backend process
  PROC.setProcessChannelMode(QProcess::MergedChannels);
  PROC.setProgram("7z");
  connect(&PROC, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished(int, QProcess::ExitStatus)) );
  connect(&PROC, SIGNAL(readyReadStandardOutput()), this, SLOT(processData()) );
  connect(&PROC, SIGNAL(started()), this, SIGNAL(ProcessStarting()) );
  LIST = STARTING = false;
}

ZBackend::~ZBackend(){

}

//===============
//        PUBLIC
//===============
void ZBackend::loadFile(QString path, QString pass){
  filepath = path;
  flags.clear();
  //contents.clear();
  //Now determine the flags which are used for this type of file (for later use)
  if(!pass.isEmpty()){ flags << "-p"+pass; } //add password entry
  //Look for some special file extensions which 7z can't seem to detect itself
  QStringList ext = path.section("/",-1).section(".",1,-1).split(".");
  if(ext.contains("tar")){
    flags << "-ttar";
  }
  flags << filepath; //add the actual archive path
  if(QFile::exists(path)){ startList(); }
  else{ contents.clear(); emit ProcessFinished(); }
}

QString ZBackend::currentFile(){
  return filepath;
}

bool ZBackend::isWorking(){
  return (PROC.state() != QProcess::Running);
}

//Listing routines
QStringList ZBackend::heirarchy(){
  return contents.keys();
}

double ZBackend::size(QString file){
  if(!contents.contains(file)){ return -1; }
  return contents.value(file)[1].toDouble();
}

double ZBackend::csize(QString file){
  if(!contents.contains(file)){ return -1; }
  return contents.value(file)[2].toDouble();
}

bool ZBackend::isDir(QString file){
  if(!contents.contains(file)){ return false; }
  return contents.value(file)[0].startsWith("D");
}

//Modification routines
void ZBackend::startAdd(QStringList paths){
  QStringList args;
  args << "a" << flags << paths;
  STARTING=true;
  PROC.start("7z", args);
}

void ZBackend::startRemove(QStringList paths){
  QStringList args;
  args << "d" << flags << paths;
  STARTING=true;
  PROC.start("7z", args);
}

void ZBackend::startExtract(QString path, bool preservepaths){
  QStringList args;
  args << (preservepaths ? "x" : "e") << flags << "-o"+path;
  STARTING=true;
  PROC.start("7z", args);
}

//===============
//   PUBLIC SLOTS
//===============

//===============
//       PRIVATE
//===============
void ZBackend::parseLines(QStringList lines){
  if(STARTING){
    //Ignore all the p7zip header info
    while(STARTING && !lines.isEmpty()){
      if(lines[0]=="--"){ STARTING = false; } //found the end of the headers
      lines.removeAt(0);
    }
  }
  for(int i=0; i<lines.length(); i++){
    if(lines[i].simplified().isEmpty() || lines[i].contains(" = ") || lines[i].contains("----") ){ continue; }
    if(LIST){
      if(lines[i].contains("  Attr  ") || lines[i].contains(", ") ){ continue; } //headers
      QStringList info = lines[i].split(" ",QString::SkipEmptyParts);
      //Format: [date, time, attr, size, compressed size, name]
      if(info.length()<6){ continue; } //invalid line
        while(info.length()>6){ info[5] = info[5]+" "+info[6]; info.removeAt(6); } //Filename has spaces in it
        contents.insert(info[5], QStringList() << info[2] << info[3] << info[4]); //Save the 
    }
  }
}

void ZBackend::startList(){
  contents.clear();
  QStringList args;
  args << "l"; //list (lowercase L)
  LIST = STARTING=true;
  PROC.start("7z", QStringList() << args << flags);
}

//===============
//  PRIVATE SLOTS
//===============
void ZBackend::procFinished(int retcode, QProcess::ExitStatus){
  processData();
  //qDebug() << "Process Finished:" << PROC.arguments() << retcode;
  LIST = STARTING = false;
  if(PROC.arguments().contains("l")){
    emit ProcessFinished();
  }else{
    if(retcode==0){
      QStringList args = PROC.arguments();
      for(int i=0; i<args.length(); i++){
        if(args[i].startsWith("-o")){ QProcess::startDetached("xdg-open \""+ args[i].section("-o",1,-1)+"\""); } //just extracted to a dir - open it now
      }
    }
    startList();
  }
}

void ZBackend::processData(){
  //Read the process 
  static QString data;
  QString read = data+PROC.readAllStandardOutput();
  if(read.endsWith("\n")){ data.clear(); }
  else{ data = read.section("\n",-1); read = read.section("\n",0,-2); }
  QStringList lines =  read.split("\n",QString::SkipEmptyParts);
  if(LIST){ parseLines(lines); }
  //qDebug() << lines;
  emit ProgressUpdate(-1, "");
}

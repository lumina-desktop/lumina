//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TarBackend.h"
#include <QFile>
#include <QDebug>

Backend::Backend(QObject *parent) : QObject(parent){
  //Setup the backend process
  PROC.setProcessChannelMode(QProcess::MergedChannels);
  PROC.setProgram("7z");
  connect(&PROC, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished(int, QProcess::ExitStatus)) );
  connect(&PROC, SIGNAL(readyReadStandardOutput()), this, SLOT(processData()) );
  connect(&PROC, SIGNAL(started()), this, SIGNAL(ProcessStarting()) );
  LIST = STARTING = false;
}

Backend::~Backend(){

}

//===============
//        PUBLIC
//===============
void Backend::loadFile(QString path){
  filepath = path;
  tmpfilepath = filepath.section("/",0,-2)+"/"+".tmp_larchiver_"+filepath.section("/",-1);
  flags.clear();
  flags << "-f" << filepath; //add the actual archive path
  if(QFile::exists(path)){ startList(); }
  else{ contents.clear(); emit ProcessFinished(); }
}

bool Backend::canModify(){
  static QStringList validEXT; 
  if(validEXT.isEmpty()){ 
    validEXT << ".zip" << ".tar.gz" << ".tgz" << ".tar.xz" << ".txz" << ".tar.bz" << ".tbz" << ".tar.bz2" << ".tbz2" << ".tar" \
      << ".tar.lzma" << ".tlz" << ".cpio" << ".pax" << ".ar" << ".shar" << ".7z";
  }
  for(int i=0; i<validEXT.length(); i++){
    if(filepath.endsWith(validEXT[i])){ return true; }
  }
  return false;
}

QString Backend::currentFile(){
  return filepath;
}

bool Backend::isWorking(){
  return (PROC.state() != QProcess::Running);
}

//Listing routines
QStringList Backend::heirarchy(){
  return contents.keys();
}

double Backend::size(QString file){
  if(!contents.contains(file)){ return -1; }
  return contents.value(file)[1].toDouble();
}

double Backend::csize(QString file){
  if(!contents.contains(file)){ return -1; }
  return contents.value(file)[1].toDouble();
}

bool Backend::isDir(QString file){
  if(!contents.contains(file)){ return false; }
  return contents.value(file)[0].startsWith("d");
}

//Modification routines
void Backend::startAdd(QStringList paths){
  //NOTE: All the "paths" have to have the same parent directory
  if(paths.contains(filepath)){ paths.removeAll(filepath); }
  if(paths.isEmpty()){ return; }
  QStringList args;
  args << "-c" << "-a";
  args << flags;
  //Now setup the parent dir 
  QString parent = paths[0].section("/",0,-2);
  for(int i=0; i<paths.length(); i++){  
    paths[i] = paths[i].section(parent,1,-1); 
    if(paths[i].startsWith("/")){ paths[i].remove(0,1); }
  }
  args << "-C" << parent;
  args << paths;
  if(QFile::exists(filepath)){ //append to existing
    args.replaceInStrings(filepath, tmpfilepath); 
    args<< "@"+filepath; 
  } 
  STARTING=true;
  PROC.start("tar", args);
}

void Backend::startRemove(QStringList paths){
  if(paths.contains(filepath)){ paths.removeAll(filepath); }
  if(contents.isEmpty() || paths.isEmpty() || !QFile::exists(filepath)){ return; } //invalid
  QStringList args;
  args << "-c" << "-a";
  args << flags;
  args.replaceInStrings(filepath, tmpfilepath); 
  //Add the include rules for all the files we want to keep (no exclude option in "tar")
  for(int i=0; i<paths.length(); i++){
     args << "--exclude" << paths[i];
  }
  args<< "@"+filepath; 
  STARTING=true;
  PROC.start("tar", args);
}

void Backend::startExtract(QString path, bool overwrite){
  QStringList args;
  args << "-x";
  if(!overwrite){ args << "-k"; }
  args << flags << "-C" << path;
  STARTING=true;
  qDebug() << "Starting command:" << "tar" << args;
  PROC.start("tar", args);
}

//===============
//   PUBLIC SLOTS
//===============

//===============
//       PRIVATE
//===============
void Backend::parseLines(QStringList lines){
  for(int i=0; i<lines.length(); i++){
      QStringList info = lines[i].split(" ",QString::SkipEmptyParts);
      //Format: [perms, ?, user, group, size, month, day, time, file]
      if(info.startsWith("x ") && filepath.endsWith(".zip")){
        //ZIP archives do not have all the extra information - just filenames
        while(info.length()>2){ info[1]=info[1]+" "+info[2]; }
        QString file = info[1]; 
        QString perms = "";
         if(file.endsWith("/")){ perms = "d"; file.chop(1); }
        contents.insert(file, QStringList() << perms << "-1" ); //Save the [perms, size ]
      }
      else if(info.length()<9){ continue; } //invalid line
      //TAR Archive parsing
        while(info.length()>9){ info[8] = info[8]+" "+info[9]; info.removeAt(9); } //Filename has spaces in it
         QString file = info[8]; 
         if(file.endsWith("/")){ file.chop(1); }
        contents.insert(file, QStringList() << info[0] << info[4] ); //Save the [perms, size ]
  }
}

void Backend::startList(){
  contents.clear();
  QStringList args;
  args << "-tv";
  LIST = STARTING=true;
  PROC.start("tar", QStringList() << args << flags);
}

//===============
//  PRIVATE SLOTS
//===============
void Backend::procFinished(int retcode, QProcess::ExitStatus){
  processData();
  qDebug() << "Process Finished:" << PROC.arguments() << retcode;
  LIST = STARTING = false;
  if(PROC.arguments().contains("-tv")){
    if(retcode!=0){ contents.clear(); } //could not read archive
    emit ProcessFinished();
  }else{
    bool needupdate = true;
    QStringList args = PROC.arguments();
    if(args.contains("-x") && retcode==0){
       needupdate=false; 
      QProcess::startDetached("xdg-open \""+ args.last()+"\""); //just extracted to a dir - open it now
    }else if(args.contains("-c") && QFile::exists(tmpfilepath)){
      if(retcode==0){
        QFile::remove(filepath);
        QFile::rename(tmpfilepath, filepath);
      }else{
        QFile::remove(tmpfilepath);
      }
    }
    if(needupdate){ startList(); }
    else{ emit ProcessFinished(); }
  }
}

void Backend::processData(){
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

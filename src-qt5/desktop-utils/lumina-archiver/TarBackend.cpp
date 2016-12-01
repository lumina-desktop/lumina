//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TarBackend.h"
#include <QFile>
#include <QDir>
#include <QDebug>

Backend::Backend(QObject *parent) : QObject(parent){
  //Setup the backend process
  PROC.setProcessChannelMode(QProcess::MergedChannels);
  PROC.setProgram("tar");
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
  else{ contents.clear(); emit ProcessFinished(true, ""); }
}

bool Backend::canModify(){
  static QStringList validEXT; 
  if( validEXT.isEmpty() ){ 
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

bool Backend::isLink(QString file){
  if(!contents.contains(file)){ return false; }
  return contents.value(file)[0].startsWith("l");
}

QString Backend::linkTo(QString file){
  if(!contents.contains(file)){ return ""; }
  return contents.value(file)[2];
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

void Backend::startExtract(QString path, bool overwrite, QString file){
  startExtract(path, overwrite, QStringList() << file); //overload for multi-file function
}

void Backend::startExtract(QString path, bool overwrite, QStringList files){
  QStringList args;
  args << "-x" << "--no-same-owner";
  if(!overwrite){ args << "-k"; }
  args << flags;
  for(int i=0; i<files.length(); i++){
    if(files[i].simplified().isEmpty()){ continue; }
    args << "--include" << files[i] << "--strip-components" << QString::number(files[i].count("/"));
  }
  args << "-C" << path;
  STARTING=true;
  //qDebug() << "Starting command:" << "tar" << args;
  PROC.start("tar", args);
}

void Backend::startViewFile(QString path){
  QStringList args;
  args << "-x";
  args << flags <<"--include" << path <<"--strip-components" << QString::number(path.count("/")) << "-C" << QDir::tempPath();
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
        contents.insert(file, QStringList() << perms << "-1" <<""); //Save the [perms, size, linkto ]
      }
      else if(info.length()<9){ continue; } //invalid line
      //TAR Archive parsing
        while(info.length()>9){ info[8] = info[8]+" "+info[9]; info.removeAt(9); } //Filename has spaces in it
         QString file = info[8]; 
         if(file.endsWith("/")){ file.chop(1); }
         QString linkto;
         //See if this file has the "link to" or "->"  notation
         if(file.contains(" -> ")){ linkto = file.section(" -> ",1,-1); file = file.section(" -> ",0,0); }
         else if(file.contains(" link to ")){
            //Special case - alternate form of a link within a tar archive (not reflected in perms)
           linkto = file.section(" link to ",1,-1);
           file = file.section(" link to ",0,0);
           if(info[0].startsWith("-")){ info[0].replace(0,1,"l"); }
         }
        contents.insert(file, QStringList() << info[0] << info[4] << linkto); //Save the [perms, size, linkto ]
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
  static QString result;
  processData();
  //qDebug() << "Process Finished:" << PROC.arguments() << retcode;
  LIST = STARTING = false;
  if(PROC.arguments().contains("-tv")){
    if(retcode!=0){ contents.clear(); } //could not read archive
    emit ProcessFinished(true,result);
    result.clear();
  }else{
    bool needupdate = true;
    QStringList args = PROC.arguments();
    if(args.contains("-x") && retcode==0){
       needupdate=false;
      if(args.count("--include")==1){
        //Need to find the full path to the (single) extracted file
        QString path = args.last() +"/"+ args[ args.indexOf("--include")+1].section("/",-1);
        QFile::setPermissions(path, QFileDevice::ReadOwner);
        QProcess::startDetached("xdg-open  \""+path+"\"");
      }else{
        //Multi-file extract - open the dir instead
        QProcess::startDetached("xdg-open \""+ args.last()+"\""); //just extracted to a dir - open it now
      }
    }else if(args.contains("-c") && QFile::exists(tmpfilepath)){
      if(retcode==0){
        QFile::remove(filepath);
        QFile::rename(tmpfilepath, filepath);
      }else{
        QFile::remove(tmpfilepath);
      }
    }
    if(args.contains("-x")){ result = tr("Extraction Finished"); }
    else if(args.contains("-c")){ result = tr("Modification Finished"); }
    if(needupdate){ startList(); }
    else{ emit ProcessFinished(retcode==0, result); result.clear(); }
  }
}

void Backend::processData(){
  //Read the process 
  static QString data;
  QString read = data+PROC.readAllStandardOutput();
  if(read.endsWith("\n")){ data.clear(); }
  else{ data = read.section("\n",-1); read = read.section("\n",0,-2); }
  QStringList lines =  read.split("\n",QString::SkipEmptyParts);
  QString info;
  if(LIST){ parseLines(lines); }
  else if(!lines.isEmpty()){ info = lines.last(); }
  //qDebug() << lines;
  emit ProgressUpdate(-1, info);
}

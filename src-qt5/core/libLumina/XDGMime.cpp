//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "XDGMime.h"
#include <LUtils.h>
#include <LuminaOS.h>

static QStringList mimeglobs;
static qint64 mimechecktime;

QString XDGMime::fromFileName(QString filename){
  if(QFile::exists(filename) && QFileInfo(filename).isDir()){
    return "inode/directory";
  }
  //Convert a filename into a mimetype
  return findAppMimeForFile(filename.section("/",-1),false);
}

QStringList XDGMime::listFromFileName(QString filename){
  //Convert a filename into a list of mimetypes (arranged in descending priority)
  return findAppMimeForFile(filename.section("/",-1),true).split("::::");
}
QString XDGMime::toIconName(QString mime){
  if(!mime.contains("/") || mime.isEmpty() ){ return "unknown"; } //not a mime type
  //Mime type to icon name
  mime.replace("/","-"); //translate to icon mime name
  return mime;
}


QStringList XDGMime::toFileExtensions(QString mime){
  QStringList out;
  QStringList mimes = XDGMime::loadMimeFileGlobs2().filter(mime);
  for(int i=0; i<mimes.length(); i++){
    out << mimes[i].section(":",2,2); // "*.<extension>"
  }
  //qDebug() << "Mime to Files:" << mime << out;
  return out;
}

// ====================
//   BACKEND FUNCTIONS
// ====================
QStringList XDGMime::systemMimeDirs(){
  //Returns a list of all the directories where *.xml MIME files can be found
  QStringList appDirs = QString(getenv("XDG_DATA_HOME")).split(":");
  appDirs << QString(getenv("XDG_DATA_DIRS")).split(":");
  if(appDirs.isEmpty()){ appDirs << "/usr/local/share" << "/usr/share"; }
  //Now create a valid list
  QStringList out;
  for(int i=0; i<appDirs.length(); i++){
    if( QFile::exists(appDirs[i]+"/mime") ){
      out << appDirs[i]+"/mime";
    }
  }
  return out;
}

QString XDGMime::findAppMimeForFile(QString filename, bool multiple){
  QString out;
  QString extension = filename.section(".",1,-1);
  if("."+extension == filename){ extension.clear(); } //hidden file without extension
  //qDebug() << "MIME SEARCH:" << filename << extension;
  QStringList mimefull = XDGMime::loadMimeFileGlobs2();
  QStringList mimes;
  //Just in case the filename is a mimetype itself
  if( mimefull.filter(":"+filename+":").length() == 1){
    return filename;
  }
while(mimes.isEmpty()){
  //Check for an exact mimetype match
  if(mimefull.filter(":"+extension+":").length() == 1){
    return extension;
  }
  //Look for globs at the end of the filename
  if(!extension.isEmpty()){
    mimes = mimefull.filter(":*."+extension);
    //If nothing found, try a case-insensitive search
    if(mimes.isEmpty()){ mimes = mimefull.filter(":*."+extension, Qt::CaseInsensitive); }
    //Now ensure that the filter was accurate (*.<extention>.<something> will still be caught)
    for(int i=0; i<mimes.length(); i++){
      if(!filename.endsWith( mimes[i].section(":*",-1), Qt::CaseInsensitive )){ mimes.removeAt(i); i--; }
      else if(mimes[i].section(":",0,0).length()==2){ mimes[i].prepend("0"); } //ensure 3-character priority number
      else if(mimes[i].section(":",0,0).length()==1){ mimes[i].prepend("00"); } //ensure 3-character priority number
    }
  }
  //Look for globs at the start of the filename
  if(mimes.isEmpty()){
    mimes = mimefull.filter(":"+filename.left(2)); //look for the first 2 characters initially
	//Note: This initial filter will only work if the wildcard (*) is not within the first 2 characters of the pattern
    //Now ensure that the filter was accurate
    for(int i=0; i<mimes.length(); i++){
      if(!filename.startsWith( mimes[i].section(":",3,-1,QString::SectionSkipEmpty).section("*",0,0), Qt::CaseInsensitive )){ mimes.removeAt(i); i--; }
    }
  }
    if(mimes.isEmpty()){
      if(extension.contains(".")){ extension = extension.section(".",1,-1); }
      else{ break; }
    }
  } //end of mimes while loop
  mimes.sort(); //this automatically puts them in reverse weight order (100 on down)
  QStringList matches;
  //qDebug() << "Mimes:" << mimes;
  for(int m=mimes.length()-1; m>=0; m--){
    QString mime = mimes[m].section(":",1,1,QString::SectionSkipEmpty);
    matches << mime;
  }
  //qDebug() << "Matches:" << matches;
  if(multiple && !matches.isEmpty() ){ out = matches.join("::::"); }
  else if( !matches.isEmpty() ){ out = matches.first(); }
  else{ //no mimetype found - assign one (internal only - no system database changes)
    if(extension.isEmpty()){ out = "unknown/"+filename.toLower(); }
    else{ out = "unknown/"+extension.toLower(); }
  }
  //qDebug() << "Out:" << out;
  return out;
}

QStringList XDGMime::listFileMimeDefaults(){
  //This will spit out a itemized list of all the mimetypes and relevant info
  // Output format: <mimetype>::::<extension>::::<default>::::<localized comment>
  QStringList mimes = XDGMime::loadMimeFileGlobs2();
  //Now start filling the output list
  QStringList out;
  for(int i=0; i<mimes.length(); i++){
    QString mimetype = mimes[i].section(":",1,1);
    QStringList tmp = mimes.filter(mimetype);
    //Collect all the different extensions with this mimetype
    QStringList extlist;
    for(int j=0; j<tmp.length(); j++){
      mimes.removeAll(tmp[j]);
      extlist << tmp[j].section(":",2,2);
    }
    extlist.removeDuplicates(); //just in case
    //Now look for a current default for this mimetype
    QString dapp = XDGMime::findDefaultAppForMime(mimetype); //default app;

    //Create the output entry
    //qDebug() << "Mime entry:" << i << mimetype << dapp;
    out << mimetype+"::::"+extlist.join(", ")+"::::"+dapp+"::::"+XDGMime::findMimeComment(mimetype);

    i--; //go back one (continue until the list is empty)
  }
  return out;
}

QString XDGMime::findMimeComment(QString mime){
  QString comment;
  QStringList dirs = XDGMime::systemMimeDirs();
  QString lang = QString(getenv("LANG")).section(".",0,0);
  QString shortlang = lang.section("_",0,0);
  for(int i=0; i<dirs.length(); i++){
    if(QFile::exists(dirs[i]+"/"+mime+".xml")){
      QStringList info = LUtils::readFile(dirs[i]+"/"+mime+".xml");
      QStringList filter = info.filter("<comment xml:lang=\""+lang+"\">");
      //First look for a full language match, then short language, then general comment
      if(filter.isEmpty()){ filter = info.filter("<comment xml:lang=\""+shortlang+"\">"); }
      if(filter.isEmpty()){ filter = info.filter("<comment>"); }
      if(!filter.isEmpty()){
        comment = filter.first().section(">",1,1).section("</",0,0);
        break;
      }
    }
  }
  return comment;
}

QString XDGMime::findDefaultAppForMime(QString mime){
  //First get the priority-ordered list of default file locations
  QStringList dirs;
  dirs << QString(getenv("XDG_CONFIG_HOME"))+"/lumina-mimeapps.list" \
	 << QString(getenv("XDG_CONFIG_HOME"))+"/mimeapps.list";
  QStringList tmp = QString(getenv("XDG_CONFIG_DIRS")).split(":");
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/lumina-mimeapps.list"; }
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/mimeapps.list"; }
  dirs << QString(getenv("XDG_DATA_HOME"))+"/applications/lumina-mimeapps.list" \
	 << QString(getenv("XDG_DATA_HOME"))+"/applications/mimeapps.list";
  tmp = QString(getenv("XDG_DATA_DIRS")).split(":");
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/applications/lumina-mimeapps.list"; }
	for(int i=0; i<tmp.length(); i++){ dirs << tmp[i]+"/applications/mimeapps.list"; }

  //Now go through all the files in order of priority until a default is found
  QString cdefault;
  for(int i=0; i<dirs.length() && cdefault.isEmpty(); i++){
    if(!QFile::exists(dirs[i])){ continue; }
    QStringList info = LUtils::readFile(dirs[i]);
    if(info.isEmpty()){ continue; }
    QStringList white; //lists to keep track of during the search (black unused at the moment)
    QString workdir = dirs[i].section("/",0,-2); //just the directory
   // qDebug() << "Check File:" << mime << dirs[i] << workdir;
    int def = info.indexOf("[Default Applications]"); //find this line to start on
    if(def>=0){
      for(int d=def+1; d<info.length(); d++){
        //qDebug() << "Check Line:" << info[d];
        if(info[d].startsWith("[")){ break; } //starting a new section now - finished with defaults
	if(info[d].contains(mime+"=") ){
	  white = info[d].section("=",1,-1).split(";") + white; //exact mime match - put at front of list
          break; //already found exact match
	}else if(info[d].contains("*") && info[d].contains("=") ){
          QRegExp rg(info[d].section("=",0,0), Qt::CaseSensitive, QRegExp::WildcardUnix);
          if(rg.exactMatch(mime)){
	    white << info[d].section("=",1,-1).split(";"); //partial mime match - put at end of list
          }
        }
      }
    }
    // Now check for any white-listed files in this work dir
    // find the full path to the file (should run even if nothing in this file)
    //qDebug() << "WhiteList:" << white;
    for(int w=0; w<white.length(); w++){
      if(white[w].isEmpty()){ continue; }
      //First check for absolute paths to *.desktop file
      if( white[w].startsWith("/") ){
	 if( QFile::exists(white[w]) ){ cdefault=white[w]; break; }
	 else{ white.removeAt(w); w--; } //invalid file path - remove it from the list
      }
      //Now check for relative paths to  file (in current priority-ordered work dir)
      else if( QFile::exists(workdir+"/"+white[w]) ){ cdefault=workdir+"/"+white[w]; break; }
      //Now go through the XDG DATA dirs and see if the file is in there
      else{
        white[w] = LUtils::AppToAbsolute(white[w]);
        if(QFile::exists(white[w])){ cdefault = white[w]; }
      }
    }
    /* WRITTEN BUT UNUSED CODE FOR MIMETYPE ASSOCIATIONS
    //Skip using this because it is simply an alternate/unsupported standard that conflicts with
      the current mimetype database standards. It is better/faster to parse 1 or 2 database glob files
      rather than have to iterate through hundreds of *.desktop files *every* time you need to
      find an application

    if(addI<0 && remI<0){
      //Simple Format: <mimetype>=<*.desktop file>;<*.desktop file>;.....
        // (usually only one desktop file listed)
      info = info.filter(mimetype+"=");
      //Load the listed default(s)
      for(int w=0; w<info.length(); w++){
        white << info[w].section("=",1,50).split(";");
      }
    }else{
      //Non-desktop specific mimetypes file: has a *very* convoluted/inefficient algorithm (required by spec)
      if(addI<0){ addI = info.length(); } //no add section
      if(remI<0){ remI = info.length(); } // no remove section:
      //Whitelist items
      for(int a=addI+1; a!=remI && a<info.length(); a++){
        if(info[a].contains(mimetype+"=")){
	  QStringList tmp = info[a].section("=",1,50).split(";");
	  for(int t=0; t<tmp.length(); t++){
	    if(!black.contains(tmp[t])){ white << tmp[t]; } //make sure this item is not on the black list
	  }
	  break;
	}
      }
      //Blacklist items
      for(int a=remI+1; a!=addI && a<info.length(); a++){
        if(info[a].contains(mimetype+"=")){ black << info[a].section("=",1,50).split(";"); break;}
      }

      //STEPS 3/4 not written yet

    } //End of non-DE mimetypes file */

  } //End loop over file
  return cdefault;
}

QStringList XDGMime::findAvailableAppsForMime(QString mime){
  QStringList dirs = LUtils::systemApplicationDirs();
  QStringList out;
  //Loop over all possible directories that contain *.destop files
  //  and check for the mimeinfo.cache file
  for(int i=0; i<dirs.length(); i++){
    if(QFile::exists(dirs[i]+"/mimeinfo.cache")){
      QStringList matches = LUtils::readFile(dirs[i]+"/mimeinfo.cache").filter(mime+"=");
      //Find any matches for our mimetype in the cache
      for(int j=0; j<matches.length(); j++){
        QStringList files = matches[j].section("=",1,1).split(";",QString::SkipEmptyParts);
	//Verify that each file exists before putting the full path to the file in the output
	for(int m=0; m<files.length(); m++){
	  if(QFile::exists(dirs[i]+"/"+files[m])){
	    out << dirs[i]+"/"+files[m];
	  }else if(files[m].contains("-")){ //kde4-<filename> -> kde4/<filename> (stupid KDE variations!!)
	    files[m].replace("-","/");
	    if(QFile::exists(dirs[i]+"/"+files[m])){
	      out << dirs[i]+"/"+files[m];
	    }
	  }
	}
      }
    }
  }
  //qDebug() << "Found Apps for Mime:" << mime << out << dirs;
  return out;
}

void XDGMime::setDefaultAppForMime(QString mime, QString app){
  QString filepath = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-mimeapps.list";
  QStringList cinfo = LUtils::readFile(filepath);
  //If this is a new file, make sure to add the header appropriately
  if(cinfo.isEmpty()){ cinfo << "#Automatically generated with lumina-config" << "# DO NOT CHANGE MANUALLY" << "[Default Applications]"; }
  //Check for any current entry for this mime type
  QStringList tmp = cinfo.filter(mime+"=");
  int index = -1;
  if(!tmp.isEmpty()){ index = cinfo.indexOf(tmp.first()); }
  //Now add the new default entry (if necessary)
  if(app.isEmpty()){
    if(index>=0){ cinfo.removeAt(index); } //Remove entry
  }else{
    if(index<0){
      cinfo << mime+"="+app+";"; //new entry
    }else{
      cinfo[index] = mime+"="+app+";"; //overwrite existing entry
    }
  }
  LUtils::writeFile(filepath, cinfo, true);
  return;
}

QStringList XDGMime::findAVFileExtensions(){
  //output format: QDir name filter for valid A/V file extensions
  QStringList globs = XDGMime::loadMimeFileGlobs2();
  QStringList av = globs.filter(":audio/");
  av << globs.filter(":video/");
  for(int i=0; i<av.length(); i++){
    //Just use all audio/video mimetypes (for now)
    av[i] = av[i].section(":",2,2);
    //Qt5 Auto detection (broken - QMediaPlayer seg faults with Qt 5.3 - 11/24/14)
    /*if( QMultimedia::NotSupported != QMediaPlayer::hasSupport(av[i].section(":",1,1)) ){ av[i] = av[i].section(":",2,2); }
    else{ av.removeAt(i); i--; }*/
  }
  av.removeDuplicates();
  return av;
}

QStringList XDGMime::loadMimeFileGlobs2(){
  //output format: <weight>:<mime type>:<file extension (*.something)>
  if(mimeglobs.isEmpty() || (mimechecktime < (QDateTime::currentMSecsSinceEpoch()-30000)) ){
    //qDebug() << "Loading globs2 mime DB files";
    mimeglobs.clear();
    mimechecktime = QDateTime::currentMSecsSinceEpoch(); //save the current time this was last checked
    QStringList dirs = XDGMime::systemMimeDirs();
    for(int i=0; i<dirs.length(); i++){
      if(QFile::exists(dirs[i]+"/globs2")){
        QFile file(dirs[i]+"/globs2");
        if(!file.exists()){ continue; }
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){ continue; }
        QTextStream in(&file);
        while(!in.atEnd()){
          QString line = in.readLine();
          if(!line.startsWith("#")){
            mimeglobs << line.simplified();
          }
        }
	file.close();
      }
      if(i==dirs.length()-1 && mimeglobs.isEmpty()){
        //Could not find the mimetype database on the system - use the fallback file distributed with Lumina
        if(dirs.last()!= LOS::LuminaShare()){ dirs << LOS::LuminaShare(); } //just in case it was not installed
      }
    }//end loop over dirs
  }
  return mimeglobs;
}

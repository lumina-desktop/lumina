//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "syntaxSupport.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QFontDatabase>
#include <QFont>

#include <LUtils.h>

// ================
//  SYNTAX FILE CLASS
// ================
QColor SyntaxFile::colorFromOption(QString opt, QSettings *settings){
  opt = opt.simplified();
  if(opt.startsWith("rgb(")){
    QStringList opts = opt.section("(",1,-1).section(")",0,0).split(",");
    if(opts.length()!=3){ return QColor(); }
    return QColor( opts[0].simplified().toInt(), opts[1].simplified().toInt(), opts[2].simplified().toInt() );
  }else if(opt.startsWith("#")){
    return QColor(opt);
  }else if(opt.startsWith("colors/")){
    return QColor(settings->value(opt,"").toString());
  }
  return QColor("transparent");
}

QString SyntaxFile::name(){
  if(!metaObj.contains("name")){ return ""; }
  return metaObj.value("name").toString();
}

int SyntaxFile::char_limit(){
  if(!formatObj.contains("columns_per_line")){ return -1; }
  int num = formatObj.value("columns_per_line").toInt();
  return num;
}

bool SyntaxFile::highlight_excess_whitespace(){
  if(!formatObj.contains("highlight_whitespace_eol")){ return false; }
  return formatObj.value("highlight_whitespace_eol").toBool();
}

bool SyntaxFile::check_spelling(){
  if(!formatObj.contains("check_spelling")){ return false; }
  return formatObj.value("check_spelling").toBool();
}

int SyntaxFile::tab_length(){
  int num = -1;
  if(formatObj.contains("tab_width")){ num = formatObj.value("tab_width").toInt(); }
  if(num<=0){ num = 8; } //UNIX Standard of 8 characters per tab
  return num;
}

void SyntaxFile::SetupDocument(QPlainTextEdit* editor){
  if(formatObj.contains("line_wrap")){
    editor->setLineWrapMode( formatObj.value("line_wrap").toBool() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
  }
  if(formatObj.contains("font_type")){
    QString type = formatObj.value("font_type").toString();
    QFont font = editor->document()->defaultFont(); // current font
    if(type=="monospace"){
      font.setFamily("monospace"); //Make sure we don't change other properties of the font like size
    }
    font.setStyle(QFont::StyleNormal);
    font.setStyleStrategy(QFont::PreferAntialias);
    editor->document()->setDefaultFont(font);
  }
  if(formatObj.contains("tab_width")){
    int num = formatObj.value("tab_width").toInt();
    if(num<=0){ num = 8; } //UNIX Standard of 8 characters per tab
    editor->setTabStopWidth( num * QFontMetrics(editor->document()->defaultFont()).width(" ") );
  }
}

bool SyntaxFile::supportsFile(QString file){
  if(metaObj.contains("file_suffix")){
    return metaObj.value("file_suffix").toArray().contains( file.section("/",-1).section(".",-1) );
  }else if(metaObj.contains("file_regex")){
    return (QRegExp( metaObj.value("file_regex").toString() ).indexIn(file.section("/",-1) ) >=0 );
  }
  return false;
}

bool SyntaxFile::supportsFirstLine(QString line){
  line = line.simplified();
  if(metaObj.contains("first_line_match")){
    return metaObj.value("first_line_match").toArray().contains(line);
  }else if(metaObj.contains("first_line_regex")){
    return (QRegExp( metaObj.value("first_line_regex").toString() ).indexIn(line) >=0 );
  }
  return false;
}

bool SyntaxFile::LoadFile(QString file, QSettings *settings){
  QStringList contents = LUtils::readFile(file);
  //Now trim the extra non-JSON off the beginning of the file
  while(!contents.isEmpty()){
    if(contents[0].startsWith("{")){ break; } //stop here
    else{ contents.removeAt(0); }
  }
  //qDebug() << "Contents:" << contents.join("\n").simplified();
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(contents.join("\n").simplified().toLocal8Bit(), &err );
  if(doc.isNull()){ qDebug() << "JSON Syntax Error:" << err.errorString(); }
  QJsonObject obj = doc.object();
  if(!obj.contains("meta") || !obj.contains("format") || !obj.contains("rules")){ qDebug() << "Could not read syntax file:" << file; return false; } //could not get any info
  //Save the raw meta/format objects for later
  fileLoaded = file;
  metaObj = obj.value("meta").toObject();
  formatObj = obj.value("format").toObject();
  //Now read/save the rules structure
  QJsonArray rulesArray = obj.value("rules").toArray();
  rules.clear();
  //Create the blank/generic text format
  for(int i=0; i<rulesArray.count(); i++){
    QJsonObject rule = rulesArray[i].toObject();
    SyntaxRule tmp;
    //First load the rule
    //qDebug() << "Load Rule:" << rule.keys();
    if(rule.contains("words")){} //valid option - handled at the end though
    else if(rule.contains("regex")){
      tmp.pattern = QRegExp(rule.value("regex").toString());
    }else if(rule.contains("regex_start") && rule.contains("regex_end")){
      tmp.startPattern = QRegExp(rule.value("regex_start").toString());
      tmp.endPattern = QRegExp(rule.value("regex_end").toString());
      //qDebug() << " -- Multi-line Rule:" << tmp.startPattern << tmp.endPattern;
    }else{ continue; } //bad rule - missing the actual detection logic
    //Now load the appearance logic
    if(rule.contains("foreground")){ tmp.format.setForeground( colorFromOption(rule.value("foreground").toString(), settings) ); }
    if(rule.contains("background")){ tmp.format.setBackground( colorFromOption(rule.value("background").toString(), settings) ); }
    if(rule.contains("font_weight")){
      QString wgt = rule.value("font_weight").toString();
      if(wgt =="bold"){ tmp.format.setFontWeight(QFont::Bold); }
      else if(wgt =="light"){ tmp.format.setFontWeight(QFont::Light); }
      else{ tmp.format.setFontWeight(QFont::Normal); }
    }
    if(rule.contains("font_style")){
      if(rule.value("font_style").toString()=="italic"){ tmp.format.setFontItalic(true); }
    }

    //Now save the rule(s) to the list
    if(rule.contains("words")){
      //special logic - this generates a bunch of rules all at once (one per word)
      QJsonArray tmpArr = rule.value("words").toArray();
      for(int a=0; a<tmpArr.count(); a++){
        tmp.pattern = QRegExp("\\b"+tmpArr[a].toString()+"\\b"); //turn each keyword into a QRegExp and insert the rule
        rules << tmp;
      }
    }else{ rules << tmp; } //just a single rule
  }
  return true;
}

//Main function for finding/loading all syntax files
QList<SyntaxFile> SyntaxFile::availableFiles(QSettings *settings){
  static QList<SyntaxFile> list; //keep this list around between calls - prevent re-reading all the files
  //Remove/update any files from the list as needed
  QStringList found;
  for(int i=0; i<list.length(); i++){
    if( !QFileInfo::exists(list[i].fileLoaded) ){ list.removeAt(i); i--; continue; } //obsolete file
    else if(QFileInfo(list[i].fileLoaded).lastModified() > list[i].lastLoaded){ list[i].LoadFile(list[i].fileLoaded, settings); } //out-of-date file
    found << list[i].fileLoaded; //save for later
  }
  //Now scan for any new files
  QStringList paths;
  paths << QString(getenv("XDG_DATA_HOME")) << QString(getenv("XDG_DATA_DIRS")).split(":");
  for(int i=0; i<paths.length(); i++){
    QDir dir(paths[i]+"/lumina-desktop/syntax_rules");
    if(!dir.exists()){ continue; }
    //qDebug() << "Found directory:" << dir.absolutePath();
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.syntax", QDir::Files, QDir::Name);
    for(int f=0; f<files.length(); f++){
      if(paths.contains(files[f].absoluteFilePath()) ){ continue; } //already handled
      //New Syntax File found
      //qDebug() << "Found File:" << files[f].fileName();
      SyntaxFile nfile;
      if( nfile.LoadFile(files[f].absoluteFilePath(), settings) ){ list << nfile; }
    }
  }
  return list;
}

QStringList Custom_Syntax::availableRules(QSettings *settings){
  QStringList avail;
  QList<SyntaxFile> files = SyntaxFile::availableFiles(settings);
  for(int i=0; i<files.length(); i++){ avail << files[i].name(); }
  return avail;
}

QStringList Custom_Syntax::knownColors(){
  //Note: All these colors should be prefixed with "colors/" when accessing them from the settings file
  QStringList avail;
    //Standard colors
    avail << "keyword" << "altkeyword" << "class" << "text" << "function" << "comment";
    //Bracket/parenthesis/brace matching
    avail << "bracket-found" << "bracket-missing";
  return avail;
}

void Custom_Syntax::SetupDefaultColors(QSettings *settings){
  if(!settings->contains("colors/keyword")){settings->setValue("colors/keyword", QColor(Qt::blue).name() ); }
  if(!settings->contains("colors/altkeyword")){settings->setValue("colors/altkeyword", QColor(Qt::darkBlue).name() ); }
  if(!settings->contains("colors/class")){settings->setValue("colors/class", QColor(Qt::darkRed).name() ); }
  if(!settings->contains("colors/text")){settings->setValue("colors/text", QColor(Qt::darkMagenta).name() ); }
  if(!settings->contains("colors/function")){settings->setValue("colors/function", QColor(Qt::darkCyan).name() ); }
  if(!settings->contains("colors/comment")){settings->setValue("colors/comment", QColor(Qt::darkGreen).name() ); }
  if(!settings->contains("colors/bracket-found")){settings->setValue("colors/bracket-found", QColor(Qt::green).name() ); }
  if(!settings->contains("colors/bracket-missing")){settings->setValue("colors/bracket-missing", QColor(Qt::red).name() ); }
  if(!settings->contains("colors/preprocessor")){settings->setValue("colors/preprocessor", QColor(Qt::darkYellow).name() ); }
}

QString Custom_Syntax::ruleForFile(QString filename, QSettings *settings){
  QList<SyntaxFile> files = SyntaxFile::availableFiles(settings);
  for(int i=0; i<files.length(); i++){
    if(files[i].supportsFile(filename)){ return files[i].name(); }
  }
  return "";
}

QString Custom_Syntax::ruleForFirstLine(QString line, QSettings *settings){
  QList<SyntaxFile> files = SyntaxFile::availableFiles(settings);
  for(int i=0; i<files.length(); i++){
    if(files[i].supportsFirstLine(line)){ return files[i].name(); }
  }
  return "";
}

void Custom_Syntax::loadRules(QString type){
  QList<SyntaxFile> files = SyntaxFile::availableFiles(settings);
  for(int i=0; i<files.length(); i++){
    if(files[i].name() == type){ syntax = files[i]; break; }
  }
  return;
}

void Custom_Syntax::loadRules(SyntaxFile sfile){
  syntax = sfile;
}

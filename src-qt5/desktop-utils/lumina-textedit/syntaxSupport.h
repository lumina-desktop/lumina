//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SYNTAX_HIGHLIGHER_CPP_H
#define _LUMINA_SYNTAX_HIGHLIGHER_CPP_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QString>
#include <QSettings>
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>
#include <QPlainTextEdit>


//Simple syntax rules
struct SyntaxRule{
  QRegExp pattern;  //single-line rule
  QRegExp startPattern, endPattern;  //multi-line rules
  QTextCharFormat format;
};

class SyntaxFile{
private:
  QJsonObject metaObj;
  QJsonObject formatObj;

  QColor colorFromOption(QString opt, QSettings *settings);

public:
  QVector<SyntaxRule> rules;
  QDateTime lastLoaded;
  QString fileLoaded;

  SyntaxFile(){}

  QString name();
  int char_limit();
  bool highlight_excess_whitespace();
  bool check_spelling();
  int tab_length();

  void SetupDocument(QPlainTextEdit *editor);
  bool supportsFile(QString file); //does this syntax set support the file?
  bool supportsFirstLine(QString line); //is the type of file defined by the first line of the file? ("#!/bin/<something>" for instance)

  //Main Loading routine (run this before other functions)
  bool LoadFile(QString file, QSettings *settings);

  //Main function for finding/loading all syntax files
  static QList<SyntaxFile> availableFiles(QSettings *settings);
};

class Custom_Syntax : public QSyntaxHighlighter{
  Q_OBJECT
private:
  QSettings *settings;
        SyntaxFile syntax;

public:
  Custom_Syntax(QSettings *set, QTextDocument *parent = 0) : QSyntaxHighlighter(parent){
    settings = set;
  }
  ~Custom_Syntax(){}

  QString loadedRules(){ return syntax.name(); }

  static QStringList availableRules(QSettings *settings);
  static QStringList knownColors();
  static void SetupDefaultColors(QSettings *settings);
  static QString ruleForFile(QString filename, QSettings *settings);
  static QString ruleForFirstLine(QString line, QSettings *settings);
  void loadRules(QString type);
  void loadRules(SyntaxFile sfile);

  void reloadRules(){
    loadRules( syntax.name() );
  }

  void setupDocument(QPlainTextEdit *edit){ syntax.SetupDocument(edit); } //simple redirect for the function in the currently-loaded rules

protected:
  void highlightBlock(const QString &text){
          //qDebug() << "Highlight Block:" << text;
    //Now look for any multi-line patterns (starting/continuing/ending)
    int start = 0;
    int splitactive = previousBlockState();
    if(splitactive>syntax.rules.length()-1){ splitactive = -1; } //just in case

    while(start>=0 && start<=text.length()-1){
      //qDebug() << "split check:" << start << splitactive;
      if(splitactive>=0){
        //Find the end of the current rule
        int end = syntax.rules[splitactive].endPattern.indexIn(text, start);
        if(end==-1){
                //qDebug() << "Highlight to end of line:" << text << start;
          //rule did not finish - apply to all
                if(start>0){ setFormat(start-1, text.length()-start+1, syntax.rules[splitactive].format); }
                else{ setFormat(start, text.length()-start, syntax.rules[splitactive].format); }
    break; //stop looking for more multi-line patterns
        }else{
    //Found end point within the same line
                //qDebug() << "Highlight to particular point:" << text << start << end;
    int len = end-start+syntax.rules[splitactive].endPattern.matchedLength();
                if(start>0){ start--; len++; } //need to include the first character as well
    setFormat(start, len , syntax.rules[splitactive].format);
    start+=len; //move pointer to the end of handled range
    splitactive = -1; //done with this rule
        }
      } //end check for end match
      //Look for the start of any new split rules
      //qDebug() << "Loop over multi-line rules";
      for(int i=0; i<syntax.rules.length() && splitactive<0; i++){
        //qDebug() << "Check Rule:" << i << syntax.rules[i].startPattern << syntax.rules[i].endPattern;
              if(syntax.rules[i].startPattern.isEmpty()){ continue; }
              //qDebug() << "Look for start of split rule:" << syntax.rules[i].startPattern << splitactive;
        int newstart = syntax.rules[i].startPattern.indexIn(text,start);
        if(newstart>=start){
                //qDebug() << "Got Start of split rule:" << start << newstart << text;
    splitactive = i;
    start = newstart+1;
                if(start>=text.length()-1){
                  //qDebug() << "Special case: start now greater than line length";
                  //Need to apply highlighting to this section too - start matches the end of the line
                  setFormat(start-1, text.length()-start+1, syntax.rules[splitactive].format);
                }
        }
      }
      if(splitactive<0){  break; } //no other rules found - go ahead and exit the loop
          } //end scan over line length and multi-line formats

    setCurrentBlockState(splitactive);
          //Do all the single-line patterns
    for(int i=0; i<syntax.rules.length(); i++){
            if(syntax.rules[i].pattern.isEmpty()){ continue; } //not a single-line rule
      QRegExp patt(syntax.rules[i].pattern); //need a copy of the rule's pattern (will be changing it below)
      int index = patt.indexIn(text);
            if(splitactive>=0 || index<start){ continue; } //skip this one - falls within a multi-line pattern above
      while(index>=0){
        int len = patt.matchedLength();
        if(format(index)==currentBlock().charFormat()){ setFormat(index, len, syntax.rules[i].format); } //only apply highlighting if not within a section already
        index = patt.indexIn(text, index+len); //go to the next match
      }
    }//end loop over normal (single-line) patterns

    //Now go through and apply any document-wide formatting rules
          QTextCharFormat fmt;
          fmt.setBackground( QColor( settings->value("colors/bracket-missing").toString() ) );
          int max = syntax.char_limit();
    if(max >= 0 && ( (text.length()+(text.count("\t")*(syntax.tab_length()-1)) )> max) ) {
      //Line longer than it should be - highlight the offending characters
      //Need to be careful about where tabs show up in the line
      int len = 0;
      for(int i=0; i<text.length() and len<=max; i++){
        len += (text[i]=='\t') ? syntax.tab_length() : 1;
        if(len>max)
          setFormat(i, text.length()-i, fmt); 
      }
    }
    if(syntax.highlight_excess_whitespace()){
      int last = text.length()-1;
      while(last>=0 && (text[last]==' ' || text[last]=='\t' ) ){ last--; }
      if(last < text.length()-1){
        setFormat(last+1, text.length()-1-last, fmt);
      }
    }
  }
};
#endif

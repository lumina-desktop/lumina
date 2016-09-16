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

//Simple syntax rules
struct SyntaxRule{
  QRegExp pattern;
  QTextCharFormat format;
};
//Complicated/multi-line rules
struct SyntaxRuleSplit{
  QRegExp startPattern, endPattern;
  QTextCharFormat format;
};

class Custom_Syntax : public QSyntaxHighlighter{
	Q_OBJECT
private:
	QSettings *settings;
	QString lasttype;
	QVector<SyntaxRule> rules;
	QVector<SyntaxRuleSplit> splitrules;

public:
	Custom_Syntax(QSettings *set, QTextDocument *parent = 0) : QSyntaxHighlighter(parent){
	  settings = set;
	}
	~Custom_Syntax(){}
		
	static QStringList availableRules();
	static QStringList knownColors();
	static void SetupDefaultColors(QSettings *settings);
	static QString ruleForFile(QString filename);
	void loadRules(QString type);
		
	void reloadRules(){
	  loadRules(lasttype);
	}
protected:
	void highlightBlock(const QString &text){
          //qDebug() << "Highlight Block:" << text;
	  //Now look for any multi-line patterns (starting/continuing/ending)
	  int start = 0;
	  int splitactive = previousBlockState();
	  if(splitactive>splitrules.length()-1){ splitactive = -1; } //just in case
	  while(start>=0 && start<=text.length()-1){
	    //qDebug() << "split check:" << start << splitactive;
	    if(splitactive>=0){
	      //Find the end of the current rule
	      int end = splitrules[splitactive].endPattern.indexIn(text, start);
	      if(end==-1){
                //qDebug() << "Highlight to end of line:" << text << start;
	        //rule did not finish - apply to all
                if(start>0){ setFormat(start-1, text.length()-start+1, splitrules[splitactive].format); }
                else{ setFormat(start, text.length()-start, splitrules[splitactive].format); }
		break; //stop looking for more multi-line patterns
	      }else{
		//Found end point within the same line
                //qDebug() << "Highlight to particular point:" << text << start << end;
		int len = end-start+splitrules[splitactive].endPattern.matchedLength();
                if(start>0){ start--; len++; } //need to include the first character as well
		setFormat(start, len , splitrules[splitactive].format);
		start+=len; //move pointer to the end of handled range
		splitactive = -1; //done with this rule
	      }
	    } //end check for end match
	    //Look for the start of any new split rule
	    for(int i=0; i<splitrules.length() && splitactive<0; i++){
              //qDebug() << "Look for start of split rule:" << splitrules[i].startPattern << splitactive;
	      int newstart = splitrules[i].startPattern.indexIn(text,start);
	      if(newstart>=start){
                //qDebug() << "Got Start of split rule:" << start << newstart << text;
		splitactive = i;
		start = newstart+1;
                if(start>=text.length()-1){
                  //qDebug() << "Special case: start now greater than line length";
                  //Need to apply highlighting to this section too - start matches the end of the line
                  setFormat(start-1, text.length()-start+1, splitrules[splitactive].format);
                }
	      }
	    }
	    if(splitactive<0){  break; } //no other rules found - go ahead and exit the loop
          }
	  setCurrentBlockState(splitactive);
          //Do all the single-line patterns
	  for(int i=0; i<rules.length(); i++){
	    QRegExp patt(rules[i].pattern); //need a copy of the rule's pattern (will be changing it below)
	    int index = patt.indexIn(text);
            if(splitactive>=0 || index<start){ continue; } //skip this one - falls within a multi-line pattern above
	    while(index>=0){
	      int len = patt.matchedLength();
	      if(format(index)==currentBlock().charFormat()){ setFormat(index, len, rules[i].format); } //only apply highlighting if not within a section already
	      index = patt.indexIn(text, index+len); //go to the next match
	    }
	  }//end loop over normal (single-line) patterns
	}
};
#endif

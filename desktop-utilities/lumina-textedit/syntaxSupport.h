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
protected:
	void highlightBlock(const QString &text){
	  for(int i=0; i<rules.length(); i++){
	    QRegExp patt(rules[i].pattern); //need a copy of the rule's pattern (will be changing it below)
	    int index = patt.indexIn(text);
	    while(index>=0){
	      int len = patt.matchedLength();
	      setFormat(index, len, rules[i].format);
	      index = patt.indexIn(text, index+len); //go to the next match
	    }
	  }//end loop over normal (single-line) patterns
	  //Now look for any multi-line patterns (starting/continuing/ending)
	  int start = 0;
	  int splitactive = previousBlockState();
	  if(splitactive>splitrules.length()-1){ splitactive = -1; } //just in case
	  while(start>=0 && start<text.length()-1){
	    //qDebug() << "split check:" << start << splitactive;
	    if(splitactive>=0){
	      //Find the end of the current rule
	      int end = splitrules[splitactive].endPattern.indexIn(text, start);
	      if(end==-1){
	        //rule did not finish - apply to all
		setFormat(start, text.length()-start, splitrules[splitactive].format);
		break; //stop looking for more multi-line patterns
	      }else{
		//Found end point
		int len = end-start+splitrules[splitactive].endPattern.matchedLength();
		setFormat(start, len , splitrules[splitactive].format);
		start+=len; //move pointer to the end of handled range
		splitactive = -1; //done with this rule
	      }
	    } //end check for end match
	    //Look for the start of any new split rule
	    for(int i=0; i<splitrules.length() && splitactive<0; i++){
	      int newstart = splitrules[i].startPattern.indexIn(text,start);
	      if(newstart>=start){
		splitactive = i;
		start = newstart;
	      }
	    }
	    if(splitactive<0){  break; } //no other rules found - go ahead and exit the loop
          }
	  setCurrentBlockState(splitactive);
	}
};
#endif
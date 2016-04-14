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

struct SyntaxRule{
  QRegExp pattern;
  QTextCharFormat format;
};

class Custom_Syntax : public QSyntaxHighlighter{
	Q_OBJECT
private:
	QVector<SyntaxRule> rules;
public:
	Custom_Syntax(QTextDocument *parent = 0) : QSyntaxHighlighter(parent){
	}
	~Custom_Syntax(){}
	
	static QStringList availableRules();
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
	  }
	}
};
#endif
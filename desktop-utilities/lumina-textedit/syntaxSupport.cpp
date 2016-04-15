//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "syntaxSupport.h"

QStringList Custom_Syntax::availableRules(){
  QStringList avail;
    avail << "C++";
	
  return avail;
}

QString Custom_Syntax::ruleForFile(QString filename){
  QString suffix = filename.section(".",-1);
  if(suffix=="cpp" || suffix=="hpp" || suffix=="c" || suffix=="h"){ return "C++"; }
  return "";
}

void Custom_Syntax::loadRules(QString type){
  //NOTE: the "multiLineComment
  rules.clear();
  splitrules.clear();
  if(type=="C++"){
    //Keywords (standard C/C++/Qt definitions)
    QStringList keywords;
	keywords << "char" << "class" << "const" << "double" << "enum" << "explicit" << "friend" << "inline" \
			<< "int" << "long" << "namespace" << "operator" << "private" << "protected" << "public" \
			<< "short" << "signals" << "signed" << "slots" << "static" << "struct" << "template" \
			<< "typedef" << "typename" << "union" << "unsigned" << "virtual" << "void" << "volatile";
	  
    SyntaxRule rule;
	rule.format.setForeground(Qt::darkBlue);
	rule.format.setFontWeight(QFont::Bold);
    for(int i=0; i<keywords.length(); i++){
      rule.pattern = QRegExp("\\b"+keywords[i]+"\\b"); //turn each keyword into a QRegExp and insert the rule
      rules << rule;
    }
    //Class Names
    rule.format.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rules << rule;
    //Quotes
    rule.format.setForeground(Qt::red);
    rule.format.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("\".*\"");
    rules << rule;
    //Functions
    rule.format.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rules << rule;
    //Comment (single line)
    rule.format.setForeground(Qt::green);
    rule.pattern = QRegExp("//[^\n]*");
    rules << rule;
    //Comment (multi-line)
    SyntaxRuleSplit srule;
    srule.format = rule.format; //re-use the single-line comment format
    srule.startPattern = QRegExp("/\\*");
    srule.endPattern = QRegExp("\\*/");
    splitrules << srule;
  }
}
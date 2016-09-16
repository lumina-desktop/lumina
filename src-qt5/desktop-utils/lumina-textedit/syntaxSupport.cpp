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
    //avail << "Python";
    avail << "Shell";
    avail << "reST";
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

QString Custom_Syntax::ruleForFile(QString filename){
  QString suffix = filename.section(".",-1);
  if(suffix=="cpp" || suffix=="hpp" || suffix=="c" || suffix=="h"){ return "C++"; }
  //else if(suffix=="py" || suffix=="pyc"){ return "Python"; }
  else if(suffix=="sh"){ return "Shell"; }
  else if(suffix=="rst"){ return "reST"; }
  return "";
}

void Custom_Syntax::loadRules(QString type){
  //NOTE: the "multiLineComment
  lasttype = type;
  rules.clear();
  splitrules.clear();
  if(type=="C++"){
    //Keywords (standard C/C++/Qt definitions)
    QStringList keywords;
	keywords << "char" << "class" << "const" << "double" << "enum" << "explicit" << "extern" << "float" << "friend" << "inline" \
			<< "int" << "long" << "namespace" << "operator" << "private" << "protected" << "public" \
			<< "short" << "signals" << "signed" << "slots" << "static" << "struct" << "template" \
			<< "typedef" << "typename" << "union" << "unsigned" << "virtual" << "void" << "volatile" \
			<< "true" << "false" << "bool";

    SyntaxRule rule;
	rule.format.setForeground( QColor(settings->value("colors/keyword").toString()) );
	rule.format.setFontWeight(QFont::Bold);
    for(int i=0; i<keywords.length(); i++){
      rule.pattern = QRegExp("\\b"+keywords[i]+"\\b"); //turn each keyword into a QRegExp and insert the rule
      rules << rule;
    }
    //Alternate Keywords (built-in functions)
    keywords.clear();
    keywords << "for" << "while" << "switch" << "case" << "if" << "else" << "return" << "exit";
    rule.format.setForeground( QColor(settings->value("colors/altkeyword").toString()) );
    for(int i=0; i<keywords.length(); i++){
      rule.pattern = QRegExp("\\b"+keywords[i]+"\\b"); //turn each keyword into a QRegExp and insert the rule
      rules << rule;
    }
    //Class Names
    rule.format.setForeground( QColor(settings->value("colors/class").toString()) );
    rule.pattern = QRegExp("\\b[A-Za-z0-9_-\\.]+(?=::)\\b");
    rules << rule;
    //Quotes
    rule.format.setForeground( QColor(settings->value("colors/text").toString()) );
    rule.format.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp( "\"[^\"\\\\]*(\\\\(.|\\n)[^\"\\\\]*)*\"|'[^'\\\\]*(\\\\(.|\\n)[^'\\\\]*)*'");
    rules << rule;
    //Functions
    rule.format.setForeground( QColor(settings->value("colors/function").toString()) );
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rules << rule;
    //Proprocessor commands
    rule.format.setForeground( QColor(settings->value("colors/preprocessor").toString()) );
    rule.pattern = QRegExp("^[\\s]*#[^\n]*");
    rules << rule;    
    //Comment (single line)
    rule.format.setForeground( QColor(settings->value("colors/comment").toString()) );
    rule.pattern = QRegExp("//[^\n]*");
    rules << rule;
    //Comment (multi-line)
    SyntaxRuleSplit srule;
    srule.format = rule.format; //re-use the single-line comment format
    srule.startPattern = QRegExp("/\\*");
    srule.endPattern = QRegExp("\\*/");
    splitrules << srule;
    
  }else if(type=="Shell"){
    //Keywords (standard Shell definitions)
    QStringList keywords;
	keywords << "alias" << "alloc" << "bg" << "bind" << " bindkey" << "break" \
     << "breaksw"<<"builtins"<<"case"<<"cd"<<"chdir"<<"command"<<"complete"<<"continue"<<"default" \
     <<"dirs"<<"do"<<"done"<<"echo"<<"echotc"<<"elif"<<"else"<<"end"<<"endif"<<"endsw"<<"esac"<<"eval" \
     <<"exec"<<"exit"<<"export"<<"false"<<"fc"<<"fg"<<"filetest"<<"fi"<<"for"<<"foreach"<<"getopts" \
     <<"glob"<<"goto"<<"hash"<<"hashstat"<<"history"<<"hup"<<"if"<<"jobid"<<"jobs"<<"kill"<<"limit" \
     <<"local"<<"log"<<"login"<<"logout"<<"ls-F"<<"nice"<<"nohup"<<"notify"<<"onintr"<<"popd" \
     <<"printenv"<<"printf"<<"pushd"<<"pwd"<<"read"<<"readonly"<<"rehash"<<"repeat"<<"return" \
     <<"sched"<<"set"<<"setenv"<<"settc"<<"setty"<<"setvar"<<"shift"<<"source"<<"stop"<<"suspend" \
     <<"switch"<<"telltc"<<"test"<<"then"<<"time"<<"times"<<"trap"<<"true"<<"type"<<"ulimit"<<"umask" \
     <<"unalias"<<"uncomplete"<<"unhash"<<"unlimit"<<"unset"<<"unsetenv"<<"until"<<"wait" \
     <<"where"<<"which"<<"while";

    SyntaxRule rule;
	rule.format.setForeground( QColor(settings->value("colors/keyword").toString()) );
	rule.format.setFontWeight(QFont::Bold);
    for(int i=0; i<keywords.length(); i++){
      rule.pattern = QRegExp("\\b"+keywords[i]+"\\b"); //turn each keyword into a QRegExp and insert the rule
      rules << rule;
    }
    //Alternate Keywords (built-in functions)
    /*keywords.clear();
    keywords << "for" << "while" << "switch" << "case" << "if" << "else" << "return" << "exit";
    rule.format.setForeground( QColor(settings->value("colors/altkeyword").toString()) );
    for(int i=0; i<keywords.length(); i++){
      rule.pattern = QRegExp("\\b"+keywords[i]+"\\b"); //turn each keyword into a QRegExp and insert the rule
      rules << rule;
    }*/
    //Variable Names
    rule.format.setForeground( QColor(settings->value("colors/class").toString()) );
    rule.pattern = QRegExp("\\$\\{[^\\n\\}]+\\}");
    rules << rule;
    rule.pattern = QRegExp("\\$[^\\s$]+(?=\\s|$)");
    rules << rule;
    //Quotes
    rule.format.setForeground( QColor(settings->value("colors/text").toString()) );
    rule.format.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp( "\"[^\"\\\\]*(\\\\(.|\\n)[^\"\\\\]*)*\"|'[^'\\\\]*(\\\\(.|\\n)[^'\\\\]*)*'");
    rules << rule;
    //Functions
    rule.format.setForeground( QColor(settings->value("colors/function").toString()) );
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rules << rule;
    //Proprocessor commands
    rule.format.setForeground( QColor(settings->value("colors/preprocessor").toString()) );
    rule.pattern = QRegExp("^#![^\n]*");
    rules << rule;    
    //Comment (single line)
    rule.format.setForeground( QColor(settings->value("colors/comment").toString()) );
    rule.pattern = QRegExp("#[^\n]*");
    rules << rule;
    //Comment (multi-line)
    //SyntaxRuleSplit srule;
    //srule.format = rule.format; //re-use the single-line comment format
    //srule.startPattern = QRegExp("/\\*");
    //srule.endPattern = QRegExp("\\*/");
    //splitrules << srule;
    
  }else if(type=="Python"){
    //Keywords
    QStringList keywords;
	keywords << "and" << "as" << "assert" << "break" << "class" << "continue" << "def" << "del" \
			<< "elif" << "else" << "except" << "exec" << "finally" << "for" << "from" \
			<< "global" << "if" << "import" << "in" << "is" << "lambda" << "not" \
			<< "or" << "pass" << "print" << "raise" << "return" << "try" << "while" << "with" << "yield";
	  
    SyntaxRule rule;
	rule.format.setForeground( QColor(settings->value("colors/keyword").toString()) );
	rule.format.setFontWeight(QFont::Bold);
    for(int i=0; i<keywords.length(); i++){
      rule.pattern = QRegExp("\\b"+keywords[i]+"\\b"); //turn each keyword into a QRegExp and insert the rule
      rules << rule;
    }
    //Class Names
    //rule.format.setForeground(Qt::darkMagenta);
    //rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    //rules << rule;
    //Quotes
    rule.format.setForeground( QColor(settings->value("colors/text").toString()) );
    rule.format.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp( "\"[^\"\\\\]*(\\\\(.|\\n)[^\"\\\\]*)*\"|'[^'\\\\]*(\\\\(.|\\n)[^'\\\\]*)*'");
    rules << rule;
    //Functions
    rule.format.setForeground( QColor(settings->value("colors/function").toString()) );
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rules << rule;
    //Comment (single line)
    rule.format.setForeground( QColor(settings->value("colors/comment").toString()) );
    rule.pattern = QRegExp("#[^\n]*");
    rules << rule;
    //Comment (multi-line)
    //SyntaxRuleSplit srule;
    //srule.format = rule.format; //re-use the single-line comment format
    //srule.startPattern = QRegExp("/\\*");
    //srule.endPattern = QRegExp("\\*/");
    //splitrules << srule;
    
  }else if(type=="reST"){
    SyntaxRule rule;
    // directives
    rule.format.setForeground( QColor(settings->value("colors/class").toString()) );
    rule.format.setFontItalic(false);
    rule.pattern = QRegExp("(\\s|^):[a-zA-Z0-9 ]*:`[^`]*`");
    rules << rule;
    // hyperlinks
    rule.format.setFontItalic(true);
    rule.format.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("`[^\\<]*\\<[^\\>]*\\>`_");
    rules << rule;
    // Code Sample
    rule.format.setFontItalic(false);
    rule.format.setFontWeight(QFont::Light);
    rule.format.setFontFixedPitch(true);
    rule.pattern = QRegExp("\\b`{2}.*`{2}\\b");
    rules << rule;
    //Quotes
    /*rule.format.setForeground( QColor(settings->value("colors/text").toString()) );
    rule.format.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp( "\"[^\"\\\\]*(\\\\(.|\\n)[^\"\\\\]*)*\"|'[^'\\\\]*(\\\\(.|\\n)[^'\\\\]*)*'");
    rules << rule;*/
    //TODO
    rule = SyntaxRule(); //reset rule
    rule.format.setFontWeight( QFont::Bold );
    rule.pattern = QRegExp("^\\.\\.\\sTODO\\b");
    rules << rule;
    rule = SyntaxRule(); //reset rule
    rule.format.setFontWeight( QFont::Bold );
    rule.pattern = QRegExp("^(\\s*)\\.\\.(\\s*)([a-zA-Z0-9]+)::");
    rules << rule;
    //Functions
    rule = SyntaxRule(); //reset rule
    rule.format.setForeground( QColor(settings->value("colors/preprocessor").toString()) );
    rule.pattern = QRegExp("^(\\s*)\\.\\.(\\s*)\\b_[a-zA-Z0-9 ]*:(\\s|$)");
    rules << rule;
    //figures and other properties for them
    rule = SyntaxRule(); //reset rule
    rule.format.setForeground( QColor(settings->value("colors/keyword").toString()) );
    rule.pattern = QRegExp("^(\\s*)\\.\\.\\sfigure::\\s");
    rules << rule;
    rule = SyntaxRule(); //reset rule
    rule.format.setForeground( QColor(settings->value("colors/altkeyword").toString()) );
    rule.pattern = QRegExp("^( ){3}:(.)*: ");
    rules << rule;    

    //Code Blocks
    SyntaxRuleSplit srule;
    srule.format.setBackground( QColor("lightblue") );
    srule.startPattern = QRegExp("\\:\\:$");
    srule.endPattern = QRegExp("^(?=[^\\s])");
    splitrules << srule;
    srule.startPattern = QRegExp("^(\\s*)\\.\\.\\scode-block::\\s"); //alternate start string for the same rule
    srule.endPattern = QRegExp("^(?=[^\\s])");
    splitrules << srule;
    //Comment (multi-line)
    srule = SyntaxRuleSplit();
    srule.format.setForeground( QColor(settings->value("colors/comment").toString()) );
    srule.startPattern = QRegExp("^(\\s*)\\.\\.\\s[^_](?![\\w-_\\.]+::(\\s|$))");
    srule.endPattern = QRegExp("^(?=([^\\s]|$))");
    splitrules << srule;
  }
}

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
  rules.clear();
  if(type=="C++"){
	  
  }
}
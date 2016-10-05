//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h"

#include <QDebug>

#define VALIDSYMBOLS QString("+-*x/.")

#include <math.h>
#define BADVALUE NAN

mainUI::mainUI() : QMainWindow(), ui(new Ui::mainUI()){
  ui->setupUi(this);
  connect(ui->tool_start, SIGNAL(clicked()), this, SLOT(start_calc()) );
  connect(ui->line_eq, SIGNAL(returnPressed()), this, SLOT(start_calc()) );
  ui->line_eq->setFocus();
}

mainUI::~mainUI(){
  
}

void mainUI::start_calc(){
  if(ui->line_eq->text().isEmpty()){ return; } //nothing to do
  double result = strToNumber(ui->line_eq->text());
  if(result!=result){ return; } //bad calculation - NaN's values are special in that they don't equal itself
  QString res = "%1 \t(%2)";
  ui->list_results->addItem(res.arg(QString::number(result), ui->line_eq->text()));
  ui->list_results->scrollToItem( ui->list_results->item( ui->list_results->count()-1) );
  ui->line_eq->clear();
}

double mainUI::performOperation(double LHS, double RHS, QChar symbol){
  if(symbol== '+'){ return (LHS+RHS); }
  else if(symbol== '-'){ return (LHS-RHS); }
  else if(symbol== '*' || symbol=='x'){ return (LHS*RHS); }
  else if(symbol== '/'){ return (LHS/RHS); }
  //else if(symbol== '%'){ return (LHS%RHS); }
  qDebug() << "Invalid Symbol:" << symbol;
  return BADVALUE;
}

double mainUI::strToNumber(QString str){
  //Look for perentheses first
  qDebug() << "String to Number: " << str;
  if(str.indexOf("(")>=0){
    //qDebug() << "Found Parenthesis";
    int start = str.indexOf("(");
    int need = 1;
    int end = -1;
    for(int i=start+1; i<str.length() && need>0; i++){
      if(str[i]=='('){ need++; }
      else if(str[i]==')'){ need--; }
      //qDebug() << "Check char:" << str[i] << need;
      if(need==0){ end = i; }
    }
    if(end<start){ return BADVALUE; }
    //qDebug() << "Replace value:" << str << start << end << str.mid(start+1,end-start);
    str.replace(start, end-start+1, QString::number( strToNumber( str.mid(start+1, end-start-1)) ) );
    //qDebug() << "Replaced:" << str;
  }
  //  -------------------------------------
  // NOTE: the order of operations appears reversed here due to the recursive nature of the algorithm
  //     the first operation *found* is actually the last one *evaluated*
  // --------------------------------------
  //Now look for add/subtract
  int sym = -1;
  QStringList symbols; symbols << "+" << "-";
  for(int i=0; i<symbols.length(); i++){
    int tmp = str.indexOf(symbols[i]);
    if(sym < tmp){ sym = tmp; }
  }
  if(sym>0){  return performOperation( strToNumber(str.left(sym)), strToNumber(str.right(str.length()-sym-1)), str[sym]); }
  if(sym==0){ return BADVALUE; }
  //Now look for multiply/divide
  symbols.clear(); symbols << "x" << "*" << "/";
  for(int i=0; i<symbols.length(); i++){
    int tmp = str.indexOf(symbols[i]);
    if(sym < tmp){ sym = tmp; }
  }
  if(sym>0){  return performOperation( strToNumber(str.left(sym)), strToNumber(str.right(str.length()-sym-1)), str[sym]); }
  if(sym==0){ return BADVALUE; }

  //Could not find any operations - must be a raw number
  //qDebug() << "Found Number:" << str.toDouble();
  return str.toDouble();
}

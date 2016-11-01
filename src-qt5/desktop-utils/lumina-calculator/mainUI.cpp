//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h"

#include <QDebug>
#include <QClipboard>
#include <QFileDialog>

#include <LuminaUtils.h>
#include <LuminaXDG.h>
#include "EqValidator.h"

#include <math.h>
#define BADVALUE NAN

#define OPS QString("+-*/x^%")

mainUI::mainUI() : QMainWindow(), ui(new Ui::mainUI()){
  ui->setupUi(this);
  connect(ui->tool_clear, SIGNAL(clicked()), this, SLOT(clear_calc()) );
  connect(ui->line_eq, SIGNAL(returnPressed()), this, SLOT(start_calc()) );
  connect(ui->line_eq, SIGNAL(textEdited(const QString&)), this, SLOT(checkInput(const QString&)) );
  connect(ui->button_1, SIGNAL (clicked()), this, SLOT (captureButton1()));
  connect(ui->button_2, SIGNAL (clicked()), this, SLOT (captureButton2()));
  connect(ui->button_3, SIGNAL (clicked()), this, SLOT (captureButton3()));
  connect(ui->button_4, SIGNAL (clicked()), this, SLOT (captureButton4()));
  connect(ui->button_5, SIGNAL (clicked()), this, SLOT (captureButton5()));
  connect(ui->button_6, SIGNAL (clicked()), this, SLOT (captureButton6()));
  connect(ui->button_7, SIGNAL (clicked()), this, SLOT (captureButton7()));
  connect(ui->button_8, SIGNAL (clicked()), this, SLOT (captureButton8()));
  connect(ui->button_9, SIGNAL (clicked()), this, SLOT (captureButton9()));
  connect(ui->button_0, SIGNAL (clicked()), this, SLOT (captureButton0()));
  connect(ui->button_Subtract, SIGNAL (clicked()), this, SLOT (captureButtonSubtract()));
  connect(ui->button_Add, SIGNAL (clicked()), this, SLOT (captureButtonAdd()));
  connect(ui->button_Divide, SIGNAL (clicked()), this, SLOT (captureButtonDivide()));
  connect(ui->button_Multiply, SIGNAL (clicked()), this, SLOT (captureButtonMultiply()));
  connect(ui->button_Decimal, SIGNAL (clicked()), this, SLOT (captureButtonDecimal()));
  connect(ui->button_Percent, SIGNAL(clicked()), this, SLOT(captureButtonPercent()) );
  connect(ui->button_Equal, SIGNAL (clicked()), this, SLOT (start_calc()));
  connect(ui->list_results, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(insert_history(QListWidgetItem*)) );
  connect(ui->tool_history_clear, SIGNAL(clicked()), ui->list_results, SLOT(clear()) );
  connect(ui->tool_history_save, SIGNAL(clicked()), this, SLOT(saveHistory()) );
  //connect(ui->list_results, SIGNAL(itemRightClicked(QListWidgetItem*)), this, SLOT(copt_to_clipboard(QListWidgetItem*)) );
  this->setWindowTitle(tr("Calculator"));
  updateIcons();
  ui->line_eq->setFocus();
  ui->line_eq->setValidator(new EqValidator(this) );
}

mainUI::~mainUI(){
}

void mainUI::updateIcons(){
  this->setWindowIcon( LXDG::findIcon("accessories-calculator","") );
  ui->tool_clear->setIcon( LXDG::findIcon("edit-clear-locationbar-rtl","dialog-cancel") );
  ui->tool_history_clear->setIcon( LXDG::findIcon("document-close","edit-clear-list") );
  ui->tool_history_save->setIcon( LXDG::findIcon("document-save-as","edit-copy") );
}

void mainUI::start_calc(){
  if(ui->line_eq->text().isEmpty()){ return; } //nothing to do
  QString eq = ui->line_eq->text();
    eq.replace("%","/(100)");
  double result = strToNumber(eq);
  if(result!=result){ return; } //bad calculation - NaN's values are special in that they don't equal itself
  QString res = "[#%1]  %2 \t= [ %3 ]";
  ui->list_results->addItem(res.arg(QString::number(ui->list_results->count()+1), QString::number(result), ui->line_eq->text()));
  ui->list_results->scrollToItem( ui->list_results->item( ui->list_results->count()-1) );
  ui->line_eq->clear();
}

void mainUI::clear_calc(){
    ui->line_eq->clear();
      ui->line_eq->setFocus();
}

void mainUI::captureButton1(){ ui->line_eq->insert(ui->button_1->text()); }
void mainUI::captureButton2(){ ui->line_eq->insert(ui->button_2->text()); }
void mainUI::captureButton3(){ ui->line_eq->insert(ui->button_3->text()); }
void mainUI::captureButton4(){ui->line_eq->insert(ui->button_4->text()); }
void mainUI::captureButton5(){ ui->line_eq->insert(ui->button_5->text()); }
void mainUI::captureButton6(){ ui->line_eq->insert(ui->button_6->text()); }
void mainUI::captureButton7(){ ui->line_eq->insert(ui->button_7->text()); }
void mainUI::captureButton8(){ ui->line_eq->insert(ui->button_8->text()); }
void mainUI::captureButton9(){ ui->line_eq->insert(ui->button_9->text()); }
void mainUI::captureButton0(){ ui->line_eq->insert(ui->button_0->text()); }
void mainUI::captureButtonSubtract(){ ui->line_eq->insert(ui->button_Subtract->text()); }
void mainUI::captureButtonAdd(){ ui->line_eq->insert(ui->button_Add->text()); }
void mainUI::captureButtonDivide(){ ui->line_eq->insert(ui->button_Divide->text()); }
void mainUI::captureButtonMultiply(){ ui->line_eq->insert(ui->button_Multiply->text()); }
//void mainUI::captureButtonEqual(){ ui->line_eq->setText(ui->line_eq->text() += ui->button_Equal->text()); }
void mainUI::captureButtonDecimal(){ ui->line_eq->insert(ui->button_Decimal->text()); }
void mainUI::captureButtonPercent(){ ui->line_eq->insert(ui->button_Percent->text()); }

void mainUI::insert_history(QListWidgetItem *it){
  QString txt = it->text().section("[",-1).section("]",0,0).simplified();
  ui->line_eq->insert("("+txt+")");
}

void mainUI::copy_to_clipboard(QListWidgetItem *it){
  QString txt = it->text();
  QApplication::clipboard()->setText(txt);
}

void mainUI::checkInput(const QString &str){
  if(str.length()==1 && ui->list_results->count()>0){
    if(OPS.contains(str)){ 
      QString lastresult = ui->list_results->item( ui->list_results->count()-1)->text().section("]",0,0).section("[",-1).simplified();
      ui->line_eq->setText( lastresult+str);
    }
  }
}

void mainUI::saveHistory(){
  QStringList history;
  for(int i=0; i<ui->list_results->count(); i++){ history << ui->list_results->item(i)->text(); }
  QString file = QFileDialog::getSaveFileName(this, tr("Save Calculator History"), QDir::homePath() );
  if(file.section(".",-1).isEmpty()){ file.append(".txt"); }
  LUtils::writeFile(file, history, true);
}

// =====================
//   PRIVATE FUNCTIONS
// =====================
double mainUI::performOperation(double LHS, double RHS, QChar symbol){
  //qDebug() << "Perform Operation:" << LHS << symbol << RHS;
  if(symbol== '+'){ return (LHS+RHS); }
  else if(symbol== '-'){ return (LHS-RHS); }
  else if(symbol== '*' || symbol=='x'){ return (LHS*RHS); }
  else if(symbol== '/'){ return (LHS/RHS); }
  else if(symbol== '^'){ return ::pow(LHS, RHS); }
  //else if(symbol== '%'){ return (LHS%RHS); }
  qDebug() << "Invalid Symbol:" << symbol;
  return BADVALUE;
}

double mainUI::strToNumber(QString str){
  //Look for history replacements first
  while(str.contains("#")){
    int index = str.indexOf("#");
    int num = -1; //history number
    for(int i=index+1; i<str.length(); i++){
      if(!str[i].isNumber() ||  i==(str.length()-1) ){
        if(!str[i].isNumber()){ i--; } //go back to the last valid char
        //qDebug() << "Replace History:" << str << index << i << str.mid(index+1, i-index);
        num = str.mid(index+1, i-index).toInt();
        //qDebug() << " H number:" << num;
        str.replace(index, i-index+1, getHistory(num));
        //qDebug() << " After Replace:" << str;
        break; //finished with this history item
      }
    }
    if(num<1){ return BADVALUE; } //could not perform substitution
  }
  //Look for perentheses first
  //qDebug() << "String to Number: " << str;
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
    double tmp = strToNumber( str.mid(start+1, end-start-1));
    if(tmp!=tmp){ return BADVALUE; } //not a number
    str.replace(start, end-start+1, QString::number( tmp, 'e', 16) ); //need as high precision as possible here - internal only, never seen
    //qDebug() << "Replaced:" << str;
  }
  //  -------------------------------------
  // NOTE: the order of operations appears reversed here due to the recursive nature of the algorithm
  //     the first operation *found* is actually the last one *evaluated*
  // --------------------------------------
  //Now look for add/subtract
  int sym = -1;
  QStringList symbols; symbols << "+" << "-";
  //qDebug() << "Get operator:" << str;
  for(int i=0; i<symbols.length(); i++){
    int tmp = str.indexOf(symbols[i]);
    while(tmp==0 || (tmp>0 && str[tmp-1].toLower()=='e') ){ tmp = str.indexOf(symbols[i], tmp+1); } //catch scientific notation
    if(sym < tmp){ 
      //qDebug() << " - found:" << tmp << sym;
      sym = tmp;
    }
  }
  if(sym>0){  return performOperation( strToNumber(str.left(sym)), strToNumber(str.right(str.length()-sym-1)), str[sym]); }
  if(sym==0){ return BADVALUE; }
  //Now look for multiply/divide/power
  symbols.clear(); symbols << "x" << "*" << "/" << "^" ;
  for(int i=0; i<symbols.length(); i++){
    int tmp = str.indexOf(symbols[i]);
    if(sym < tmp){ sym = tmp; }
  }
  if(sym>0){  return performOperation( strToNumber(str.left(sym)), strToNumber(str.right(str.length()-sym-1)), str[sym]); }
  if(sym==0){ return BADVALUE; }

  //Could not find any operations - must be a raw number
  //qDebug() << " - Found Number:" << str << str.toDouble();
  return str.toDouble();
}

QString mainUI::getHistory(int number){
  if(number <1 || number > ui->list_results->count()){
    //use the last history item
    number = ui->list_results->count();
  }
  QString ans = ui->list_results->item(number-1)->text().section("=",0,0).section("]",-1).simplified();
  QString eq = ui->list_results->item(number-1)->text().section("[",-1).section("]",0,0).simplified();
  //See if the text answer is accurate enough (does not look rounded)
  if(ans.length()<7){ 
    return ("("+ans+")"); //short enough answer that it was probably not rounded
  }else{
    //need to re-calculate the last equation instead for exact result
    return ("("+eq+")");
  }
}

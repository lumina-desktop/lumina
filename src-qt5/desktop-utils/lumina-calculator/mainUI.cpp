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
#include <QMenu>

#include <LUtils.h>
#include <LuminaXDG.h>
#include "EqValidator.h"

#include <math.h>
#define BADVALUE NAN

#define OPS QString("+-*/x^%")


const double PI = (::acos(1.0)+::acos(-1.0));

mainUI::mainUI() : QMainWindow(), ui(new Ui::mainUI()){
  ui->setupUi(this);
  advMenu = 0;
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
  connect(ui->button_Equal, SIGNAL (clicked()), this, SLOT (start_calc()));
  connect(ui->list_results, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(insert_history(QListWidgetItem*)) );
  connect(ui->tool_history_clear, SIGNAL(clicked()), ui->list_results, SLOT(clear()) );
  connect(ui->tool_history_save, SIGNAL(clicked()), this, SLOT(saveHistory()) );
  //connect(ui->list_results, SIGNAL(itemRightClicked(QListWidgetItem*)), this, SLOT(copt_to_clipboard(QListWidgetItem*)) );
  this->setWindowTitle(tr("Calculator"));
  updateIcons();
  updateMenus();
  ui->line_eq->setFocus();
  ui->line_eq->setValidator(new EqValidator(this) );

  // shortcuts
  escShortcut = new QShortcut(Qt::Key_Escape, this);
  connect(escShortcut, SIGNAL(activated()), this, SLOT(clear_calc()) );
  quitShortcut = new QShortcut(Qt::CTRL + Qt::Key_Q, this);
  connect(quitShortcut, SIGNAL(activated()), this, SLOT(quitShortcut_Triggered()) );
}

mainUI::~mainUI(){
}

void mainUI::updateIcons(){
  this->setWindowIcon( LXDG::findIcon("accessories-calculator","") );
  ui->tool_clear->setIcon( LXDG::findIcon("edit-clear-locationbar-rtl","dialog-cancel") );
  ui->tool_history_clear->setIcon( LXDG::findIcon("document-close","edit-clear-list") );
  ui->tool_history_save->setIcon( LXDG::findIcon("document-save-as","edit-copy") );
  ui->tool_adv->setIcon( LXDG::findIcon("formula","") );
}

void mainUI::updateMenus(){
  if(advMenu==0){
    advMenu = new QMenu(this);
    ui->tool_adv->setMenu(advMenu);
    connect(advMenu, SIGNAL(triggered(QAction*)), this, SLOT(advMenuTriggered(QAction*)) );
  }
  QAction *tmp = advMenu->addAction( QString(tr("Percentage %1")).arg("\t%") );
    tmp->setWhatsThis("%");
  tmp = advMenu->addAction( QString(tr("Power %1")).arg("\t^") );
    tmp->setWhatsThis("^");
  tmp = advMenu->addAction( QString(tr("Base-10 Exponential %1")).arg("\tE") );
    tmp->setWhatsThis("E");
  tmp = advMenu->addAction( QString(tr("Exponential %1")).arg("\te") );
    tmp->setWhatsThis("e");
  tmp = advMenu->addAction( QString(tr("Constant Pi %1")).arg("\t\u03C0") );
    tmp->setWhatsThis("\u03C0");
  advMenu->addSeparator();
  tmp = advMenu->addAction( QString(tr("Square Root %1")).arg("\tsqrt(") );
    tmp->setWhatsThis("sqrt(");
  tmp = advMenu->addAction( QString(tr("Logarithm %1")).arg("\tlog(") );
    tmp->setWhatsThis("log(");
  tmp = advMenu->addAction( QString(tr("Natural Log %1")).arg("\tln(") );
    tmp->setWhatsThis("ln(");
  advMenu->addSeparator();
  tmp = advMenu->addAction( QString(tr("Sine %1")).arg("\tsin(") );
    tmp->setWhatsThis("sin(");
  tmp = advMenu->addAction( QString(tr("Cosine %1")).arg("\tcos(") );
    tmp->setWhatsThis("cos(");
  tmp = advMenu->addAction( QString(tr("Tangent %1")).arg("\ttan(") );
    tmp->setWhatsThis("tan(");
  advMenu->addSeparator();
  tmp = advMenu->addAction( QString(tr("Arc Sine %1")).arg("\tasin(") );
    tmp->setWhatsThis("asin(");
  tmp = advMenu->addAction( QString(tr("Arc Cosine %1")).arg("\tacos(") );
    tmp->setWhatsThis("acos(");
  tmp = advMenu->addAction( QString(tr("Arc Tangent %1")).arg("\tatan(") );
    tmp->setWhatsThis("atan(");
  advMenu->addSeparator();
  tmp = advMenu->addAction( QString(tr("Hyperbolic Sine %1")).arg("\tsinh(") );
    tmp->setWhatsThis("sinh(");
  tmp = advMenu->addAction( QString(tr("Hyperbolic Cosine %1")).arg("\tcosh(") );
    tmp->setWhatsThis("cosh(");
  tmp = advMenu->addAction( QString(tr("Hyperbolic Tangent %1")).arg("\ttanh(") );
    tmp->setWhatsThis("tanh(");
}

void mainUI::start_calc(){
  if(ui->line_eq->text().isEmpty()){ return; } //nothing to do
  QString eq = ui->line_eq->text();
    eq.replace("%","/(100)");
  double result = strToNumber(eq);
  if(result!=result){ return; } //bad calculation - NaN's values are special in that they don't equal itself
  QString res = "[#%1]  %2 \t= [ %3 ]";
  ui->list_results->addItem(res.arg(QString::number(ui->list_results->count()+1), QString::number(result, 'G', 7), ui->line_eq->text()));
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
void mainUI::captureButtonDecimal(){ ui->line_eq->insert(ui->button_Decimal->text()); }

void mainUI::advMenuTriggered(QAction *act){
  if(act->whatsThis().isEmpty()){ return; }
  ui->line_eq->insert(act->whatsThis());
}
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
  else if(symbol=='e'){ return (LHS * ::exp(RHS) ); }
  //else if(symbol== '%'){ return (LHS%RHS); }
  qDebug() << "Invalid Symbol:" << symbol;
  return BADVALUE;
}

double mainUI::performSciOperation(QString func, double arg){
  //This function is for performing calculations of scientific functions "<function>(<arg>)"
  //qDebug() << "Perform Sci Op:" << func << arg;
  double res;
  if(func=="ln"){ return ::log(arg); }
  else if(func=="log"){ return ::log10(arg); }
  else if(func=="sqrt"){ return ::sqrt(arg); }
  else if(func=="sin"){ res = ::sin(arg); } //needs rounding check
  else if(func=="cos"){ res = ::cos(arg); } //needs rounding check
  else if(func=="tan"){ return ::tan(arg); }
  else if(func=="asin"){ return ::asin(arg); }
  else if(func=="acos"){ return ::acos(arg); }
  else if(func=="atan"){ return ::atan(arg); }
  else if(func=="sinh"){ return ::sinh(arg); }
  else if(func=="cosh"){ return ::cosh(arg); }
  else if(func=="tanh"){ return ::tanh(arg); }
  else{
    qDebug() << "Unknown Scientific Function:" << func;
    return BADVALUE;
  }
  //Special cases:
  // Check for whether "PI" was input as an argument (in some form) and round off the answer as needed
  //  since PI is itself a rounded number
  if(res < 0.000000000000001){ return 0; }
  return res;
}


double mainUI::strToNumber(QString str){
  //qDebug() << "String To Number:" << str;
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
    //qDebug() << "Replace value:" << str << start << end << str.mid(start+1,end-start-1);
    double tmp = strToNumber( str.mid(start+1, end-start-1));
    if(tmp!=tmp){ return BADVALUE; } //not a number
    //Now check really quick if this was an argument to a scientific operation
    //qDebug() << "Got Number:" << tmp;
    for(int i=start-1; i>=0; i-- ){
      //qDebug() << "Check for function:" << start << i << str[i];
      if( !str[i].isLower() || i==0 ){
        if(!str[i].isLower()){ i++; }//don't need the invalid character
        if(start-i<2){ break; } //not long enough - 2+ chars for sci functions
        //Got a scientific operation - run it through the routine
        tmp = performSciOperation( str.mid(i, start-i), tmp);
        if(tmp!=tmp){ return BADVALUE; } //got a bad value
        start = i; //new start point for the replacement
        break;
      }
    }
    str.replace(start, end-start+1, QString::number( tmp, 'E', 16) ); //need as high precision as possible here - internal only, never seen
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
  symbols.clear(); symbols << "x" << "*" << "/" << "^" << "e";
  for(int i=0; i<symbols.length(); i++){
    int tmp = str.indexOf(symbols[i]);
    if(sym < tmp){ sym = tmp; }
  }
  if(sym>0){  return performOperation( strToNumber(str.left(sym)), strToNumber(str.right(str.length()-sym-1)), str[sym]); }
  if(sym==0){ return BADVALUE; }

  //Could not find any operations - must be a raw number
  //qDebug() << " - Found Number:" << str;// << str.toDouble();
  if(str=="\u03C0"){ return PI; }
  //else if(str.endsWith("\u03C0")){
     //return performOperation( strToNumber(str.section("\u03C0",0,-2)), PI, '*');
  else if(str.contains("\u03C0")){
     qDebug() << " Has Pi:" << str.count("\u03C0");
    //Pi is mixed into the number - need to multiply it all out
    double res = 1;
    QStringList vals = str.split("\u03C0", QString::SkipEmptyParts);
    for(int i=0; i<vals.length(); i++){ res = res * strToNumber(vals[i]); }
    //Now multiply in the PI's
    for(int i=0; i<str.count("\u03C0"); i++){
      res = res * PI;
    }
    return res;
  }
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

void mainUI::quitShortcut_Triggered(){
    QApplication::quit();
}

//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TerminalWidget.h"

#include <QProcessEnvironment>
#include <QDebug>
#include <QApplication>
#include <QScrollBar>

TerminalWidget::TerminalWidget(QWidget *parent, QString dir) : QTextEdit(parent){
  //Setup the text widget
  this->setLineWrapMode(QTextEdit::WidgetWidth);
  //this->setStyleSheet("");
  DEFFMT = this->textCursor().charFormat(); //save the default structure for later
  CFMT = this->textCursor().charFormat(); //current format
  
  //Create/open the TTY port
  PROC = new TTYProcess(this);
  qDebug() << "Open new TTY";
  //int fd;
  bool ok = PROC->startTTY( QProcessEnvironment::systemEnvironment().value("SHELL","/bin/sh"), QStringList(), dir);
  qDebug() << " - opened:" << ok;
  this->setEnabled(PROC->isOpen());

  //Connect the signals/slots
  connect(PROC, SIGNAL(readyRead()), this, SLOT(UpdateText()) );
  connect(PROC, SIGNAL(processClosed()), this, SLOT(ShellClosed()) );
  
}

TerminalWidget::~TerminalWidget(){
  aboutToClose();
}

void TerminalWidget::aboutToClose(){
  if(PROC->isOpen()){ PROC->closeTTY(); } //TTY PORT
}

// ==================
//          PRIVATE
// ==================
void TerminalWidget::applyData(QByteArray data){
  //Iterate through the data and apply it when possible
  for(int i=0; i<data.size(); i++){
    if( data.at(i)=='\b' ){
      //Simple cursor backward 1 (NOT backspace in this context!!)
      this->moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
    }else if( data.at(i)=='\x1B' ){
      //ANSI Control Code start
      //Look for the end of the code
      int end = -1;
      for(int j=1; j<(data.size()-i) && end<0; j++){
        if(QChar(data.at(i+j)).isLetter()){ end = j; }
      }
      if(end<0){ return; } //skip everything else - no end to code found
      applyANSI(data.mid(i+1, end));
      i+=end; //move the final loop along - already handled these bytes
      
    }else if( data.at(i) != '\r' ) {
      //Special Check: if inserting text within a line, clear the rest of this line first
      if(i==0 && this->textCursor().position() < this->document()->characterCount()-1){
        applyANSI("[K");
      }
      //Plaintext character - just add it here
      //qDebug() << "Insert Text:" << data.at(i) << CFMT.foreground().color() << CFMT.background().color();
      this->textCursor().insertText( QChar(data.at(i)), CFMT );
    }
    
  } //end loop over data
}

void TerminalWidget::applyANSI(QByteArray code){
  //Note: the first byte is often the "[" character
  //qDebug() << "Handle ANSI:" << code;
	
  //CURSOR MOVEMENT
  if( code.endsWith("A") ){ //Move Up
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    this->textCursor().movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, num);
  }else if(code.endsWith("B")){ //Move Down
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    this->textCursor().movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, num);
  }else if(code.endsWith("C")){ //Move Forward
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    for(int i=0; i<num; i++){ this->moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor); }
    //this->textCursor().movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, num);
  }else if(code.endsWith("D")){ //Move Back
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    for(int i=0; i<num; i++){ this->moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor); }
    //this->textCursor().movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, num);
  }else if(code.endsWith("E")){ //Move Next/down Lines (go to beginning)
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    this->textCursor().movePosition(QTextCursor::NextRow, QTextCursor::MoveAnchor, num);
  }else if(code.endsWith("F")){ //Move Previous/up Lines (go to beginning)
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    this->textCursor().movePosition(QTextCursor::PreviousRow, QTextCursor::MoveAnchor, num);
  }else if(code.endsWith("G")){ //Move to specific column
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    this->textCursor().setPosition(num);
  }else if(code.endsWith("H") || code.endsWith("f") ){ //Move to specific position (row/column)
    int mid = code.indexOf(";");
    if(mid>0){
      int numR, numC; numR = numC = 1;
      if(mid >=3){ numR = code.mid(1,mid-1).toInt(); }
      if(mid < code.size()-1){ numC = code.mid(mid+1,code.size()-mid-1).toInt(); }
      //this->textCursor().setPosition(
      qDebug() << "Set Text Position (absolute):" << "Row:" << numR << "Col:" << numC;
      // TO-DO
    }
    
   // DISPLAY CLEAR CODES
  }else if(code.endsWith("J")){ //ED - Erase Display
    int num = 0;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    if(num==1){
      //Clear from cursor to beginning of screen
      for(int i=this->textCursor().position(); i>=0; i--){
        this->textCursor().deletePreviousChar();
      }
    }else if(num==2){
      //Clear the whole screen
      this->clear();
    }else{
      //Clear from cursor to end of screen
      for(int i=this->textCursor().position(); i<this->document()->characterCount()+1; i++){
        this->textCursor().deleteChar();
      } 
    }	    
  }else if(code.endsWith("K")){ //EL - Erase in Line
    int num = 0;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    //qDebug() << "Erase Number" << num;
    //Now determine what should be cleared based on code
    if(num==1){
      //Clear from current cursor to beginning of line
      for(int i=this->textCursor().position(); i>=0; i--){
	if(this->document()->characterAt(this->textCursor().position())=='\n'){ break; }
        this->textCursor().deleteChar();
      }	    
    }else if(num==2){
      //Clear the entire line
      //  rewind the starting point to the beginning of the line
      int start = this->document()->find("\n", this->textCursor().position(), QTextDocument::FindBackward).position();
      for(int i=start+1; i<this->document()->characterCount()+1; i++){
	if(this->document()->characterAt(this->textCursor().position())=='\n'){ break; }
        this->textCursor().deleteChar();
      }
    }else{
      //Clear from current cursor to end of line
      for(int i=this->textCursor().position(); i<this->document()->characterCount()+1; i++){
	if(this->document()->characterAt(this->textCursor().position())=='\n'){ break; }
        this->textCursor().deleteChar();
      }
    }
    
   //SCROLL MOVEMENT CODES 
  }else if(code.endsWith("S")){ // SU - Scrolll Up
	  
  }else if(code.endsWith("T")){ // SD - Scroll Down
	  
	  
  // GRAPHICS RENDERING
  }else if(code.endsWith("m")){
    //Format: "[<number>;<number>m" (no limit to sections separated by ";")
    int start = 1;
    int end = code.indexOf(";");
    while(end>start){
      applyANSIColor(code.mid(start, end-start).toInt());
      //Now update the iterators and try again
      start = end;
      end = code.indexOf(";",start+1); //go to the next one
    }
    //Need the last section as well
    end = code.size()-1;
    if(end>start){ applyANSIColor(code.mid(start, end-start).toInt());}
    else{ applyANSIColor(0); }
  }
  
  
}

void TerminalWidget::applyANSIColor(int code){
  qDebug() << "Apply Color code:" << code;
  if(code <=0){ CFMT = DEFFMT; } //Reset back to default
  else if(code==1){  CFMT.setFontWeight(75); } //BOLD font
  else if(code==2){ CFMT.setFontWeight(25); } //Faint font (smaller than normal by a bit)
  else if(code==3){ CFMT.setFontWeight(75); } //Italic font
  else if(code==4){ CFMT.setFontUnderline(true); } //Underline
  //5-6: Blink text (unsupported)
  //7: Reverse foreground/background (unsupported)
  //8: Conceal (unsupported)
  else if(code==9){ CFMT.setFontStrikeOut(true); } //Crossed out
  //10-19: Change font family (unsupported)
  //20: Fraktur Font (unsupported)
  //21: Bold:off or Underline:Double (unsupported)
  else if(code==22){ CFMT.setFontWeight(50); } //Normal weight
  //23: Reset font (unsupported)
  else if(code==24){ CFMT.setFontUnderline(false); } //disable underline
  //25: Disable blinking (unsupported)
  //26: Reserved
  //27: Reset reversal (7) (unsupported)
  //28: Reveal (cancel 8) (unsupported)
  else if(code==29){ CFMT.setFontStrikeOut(false); } //Not Crossed out
  else if(code>=30 && code<=39){
    //Set the font color
   QColor color;
    if(code==30){color=QColor(Qt::black); }
    else if(code==31){ color=QColor(Qt::red); }
    else if(code==32){ color=QColor(Qt::green); }
    else if(code==33){ color=QColor(Qt::yellow); }
    else if(code==34){ color=QColor(Qt::blue); }
    else if(code==35){ color=QColor(Qt::magenta); }
    else if(code==36){ color=QColor(Qt::cyan); }
    else if(code==37){ color=QColor(Qt::white); }
    //48: Special extended color setting (unsupported)
    else if(code==39){ color= DEFFMT.foreground().color(); } //reset to default color
QBrush brush = CFMT.background();
    color.setAlpha(255); //fully opaque
    brush.setColor(color);
    CFMT.setForeground( brush );
  }
  else if(code>=40 && code<=49){
    //Set the font color
   QColor color;
    if(code==40){color=QColor(Qt::black); }
    else if(code==41){ color=QColor(Qt::red); }
    else if(code==42){ color=QColor(Qt::green); }
    else if(code==43){ color=QColor(Qt::yellow); }
    else if(code==44){ color=QColor(Qt::blue); }
    else if(code==45){ color=QColor(Qt::magenta); }
    else if(code==46){ color=QColor(Qt::cyan); }
    else if(code==47){ color=QColor(Qt::white); }
    //48: Special extended color setting (unsupported)
    else if(code==49){ color= DEFFMT.background().color(); } //reset to default color
    QBrush brush = CFMT.background();
    color.setAlpha(255); //fully opaque
    brush.setColor(color);
    CFMT.setBackground( brush );
  }
  //50: Reserved
  //51: Framed
  //52: Encircled
  else if(code==53){ CFMT.setFontOverline(true); } //enable overline
  //54: Not framed/circled (51/52)
  else if(code==55){ CFMT.setFontOverline(false); } //disable overline
  //56-59: Reserved
  //60+: Not generally supported (special code for particular terminals such as aixterm)
}

//Outgoing Data parsing
void TerminalWidget::sendKeyPress(int key){
  QByteArray ba;
  //Check for special keys
  switch(key){
    case Qt::Key_Delete:
	ba.append("\x7F");
        break;
    case Qt::Key_Backspace:
	ba.append("\x08");    
        break;
    case Qt::Key_Left:
	ba.append("\x1b[D");
        break;
    case Qt::Key_Right:
	ba.append("\x1b[C");
        break;
    case Qt::Key_Up:
        ba.append("\x1b[A");
        break;
    case Qt::Key_Down:
        ba.append("\x1b[B");
        break;
  }
   
  //qDebug() << "Forward Input:" << txt << ev->key() << ba;
  if(!ba.isEmpty()){ PROC->writeTTY(ba); }
}

// ==================
//    PRIVATE SLOTS
// ==================
void TerminalWidget::UpdateText(){
  //read the data from the process
  //qDebug() << "UpdateText";
  if(!PROC->isOpen()){ return; }
  applyData(PROC->readTTY());
  //adjust the scrollbar as needed
  this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}

void TerminalWidget::ShellClosed(){
  emit ProcessClosed(this->whatsThis());
}

// ==================
//       PROTECTED
// ==================
void TerminalWidget::keyPressEvent(QKeyEvent *ev){
	
  if(ev->text().isEmpty() || ev->text()=="\b" ){
    sendKeyPress(ev->key());
  }else{
    QByteArray ba; ba.append(ev->text()); //avoid any byte conversions
    PROC->writeTTY(ba);
  }
  
  ev->ignore();
}

void TerminalWidget::mousePressEvent(QMouseEvent *ev){
  this->setFocus();
  if(ev->button()==Qt::RightButton){
    QTextEdit::mousePressEvent(ev);	  
  }else{
    Q_UNUSED(ev);
  }
}

void TerminalWidget::mouseDoubleClickEvent(QMouseEvent *ev){
  Q_UNUSED(ev);	
}

void TerminalWidget::contextMenuEvent(QContextMenuEvent *ev){
  Q_UNUSED(ev);	
  //QTextEdit::contextMenuEvent(ev);
  //Need to implement a custom context menu
}

void TerminalWidget::resizeEvent(QResizeEvent *ev){
  if(!PROC->isOpen()){ return; }
  QSize pix = ev->size(); //pixels
  QSize chars; 
    chars.setWidth( pix.width()/this->fontMetrics().width("W") );
    chars.setHeight( pix.height()/this->fontMetrics().lineSpacing() );
  
  PROC->setTerminalSize(chars,pix);
  QTextEdit::resizeEvent(ev);
}

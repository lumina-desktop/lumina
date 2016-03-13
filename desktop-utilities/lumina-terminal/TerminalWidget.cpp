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
  //this->setReadOnly(true); //the key event catch will do the process/widget forwarding
  //this->setPlainText("WARNING: This utility is still incomplete and does not function properly yet");
  
  //Create/open the TTY port
  PROC = new TTYProcess(this);
  qDebug() << "Open new TTY";
  //int fd;
  bool ok = PROC->startTTY( QProcessEnvironment::systemEnvironment().value("SHELL","/bin/sh") );
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
  //Quick global replacement (this widget reads both as newlines)
  data = data.replace("\r\n","\n");
  //Iterate through the data and apply it when possible
  for(int i=0; i<data.size(); i++){
    if( data.at(i)=='\b' ){
      //Simple Backspace
      this->textCursor().deletePreviousChar();
	    
    }else if( data.at(i)=='\x1B' ){
      //ANSI Control Code start
      //Look for the end of the code
      int end = -1;
      for(int j=1; j<(data.size()-i) && end<0; j++){
        if(QChar(data.at(i+j)).isLetter()){ end = j; }
      }
      if(end<0){ return; } //skip everything else - no end to code found
      applyANSI(data.mid(i, end));
      i+=end; //move the final loop along - already handled these bytes
      
    }else{
      //Plaintext character - just add it here
      this->insertPlainText( QChar(data.at(i)) );
    }
    
  } //end loop over data
}

void TerminalWidget::applyANSI(QByteArray code){
  //Note: the first byte is often the "[" character
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
    this->textCursor().movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, num);
  }else if(code.endsWith("D")){ //Move Back
    int num = 1;
    if(code.size()>2){ num = code.mid(1, code.size()-2).toInt(); } //everything in the middle
    this->textCursor().movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, num);
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
  }else if(code.endsWith("H")){ //Move to specific position (row/column)
    int mid = code.indexOf(";");
    if(mid>0){
      int numR, numC; numR = numC = 1;
      if(mid >=3){ numR = code.mid(1,mid-1).toInt(); }
      if(mid < code.size()-1){ numC = code.mid(mid+1,code.size()-mid-1).toInt(); }
      //this->textCursor().setPosition(
      qDebug() << "Set Text Position (absolute):" << "Row:" << numR << "Col:" << numC;
      // TO-DO
    }
  }else if(code.endsWith("J")){ //ED - Erase Display
    
  }
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
    PROC->writeQtKey(ev->key());
  }else{
    QByteArray ba; ba.append(ev->text()); //avoid any byte conversions
    PROC->writeTTY(ba);
  }
  
  ev->ignore();
}

void TerminalWidget::mousePressEvent(QMouseEvent *ev){
  this->setFocus();
  Q_UNUSED(ev);
}

void TerminalWidget::mouseDoubleClickEvent(QMouseEvent *ev){
  Q_UNUSED(ev);	
}

void TerminalWidget::contextMenuEvent(QContextMenuEvent *ev){
  Q_UNUSED(ev);	
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

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

TerminalWidget::TerminalWidget(QWidget *parent, QString dir) : QTextEdit(parent){
  //Setup the text widget
  this->setLineWrapMode(QTextEdit::WidgetWidth);
  this->setReadOnly(true); //the key event catch will do the process/widget forwarding
  this->setPlainText("WARNING: This utility is still incomplete and does not function properly yet");
  //Create/launch the process
  PROC = new QProcess(this);
    PROC->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    PROC->setWorkingDirectory(dir);
    PROC->setProcessChannelMode(QProcess::MergedChannels);
  //Connect the signals/slots
  connect(PROC, SIGNAL(readyReadStandardOutput()), this, SLOT(UpdateText()) );
  connect(PROC, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(ShellClosed()) );
  //Now start the shell
  PROC->start( PROC->processEnvironment().value("SHELL","/bin/sh"), QIODevice::ReadWrite);
  
}

TerminalWidget::~TerminalWidget(){
	
}

void TerminalWidget::aboutToClose(){
  if(PROC->state()!=QProcess::NotRunning){ PROC->close(); }
  if(PROC->state()!=QProcess::NotRunning){ PROC->kill(); }
  if(PROC->state()!=QProcess::NotRunning){ PROC->terminate(); }
}

// ==================
//    PRIVATE SLOTS
// ==================
void TerminalWidget::UpdateText(){
  while(PROC->canReadLine()){
    QString line = PROC->readLine();
    this->insertPlainText( line );
  }
}

void TerminalWidget::ShellClosed(){
  emit ProcessClosed(this->whatsThis());
}

// ==================
//       PROTECTED
// ==================
void TerminalWidget::keyPressEvent(QKeyEvent *ev){
  //The way this works is by printing the keys directly to the text edit widget
  //  While also keeping an internal "buffer" of the input string so it can all be written to the process at the same time
  
  QString key = ev->text();
    //Check for special key combinations first
    switch(ev->key()){
	case Qt::Key_Enter:
	case Qt::Key_Return:
	  //send the current input buffer to the process
	  qDebug() << "Write to process:" << inBuffer;
	  this->insertPlainText("\n");
	  PROC->write( QString(inBuffer+"\n").toLocal8Bit() );
	  inBuffer.clear();
	  qDebug() << "Current Dir:" << PROC->workingDirectory();
	  break;
	case Qt::Key_Backspace:
	  inBuffer.chop(1);
	  break;
	default:
	  this->insertPlainText(key); 
	  inBuffer.append(key);	
    }

  ev->ignore();
}
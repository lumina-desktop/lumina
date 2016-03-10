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
  //this->setReadOnly(true); //the key event catch will do the process/widget forwarding
  //this->setPlainText("WARNING: This utility is still incomplete and does not function properly yet");
  
  //Create/open the TTY port
  PROC = new TTYProcess(this);
  qDebug() << "Open new TTY";
  int fd;
  bool ok = PROC->startTTY( fd, QProcessEnvironment::systemEnvironment().value("SHELL","/bin/sh") );
  qDebug() << " - opened:" << ok;
  this->setEnabled(PROC->isOpen());

  //Connect the signals/slots
  //connect(PROC, SIGNAL(readyRead()), this, SLOT(UpdateText()) );
  connect(PROC, SIGNAL(aboutToClose()), this, SLOT(ShellClosed()) );

  sn= new QSocketNotifier(fd, QSocketNotifier::Read);
  sn->setEnabled(true);
  connect(sn, SIGNAL(activated(int)), this, SLOT(UpdateText()));
  
  upTimer = new QTimer(this);
    upTimer->setInterval(1000);
  //  connect(upTimer, SIGNAL(timeout()), this, SLOT(UpdateText()) );

  upTimer->start();
}

TerminalWidget::~TerminalWidget(){
  aboutToClose();
}

void TerminalWidget::aboutToClose(){
  if(PROC->isOpen()){ PROC->close(); } //TTY PORT
  //if(PROC->state()!=QProcess::NotRunning){ PROC->close(); PROC->kill(); } //QProcess
}

// ==================
//    PRIVATE SLOTS
// ==================
void TerminalWidget::UpdateText(){
  //read the data from the process
  qDebug() << "UpdateText";
  if(!PROC->isOpen()){ return; }
  //if ( PROC->bytesAvailable() <= 0 )
  //  return;

  qDebug() << "Reading all data";
  char buffer[64];
  ssize_t rtot = read(sn->socket(),&buffer,64);
  qDebug()<<(quint8)buffer[0]<<(quint8)buffer[1]<<(quint8)buffer[2]<<(quint8)buffer[3];
  buffer[rtot]='\0';
  qDebug() << "Process Data:" << QString(buffer);
  this->insertPlainText(QString(buffer));
  //adjust the scrollbar as needed
	
}

void TerminalWidget::ShellClosed(){
  emit ProcessClosed(this->whatsThis());
}

// ==================
//       PROTECTED
// ==================
void TerminalWidget::keyPressEvent(QKeyEvent *ev){
    //Check for special key combinations first
    QString txt = ev->text();
    if(txt.isEmpty()){ return; } // modifier key - nothing to send yet
    switch(ev->key()){
	//case Qt::Key_Backspace:
	case Qt::Key_Left:
	case Qt::Key_Right:
	case Qt::Key_Up:
	case Qt::Key_Down:
	  break;
	/*case Qt::Key_Return:
	case Qt::Key_Enter:
	  txt = "\r";*/
	default:
	  //All other events can get echoed onto the widget (non-movement)
	  QTextEdit::keyPressEvent(ev); //echo the input on the widget
    }
  QByteArray ba; ba.append(txt); //avoid any byte conversions
  qDebug() << "Forward Input:" << txt << ev->key();
  PROC->write(ba);
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

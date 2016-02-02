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
  
  //Create/open the serial port
  /*PROC = new QSerialPort(this);
    QList<QSerialPortInfo>  openports = QSerialPortInfo::availablePorts();
    //Now print out all the information
    if(openports.isEmpty()){ this->setEnabled(false);}
    else{
      //Go through the open ports until we find one that can be used
      for(int i=0; i<openports.length(); i++){
	qDebug() << "Port:" << openports[i].description();
	qDebug() << "Name:" << openports[i].portName();
	qDebug() << "Serial Number:" << openports[i].serialNumber();
	qDebug() << "System Location:" << openports[i].systemLocation();
	PROC->setPort(openports[i]);
	if(PROC->open(QIODevice::ReadWrite) ){ break; }
      }
    }
  this->setEnabled(PROC->isOpen());
  //Connect the signals/slots
  connect(PROC, SIGNAL(readyRead()), this, SLOT(UpdateText()) );
  connect(PROC, SIGNAL(aboutToClose()), this, SLOT(ShellClosed()) );
  */
    
   //Create/launch the process 
  PROC = new QProcess(this);
    PROC->setProcessChannelMode(QProcess::MergedChannels);
    PROC->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    PROC->setProgram( PROC->processEnvironment().value("SHELL","/bin/sh") );
    PROC->setWorkingDirectory(dir);
  //Connect the signals/slots
  connect(PROC, SIGNAL(readyReadStandardOutput()), this, SLOT(UpdateText()) );
  connect(PROC, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(ShellClosed()) );
  //Now start the shell
  //PROC->start("login" , QStringList() << "-f" << getlogin(), QIODevice::ReadWrite);
  PROC->start(QIODevice::ReadWrite);
  
}

TerminalWidget::~TerminalWidget(){
	
}

void TerminalWidget::aboutToClose(){
  //if(PROC->isOpen()){ PROC->close(); }
  if(PROC->state()!=QProcess::NotRunning){ PROC->kill(); }
}

// ==================
//    PRIVATE SLOTS
// ==================
void TerminalWidget::UpdateText(){
  //read the data from the process
  //QByteArray data = PROC->readAll();
  qDebug() << "Process Data Available";
  QByteArray data = PROC->readAllStandardOutput();
  this->insertPlainText(QString(data));
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
    switch(ev->key()){
	case Qt::Key_Backspace:
	case Qt::Key_Left:
	case Qt::Key_Right:
	case Qt::Key_Up:
	case Qt::Key_Down:
	  break;
	case Qt::Key_Return:
	case Qt::Key_Enter:
	  txt = "\r";
	default:
	  //All other events can get echoed onto the widget (non-movement)
	  QTextEdit::keyPressEvent(ev); //echo the input on the widget
    }
  qDebug() << "Forward Input:" << txt << ev->key();
  PROC->write(txt.toLocal8Bit());
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
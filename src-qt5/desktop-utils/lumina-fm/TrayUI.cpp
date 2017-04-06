//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TrayUI.h"

#include <LuminaXDG.h>
#include<QUuid>

TrayUI::TrayUI(QObject *parent) : QSystemTrayIcon(parent){
  this->setContextMenu( new QMenu() );
  this->setIcon(LXDG::findIcon("Insight-FileManager",""));
  connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(TrayActivated()));
}

TrayUI::~TrayUI(){
  this->contextMenu()->deleteLater();
}

void TrayUI::StartOperation( FILEOP op, QStringList oldF, QStringList newF){
  createOP(op, oldF, newF);
  QTimer::singleShot(1000, this, SLOT(checkJobs()));
}

void TrayUI::createOP( FILEOP type, QStringList oldF, QStringList newF){
  OPWidget *OP = new OPWidget();
  if(type==MOVE){ OP->setupOperation("move", oldF, newF); }
  else if(type==COPY){ OP->setupOperation("copy", oldF, newF); }
  else if(type==DELETE){ OP->setupOperation("delete",oldF, QStringList()); }
  else{ OP->deleteLater(); return; } //invalid type of operation
  OP->setWhatsThis( QUuid::createUuid().toString() );
  this->contextMenu()->addAction(OP->widgetAction());
  OPS << OP;
  connect(OP, SIGNAL(starting(QString)), this, SLOT(OperationStarted(QString)) );
  connect(OP, SIGNAL(finished(QString)), this, SLOT(OperationFinished(QString)) );
  connect(OP, SIGNAL(closed(QString)), this, SLOT(OperationClosed(QString)) );
  QTimer::singleShot(0, OP, SLOT(startOperation()) );
}

void TrayUI::TrayActivated(){
  this->contextMenu()->popup( this->geometry().center() );
}

//Operation Widget Responses
void TrayUI::OperationClosed(QString ID){
  for(int i=0; i<OPS.length(); i++){
    if(OPS[i]->whatsThis()==ID){
      //qDebug() << "Removing OPWidget:" << ID;
      //this->contextMenu()->removeAction(OPS[i]->widgetAction());
      OPS.takeAt(i)->deleteLater();
      break;
    }
  }
  QTimer::singleShot(1000, this, SLOT(checkJobs()) );
}

void TrayUI::OperationStarted(QString ID){
  for(int i=0; i<OPS.length(); i++){
    if(OPS[i]->whatsThis()==ID){
      //NOTHING FOR NOW - ENABLE POPUPS LATER (if desired - they can get annoying for short operations)
    }
  }
}

void TrayUI::OperationFinished(QString ID){
  //qDebug() << "Op Finished:" << ID;
  for(int i=0; i<OPS.length(); i++){
    if(OPS[i]->whatsThis()!=ID){ continue; }
    //qDebug() << " - found widget";
    bool err = OPS[i]->hasErrors();
    //qDebug() << " -- Errors:" << err << "Duration:" << OPS[i]->duration();
    //Assemble the notification (if more than 1 second to perform operation)
    if(OPS[i]->duration()>1){
      this->showMessage( tr("Finished"), err ? tr("Errors during operation. Click to view details") : "", err ? QSystemTrayIcon::Warning : QSystemTrayIcon::Information);
    }
    //Close the widget if no errors
    if(!err){ OperationClosed(ID); }
    break;
  }
}

void TrayUI::checkJobs(){
  if(OPS.isEmpty()){
    emit JobsFinished();
    this->hide();
  }else{
    bool showNotify = !this->isVisible();
    this->show();
    if(showNotify){ this->showMessage(tr("New Tasks Running"),"",QSystemTrayIcon::NoIcon, 2000); }
  }
}

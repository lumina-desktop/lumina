//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "OPWidget.h"
#include "ui_OPWidget.h"

#include "ScrollDialog.h"

OPWidget::OPWidget(QWidget *parent) : QWidget(parent), ui(new Ui::OPWidget()){
  starttime = endtime = -1;
  WA = new QWidgetAction(0);
  WA->setDefaultWidget(this);
  worker = 0;
  workthread = 0;
  dlg = 0;
  //Now create the widget
  ui->setupUi(this);
  ui->tool_close->setIcon( LXDG::findIcon("dialog-close","view-close") );
  ui->tool_showerrors->setIcon(LXDG::findIcon("view-choose","view-preview"));
  //connect the widget buttons
  connect(ui->tool_close, SIGNAL(clicked()), this, SLOT(closeWidget()) );
  connect(ui->tool_showerrors, SIGNAL(clicked()), this, SLOT(showErrors()) );
}

OPWidget::~OPWidget(){
  if(worker!=0){ worker->stopped = true; worker->deleteLater(); }
  if(workthread!=0){ workthread->quit(); workthread->wait(); delete workthread; }
  WA->deleteLater();
  if(dlg!=0){ dlg->deleteLater(); }
}

QWidgetAction* OPWidget::widgetAction(){
  return WA;
}

void OPWidget::setupOperation(QString optype, QStringList oldF, QStringList newF){
  if(workthread==0){ workthread = new QThread(); }
  if(worker==0){
    worker = new FOWorker();
    connect(worker, SIGNAL(startingItem(int,int,QString,QString)), this, SLOT(opUpdate(int,int,QString,QString)) );
    connect(worker, SIGNAL(finished(QStringList)), this, SLOT(opFinished(QStringList)) );
    worker->moveToThread(workthread);
  }
  workthread->start();
  //Now setup the worker with the desired operation
  optype = optype.toLower();
  worker->ofiles = oldF;
  worker->nfiles = newF;
  if(optype=="move"){ worker->isMV = true; tract = tr("Move"); }
  else if(optype=="copy"){ worker->isCP = true; tract = tr("Copy"); }
  else if(optype=="delete"){ worker->isRM = true; tract = tr("Remove"); }

}


bool OPWidget::isDone(){
  return (endtime>0);
}

bool OPWidget::hasErrors(){
  return !Errors.isEmpty();
}

float OPWidget::duration(){
  return ( (endtime-starttime)/1000.0); //convert from ms to s
}


QString OPWidget::finalStat(){
  return ui->label->text();
}


//PUBLIC SLOTS
void OPWidget::startOperation(){
  starttime = QDateTime::currentMSecsSinceEpoch();
  endtime = -1;
  QTimer::singleShot(0, worker, SLOT(slotStartOperations()) );
  emit starting(this->whatsThis());
}


// PRIVATE SLOTS
void OPWidget::closeWidget(){
  if(!isDone()){ worker->stopped = true; }
  else{ emit closed(this->whatsThis()); }
}

void OPWidget::showErrors(){
  qDebug() << "Errors:" << Errors;
  if(dlg==0){
    dlg = new ScrollDialog(); //need this to persist outside this function
    dlg->setWindowTitle(tr("File Operation Errors"));
    dlg->setText( Errors.join("\n") );
  }
  dlg->showNormal();
}

void OPWidget::opFinished(QStringList errors){
  Errors = errors;
  endtime = QDateTime::currentMSecsSinceEpoch();
  emit finished(this->whatsThis());
  ui->progressBar->setValue(ui->progressBar->maximum()); //last item finished
  ui->tool_showerrors->setVisible(!Errors.isEmpty());
  ui->label->setText( QString(tr("%1 Finished")).arg(tract) + (errors.isEmpty() ? "" : (" ("+tr("Errors Occured")+")") ) );
}

void OPWidget::opUpdate(int cur, int tot, QString ofile, QString nfile){ //current, total, old file, new file
  ui->progressBar->setRange(0,tot);
  ui->progressBar->setValue(cur);
  QString txt = tract +": "+ofile.section("/",-1);
  if(!nfile.isEmpty()){txt.append(" -> "+nfile.section("/",-1) ); }
  ui->label->setText( txt);
}

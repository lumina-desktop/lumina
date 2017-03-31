//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h"
#include <QDebug>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI()){
  ui->setupUi(this);
  //Any special UI changes
  QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ui->toolBar->insertWidget(ui->actionVolUp, spacer);

  setupPandora();
  ui->radio_pandora->setChecked(true);
  setupConnections();
}

MainUI::~MainUI(){

}

void MainUI::loadArguments(QStringList){

}


// ==== PRIVATE ====
void MainUI::setupPandora(){
  PANDORA = new PianoBarProcess(this);
  connect(PANDORA, SIGNAL(currentStateChanged(PianoBarProcess::State)), this, SLOT(PandoraStateChanged(PianoBarProcess::State)) );
  connect(PANDORA, SIGNAL(NewInformation(QString)), this, SLOT(NewPandoraInfo(QString)) );
  connect(PANDORA, SIGNAL(NowPlayingStation(QString, QString)), this, SLOT(PandoraStationChanged(QString)) );
  connect(PANDORA, SIGNAL(NowPlayingSong(bool, QString,QString,QString, QString, QString)), this, SLOT(PandoraSongChanged(bool, QString, QString, QString, QString, QString)) );
  connect(PANDORA, SIGNAL(TimeUpdate(int, int)), this, SLOT(PandoraTimeUpdate(int,int)) );
  connect(PANDORA, SIGNAL(NewList(QStringList)), this, SLOT(PandoraListInfo(QStringList)) );
  connect(PANDORA, SIGNAL(StationListChanged(QStringList)), this, SLOT(PandoraStationListChanged(QStringList)) );
  //Setup a couple of the option lists
  ui->combo_pandora_quality->clear();
  ui->combo_pandora_quality->addItem(tr("Low"),"low");
  ui->combo_pandora_quality->addItem(tr("Medium"), "medium");
  ui->combo_pandora_quality->addItem(tr("High"),"high");
  //Now load the current settings into the UI
  int qual =   ui->combo_pandora_quality->findData(PANDORA->audioQuality());
  if(qual>=0){ ui->combo_pandora_quality->setCurrentIndex(qual); }
  else{   ui->combo_pandora_quality->setCurrentIndex(1); } //medium quality by default
  ui->line_pandora_email->setText( PANDORA->email() );
  ui->line_pandora_pass->setText( PANDORA->password() );
  ui->line_pandora_proxy->setText( PANDORA->proxy() );
  ui->line_pandora_cproxy->setText( PANDORA->controlProxy() );
}

void MainUI::setupConnections(){
  connect(ui->actionPlay, SIGNAL(triggered()), this, SLOT(playToggled()) );
  connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pauseToggled()) );
  connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(stopToggled()) );
  connect(ui->actionNext, SIGNAL(triggered()), this, SLOT(nextToggled()) );
  connect(ui->actionBack, SIGNAL(triggered()), this, SLOT(backToggled()) );
  connect(ui->actionVolUp, SIGNAL(triggered()), this, SLOT(volupToggled()) );
  connect(ui->actionVolDown, SIGNAL(triggered()), this, SLOT(voldownToggled()) );
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()) );
  connect(ui->push_pandora_apply, SIGNAL(clicked()), this, SLOT(applyPandoraSettings()) );
}


// ==== PRIVATE SLOTS ====
void MainUI::PlayerTypeChanged(){
  if(ui->radio_pandora->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_pandora); }
  else{ ui->stackedWidget->setCurrentWidget(ui->page_local); }
  //Now hide/deactivate any toolbar buttons which are not used
  ui->actionBack->setVisible(!ui->radio_pandora->isChecked());
}


//Toolbar actions
void MainUI::playToggled(){
  if(ui->radio_pandora->isChecked()){
    PANDORA->play();
  }
}

void MainUI::pauseToggled(){
  if(ui->radio_pandora->isChecked()){
    PANDORA->pause();
  }  
}

void MainUI::stopToggled(){
  if(ui->radio_pandora->isChecked()){
    PANDORA->closePianoBar();
  }
}

void MainUI::nextToggled(){
  if(ui->radio_pandora->isChecked()){
    PANDORA->skipSong();
  }
}

void MainUI::backToggled(){

}

void MainUI::volupToggled(){

}

void MainUI::voldownToggled(){

}


//Pandora Options
void MainUI::showPandoraSongInfo(){

}

void MainUI::changePandoraStation(QString){

}

void MainUI::applyPandoraSettings(){
  PANDORA->setLogin(ui->line_pandora_email->text(), ui->line_pandora_pass->text());
  PANDORA->setAudioQuality(ui->combo_pandora_quality->currentData().toString());
  PANDORA->setProxy(ui->line_pandora_proxy->text());
  PANDORA->setControlProxy(ui->line_pandora_cproxy->text());
}

//Pandora Process Feedback
void MainUI::PandoraStateChanged(PianoBarProcess::State){

}

void MainUI::NewPandoraInfo(QString info){
  qDebug() << "[INFO]" << info;
}

void MainUI::PandoraStationChanged(QString station){
  qDebug() << "[STATION CHANGE]" << station;
}

void MainUI::PandoraSongChanged(bool isLoved, QString title, QString artist, QString album, QString detailsURL, QString fromStation){
  qDebug() << "[SONG CHANGE]" << isLoved << title << artist << album << detailsURL << fromStation;
}

void MainUI::PandoraTimeUpdate(int curS, int totS){
  qDebug() << "[TIME UPDATE]" << curS << "/" << totS;
}

void MainUI::PandoraStationListChanged(QStringList list){
  qDebug() << "[STATION LIST]" << list;
}

void MainUI::PandoraListInfo(QStringList list){
  qDebug() << "[LIST INFO]" << list;
}

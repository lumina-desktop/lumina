//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h"
#include <QDebug>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <QDesktopServices>
#include <QUrl>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI()){
  ui->setupUi(this);
  closing = false;
  //Any special UI changes
  QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ui->toolBar->insertWidget(ui->actionVolDown, spacer);
  //Setup an action group for the various modes/streams
  QButtonGroup *grp = new QButtonGroup(this);
    grp->addButton(ui->radio_local);
    grp->addButton(ui->radio_pandora);
    grp->setExclusive(true);
  setupPandora();
  ui->radio_pandora->setChecked(true);
  setupTrayIcon();
  setupConnections();
  setupIcons();
  PlayerTypeChanged();
  SYSTRAY->show();
  checkPandoraSettings();
}

MainUI::~MainUI(){

}

void MainUI::loadArguments(QStringList){

}


// ==== PRIVATE ====
void MainUI::setupPandora(){
  if(!LUtils::isValidBinary("pianobar")){
    ui->radio_pandora->setEnabled(false);
    ui->radio_local->setChecked(true);
    ui->radio_pandora->setToolTip(tr("Please install the `pianobar` utility to enable this functionality"));
    ui->radio_pandora->setStatusTip(ui->radio_pandora->toolTip());
  }
  ui->radio_pandora->setToolTip(tr("Stream music from the Pandora online radio service"));
  ui->radio_pandora->setStatusTip(ui->radio_pandora->toolTip());
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
  //Make sure the interface is enabled/disabled as needed
  PandoraStateChanged(PANDORA->currentState());
  ui->progress_pandora->setRange(0,1);
  ui->progress_pandora->setValue(0);

  //Setup the menu for new stations
  QMenu *tmp = new QMenu(this);
  tmp->addAction(ui->action_pandora_newstation_song);
  tmp->addAction(ui->action_pandora_newstation_artist);
  ui->tool_pandora_stationadd->setMenu( tmp );

}

void MainUI::setupConnections(){
  connect(ui->radio_local, SIGNAL(toggled(bool)), this, SLOT(PlayerTypeChanged(bool)) );
  connect(ui->radio_pandora, SIGNAL(toggled(bool)), this, SLOT(PlayerTypeChanged(bool)) );

  connect(ui->actionPlay, SIGNAL(triggered()), this, SLOT(playToggled()) );
  connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pauseToggled()) );
  connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(stopToggled()) );
  connect(ui->actionNext, SIGNAL(triggered()), this, SLOT(nextToggled()) );
  connect(ui->actionBack, SIGNAL(triggered()), this, SLOT(backToggled()) );
  connect(ui->actionVolUp, SIGNAL(triggered()), this, SLOT(volupToggled()) );
  connect(ui->actionVolDown, SIGNAL(triggered()), this, SLOT(voldownToggled()) );
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeApplication()) );

  connect(ui->push_pandora_apply, SIGNAL(clicked()), this, SLOT(applyPandoraSettings()) );
  connect(ui->combo_pandora_station, SIGNAL(activated(QString)), this, SLOT(changePandoraStation(QString)) );
  connect(ui->tool_pandora_ban, SIGNAL(clicked()), PANDORA, SLOT(banSong()) );
  connect(ui->tool_pandora_love, SIGNAL(clicked()), PANDORA, SLOT(loveSong()) );
  connect(ui->tool_pandora_tired, SIGNAL(clicked()), PANDORA, SLOT(tiredSong()) );
  connect(ui->tool_pandora_info, SIGNAL(clicked()), this, SLOT(showPandoraSongInfo()) );
  connect(ui->tool_pandora_stationrm, SIGNAL(clicked()), PANDORA, SLOT(deleteCurrentStation()) );
  connect(ui->action_pandora_newstation_artist, SIGNAL(triggered()), PANDORA, SLOT(createStationFromCurrentArtist()) );
  connect(ui->action_pandora_newstation_song, SIGNAL(triggered()), PANDORA, SLOT(createStationFromCurrentSong()) );
  connect(ui->line_pandora_email, SIGNAL(textChanged(QString)), this, SLOT(checkPandoraSettings()) );
  connect(ui->line_pandora_pass, SIGNAL(textChanged(QString)), this, SLOT(checkPandoraSettings()) );
  connect(ui->line_pandora_proxy, SIGNAL(textChanged(QString)), this, SLOT(checkPandoraSettings()) );
  connect(ui->line_pandora_cproxy, SIGNAL(textChanged(QString)), this, SLOT(checkPandoraSettings()) );
  connect(ui->combo_pandora_quality, SIGNAL(currentIndexChanged(int)), this, SLOT(checkPandoraSettings()) );


  connect(SYSTRAY, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)) );
  connect(SYSTRAY, SIGNAL(messageClicked()), this, SLOT(trayMessageClicked()) );
}

void MainUI::setupIcons(){
  ui->radio_local->setIcon( LXDG::findIcon("audio-x-generic","") );

  ui->actionClose->setIcon( LXDG::findIcon("application-close","dialog-close") );
  ui->actionPlay->setIcon( LXDG::findIcon("media-playback-start","") );
  ui->actionPause->setIcon( LXDG::findIcon("media-playback-pause","") );
  ui->actionStop->setIcon( LXDG::findIcon("media-playback-stop","") );
  ui->actionNext->setIcon( LXDG::findIcon("media-skip-forward","") );
  ui->actionBack->setIcon( LXDG::findIcon("media-skip-backward","") );
  ui->actionVolUp->setIcon( LXDG::findIcon("audio-volume-high","") );
  ui->actionVolDown->setIcon( LXDG::findIcon("audio-volume-low","") );

  //Pandora Pages
  ui->push_pandora_apply->setIcon( LXDG::findIcon("dialog-ok-apply","dialog-ok") );
  ui->tool_pandora_ban->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->tool_pandora_info->setIcon( LXDG::findIcon("help-about","") );
  ui->tool_pandora_love->setIcon( LXDG::findIcon("emblem-favorite","") );
  ui->tool_pandora_tired->setIcon( LXDG::findIcon("flag-yellow","flag") );
  ui->tool_pandora_stationrm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_pandora_stationadd->setIcon( LXDG::findIcon("list-add","") );
  ui->action_pandora_newstation_artist->setIcon( LXDG::findIcon("list-add-user","") );
  ui->action_pandora_newstation_song->setIcon( LXDG::findIcon("audio-x-generic","") );

}

void MainUI::setupTrayIcon(){
  SYSTRAY = new QSystemTrayIcon(this);
  QMenu *tmp = new QMenu(this);
  SYSTRAY->setContextMenu(tmp);
  tmp->addAction(ui->actionPlay);
  tmp->addAction(ui->actionPause);
  tmp->addAction(ui->actionStop);
  tmp->addAction(ui->actionBack);
  tmp->addAction(ui->actionNext);
  tmp->addSeparator();
  tmp->addAction(ui->actionClose);
}

void MainUI::closeTrayIcon(){

}

// ==== PRIVATE SLOTS ====
void MainUI::closeApplication(){
  closing = true;
  if(PANDORA->currentState()!= PianoBarProcess::Stopped){ 
    PANDORA->closePianoBar();
    this->hide();
    QTimer::singleShot(500, this, SLOT(close()) );
  }else{
    this->close();
  } 
}

void MainUI::PlayerTypeChanged(bool active){
  if(!active){ return; } //this gets rid of the "extra" signals from the radio button functionality (1 signal from each button on change)
  if(ui->radio_pandora->isChecked()){ 
    ui->stackedWidget->setCurrentWidget(ui->page_pandora); 
    PandoraStateChanged(PANDORA->currentState());
    SYSTRAY->setIcon( QIcon(":pandora") );
    this->setWindowIcon( QIcon(":pandora") );
    this->setWindowTitle( tr("Pandora Radio") );
  }else{ 
    ui->stackedWidget->setCurrentWidget(ui->page_local);
    SYSTRAY->setIcon( LXDG::findIcon("audio-x-generic","") );
    this->setWindowIcon( LXDG::findIcon("audio-x-generic","") );
    this->setWindowTitle( tr("Media Player") );
  }
  //Now close down any currently running streams as needed
  if(!ui->radio_pandora->isChecked() && PANDORA->currentState()!=PianoBarProcess::Stopped){ PANDORA->closePianoBar(); }

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
  if(ui->radio_pandora->isChecked()){ return; }
}

void MainUI::volupToggled(){
  if(ui->radio_pandora->isChecked()){
    PANDORA->volumeUp();
  }
}

void MainUI::voldownToggled(){
  if(ui->radio_pandora->isChecked()){
    PANDORA->volumeDown();
  }
}


//Pandora Options
void MainUI::showPandoraSongInfo(){
  QDesktopServices::openUrl( QUrl(ui->tool_pandora_info->whatsThis()) );
}

void MainUI::changePandoraStation(QString station){
  if(station == PANDORA->currentStation()){ return; }
  //qDebug() << "[CHANGE STATION]" << station << "from:" << PANDORA->currentStation();
  PANDORA->setCurrentStation(station);
}

void MainUI::checkPandoraSettings(){
  bool changes = (PANDORA->email() != ui->line_pandora_email->text()) 
	|| (PANDORA->password() != ui->line_pandora_pass->text())
	|| (PANDORA->audioQuality() != ui->combo_pandora_quality->currentData().toString())
	|| (PANDORA->proxy() != ui->line_pandora_proxy->text())
	|| (PANDORA->controlProxy() != ui->line_pandora_cproxy->text());
  ui->push_pandora_apply->setEnabled(changes);
}

void MainUI::applyPandoraSettings(){
  PANDORA->setLogin(ui->line_pandora_email->text(), ui->line_pandora_pass->text());
  PANDORA->setAudioQuality(ui->combo_pandora_quality->currentData().toString());
  PANDORA->setProxy(ui->line_pandora_proxy->text());
  PANDORA->setControlProxy(ui->line_pandora_cproxy->text());
  if(PANDORA->isSetup()){
    //Go ahead and (re)start the Pandora process so it is aware of the new changes
    if(PANDORA->currentState()!=PianoBarProcess::Stopped){ PANDORA->closePianoBar(); }
    QTimer::singleShot(500, PANDORA, SLOT(play()) ); //give it a moment for the file to get written first
  }
}

//Pandora Process Feedback
void MainUI::PandoraStateChanged(PianoBarProcess::State state){
  //qDebug() << "[STATE CHANGE]" << state;
  ui->actionPlay->setVisible(state != PianoBarProcess::Running);
  ui->actionPause->setVisible(state == PianoBarProcess::Running);
  ui->actionStop->setVisible(state != PianoBarProcess::Stopped);
  ui->actionBack->setVisible(false); //never available for Pandora streams
  ui->actionNext->setVisible(state!=PianoBarProcess::Stopped);
  ui->tabWidget_pandora->setTabEnabled(0, state !=PianoBarProcess::Stopped);
  if(!ui->tabWidget_pandora->isTabEnabled(0) && ui->tabWidget_pandora->currentIndex()==0){
    ui->tabWidget_pandora->setCurrentWidget(ui->tab_pandora_settings);
  }else  if(state == PianoBarProcess::Running){
    ui->tabWidget_pandora->setCurrentWidget(ui->tab_pandora_playing);
  }
  ui->actionVolUp->setVisible(false);//state != PianoBarProcess::Stopped);
  ui->actionVolDown->setVisible(false); //state != PianoBarProcess::Stopped);
}

void MainUI::NewPandoraInfo(QString info){
  //qDebug() << "[INFO]" << info;
  ui->statusbar->showMessage(info, 2000);
}

void MainUI::PandoraStationChanged(QString station){
  //qDebug() << "[STATION CHANGE]" << station;
  int index = ui->combo_pandora_station->findText( station );
  if(index>=0){ 
    //qDebug() <<"   [FOUND]" << ui->combo_pandora_station->itemText(index);
    ui->combo_pandora_station->setCurrentIndex(index); 
  }else{
    //Could not find the station in the current list - need to update that first
    //qDebug() <<"   [NOT FOUND]";
    PandoraStationListChanged(PANDORA->stations());
  }
}

void MainUI::PandoraSongChanged(bool isLoved, QString title, QString artist, QString album, QString detailsURL, QString fromStation){
  //qDebug() << "[SONG CHANGE]" << isLoved << title << artist << album << detailsURL << fromStation;
  ui->tool_pandora_info->setWhatsThis(detailsURL);
  ui->tool_pandora_love->setChecked(isLoved);
  ui->tool_pandora_love->setEnabled(!isLoved); //pianobar cannot "unlove" a song
  ui->label_pandora_album->setText(album);
  ui->label_pandora_artist->setText(artist);
  ui->label_pandora_title->setText(title);
  ui->progress_pandora->setRange(0,1);
  ui->progress_pandora->setValue(0);
  ui->progress_pandora->setFormat("");
  QString msg = QString("%1\n%2\n%3").arg(title, artist, album);
  SYSTRAY->setToolTip(msg);
  if(ui->action_showNotifications->isChecked()){
    SYSTRAY->showMessage(tr("Now Playing"), msg, QSystemTrayIcon::NoIcon, 2000); //2 seconds
  }
}

void MainUI::PandoraTimeUpdate(int curS, int totS){
  //qDebug() << "[TIME UPDATE]" << curS << "/" << totS;
  ui->progress_pandora->setRange(0, totS);
  ui->progress_pandora->setValue(curS);
  QString time = QTime(0, curS/60, curS%60,0).toString("m:ss") + "/" + QTime(0, totS/60, totS%60,0).toString("m:ss");
  ui->progress_pandora->setFormat(time);
}

void MainUI::PandoraStationListChanged(QStringList list){
  //qDebug() << "[STATION LIST]" << list;
  ui->combo_pandora_station->clear();
  if(list.isEmpty()){ return; }
  ui->combo_pandora_station->addItems(list);
  int index = ui->combo_pandora_station->findText( PANDORA->currentStation() );
  //qDebug() << "[CURRENT STATION]" << PANDORA->currentStation() << index;
  if(index>=0){ ui->combo_pandora_station->setCurrentIndex(index); }
}

void MainUI::PandoraListInfo(QStringList list){
  qDebug() << "[LIST INFO]" << list;
}

//System Tray interactions
void MainUI::toggleVisibility(){
  if(this->isVisible()){ this->hide(); }
  else{ this->showNormal(); }
}

void MainUI::trayMessageClicked(){
  this->showNormal();
  this->activateWindow();
  this->raise();
}

void MainUI::trayClicked(QSystemTrayIcon::ActivationReason rsn){
  if(rsn == QSystemTrayIcon::Context){
    SYSTRAY->contextMenu()->popup(QCursor::pos());
  }else{
    toggleVisibility();
  }
}

void MainUI::closeEvent(QCloseEvent *ev){
  if(!closing){
    //Check if we have audio playing to determine if we should just minimize instead
    if(ui->action_closeToTray->isChecked()){
      closing = (PANDORA->currentState()!=PianoBarProcess::Running);
    }else if(PANDORA->currentState()!=PianoBarProcess::Stopped){
      //Make sure we close the stream down first
      PANDORA->closePianoBar();
      QTimer::singleShot(500, this, SLOT(close()) ); //try again in a moment
    }else{
      closing = true;
    }
  }
  if(closing){ QMainWindow::closeEvent(ev); } //normal close procedure
  else{
    ev->ignore();
    this->hide();
  }
}

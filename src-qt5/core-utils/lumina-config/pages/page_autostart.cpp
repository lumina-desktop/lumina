//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_autostart.h"
#include "ui_page_autostart.h"

#include "../AppDialog.h"
//==========
//    PUBLIC
//==========
page_autostart::page_autostart(QWidget *parent) : PageWidget(parent), ui(new Ui::page_autostart()){
  ui->setupUi(this);
  ui->list_session_start->setMouseTracking(true);
  updateIcons();
  connect(ui->tool_session_addapp, SIGNAL(clicked()), this, SLOT(addsessionstartapp()) );
  connect(ui->tool_session_addbin, SIGNAL(clicked()), this, SLOT(addsessionstartbin()) );
  connect(ui->tool_session_addfile, SIGNAL(clicked()), this, SLOT(addsessionstartfile()) );
  connect(ui->list_session_start, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(settingChanged()) );
}

page_autostart::~page_autostart(){

}



//================
//    PUBLIC SLOTS
//================
void page_autostart::SaveSettings(){
  //qDebug() << "Load AutoStart Files for saving";
  QList<XDGDesktop*> STARTAPPS = LXDG::findAutoStartFiles(true); //also want invalid/disabled items
  //qDebug() << " - done";
  //bool newstartapps = false;
  for(int i=0; i<ui->list_session_start->count(); i++){
    QString file = ui->list_session_start->item(i)->whatsThis();
    bool enabled = ui->list_session_start->item(i)->checkState()==Qt::Checked;
    bool found = false;
    for(int i=0; i<STARTAPPS.length(); i++){
      if(STARTAPPS[i]->filePath==file){
        found = true;
	if(enabled != !STARTAPPS[i]->isHidden){
	  //value is different
	  qDebug() << "Setting Autostart:" << enabled << STARTAPPS[i]->filePath;
	  STARTAPPS[i]->setAutoStarted(enabled);
	}
	break;
      }
    }
    if(!found && enabled){
      //New file/binary/app
      qDebug() << "Adding new AutoStart File:" << file;
      LXDG::setAutoStarted(enabled, file);
      //newstartapps = true;
    }
  } //end loop over GUI items
  //Now cleanup all the STARTAPPS data
  for(int i=STARTAPPS.length()-1; i>=0; i--){ STARTAPPS[i]->deleteLater(); }
}

void page_autostart::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Startup Services") );
  //qDebug() << "Load AutoStart Files";
  QList<XDGDesktop*> STARTAPPS = LXDG::findAutoStartFiles(true); //also want invalid/disabled items
  //qDebug() << " - done:" << STARTAPPS.length();
  //qDebug() << "StartApps:";
  ui->list_session_start->clear();
  for(int i=0; i<STARTAPPS.length(); i++){
  //qDebug() << STARTAPPS[i]->filePath +" -> " +STARTAPPS[i]->name << STARTAPPS[i]->isHidden;
    if( !STARTAPPS[i]->isValid() || !QFile::exists(STARTAPPS[i]->filePath) ){ continue; }
    QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(STARTAPPS[i]->icon,"application-x-executable"), STARTAPPS[i]->name );
	it->setWhatsThis(STARTAPPS[i]->filePath); //keep the file location
        it->setToolTip(STARTAPPS[i]->comment);
	if(STARTAPPS[i]->isHidden){ it->setCheckState( Qt::Unchecked); }
	else{it->setCheckState( Qt::Checked); }
	ui->list_session_start->addItem(it);
  }
  //Now cleanup all the STARTAPPS data
  for(int i=STARTAPPS.length()-1; i>=0; i--){ STARTAPPS[i]->deleteLater(); }
}

void page_autostart::updateIcons(){
  ui->tool_session_addapp->setIcon( LXDG::findIcon("system-run","") );
  ui->tool_session_addbin->setIcon( LXDG::findIcon("system-search","") );
  ui->tool_session_addfile->setIcon( LXDG::findIcon("quickopen-file","") );
}

//=================
//         PRIVATE
//=================
QString page_autostart::getSysApp(bool allowreset){
  AppDialog dlg(this);
    dlg.allowReset(allowreset);
    dlg.exec();
  if(dlg.appreset && allowreset){
    return "reset";
  }else{
    return dlg.appselected;
  }
}

//=================
//    PRIVATE SLOTS
//=================
void page_autostart::rmsessionstartitem(){
  if(ui->list_session_start->currentRow() < 0){ return; } //no item selected
  delete ui->list_session_start->takeItem(ui->list_session_start->currentRow());
    settingChanged();
}

void page_autostart::addsessionstartapp(){
  //Prompt for the application to start
  QString app = getSysApp(false); //no reset
  if(app.isEmpty()){ return; } //cancelled
  XDGDesktop desk(app);
  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(desk.icon,""), desk.name );
    it->setWhatsThis(desk.filePath);
    it->setToolTip(desk.comment);
    it->setCheckState(Qt::Checked);
  
  ui->list_session_start->addItem(it);
  ui->list_session_start->setCurrentItem(it);
    settingChanged();
}

void page_autostart::addsessionstartbin(){
  QString chkpath = LOS::AppPrefix() + "bin";
  if(!QFile::exists(chkpath)){ chkpath = QDir::homePath(); }
  QString bin = QFileDialog::getOpenFileName(this, tr("Select Binary"), chkpath, tr("Application Binaries (*)") );
  if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
  if( !QFileInfo(bin).isExecutable() ){
    QMessageBox::warning(this, tr("Invalid Binary"), tr("The selected file is not executable!"));
    return;
  }
  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon("application-x-executable",""), bin.section("/",-1) );
    it->setWhatsThis(bin); //command to be saved/run
    it->setToolTip(bin);
    it->setCheckState(Qt::Checked);
  ui->list_session_start->addItem(it);
  ui->list_session_start->setCurrentItem(it);
  settingChanged();
}

void page_autostart::addsessionstartfile(){
  QString chkpath = QDir::homePath();
  QString bin = QFileDialog::getOpenFileName(this, tr("Select File"), chkpath, tr("All Files (*)") );
  if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
  QListWidgetItem *it = new QListWidgetItem( LXDG::findMimeIcon(bin), bin.section("/",-1) );
    it->setWhatsThis(bin); //file to be saved/run
    it->setToolTip(bin);
    it->setCheckState(Qt::Checked);
  ui->list_session_start->addItem(it);
  ui->list_session_start->setCurrentItem(it);
    settingChanged();
}

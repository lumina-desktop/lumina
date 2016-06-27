//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_defaultapps.h"
#include "ui_page_defaultapps.h"
#include "getPage.h"
#include "../AppDialog.h"

//==========
//    PUBLIC
//==========
page_defaultapps::page_defaultapps(QWidget *parent) : PageWidget(parent), ui(new Ui::page_defaultapps()){
  ui->setupUi(this);

  connect(ui->tool_default_filemanager, SIGNAL(clicked()), this, SLOT(changeDefaultFileManager()) );
  connect(ui->tool_default_terminal, SIGNAL(clicked()), this, SLOT(changeDefaultTerminal()) );
  connect(ui->tool_default_webbrowser, SIGNAL(clicked()), this, SLOT(changeDefaultBrowser()) );
  connect(ui->tool_default_email, SIGNAL(clicked()), this, SLOT(changeDefaultEmail()) );
  connect(ui->tool_defaults_clear, SIGNAL(clicked()), this, SLOT(cleardefaultitem()) );
  connect(ui->tool_defaults_set, SIGNAL(clicked()), this, SLOT(setdefaultitem()) );
  connect(ui->tool_defaults_setbin, SIGNAL(clicked()), this, SLOT(setdefaultbinary()) );
  connect(ui->tree_defaults, SIGNAL(itemSelectionChanged()), this, SLOT(checkdefaulticons()) );
  updateIcons();
  ui->tabWidget_apps->setCurrentWidget(ui->tab_auto);
}

page_defaultapps::~page_defaultapps(){

}

//================
//    PUBLIC SLOTS
//================
void page_defaultapps::SaveSettings(){

}

void page_defaultapps::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Default Applications") );

//First load the lumina-open specific defaults
    //  - Default File Manager
  QString tmp = LXDG::findDefaultAppForMime("inode/directory"); 
  if(tmp.isEmpty()){ tmp = "lumina-fm"; }
  if( !QFile::exists(tmp) && !LUtils::isValidBinary(tmp) ){ qDebug() << "Invalid Settings:" << tmp; tmp.clear(); } //invalid settings
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_filemanager->setText(tmp.section("/",-1));
      ui->tool_default_filemanager->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_filemanager->setText(file.name);
      ui->tool_default_filemanager->setIcon(LXDG::findIcon(file.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_filemanager->setText(tr("Click to Set"));
    ui->tool_default_filemanager->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_filemanager->setText(tmp.section("/",-1));
      ui->tool_default_filemanager->setIcon( LXDG::findIcon("application-x-executable","") );
  }
  // - Default Terminal
  tmp =LXDG::findDefaultAppForMime("application/terminal"); //sessionsettings->value("default-terminal", "xterm").toString();
  if( !QFile::exists(tmp) && !LUtils::isValidBinary(tmp) ){ qDebug() << "Invalid Settings:" << tmp; tmp.clear(); } //invalid settings
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_terminal->setText(tmp.section("/",-1));
      ui->tool_default_terminal->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_terminal->setText(file.name);
      ui->tool_default_terminal->setIcon(LXDG::findIcon(file.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_terminal->setText(tr("Click to Set"));
    ui->tool_default_terminal->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_terminal->setText(tmp.section("/",-1));
      ui->tool_default_terminal->setIcon( LXDG::findIcon("application-x-executable","") );
  }
  // - Default Web Browser
  tmp = LXDG::findDefaultAppForMime("x-scheme-handler/http"); //appsettings->value("default/webbrowser", "").toString();
  if( !QFile::exists(tmp) && !LUtils::isValidBinary(tmp) ){ qDebug() << "Invalid Settings:" << tmp; tmp.clear(); } //invalid settings
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_webbrowser->setText(tmp.section("/",-1));
      ui->tool_default_webbrowser->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_webbrowser->setText(file.name);
      ui->tool_default_webbrowser->setIcon(LXDG::findIcon(file.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_webbrowser->setText(tr("Click to Set"));
    ui->tool_default_webbrowser->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_webbrowser->setText(tmp.section("/",-1));
      ui->tool_default_webbrowser->setIcon( LXDG::findIcon("application-x-executable","") );
  }
  // - Default Email Client
  tmp = LXDG::findDefaultAppForMime("application/email"); //appsettings->value("default/email", "").toString();
  if( !QFile::exists(tmp) && !LUtils::isValidBinary(tmp) ){ qDebug() << "Invalid Settings:" << tmp; tmp.clear(); } //invalid settings
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_email->setText(tmp.section("/",-1));
      ui->tool_default_email->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_email->setText(file.name);
      ui->tool_default_email->setIcon(LXDG::findIcon(file.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_email->setText(tr("Click to Set"));
    ui->tool_default_email->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_email->setText(tmp.section("/",-1));
      ui->tool_default_email->setIcon( LXDG::findIcon("application-x-executable","") );
  }
  
  //Now load the XDG mime defaults
  ui->tree_defaults->clear();
  QStringList defMimeList = LXDG::listFileMimeDefaults();
  //qDebug() << "Mime List:\n" << defMimeList.join("\n");
  defMimeList.sort(); //sort by group/mime
  //Now fill the tree by group/mime
  QTreeWidgetItem *group = new QTreeWidgetItem(0); //nothing at the moment
  QString ccat;
  for(int i=0; i<defMimeList.length(); i++){
    //Get the info from this entry
    QString mime = defMimeList[i].section("::::",0,0);
    QString cat = mime.section("/",0,0);
    QString extlist = defMimeList[i].section("::::",1,1);
    QString def = defMimeList[i].section("::::",2,2);
    QString comment = defMimeList[i].section("::::",3,50);
    //Now check if this is a new category
    if(ccat!=cat){
	//New group
	group = new QTreeWidgetItem(0);
	    group->setText(0, cat); //add translations for known/common groups later
	ui->tree_defaults->addTopLevelItem(group);
	ccat = cat;
    }
    //Now create the entry
    QTreeWidgetItem *it = new QTreeWidgetItem();
      it->setWhatsThis(0,mime); // full mimetype
      it->setText(0, QString(tr("%1 (%2)")).arg(mime.section("/",-1), extlist) );
      it->setText(2,comment);
      it->setToolTip(0, comment); it->setToolTip(1,comment);
      //Now load the default (if there is one)
      it->setWhatsThis(1,def); //save for later
      if(def.endsWith(".desktop")){
	bool ok = false;
	XDGDesktop file = LXDG::loadDesktopFile(def, ok);
	if(!ok || file.filePath.isEmpty()){
	  //Might be a binary - just print out the raw "path"
	  it->setText(1,def.section("/",-1));
	  it->setIcon(1, LXDG::findIcon("application-x-executable","") );
	}else{
	  it->setText(1, file.name);
	  it->setIcon(1, LXDG::findIcon(file.icon,"") );
	}     
      }else if(!def.isEmpty()){
	//Binary/Other default
	it->setText(1, def.section("/",-1));
	it->setIcon(1, LXDG::findIcon("application-x-executable","") );
      }
      group->addChild(it);
  }
  
  ui->tree_defaults->sortItems(0,Qt::AscendingOrder);
  
  checkdefaulticons();
}

void page_defaultapps::updateIcons(){
  ui->tool_defaults_clear->setIcon( LXDG::findIcon("edit-clear","") );
  ui->tool_defaults_set->setIcon( LXDG::findIcon("system-run","") );
  ui->tool_defaults_setbin->setIcon( LXDG::findIcon("application-x-executable","") );
  ui->tabWidget_apps->setTabIcon( ui->tabWidget_apps->indexOf(ui->tab_auto), LXDG::findIcon("system-run", "") );
  ui->tabWidget_apps->setTabIcon( ui->tabWidget_apps->indexOf(ui->tab_defaults), LXDG::findIcon("preferences-desktop-filetype-association", "") );
}

//=================
//         PRIVATE
//=================
XDGDesktop page_defaultapps::getSysApp(bool allowreset){
  AppDialog dlg(this, LXDG::sortDesktopNames( LXDG::systemDesktopFiles() ) );
    dlg.allowReset(allowreset);
    dlg.exec();
  XDGDesktop desk;
  if(dlg.appreset && allowreset){
    desk.filePath = "reset"; //special internal flag
  }else{
    desk = dlg.appselected;
  }
  return desk;
}

//=================
//    PRIVATE SLOTS
//=================
void page_defaultapps::changeDefaultBrowser(){
  //Prompt for the new app
  XDGDesktop desk = getSysApp(true);
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    if(desk.filePath=="reset"){
      desk.filePath="";
    }
  //save the new app setting and adjust the button appearance
  //appsettings->setValue("default/webbrowser", desk.filePath);
  LXDG::setDefaultAppForMime("x-scheme-handler/http", desk.filePath.section("/",-1));
  LXDG::setDefaultAppForMime("x-scheme-handler/https", desk.filePath.section("/",-1));
  QString tmp = desk.filePath;
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_webbrowser->setText(tmp.section("/",-1));
      ui->tool_default_webbrowser->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_webbrowser->setText(desk.name);
      ui->tool_default_webbrowser->setIcon(LXDG::findIcon(desk.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_webbrowser->setText(tr("Click to Set"));
    ui->tool_default_webbrowser->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_webbrowser->setText(tmp.section("/",-1));
      ui->tool_default_webbrowser->setIcon( LXDG::findIcon("application-x-executable","") );
  }
}

void page_defaultapps::changeDefaultEmail(){
  //Prompt for the new app
  XDGDesktop desk = getSysApp(true); //allow reset to default
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    if(desk.filePath=="reset"){
      desk.filePath="";
    }
  //save the new app setting and adjust the button appearance
  LXDG::setDefaultAppForMime("application/email",desk.filePath);
 // appsettings->setValue("default/email", desk.filePath);
  QString tmp = desk.filePath;
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_email->setText(tmp.section("/",-1));
      ui->tool_default_email->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_email->setText(file.name);
      ui->tool_default_email->setIcon(LXDG::findIcon(file.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_email->setText(tr("Click to Set"));
    ui->tool_default_email->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_email->setText(tmp.section("/",-1));
      ui->tool_default_email->setIcon( LXDG::findIcon("application-x-executable","") );
  }	
}

void page_defaultapps::changeDefaultFileManager(){
  //Prompt for the new app
  XDGDesktop desk = getSysApp(true);
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    if(desk.filePath=="reset"){
      desk.filePath="lumina-fm";
    }
  //save the new app setting and adjust the button appearance
  //appsettings->setValue("default/directory", desk.filePath);
  //sessionsettings->setValue("default-filemanager", desk.filePath);
  LXDG::setDefaultAppForMime("inode/directory", desk.filePath.section("/",-1));
  QString tmp = desk.filePath;
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_filemanager->setText(tmp.section("/",-1));
      ui->tool_default_filemanager->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_filemanager->setText(file.name);
      ui->tool_default_filemanager->setIcon(LXDG::findIcon(file.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_filemanager->setText(tr("Click to Set"));
    ui->tool_default_filemanager->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_filemanager->setText(tmp.section("/",-1));
      ui->tool_default_filemanager->setIcon( LXDG::findIcon("application-x-executable","") );
  }	
}

void page_defaultapps::changeDefaultTerminal(){
  //Prompt for the new app
  XDGDesktop desk = getSysApp(true);
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    if(desk.filePath=="reset"){
      desk.filePath="xterm";
    }
  //save the new app setting and adjust the button appearance
  LXDG::setDefaultAppForMime("application/terminal",desk.filePath);
 //sessionsettings->setValue("default-terminal", desk.filePath);
  QString tmp = desk.filePath;
  if(tmp.endsWith(".desktop")){
    bool ok = false;
    XDGDesktop file = LXDG::loadDesktopFile(tmp, ok);
    if(!ok || file.filePath.isEmpty()){
      //Might be a binary - just print out the raw "path"
      ui->tool_default_terminal->setText(tmp.section("/",-1));
      ui->tool_default_terminal->setIcon( LXDG::findIcon("application-x-executable","") );
    }else{
      ui->tool_default_terminal->setText(file.name);
      ui->tool_default_terminal->setIcon(LXDG::findIcon(file.icon,"") );
    }
  }else if(tmp.isEmpty()){
    ui->tool_default_terminal->setText(tr("Click to Set"));
    ui->tool_default_terminal->setIcon( LXDG::findIcon("system-help","") );
  }else{
    //Might be a binary - just print out the raw "path"
      ui->tool_default_terminal->setText(tmp.section("/",-1));
      ui->tool_default_terminal->setIcon( LXDG::findIcon("application-x-executable","") );
  }
}

void page_defaultapps::cleardefaultitem(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  if(it==0){ return; } //no item selected
  QList<QTreeWidgetItem*> list;
  for(int i=0; i<it->childCount(); i++){
    list << it->child(i);
  }
  if(list.isEmpty()){ list << it; } //just do the current item
  //Now clear the items
  for(int i=0; i<list.length(); i++){
    //Clear it in the back end
    LXDG::setDefaultAppForMime(list[i]->whatsThis(0), "");
    //Now clear it in the UI
    list[i]->setWhatsThis(1,""); //clear the app path
    list[i]->setIcon(1,QIcon()); //clear the icon
    list[i]->setText(1,""); //clear the name
  }

}

void page_defaultapps::setdefaultitem(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  if(it==0){ return; } //no item selected
  QList<QTreeWidgetItem*> list;
  for(int i=0; i<it->childCount(); i++){
    list << it->child(i);
  }
  if(list.isEmpty()){ list << it; } //just do the current item
  //Prompt for which application to use
  XDGDesktop desk = getSysApp(false); //no "reset"  option
    if(desk.filePath.isEmpty()){ return; }//nothing selected
  //Now set the items
  for(int i=0; i<list.length(); i++){
    //Set it in the back end
    LXDG::setDefaultAppForMime(list[i]->whatsThis(0), desk.filePath);
    //Set it in the UI
    list[i]->setWhatsThis(1,desk.filePath); //app path
    list[i]->setIcon(1,LXDG::findIcon(desk.icon,"")); //reset the icon
    list[i]->setText(1,desk.name); //reset the name
  }

}

void page_defaultapps::setdefaultbinary(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  if(it==0){ return; } //no item selected
  QList<QTreeWidgetItem*> list;
  for(int i=0; i<it->childCount(); i++){
    list << it->child(i);
  }
  if(list.isEmpty()){ list << it; } //just do the current item
  //Prompt for which binary to use
  QFileDialog dlg(this);
    //dlg.setFilter(QDir::Executable | QDir::Files); //Does not work! Filters executable files as well as breaks browsing capabilities
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setDirectory( LOS::AppPrefix()+"bin" );
    dlg.setWindowTitle(tr("Select Binary"));
  if( !dlg.exec() || dlg.selectedFiles().isEmpty() ){
    return; //cancelled
  }
  QString path = dlg.selectedFiles().first();
  //Make sure it is executable
  if( !QFileInfo(path).isExecutable()){
    QMessageBox::warning(this, tr("Invalid Binary"), tr("The selected binary is not executable!"));
    return;
  }
  //Now set the items
  for(int i=0; i<list.length(); i++){
    //Set it in the back end
    LXDG::setDefaultAppForMime(list[i]->whatsThis(0), path);
    //Set it in the UI
    list[i]->setWhatsThis(1,path); //app path
    list[i]->setIcon(1,LXDG::findIcon("application-x-executable","")); //clear the icon
    list[i]->setText(1,path.section("/",-1)); //clear the name
  }
}

void page_defaultapps::checkdefaulticons(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  ui->tool_defaults_set->setEnabled(it!=0);
  ui->tool_defaults_clear->setEnabled(it!=0);
  ui->tool_defaults_setbin->setEnabled(it!=0);
}

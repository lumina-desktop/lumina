//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h" //the designer *.ui file

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI()){
  ui->setupUi(this); //load the designer file
  this->setWindowIcon( LXDG::findIcon("preferences-desktop-display","") );
  PINFO = new LPlugins(); //load the info class
	
  //Be careful about the QSettings setup, it must match the lumina-desktop setup
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  settings = new QSettings( QSettings::UserScope, "LuminaDE", "desktopsettings", this);
  appsettings = new QSettings( QSettings::UserScope, "LuminaDE", "lumina-open", this);
  sessionsettings = new QSettings( QSettings::UserScope, "LuminaDE","sessionsettings", this);
  qDebug() << "Settings File:" << settings->fileName();
  desktop = new QDesktopWidget();
  ui->spin_screen->setMinimum(1);
    //Make sure this is only allows the current number of screens
    ui->spin_screen->setMaximum(desktop->screenCount());
    ui->spin_screen->setValue(desktop->primaryScreen()+1); //have the current screen auto-selected
  //qDebug() << "Number of Screens:" << desktop->screenCount();
  sysApps = LXDG::sortDesktopNames( LXDG::systemDesktopFiles() );
  
  //Now finish setting up the UI
  setupIcons();
  setupMenus();
  setupConnections();

  //Start on the Desktop page
  ui->stackedWidget->setCurrentWidget(ui->page_desktop);
  slotChangePage(false);
  QTimer::singleShot(10, this, SLOT(loadCurrentSettings()) );

  //Disable the incomplete pages/items at the moment
  ui->check_session_playloginaudio->setVisible(false);
  ui->check_session_playlogoutaudio->setVisible(false);
}

MainUI::~MainUI(){
	
}

void MainUI::slotSingleInstance(){
  //Make sure this window is visible
  this->showNormal();
  this->activateWindow();
  this->raise();
}

//================
//  PRIVATE FUNCTIONS
//================
void MainUI::setupIcons(){
  //Pull all the icons from the current theme using libLumina (LXDG)
	
  //General UI
  ui->actionDesktop->setIcon( LXDG::findIcon("preferences-desktop-wallpaper","") );
  ui->actionPanels->setIcon( LXDG::findIcon("configure-toolbars","") );
  ui->actionMenu->setIcon( LXDG::findIcon("preferences-desktop-icons","") );
  ui->actionShortcuts->setIcon( LXDG::findIcon("configure-shortcuts","") );
  ui->actionDefaults->setIcon( LXDG::findIcon("preferences-desktop-filetype-association","") );
  ui->actionSession->setIcon( LXDG::findIcon("preferences-system-session-services","") );
  ui->push_save->setIcon( LXDG::findIcon("document-save","") );  
  
	
  //Desktop Page
  ui->tool_desk_addbg->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_desk_rmbg->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_desk_addplug->setIcon( LXDG::findIcon("list-add","") );
	
  //Panels Page
  ui->tool_panel1_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_panel1_rm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_panel1_addplugin->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_panel1_rmplugin->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_panel1_upplug->setIcon( LXDG::findIcon("go-up","") );
  ui->tool_panel1_dnplug->setIcon( LXDG::findIcon("go-down","") );
  ui->tool_panel1_getcolor->setIcon( LXDG::findIcon("preferences-desktop-color","") );
  ui->toolBox_panel1->setItemIcon(0,LXDG::findIcon("preferences-desktop-display",""));
  ui->toolBox_panel1->setItemIcon(1,LXDG::findIcon("preferences-plugin",""));
  ui->tool_panel2_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_panel2_rm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_panel2_addplugin->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_panel2_rmplugin->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_panel2_upplug->setIcon( LXDG::findIcon("go-up","") );
  ui->tool_panel2_dnplug->setIcon( LXDG::findIcon("go-down","") );
  ui->tool_panel2_getcolor->setIcon( LXDG::findIcon("preferences-desktop-color","") );
  ui->toolBox_panel2->setItemIcon(0,LXDG::findIcon("preferences-desktop-display",""));
  ui->toolBox_panel2->setItemIcon(1,LXDG::findIcon("preferences-plugin",""));

  //Menu Page
  ui->tool_menu_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_menu_rm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_menu_up->setIcon( LXDG::findIcon("go-up","") );
  ui->tool_menu_dn->setIcon( LXDG::findIcon("go-down","") );
  ui->tool_menu_findterm->setIcon( LXDG::findIcon("system-search","") );
  
  //Shortcuts Page
  ui->tool_shortcut_set->setIcon( LXDG::findIcon("input-keyboard","") );
  ui->tool_shortcut_clear->setIcon( LXDG::findIcon("edit-clear","") );
  
  //Defaults Page
  ui->tool_defaults_addextension->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_defaults_addgroup->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_defaults_clear->setIcon( LXDG::findIcon("edit-clear","") );
  ui->tool_defaults_set->setIcon( LXDG::findIcon("system-run","") );
  
  //Session Page
  ui->tool_session_rmapp->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_session_addapp->setIcon( LXDG::findIcon("system-run","") );
  ui->tool_session_addbin->setIcon( LXDG::findIcon("system-search","") );
  ui->tool_session_addfile->setIcon( LXDG::findIcon("run-build-file","") );
  
}

void MainUI::setupConnections(){
  //General UI
  connect(ui->actionDesktop, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionPanels, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionMenu, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionShortcuts, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionDefaults, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionSession, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->push_save, SIGNAL(clicked()), this, SLOT(saveCurrentSettings()) );
  connect(ui->spin_screen, SIGNAL(valueChanged(int)), this, SLOT(slotChangeScreen()) );
	
  //Desktop Page
  connect(ui->combo_desk_plugs, SIGNAL(currentIndexChanged(int)), this, SLOT(deskplugchanged()) );
  connect(ui->combo_desk_bg, SIGNAL(currentIndexChanged(int)), this, SLOT(deskbgchanged()) );
  connect(ui->radio_desk_multi, SIGNAL(toggled(bool)), this, SLOT(desktimechanged()) );
  connect(ui->tool_desk_addplug, SIGNAL(clicked()), this, SLOT(deskplugadded()) );
  connect(ui->tool_desk_addbg, SIGNAL(clicked()), this, SLOT(deskbgadded()) );
  connect(ui->tool_desk_rmbg, SIGNAL(clicked()), this, SLOT(deskbgremoved()) );
  connect(ui->spin_desk_min, SIGNAL(valueChanged(int)), this, SLOT(desktimechanged()) );
	
  //Panels Page
  connect(ui->tool_panel1_add,SIGNAL(clicked()), this, SLOT(addpanel1()) );
  connect(ui->tool_panel2_add,SIGNAL(clicked()), this, SLOT(addpanel2()) );
  connect(ui->tool_panel1_rm,SIGNAL(clicked()), this, SLOT(rmpanel1()) );
  connect(ui->tool_panel2_rm,SIGNAL(clicked()), this, SLOT(rmpanel2()) );
  connect(ui->tool_panel1_getcolor,SIGNAL(clicked()), this, SLOT(getpanel1color()) );
  connect(ui->tool_panel2_getcolor,SIGNAL(clicked()), this, SLOT(getpanel2color()) );
  connect(ui->toolBox_panel1, SIGNAL(currentChanged(int)), this, SLOT(adjustpanel2()) );
  connect(ui->toolBox_panel2, SIGNAL(currentChanged(int)), this, SLOT(adjustpanel1()) );
  connect(ui->combo_panel1_loc, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustpanel2()) );
  connect(ui->combo_panel2_loc, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustpanel1()) );
  connect(ui->spin_panel1_size, SIGNAL(valueChanged(int)), this, SLOT(adjustpanel2()) );
  connect(ui->spin_panel2_size, SIGNAL(valueChanged(int)), this, SLOT(adjustpanel1()) );
  connect(ui->tool_panel1_addplugin, SIGNAL(clicked()), this, SLOT(addpanel1plugin()) );
  connect(ui->tool_panel1_rmplugin, SIGNAL(clicked()), this, SLOT(rmpanel1plugin()) );
  connect(ui->tool_panel1_upplug, SIGNAL(clicked()), this, SLOT(uppanel1plugin()) );
  connect(ui->tool_panel1_dnplug, SIGNAL(clicked()), this, SLOT(dnpanel1plugin()) );
  connect(ui->tool_panel2_addplugin, SIGNAL(clicked()), this, SLOT(addpanel2plugin()) );
  connect(ui->tool_panel2_rmplugin, SIGNAL(clicked()), this, SLOT(rmpanel2plugin()) );
  connect(ui->tool_panel2_upplug, SIGNAL(clicked()), this, SLOT(uppanel2plugin()) );
  connect(ui->tool_panel2_dnplug, SIGNAL(clicked()), this, SLOT(dnpanel2plugin()) );

  //Menu Page
  connect(ui->tool_menu_add, SIGNAL(clicked()), this, SLOT(addmenuplugin()) );
  connect(ui->tool_menu_rm, SIGNAL(clicked()), this, SLOT(rmmenuplugin()) );
  connect(ui->tool_menu_up, SIGNAL(clicked()), this, SLOT(upmenuplugin()) );
  connect(ui->tool_menu_dn, SIGNAL(clicked()), this, SLOT(downmenuplugin()) );
  connect(ui->tool_menu_findterm, SIGNAL(clicked()), this, SLOT(findmenuterminal()) );
  connect(ui->list_menu, SIGNAL(currentRowChanged(int)), this, SLOT(checkmenuicons()) );
  connect(ui->line_menu_term, SIGNAL(textChanged(QString)), this, SLOT(checkmenuicons()) );
	
  //Shortcuts Page
  connect(ui->tool_shortcut_clear, SIGNAL(clicked()), this, SLOT(clearKeyBinding()) );
  connect(ui->tool_shortcut_set, SIGNAL(clicked()), this, SLOT(getKeyPress()) );
  
  //Defaults Page
  connect(ui->tool_defaults_addextension, SIGNAL(clicked()), this, SLOT(adddefaultextension()) );
  connect(ui->tool_defaults_addgroup, SIGNAL(clicked()), this, SLOT(adddefaultgroup()) );
  connect(ui->tool_defaults_clear, SIGNAL(clicked()), this, SLOT(cleardefaultitem()) );
  connect(ui->tool_defaults_set, SIGNAL(clicked()), this, SLOT(setdefaultitem()) );
  connect(ui->tree_defaults, SIGNAL(itemSelectionChanged()), this, SLOT(checkdefaulticons()) );
  
  //Session Page
  connect(ui->tool_session_addapp, SIGNAL(clicked()), this, SLOT(addsessionstartapp()) );
  connect(ui->tool_session_addbin, SIGNAL(clicked()), this, SLOT(addsessionstartbin()) );
  connect(ui->tool_session_addfile, SIGNAL(clicked()), this, SLOT(addsessionstartfile()) );
  connect(ui->tool_session_rmapp, SIGNAL(clicked()), this, SLOT(rmsessionstartitem()) );
  connect(ui->combo_session_wfocus, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_session_wloc, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_session_wtheme, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->check_session_numlock, SIGNAL(stateChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->check_session_playloginaudio, SIGNAL(stateChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->check_session_playlogoutaudio, SIGNAL(stateChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->spin_session_wkspaces, SIGNAL(valueChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->list_session_start, SIGNAL(currentRowChanged(int)), this, SLOT(sessionstartchanged()) );
}

void MainUI::setupMenus(){
  //Desktop Plugin Menu
  ui->combo_desk_plugs->clear();
  QStringList plugs = PINFO->desktopPlugins();
  for(int i=0; i<plugs.length(); i++){ 
    LPI info = PINFO->desktopPluginInfo(plugs[i]);
    ui->combo_desk_plugs->addItem( LXDG::findIcon(info.icon,""), info.name, plugs[i]);
  }
  ui->tool_desk_addplug->setEnabled(!plugs.isEmpty());
  deskplugchanged(); //make sure it loads the right info
	
  //Panel locations
  ui->combo_panel1_loc->clear();
  ui->combo_panel2_loc->clear();
  QStringList loc; loc << tr("Top") << tr("Bottom") << tr("Left") << tr("Right");
  ui->combo_panel1_loc->addItems(loc);
  ui->combo_panel2_loc->addItems(loc);

  //Session window manager settings
  ui->combo_session_wfocus->clear();
  ui->combo_session_wfocus->addItem( tr("Click To Focus"), "ClickToFocus");
  ui->combo_session_wfocus->addItem( tr("Active Mouse Focus"), "MouseFocus");
  ui->combo_session_wfocus->addItem( tr("Strict Mouse Focus"), "StrictMouseFocus");
  ui->combo_session_wloc->clear();
  ui->combo_session_wloc->addItem( tr("Align in a Row"), "RowSmartPlacement");
  ui->combo_session_wloc->addItem( tr("Align in a Column"), "ColSmartPlacement");
  ui->combo_session_wloc->addItem( tr("Cascade"), "CascadePlacement");
  ui->combo_session_wloc->addItem( tr("Underneath Mouse"), "UnderMousePlacement");
  ui->combo_session_wtheme->clear();
  #ifdef __FreeBSD__
  QDir fbdir("/usr/local/share/fluxbox/styles");
  #endif
  #ifdef __linux__
  QDir fbdir("/usr/share/fluxbox/styles");
  #endif
  QStringList fbstyles = fbdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for(int i=0; i<fbstyles.length(); i++){
    ui->combo_session_wtheme->addItem(fbstyles[i], fbdir.absoluteFilePath(fbstyles[i]));
  }
  
  
}

int MainUI::currentDesktop(){
  return ui->spin_screen->value()-1; //backend starts at 0, not 1
}

QString MainUI::getColorStyle(QString current){
  QString out;
  //Convert the current color string into a QColor
  QStringList col = current.section(")",0,0).section("(",1,1).split(",");
  if(col.length()!=4){ col.clear(); col << "255" << "255" << "255" << "255"; }
  QColor ccol = QColor(col[0].toInt(), col[1].toInt(), col[2].toInt(), col[3].toInt()); //RGBA
  QColor ncol = QColorDialog::getColor(ccol, this, tr("Select Panel Color"), QColorDialog::ShowAlphaChannel);
  //Now convert the new color into a usable string and return
  if(ncol.isValid()){ //if the dialog was not cancelled
    out = "rgba("+QString::number(ncol.red())+","+QString::number(ncol.green())+","+QString::number(ncol.blue())+","+QString::number(ncol.alpha())+")";
  }
  return out;
}

QString MainUI::getNewPanelPlugin(){
  QString out;
  //Now let the user select a new panel plugin
  QStringList plugs = PINFO->panelPlugins();
  QStringList names;
  for(int i=0; i<plugs.length(); i++){
    names << PINFO->panelPluginInfo(plugs[i]).name;
  }
  bool ok = false;
  QString sel = QInputDialog::getItem(this, tr("New Panel Plugin"), tr("Add Plugin:"), names, 0, false, &ok);
  if(ok && !sel.isEmpty()){
    out = plugs[ names.indexOf(sel) ];	  
  }
  return out;
}

XDGDesktop MainUI::getSysApp(){
  //Prompt the user to select an application on the system
  QStringList apps;
    for(int i=0; i<sysApps.length(); i++){
      apps << sysApps[i].name;
    }
    bool ok = false;
    QString app = QInputDialog::getItem(this, tr("Select Application"), tr("App Name:"), apps, 0, false, &ok);
    int index = apps.indexOf(app);
    if(app.isEmpty() || index < 0 || !ok){ return XDGDesktop(); } //nothing selected
    else{ return sysApps[index]; }
}

//Convert to/from fluxbox key codes
QString MainUI::dispToFluxKeys(QString in){
  in.replace("Ctrl", "Control");
  in.replace("Shift", "Shift");
  in.replace("Alt", "Mod1");
  in.replace("Meta", "Mod4");
  in.replace("PgUp", "Prior");
  in.replace("PgDown", "Next");
  in.replace("Del", "Delete");
  in.replace("Backspace", "BackSpace");
  in.replace("Ins","Insert");
  in.replace("Volume Up", "XF86AudioRaiseVolume"); //multimedia key
  in.replace("Volume Down", "XF86AudioLowerVolume"); //multimedia key
  return in;
}

QString MainUI::fluxToDispKeys(QString in){
  in.replace("Control", "Ctrl");
  in.replace("Shift", "Shift");
  in.replace("Mod1", "Alt");
  in.replace("Mod4", "Meta");
  in.replace("Prior", "PgUp");
  in.replace("Next", "PgDown");
  //in.replace("Delete", "Del"); //the "Delete" is better looking
  in.replace("BackSpace", "Backspace");
  //in.replace("Insert", "Ins"); //the "Insert" is better looking
  in.replace("XF86AudioRaiseVolume", "Volume Up"); //multimedia key
  in.replace("XF86AudioLowerVolume", "Volume Down"); //multimedia key
  return in;	
}

//Read/overwrite a text file
QStringList MainUI::readFile(QString path){
  QStringList out;
  QFile file(path);
  if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
    QTextStream txt(&file);
    while(!txt.atEnd()){
      out << txt.readLine();
    }
    file.close();
  }
  return out;
}

bool MainUI::overwriteFile(QString path, QStringList contents){
  QFile file(path);
  if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
    QTextStream txt(&file);
    for(int i=0; i<contents.length(); i++){
      txt << contents[i]+"\n";
    }
    file.close();
    return true;
  }
  return false;	
}

//================
//    PRIVATE SLOTS
//================
void MainUI::slotChangePage(bool enabled){
  //Do not allow the user to de-select a button (make them act like radio buttons)
  //qDebug() << "Page Change:" << enabled;
  bool showScreen = false; //set this for pages that have per-screen settings
  if(!enabled){
    //Re-enable the current button
    ui->actionDesktop->setChecked(ui->stackedWidget->currentWidget()==ui->page_desktop);
    ui->actionPanels->setChecked(ui->stackedWidget->currentWidget()==ui->page_panels);
    ui->actionMenu->setChecked(ui->stackedWidget->currentWidget()==ui->page_menu);
    ui->actionShortcuts->setChecked(ui->stackedWidget->currentWidget()==ui->page_shortcuts);
    ui->actionDefaults->setChecked(ui->stackedWidget->currentWidget()==ui->page_defaults);
    ui->actionSession->setChecked(ui->stackedWidget->currentWidget()==ui->page_session);
    showScreen = (ui->actionDesktop->isChecked() || ui->actionPanels->isChecked());
    //Ask if they want to reset any changes on the current page
	  
  }else{
    //uncheck the button associated with the currently open page
    if(ui->stackedWidget->currentWidget()==ui->page_desktop){ ui->actionDesktop->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_panels){ ui->actionPanels->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_menu){ ui->actionMenu->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_shortcuts){ ui->actionShortcuts->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_defaults){ ui->actionDefaults->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_session){ ui->actionSession->setChecked(false); }
    //switch to the new page
    if(ui->actionDesktop->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_desktop); showScreen=true;}
    else if(ui->actionPanels->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_panels); showScreen=true; }
    else if(ui->actionMenu->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_menu); }
    else if(ui->actionShortcuts->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_shortcuts); }
    else if(ui->actionDefaults->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_defaults); }
    else if(ui->actionSession->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_session); }
  }
  ui->group_screen->setVisible(showScreen && (ui->spin_screen->maximum()>1) );
  if(ui->page_panels->isVisible()){ checkpanels(); }
  
}

void MainUI::slotChangeScreen(){
  static int cscreen = 0; //current screen
  int newscreen = currentDesktop();
  if(cscreen!=newscreen){
    if(moddesk || modpan){
      if(QMessageBox::Yes == QMessageBox::question(this, tr("Save Changes?"), tr("You currently have unsaved changes for this screen. Do you want to save them first?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ){
        saveCurrentSettings(true); //only save current screen settings
      }
    }
    loadCurrentSettings(true);
    cscreen = newscreen; //save that this screen is current now
  }
}

void MainUI::saveAndQuit(){
  saveCurrentSettings();
  this->close();	
}

//General Utility Functions
void MainUI::loadCurrentSettings(bool screenonly){
  loading = true;
  settings->sync();
  appsettings->sync();
  int cdesk = currentDesktop();
  QString DPrefix = "desktop-"+QString::number(cdesk)+"/";
  bool primary = (desktop->screenGeometry(cdesk).x()==0);
	
  //Desktop Page
  QStringList bgs = settings->value(DPrefix+"background/filelist", QStringList()<<"default").toStringList();
  ui->combo_desk_bg->clear();
  for(int i=0; i<bgs.length(); i++){
    if(bgs[i]=="default"){ ui->combo_desk_bg->addItem( QIcon(DEFAULTBG), tr("System Default"), bgs[i] ); }
    else{ ui->combo_desk_bg->addItem( QIcon(bgs[i]), bgs[i].section("/",-1), bgs[i] ); }
  }
  ui->radio_desk_multi->setEnabled(bgs.length()>1);
  if(bgs.length()>1){ ui->radio_desk_multi->setChecked(true);}
  else{ ui->radio_desk_single->setChecked(true); }
  ui->spin_desk_min->setValue( settings->value(DPrefix+"background/minutesToChange", 5).toInt() );
  desktimechanged(); //ensure the display gets updated (in case the radio selection did not change);
  ui->label_desk_res->setText( tr("Screen Resolution:")+"\n"+QString::number(desktop->screenGeometry(cdesk).width())+"x"+QString::number(desktop->screenGeometry(cdesk).height()) );
  
  //Panels Page
  int panels = settings->value(DPrefix+"panels",-1).toInt();
  if(panels==-1 && primary){ panels=1; }
  panelnumber = 0;
  if(panels >= 1){
    //Load the panel 1 information
    QString PPrefix = "panel"+QString::number(cdesk)+".0/";
    ui->toolBox_panel1->setVisible(true);
    ui->spin_panel1_size->setValue( settings->value( PPrefix+"height",30).toInt() );
    QString loc = settings->value(PPrefix+"location","top").toString().toLower();
    if(loc=="top"){ ui->combo_panel1_loc->setCurrentIndex(0); }
    else if(loc=="bottom"){ ui->combo_panel1_loc->setCurrentIndex(1); }
    else if(loc=="left"){ ui->combo_panel1_loc->setCurrentIndex(2); }
    else{ ui->combo_panel1_loc->setCurrentIndex(3); } //right
    QStringList plugs = settings->value(PPrefix+"pluginlist", QStringList()).toStringList();
    if(plugs.isEmpty() && primary){ plugs << "userbutton" << "taskmanager" << "systemtray" << "clock" << "systemdashboard"; }
    ui->list_panel1_plugins->clear();
    for(int i=0; i<plugs.length(); i++){
      QString pid = plugs[i].section("---",0,0);
      LPI info = PINFO->panelPluginInfo(pid);
      if(!info.ID.isEmpty()){
        QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name );
	      it->setWhatsThis(plugs[i]); //make sure to preserve the entire plugin ID (is the unique version)
	ui->list_panel1_plugins->addItem(it);
      }
    }
    QString color = settings->value(PPrefix+"color","rgba(255,255,255,160)").toString();
    ui->label_panel1_sample->setWhatsThis(color);
    ui->label_panel1_sample->setStyleSheet("background: "+color);
    panelnumber++;
  }else{
    //Panel 1 defaults
    ui->toolBox_panel1->setVisible(false); //not initially visible
    ui->spin_panel1_size->setValue(30);
    ui->combo_panel1_loc->setCurrentIndex(0); //Top
    ui->list_panel1_plugins->clear();
    ui->label_panel1_sample->setWhatsThis("rgba(255,255,255,160)");
    ui->label_panel1_sample->setStyleSheet("background: rgba(255,255,255,160)");
  }
  if(panels >= 2){
    //Load the panel 2 information
    ui->toolBox_panel2->setVisible(true);
    QString PPrefix = "panel"+QString::number(cdesk)+".1/";
    ui->spin_panel2_size->setValue( settings->value( PPrefix+"height",30).toInt() );
    QString loc = settings->value(PPrefix+"location","top").toString().toLower();
    if(loc=="top"){ ui->combo_panel2_loc->setCurrentIndex(0); }
    else if(loc=="bottom"){ ui->combo_panel2_loc->setCurrentIndex(1); }
    else if(loc=="left"){ ui->combo_panel2_loc->setCurrentIndex(2); }
    else{ ui->combo_panel2_loc->setCurrentIndex(3); } //right
    QStringList plugs = settings->value(PPrefix+"pluginlist", QStringList()).toStringList();
    ui->list_panel2_plugins->clear();
    for(int i=0; i<plugs.length(); i++){
      QString pid = plugs[i].section("---",0,0);
      LPI info = PINFO->panelPluginInfo(pid);
      if(!info.ID.isEmpty()){
        QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name );
	      it->setWhatsThis(plugs[i]); //make sure to preserve the entire plugin ID (is the unique version)
	ui->list_panel2_plugins->addItem(it);
      }
    }
    QString color = settings->value(PPrefix+"color","rgba(255,255,255,160)").toString();
    ui->label_panel2_sample->setWhatsThis(color);
    ui->label_panel2_sample->setStyleSheet("background: "+color);
    panelnumber++;
  }else{
    //Panel 2 defaults
    ui->toolBox_panel2->setVisible(false); //not initially visible
    ui->spin_panel2_size->setValue(30);
    ui->combo_panel2_loc->setCurrentIndex(1); //Bottom
    ui->list_panel2_plugins->clear();
    ui->label_panel2_sample->setWhatsThis("rgba(255,255,255,160)");
    ui->label_panel2_sample->setStyleSheet("background: rgba(255,255,255,160)");
  }
  checkpanels(); //make sure buttons are updated
  
  
  if(!screenonly){
  // Menu Page
  //Default terminal binary
  ui->line_menu_term->setText( settings->value("default-terminal","xterm").toString() );
  //Menu Items
  QStringList items = settings->value("menu/itemlist", QStringList() ).toStringList();
  if(items.isEmpty()){ items << "terminal" << "filemanager" << "applications" << "line" << "settings"; }
  //qDebug() << "Menu Items:" << items;
  ui->list_menu->clear();
  for(int i=0; i<items.length(); i++){
    LPI info = PINFO->menuPluginInfo(items[i]);
    if(items[i].startsWith("app::::")){
      bool ok = false;
      XDGDesktop desk = LXDG::loadDesktopFile(items[i].section("::::",1,1), ok);
      if(!ok){ continue; } //invalid application file (no longer installed?)
      QListWidgetItem *item = new QListWidgetItem();
        item->setWhatsThis( items[i] );
        item->setIcon( LXDG::findIcon(desk.icon) );
        item->setText( desk.name );
        item->setToolTip( desk.comment );
      ui->list_menu->addItem(item);
      continue; //now go to the next item
    }
    if(info.ID.isEmpty()){ continue; } //invalid plugin
    //qDebug() << "Add Menu Item:" << info.ID;
    QListWidgetItem *item = new QListWidgetItem();
      item->setWhatsThis( info.ID );
      item->setIcon( LXDG::findIcon(info.icon,"") );
      item->setText( info.name );
      item->setToolTip( info.description );
    ui->list_menu->addItem(item);
  }
  checkmenuicons(); //update buttons
  }
  //Shortcuts Page
  if(!screenonly){ loadKeyboardShortcuts(); }
  
  //Defaults Page
  if(!screenonly){ loadDefaultSettings(); }
  
  //Session Page
  if(!screenonly){ loadSessionSettings(); }
  
  //Now disable the save button since nothing has changed yet
  loading = false;
  moddesk = modpan =false;
  if(!screenonly){ modmenu = modshort = moddef = modses = false; }//all setup back to original
  ui->push_save->setEnabled(modmenu || modshort || moddef || modses);
}

void MainUI::saveCurrentSettings(bool screenonly){
  QString DPrefix = "desktop-"+QString::number(currentDesktop())+"/";

    // Desktop Page
    if(moddesk){
    QStringList bgs; //get the list of backgrounds to use
    if(ui->radio_desk_multi->isChecked()){
      for(int i=0; i<ui->combo_desk_bg->count(); i++){
	bgs << ui->combo_desk_bg->itemData(i).toString();
      }
    }else if(ui->combo_desk_bg->count() > 0){
	bgs << ui->combo_desk_bg->itemData( ui->combo_desk_bg->currentIndex() ).toString();
	bgs.removeAll("default");
    }
    settings->setValue(DPrefix+"background/filelist", bgs);
    settings->setValue(DPrefix+"background/minutesToChange", ui->spin_desk_min->value());
    }
    
    // Panels Page
    if(modpan){
    settings->setValue(DPrefix+"panels", panelnumber);
    if(panelnumber>=1){
      QString PPrefix = "panel"+QString::number(currentDesktop())+".0/";
      settings->setValue(PPrefix+"color", ui->label_panel1_sample->whatsThis());
      settings->setValue(PPrefix+"height", ui->spin_panel1_size->value());
      int loc = ui->combo_panel1_loc->currentIndex();
      if(loc==0){ settings->setValue(PPrefix+"location", "top"); }
      else if(loc==1){ settings->setValue(PPrefix+"location", "bottom"); }
      else if(loc==2){ settings->setValue(PPrefix+"location", "left"); }
      else{ settings->setValue(PPrefix+"location", "right"); }
      QStringList plugs;
      for(int i=0; i<ui->list_panel1_plugins->count(); i++){
	plugs << ui->list_panel1_plugins->item(i)->whatsThis();
      }
      settings->setValue(PPrefix+"pluginlist",plugs);
      
    }else{
      //Clear that panel's saved settings
      QStringList keys = settings->allKeys().filter("panel"+QString::number(currentDesktop())+".0/");
      for(int i=0; i<keys.length(); i++){  settings->remove(keys[i]); }
    }
    if(panelnumber>=2){
      QString PPrefix = "panel"+QString::number(currentDesktop())+".1/";
      settings->setValue(PPrefix+"color", ui->label_panel2_sample->whatsThis());
      settings->setValue(PPrefix+"height", ui->spin_panel2_size->value());
      int loc = ui->combo_panel2_loc->currentIndex();
      if(loc==0){ settings->setValue(PPrefix+"location", "top"); }
      else if(loc==1){ settings->setValue(PPrefix+"location", "bottom"); }
      else if(loc==2){ settings->setValue(PPrefix+"location", "left"); }
      else{ settings->setValue(PPrefix+"location", "right"); }
      QStringList plugs;
      for(int i=0; i<ui->list_panel2_plugins->count(); i++){
	plugs << ui->list_panel2_plugins->item(i)->whatsThis();
      }
      settings->setValue(PPrefix+"pluginlist",plugs);
    }else{
      //Clear that panel's saved settings
      QStringList keys = settings->allKeys().filter("panel"+QString::number(currentDesktop())+".1/");
      for(int i=0; i<keys.length(); i++){  settings->remove(keys[i]); }
    }
    }

    // Menu Page
    if(modmenu && !screenonly){
    settings->setValue("default-terminal", ui->line_menu_term->text() );
    QStringList items;
    for(int i=0; i<ui->list_menu->count(); i++){
      items << ui->list_menu->item(i)->whatsThis();
    }
    settings->setValue("menu/itemlist", items);
    }
    
    //Shortcuts page
    if(modshort && !screenonly){
      saveKeyboardShortcuts();
    }
    
    //Defaults page
    if(moddef && !screenonly){
      saveDefaultSettings();
    }
    
    //Session Page
    if(modses && !screenonly){
      saveSessionSettings();
    }
    
    //All done - make sure the changes get saved to file right now
    settings->sync();
    appsettings->sync();
    moddesk = modpan = false;
    if(!screenonly){ modmenu = modshort = moddef = modses = false; }
    ui->push_save->setEnabled(modmenu || modshort || moddef || modses); //wait for new changes
}


//===============
//    DESKTOP PAGE
//===============
void MainUI::deskplugchanged(){
  //NOTE: This is not a major change and will not enable the save button
  if(ui->combo_desk_plugs->count()==0){
    //No plugins available
    ui->label_desk_pluginfo->setText("");
    return;
  }
  //Load the new plugin summary
  QString plug = ui->combo_desk_plugs->itemData( ui->combo_desk_plugs->currentIndex() ).toString();
  LPI info = PINFO->desktopPluginInfo(plug);
  ui->label_desk_pluginfo->setText( info.description );
}

void MainUI::deskbgchanged(){
  //Load the new image preview
  if(ui->combo_desk_bg->count()==0){
    ui->label_desk_bgview->setPixmap(QPixmap());
    ui->label_desk_bgview->setText(tr("No Background")+"\n"+tr("(use system default)"));
  }else{
    QString path = ui->combo_desk_bg->itemData( ui->combo_desk_bg->currentIndex() ).toString();
    if(path=="default"){ path = DEFAULTBG; }
    if(QFile::exists(path)){
      QSize sz = ui->label_desk_bgview->size();
      sz.setWidth( sz.width() - (2*ui->label_desk_bgview->frameWidth()) );
      sz.setHeight( sz.height() - (2*ui->label_desk_bgview->frameWidth()) );
      ui->label_desk_bgview->setPixmap( QPixmap(path).scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    }else{
      ui->label_desk_bgview->setPixmap(QPixmap());
      ui->label_desk_bgview->setText(tr("File does not exist"));
    }
  }
  //See if this constitues a change to the current settings and enable the save button
  if(!loading && ui->radio_desk_single->isChecked()){ ui->push_save->setEnabled(true); moddesk=true;}
  //Disable the background rotation option if only one background selected
  if(ui->combo_desk_bg->count()<2){
    ui->radio_desk_single->setChecked(true);
    ui->radio_desk_multi->setEnabled(false);
    ui->spin_desk_min->setEnabled(false);
  }else{
    ui->radio_desk_multi->setEnabled(true);
    ui->spin_desk_min->setEnabled(ui->radio_desk_multi->isChecked());
  }
  
  //Disable the bg remove button if no backgrounds loaded
  ui->tool_desk_rmbg->setEnabled(ui->combo_desk_bg->count()>0);
}

void MainUI::desktimechanged(){
  ui->spin_desk_min->setEnabled(ui->radio_desk_multi->isChecked());
  if(!loading){ ui->push_save->setEnabled(true); moddesk = true; }
}

void MainUI::deskbgremoved(){
  if(ui->combo_desk_bg->count()<1){ return; } //nothing to remove
  ui->combo_desk_bg->removeItem( ui->combo_desk_bg->currentIndex() );
  ui->push_save->setEnabled(true);
  moddesk = true;
}

void MainUI::deskbgadded(){
  //Prompt the user to find an image file to use for a background
  QString dir = "/usr/local/share/wallpapers/Lumina-DE";
  if( !QFile::exists(dir) ){ dir = QDir::homePath(); }
  QStringList bgs = QFileDialog::getOpenFileNames(this, tr("Find Background Image(s)"), dir, "Images (*.png *.xpm *.jpg)");
  if(bgs.isEmpty()){ return; }
  for(int i=0; i<bgs.length(); i++){
    ui->combo_desk_bg->addItem( QIcon(bgs[i]), bgs[i].section("/",-1), bgs[i]);
  }
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );
  ui->push_save->setEnabled(true); //this is definitely a change
  moddesk = true;
}

void MainUI::deskplugadded(){
  settings->sync(); //make sure we have the newly-modified list from the desktop (unique IDs for new plugins)
  QString DPrefix = "desktop-"+QString::number(currentDesktop())+"/";
  QStringList plugins = settings->value(DPrefix+"pluginlist").toStringList();
  //qDebug() << "Current Plugins:" << plugins;
  plugins << ui->combo_desk_plugs->itemData( ui->combo_desk_plugs->currentIndex() ).toString();
  //qDebug() << "New Plugins:" << plugins;
  settings->setValue(DPrefix+"pluginlist", plugins);
  settings->sync();
}


//=============
//  PANELS PAGE
//=============
void MainUI::addpanel1(){
  ui->toolBox_panel1->setVisible(true);
  checkpanels();
  ui->push_save->setEnabled(true);
  modpan = true;
  panelnumber = 1;
}

void MainUI::addpanel2(){
  ui->toolBox_panel2->setVisible(true);
  checkpanels();	
  ui->push_save->setEnabled(true);
  modpan = true;
  panelnumber = 2;
}

void MainUI::rmpanel1(){
  ui->toolBox_panel1->setVisible(false);
  checkpanels();	
  ui->push_save->setEnabled(true);
  modpan = true;
  panelnumber = 0;
}

void MainUI::rmpanel2(){
  ui->toolBox_panel2->setVisible(false);
  checkpanels();	
  ui->push_save->setEnabled(true);
  modpan = true;
  panelnumber = 1;
}

void MainUI::checkpanels(){
  //This checks the primary panel buttons/visibility
  ui->tool_panel1_add->setVisible(!ui->toolBox_panel1->isVisible());
  ui->tool_panel1_rm->setVisible(ui->toolBox_panel1->isVisible());
  if(ui->tool_panel1_add->isVisible()){
    //No panels at all yet - disable the 2nd panel options
    ui->tool_panel2_add->setVisible(false);
    ui->tool_panel2_rm->setVisible(false);
    ui->toolBox_panel2->setVisible(false);
    ui->label_panel2->setVisible(false);
    ui->gridLayout_panels->setColumnStretch(2,1);
    panelnumber = 0; //no panels at the moment
  }else{
    //Panel 1 is visible - also show options for panel 2 appropriately
    ui->tool_panel2_add->setVisible(!ui->toolBox_panel2->isVisible());
    ui->tool_panel2_rm->setVisible(ui->toolBox_panel2->isVisible());
    ui->label_panel2->setVisible(true);
    ui->tool_panel1_rm->setVisible(!ui->toolBox_panel2->isVisible());
    ui->gridLayout_panels->setColumnStretch(2,0);
    if(ui->tool_panel2_add->isVisible()){ panelnumber = 1; }
    else{panelnumber = 2; }
  }
  
}

void MainUI::adjustpanel1(){
  //Adjust panel 1 to complement a panel 2 change
  if(loading){ return; }
  qDebug() << "Adjust Panel 1:";
  ui->toolBox_panel1->setCurrentIndex( ui->toolBox_panel2->currentIndex() );
  switch(ui->combo_panel2_loc->currentIndex()){
    case 0:
	ui->combo_panel1_loc->setCurrentIndex(1); break;
    case 1:
	ui->combo_panel1_loc->setCurrentIndex(0); break;
    case 2:
	ui->combo_panel1_loc->setCurrentIndex(3); break;
    case 3:
	ui->combo_panel1_loc->setCurrentIndex(2); break;
  }
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::adjustpanel2(){
  if(loading){ return; }
  //Adjust panel 2 to complement a panel 1 change
  qDebug() << "Adjust Panel 2:";
  ui->toolBox_panel2->setCurrentIndex( ui->toolBox_panel1->currentIndex() );
  switch(ui->combo_panel1_loc->currentIndex()){
    case 0:
	ui->combo_panel2_loc->setCurrentIndex(1); break;
    case 1:
	ui->combo_panel2_loc->setCurrentIndex(0); break;
    case 2:
	ui->combo_panel2_loc->setCurrentIndex(3); break;
    case 3:
	ui->combo_panel2_loc->setCurrentIndex(2); break;
  }
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

	
void MainUI::getpanel1color(){
  QString color = getColorStyle(ui->label_panel1_sample->whatsThis());
  if(color.isEmpty()){ return; } //nothing selected
  ui->label_panel1_sample->setStyleSheet("background: "+color);
  ui->label_panel1_sample->setWhatsThis(color);
  ui->push_save->setEnabled(true);
  modpan = true;
}

void MainUI::getpanel2color(){
  QString color = getColorStyle(ui->label_panel2_sample->whatsThis());
  if(color.isEmpty()){ return; } //nothing selected
  ui->label_panel2_sample->setStyleSheet("background: "+color);
  ui->label_panel2_sample->setWhatsThis(color);
  ui->push_save->setEnabled(true);
  modpan = true;
}

void MainUI::addpanel1plugin(){
  QString pan = getNewPanelPlugin();
  if(pan.isEmpty()){ return; } //nothing selected
  //Add the new plugin to the list
  LPI info = PINFO->panelPluginInfo(pan);
  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name);
    it->setWhatsThis(info.ID);
  ui->list_panel1_plugins->addItem(it);
  ui->list_panel1_plugins->setCurrentItem(it);
  ui->list_panel1_plugins->scrollToItem(it);
  checkpanels(); //update buttons
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::addpanel2plugin(){
  QString pan = getNewPanelPlugin();
  if(pan.isEmpty()){ return; } //nothing selected
  //Add the new plugin to the list
  LPI info = PINFO->panelPluginInfo(pan);
  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name);
    it->setWhatsThis(info.ID);
  ui->list_panel2_plugins->addItem(it);
  ui->list_panel2_plugins->setCurrentItem(it);
  ui->list_panel2_plugins->scrollToItem(it);
  checkpanels(); //update buttons
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::rmpanel1plugin(){
  if(ui->list_panel1_plugins->currentRow() < 0){ return; }
  delete ui->list_panel1_plugins->takeItem( ui->list_panel1_plugins->currentRow() );
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::rmpanel2plugin(){
  if(ui->list_panel2_plugins->currentRow() < 0){ return; }
  delete ui->list_panel2_plugins->takeItem( ui->list_panel2_plugins->currentRow() );
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::uppanel1plugin(){
  int row = ui->list_panel1_plugins->currentRow();
  if( row <= 0){ return; }
  ui->list_panel1_plugins->insertItem(row-1, ui->list_panel1_plugins->takeItem(row));
  ui->list_panel1_plugins->setCurrentRow(row-1);
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::uppanel2plugin(){
  int row = ui->list_panel2_plugins->currentRow();
  if( row <= 0){ return; }
  ui->list_panel2_plugins->insertItem(row-1, ui->list_panel2_plugins->takeItem(row));	
  ui->list_panel2_plugins->setCurrentRow(row-1);
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::dnpanel1plugin(){
  int row = ui->list_panel1_plugins->currentRow();
  if( row < 0 || row >= (ui->list_panel1_plugins->count()-1) ){ return; }
  ui->list_panel1_plugins->insertItem(row+1, ui->list_panel1_plugins->takeItem(row));
  ui->list_panel1_plugins->setCurrentRow(row+1);
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::dnpanel2plugin(){
  int row = ui->list_panel2_plugins->currentRow();
  if( row < 0 || row >= (ui->list_panel2_plugins->count()-1) ){ return; }
  ui->list_panel2_plugins->insertItem(row+1, ui->list_panel2_plugins->takeItem(row));	
  ui->list_panel2_plugins->setCurrentRow(row+1);
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}


//============
//    MENU PAGE
//============
void MainUI::addmenuplugin(){
  QStringList names;
  QStringList plugs = PINFO->menuPlugins();
  for(int i=0; i<plugs.length(); i++){ names << PINFO->menuPluginInfo(plugs[i]).name; }
  bool ok = false;
  QString sel = QInputDialog::getItem(this,tr("New Menu Plugin"),tr("Plugin:"), names,0,false,&ok);
  if(sel.isEmpty() || names.indexOf(sel) < 0 || !ok){ return; }
  //Now add the item to the list
  LPI info = PINFO->menuPluginInfo( plugs[ names.indexOf(sel) ] );
  QListWidgetItem *it;
  if(info.ID=="app"){
    //Need to prompt for the exact application to add to the menu
    // Note: whatsThis() format: "app::::< *.desktop file path >"
    XDGDesktop desk = getSysApp();
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    //Create the item for the list
    it = new QListWidgetItem(LXDG::findIcon(desk.icon,""), desk.name );
      it->setWhatsThis(info.ID+"::::"+desk.filePath);
      it->setToolTip( desk.comment );
  }else{
    it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name );
    it->setWhatsThis(info.ID);
    it->setToolTip( info.description );
  }
  ui->list_menu->addItem(it);
  ui->list_menu->setCurrentRow(ui->list_menu->count()-1); //make sure it is auto-selected
  ui->push_save->setEnabled(true);
  modmenu = true;
}

void MainUI::rmmenuplugin(){
  if(ui->list_menu->currentRow() < 0){ return; } //no selection
  delete ui->list_menu->takeItem( ui->list_menu->currentRow() );
  ui->push_save->setEnabled(true);
  modmenu = true;
}

void MainUI::upmenuplugin(){
  int row = ui->list_menu->currentRow();
  if(row <= 0){ return; }
  ui->list_menu->insertItem(row-1, ui->list_menu->takeItem(row));
  ui->list_menu->setCurrentRow(row-1);
  ui->push_save->setEnabled(true);
  checkmenuicons();
  modmenu = true;
}

void MainUI::downmenuplugin(){
  int row = ui->list_menu->currentRow();
  if(row < 0 || row >= (ui->list_menu->count()-1) ){ return; }
  ui->list_menu->insertItem(row+1, ui->list_menu->takeItem(row));
  ui->list_menu->setCurrentRow(row+1);
  ui->push_save->setEnabled(true);
  checkmenuicons();
  modmenu = true;
}

void MainUI::findmenuterminal(){
  QString chkpath = "/usr/local/bin";
  if(!QFile::exists(chkpath)){ chkpath = QDir::homePath(); }
  QString bin = QFileDialog::getOpenFileName(this, tr("Set Default Terminal Application"), chkpath, tr("Application Binaries (*)") );
  if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
  if( !QFileInfo(bin).isExecutable() ){ 
    QMessageBox::warning(this, tr("Invalid Binary"), tr("The selected file is not executable!"));
    return;
  }
  ui->line_menu_term->setText(bin);
  ui->push_save->setEnabled(true);
  modmenu = true;
}

void MainUI::checkmenuicons(){
  ui->tool_menu_up->setEnabled( ui->list_menu->currentRow() > 0 );
  ui->tool_menu_dn->setEnabled( ui->list_menu->currentRow() < (ui->list_menu->count()-1) );
  ui->tool_menu_rm->setEnabled( ui->list_menu->currentRow() >=0 );
  if(settings->value("default-terminal","").toString()!=ui->line_menu_term->text()){ 
    ui->push_save->setEnabled(true);
    modmenu = true;
  }
}

//===========
// Shortcuts Page
//===========
void MainUI::loadKeyboardShortcuts(){
  ui->tree_shortcut->clear();
  QStringList info = readFile(QDir::homePath()+"/.lumina/fluxbox-keys");
  //First take care of the special Lumina options
  QStringList special;
  special << "Exec lumina-open -volumeup::::"+tr("Audio Volume Up") \
	<< "Exec lumina-open -volumedown::::"+tr("Audio Volume Down") \
	<< "Exec lumina-open -brightnessup::::"+tr("Screen Brightness Up") \
	<< "Exec lumina-open -brightnessdown::::"+tr("Screen Brightness Down") \
	<< "Exec lumina-screenshot::::"+tr("Take Screenshot");
  for(int i=0; i<special.length(); i++){
    QString spec = info.filter(":"+special[i].section("::::",0,0)).join("").simplified();
    QTreeWidgetItem *it = new QTreeWidgetItem();
      it->setText(0, special[i].section("::::",1,1));
      it->setWhatsThis(0, special[i].section("::::",0,0));
    if(!spec.isEmpty()){
      info.removeAll(spec); //this line has been dealt with - remove it
      it->setText(1, fluxToDispKeys(spec.section(":",0,0)) ); //need to make this easier to read later
      it->setWhatsThis(1, spec.section(":",0,0) );
    }
    ui->tree_shortcut->addTopLevelItem(it);
  }
  //Now add support for all the other fluxbox shortcuts
  for(int i=0; i<info.length(); i++){
    //skip empty/invalid lines, as well as non-global shortcuts (OnMenu, OnWindow, etc..)
    if(info[i].isEmpty() || info[i].startsWith("#") || info[i].startsWith("!") || info[i].startsWith("On")){ continue; }
    QTreeWidgetItem *it = new QTreeWidgetItem();
      it->setText(0, info[i].section(":",1,10).replace("Exec ","Run ").section("{",0,0).simplified());
      it->setWhatsThis(0, info[i].section(":",1,10));
      it->setText(1, fluxToDispKeys(info[i].section(":",0,0)) ); //need to make this easier to read later
      it->setWhatsThis(1, info[i].section(":",0,0) );
    ui->tree_shortcut->addTopLevelItem(it);
  }
}

void MainUI::saveKeyboardShortcuts(){
  //First get all the current listings
  QStringList current;
  for(int i=0; i<ui->tree_shortcut->topLevelItemCount(); i++){
    QTreeWidgetItem *it = ui->tree_shortcut->topLevelItem(i);
    current << it->whatsThis(1)+" :"+it->whatsThis(0); //Full Fluxbox command line
  }
  
  QStringList info = readFile(QDir::homePath()+"/.lumina/fluxbox-keys");
  for(int i=0; i<info.length(); i++){
    if(info[i].isEmpty() || info[i].startsWith("#") || info[i].startsWith("!")){ continue; }
    if(current.filter(info[i].section(":",1,10)).length() > 0){
      //Found Item to be replaced/removed
      QString it = current.filter(info[i].section(":",1,10)).join("\n").section("\n",0,0); //ensure only the first match
      if(it.section(" :",0,0).isEmpty()){ info.removeAt(i); i--; } //remove this entry
      else{ info[i] = it; } //replace this entry
      current.removeAll(it); //already taken care of - remove it from the current list
    }
  }
  //Now save the new contents
  for(int i=0; i<current.length(); i++){
    if(!current[i].section(" :",0,0).isEmpty()){ info << current[i]; }
  }
  bool ok = overwriteFile(QDir::homePath()+"/.lumina/fluxbox-keys", info);
  if(!ok){ qDebug() << "Warning: Could not save ~/.lumina/fluxbox-keys"; }
}

void MainUI::clearKeyBinding(){
  if(ui->tree_shortcut->currentItem()==0){ return; }
  ui->tree_shortcut->currentItem()->setText(1,"");
  ui->tree_shortcut->currentItem()->setWhatsThis(1,"");
  ui->push_save->setEnabled(true);
  modshort=true;
}

void MainUI::getKeyPress(){
  if(ui->tree_shortcut->currentItem()==0){ return; } //nothing selected
  KeyCatch dlg(this);
  dlg.exec();
  if(dlg.cancelled){ return; }
  qDebug() << "Key Press:" << dlg.xkeys << dlg.qkeys;
  QTreeWidgetItem *it = ui->tree_shortcut->currentItem();
  //if(dlg.qkeys.endsWith("+")){ dlg.qkeys.replace("+"," "); dlg.qkeys = dlg.qkeys.append("+").simplified(); }
  //else{ dlg.qkeys.replace("+"," "); }
  it->setText(1,dlg.qkeys);
  it->setWhatsThis(1,dispToFluxKeys(dlg.xkeys));
  ui->push_save->setEnabled(true);
  modshort=true;
}

//===========
// Defaults Page
//===========
void MainUI::loadDefaultSettings(){
  ui->tree_defaults->clear();
  QStringList keys = appsettings->allKeys();
  QStringList groups = keys.filter("Groups/");
  if(groups.isEmpty()){
    //setup default groups
    appsettings->setValue("Groups/Web", QStringList() << "http" << "https" << "ftp");
    appsettings->setValue("Groups/Email", QStringList() << "eml" << "msg");
    appsettings->setValue("Groups/Development-C",QStringList() << "c" << "cpp" << "h" << "hpp");
    appsettings->setValue("Groups/Development-Ruby",QStringList() << "rb" << "rbw");
    appsettings->setValue("Groups/Development-Python",QStringList() << "py" << "pyw");
    appsettings->setValue("Groups/Development-Fortran",QStringList() <<"f"<<"for"<<"f90"<<"f95"<<"f03"<<"f15");
    appsettings->setValue("Groups/Images",QStringList() <<"jpg"<<"png"<<"tif"<<"gif"<<"bmp"<<"raw"<<"svg"<<"jpeg");
    //Add more default groups later

    appsettings->sync();
    groups = appsettings->allKeys().filter("Groups/");
  }
  groups << "Uncategorized";
  QStringList defaults = keys.filter("default/");
  for(int g=0; g<groups.length(); g++){
    //Create the group entry
    QTreeWidgetItem *group = new QTreeWidgetItem( QStringList() << groups[g].section("/",-1) << "" );
    ui->tree_defaults->addTopLevelItem(group);
    //Now populate the group
    if(g == groups.length()-1){
      //uncategorized - everything leftover
      for(int i=0; i<defaults.length(); i++){
	QString path = appsettings->value(defaults[i],"").toString();
	if(path.isEmpty()){ continue; } //ignore empty/uncategoried defaults
	bool ok = false;
	XDGDesktop file = LXDG::loadDesktopFile(path, ok);
	QTreeWidgetItem *it = new QTreeWidgetItem(QStringList() << defaults[i].section("/",-1) << "");
	it->setWhatsThis(1,path);
	if(!ok || file.filePath.isEmpty()){
	  //Might be a binary - just print out the raw "path"
	  it->setText(1,path);
	}else{
	  it->setText(1, file.name);
	  it->setIcon(1, LXDG::findIcon(file.icon,"") );
	}
	group->addChild(it);
      }
    }else{
      QStringList ch = appsettings->value(groups[g],QStringList()).toStringList();
      for(int i=0; i<ch.length(); i++){
	int index = defaults.indexOf("default/"+ch[i]);
	if(index>=0){ defaults.removeAt(index); } //remove this item from the list
        QString path = appsettings->value("default/"+ch[i],"").toString();
	QTreeWidgetItem *it = new QTreeWidgetItem(QStringList() << ch[i] << "");
	if( !path.isEmpty() ){
	  //has something saved
	  bool ok = false;
	  XDGDesktop file = LXDG::loadDesktopFile(path, ok);
	  it->setWhatsThis(1,path);
	  if(!ok || file.filePath.isEmpty()){
	    //Might be a binary - just print out the raw "path"
	    it->setText(1,path);
	  }else{
	    it->setText(1, file.name);
	    it->setIcon(1, LXDG::findIcon(file.icon,"") );
	  }
	}
	group->addChild(it);
      }
    }
  }
  checkdefaulticons();
}

void MainUI::saveDefaultSettings(){
  for(int i=0; i<ui->tree_defaults->topLevelItemCount(); i++){
    //Groups
    QTreeWidgetItem *group = ui->tree_defaults->topLevelItem(i);
    QStringList items;
    for(int c=0; c<group->childCount(); c++){
      //Save this individual default value (and remember it later)
      QTreeWidgetItem *it = group->child(c);
      items << it->text(0);
      if( !it->whatsThis(1).isEmpty()){
	appsettings->setValue("default/"+it->text(0), it->whatsThis(1));
      }
    }
    //Do not save the uncategorized group header (internal only)
    if(group->text(0).toLower()!="uncategorized" && !items.isEmpty()){
      appsettings->setValue("Groups/"+group->text(0), items);
    }
  }
}

void MainUI::adddefaultgroup(){
  //Prompt for the group name
  bool ok = false;
  QString name = QInputDialog::getText(this, tr("New Application Group"), tr("Name:"), QLineEdit::Normal, "", &ok);
  if(name.isEmpty() || !ok){ return; } //cancelled
  //Make sure that name is not already taken
	
  //Add it as a new top-level item
  ui->tree_defaults->addTopLevelItem( new QTreeWidgetItem( QStringList() << name << "" ) );
  ui->push_save->setEnabled(true);
  moddef = true;
}

void MainUI::adddefaultextension(){
  //Verify which group is selected
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  if(it==0){ return; } //no selection
  if(it->parent()!=0){ it = it->parent(); } //make sure to get the group item
  //Prompt for the extension name
  bool ok = false;
  QString name = QInputDialog::getText(this, tr("New File Extension"), tr("Extension:"), QLineEdit::Normal, "", &ok);
  if(name.isEmpty() || !ok){ return; } //cancelled
  //Make sure that name is not already taken
	
  //Add it as a new child of this group item
  it->addChild( new QTreeWidgetItem( QStringList() << name << "" ) );
  ui->push_save->setEnabled(true);
  moddef = true;
}

void MainUI::cleardefaultitem(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  if(it==0){ return; } //no item selected
  QList<QTreeWidgetItem*> list;
  for(int i=0; i<it->childCount(); i++){
    list << it->child(i);
  }
  if(list.isEmpty()){ list << it; } //just do the current item
  //Now clear the items
  for(int i=0; i<list.length(); i++){
    list[i]->setWhatsThis(1,""); //clear the app path
    list[i]->setIcon(1,QIcon()); //clear the icon
    list[i]->setText(1,""); //clear the name
  }
  ui->push_save->setEnabled(true);
  moddef = true;
}

void MainUI::setdefaultitem(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  if(it==0){ return; } //no item selected
  QList<QTreeWidgetItem*> list;
  for(int i=0; i<it->childCount(); i++){
    list << it->child(i);
  }
  if(list.isEmpty()){ list << it; } //just do the current item
  //Prompt for which application to use
  XDGDesktop desk = getSysApp();
    if(desk.filePath.isEmpty()){ return; }//nothing selected
  //Now set the items
  for(int i=0; i<list.length(); i++){
    list[i]->setWhatsThis(1,desk.filePath); //app path
    list[i]->setIcon(1,LXDG::findIcon(desk.icon,"")); //clear the icon
    list[i]->setText(1,desk.name); //clear the name
  }
  ui->push_save->setEnabled(true);
  moddef = true;
}

void MainUI::checkdefaulticons(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  ui->tool_defaults_set->setEnabled(it!=0);
  ui->tool_defaults_clear->setEnabled(it!=0);
  ui->tool_defaults_addextension->setEnabled( it!=0);
  if(it!=0){
    if(it->text(0)=="Uncategorized"){
     ui->tool_defaults_set->setEnabled(false);
     ui->tool_defaults_clear->setEnabled(false);   
    }
  }
}

//===========
// Session Page
//===========
void MainUI::loadSessionSettings(){
  QStringList FB = readFile(QDir::homePath()+"/.lumina/fluxbox-init");
  QString val;
  //Do the window placement
  val = FB.filter("session.screen0.windowPlacement:").join("").section(":",1,1).simplified();
  //qDebug() << "Window Placement:" << val;
  int index = ui->combo_session_wloc->findData(val);
  if(index<0){ index = 0;} //use the default
  ui->combo_session_wloc->setCurrentIndex(index);

  //Do the window focus
  val = FB.filter("session.screen0.focusModel:").join("").section(":",1,1).simplified();
  //qDebug() << "Window Focus:" <<  val;
  index = ui->combo_session_wfocus->findData(val);
  if(index<0){ index = 0;} //use the default
  ui->combo_session_wfocus->setCurrentIndex(index);
  
  //Do the window theme
  val = FB.filter("session.styleFile:").join("").section(":",1,1).simplified();
  //qDebug() << "Window Theme:" << val;
  index = ui->combo_session_wtheme->findData(val);
  if(index<0){ index = 0;} //use the default
  ui->combo_session_wtheme->setCurrentIndex(index);
  
  //Now the number of workspaces
  val = FB.filter("session.screen0.workspaces:").join("").section(":",1,1).simplified();
  //qDebug() << "Number of Workspaces:" << val;
  if(!val.isEmpty()){ ui->spin_session_wkspaces->setValue(val.toInt()); }
  
  //Now do the startup applications
  QStringList STARTUP = readFile(QDir::homePath()+"/.lumina/startapps");
  ui->list_session_start->clear();
  for(int i=0; i<STARTUP.length(); i++){
    if(STARTUP[i].startsWith("#")){ continue; }
    else if(STARTUP[i].startsWith("lumina-open ")){
      //Application or file
      QString file = STARTUP[i].section("lumina-open ",0,0,QString::SectionSkipEmpty).simplified();
      bool ok = false;
      XDGDesktop desk = LXDG::loadDesktopFile(file, ok);
      if(!desk.filePath.isEmpty() && ok && desk.filePath.endsWith(".desktop") ){
        //Application
	QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(desk.icon,""), desk.name);
	      it->setWhatsThis(STARTUP[i]); //keep the raw line
	ui->list_session_start->addItem(it);
      }else{
	//Some other file
	QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon("unknown",""), file.section("/",-1));
	      it->setWhatsThis(STARTUP[i]); //keep the raw line
	ui->list_session_start->addItem(it);
      }
    }else{
      //Some other utility (binary?)
      QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon("application-x-executable",""), STARTUP[i].section(" ",0,0) );
	      it->setWhatsThis(STARTUP[i]); //keep the raw line
	ui->list_session_start->addItem(it);
    }
  }
  
  //Now do the general session options
  ui->check_session_numlock->setChecked( sessionsettings->value("EnableNumlock", true).toBool() );
  ui->check_session_playloginaudio->setChecked( sessionsettings->value("PlayStartupAudio",true).toBool() );
  ui->check_session_playlogoutaudio->setChecked( sessionsettings->value("PlayLogoutAudio",true).toBool() );
  
  sessionstartchanged(); //make sure to update buttons
}

void MainUI::saveSessionSettings(){
  //Do the fluxbox settings first
  QStringList FB = readFile(QDir::homePath()+"/.lumina/fluxbox-init");
  // - window placement
  int index = FB.indexOf( FB.filter("session.screen0.windowPlacement:").join("") );
  QString line = "session.screen0.windowPlacement:\t"+ui->combo_session_wloc->itemData( ui->combo_session_wloc->currentIndex() ).toString();
  if(index < 0){ FB << line; } //add line to the end of the file
  else{ FB[index] = line; } //replace the current setting with the new one
  // - window focus
  index = FB.indexOf( FB.filter("session.screen0.focusModel:").join("") );
  line = "session.screen0.focusModel:\t"+ui->combo_session_wfocus->itemData( ui->combo_session_wfocus->currentIndex() ).toString();
  if(index < 0){ FB << line; } //add line to the end of the file
  else{ FB[index] = line; } //replace the current setting with the new one
  // - window theme
  index = FB.indexOf( FB.filter("session.styleFile:").join("") );
  line = "session.styleFile:\t"+ui->combo_session_wtheme->itemData( ui->combo_session_wtheme->currentIndex() ).toString();
  if(index < 0){ FB << line; } //add line to the end of the file
  else{ FB[index] = line; } //replace the current setting with the new one
  // - workspace number
  index = FB.indexOf( FB.filter("session.screen0.workspaces:").join("") );
  line = "session.screen0.workspaces:\t"+QString::number(ui->spin_session_wkspaces->value());
  if(index < 0){ FB << line; } //add line to the end of the file
  else{ FB[index] = line; } //replace the current setting with the new one

  //Save the fluxbox settings
  bool ok = overwriteFile(QDir::homePath()+"/.lumina/fluxbox-init", FB);
  if(!ok){ qDebug() << "Warning: Could not save ~/.lumina/startapps"; }
  //Now do the start apps
  QStringList STARTUP;
  for(int i=0; i<ui->list_session_start->count(); i++){
    STARTUP << ui->list_session_start->item(i)->whatsThis();
  }
  ok = overwriteFile(QDir::homePath()+"/.lumina/startapps", STARTUP);
  if(!ok){ qDebug() << "Warning: Could not save ~/.lumina/startapps"; }
  
  //Now do the general session options
  sessionsettings->setValue("EnableNumlock", ui->check_session_numlock->isChecked());
  sessionsettings->setValue("PlayStartupAudio", ui->check_session_playloginaudio->isChecked());
  sessionsettings->setValue("PlayLogoutAudio", ui->check_session_playlogoutaudio->isChecked());
}

void MainUI::rmsessionstartitem(){
  if(ui->list_session_start->currentRow() < 0){ return; } //no item selected
  delete ui->list_session_start->takeItem(ui->list_session_start->currentRow());
  sessionoptchanged();
}

void MainUI::addsessionstartapp(){
  //Prompt for the application to start
  XDGDesktop desk = getSysApp();
  if(desk.filePath.isEmpty()){ return; } //cancelled
  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(desk.icon,""), desk.name );
    it->setWhatsThis("lumina-open "+desk.filePath); //command to be saved/run
  ui->list_session_start->addItem(it);
  ui->list_session_start->setCurrentItem(it);
  sessionoptchanged();
}

void MainUI::addsessionstartbin(){
  QString chkpath = "/usr/local/bin";
  if(!QFile::exists(chkpath)){ chkpath = QDir::homePath(); }
  QString bin = QFileDialog::getOpenFileName(this, tr("Select Binary"), chkpath, tr("Application Binaries (*)") );
  if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
  if( !QFileInfo(bin).isExecutable() ){ 
    QMessageBox::warning(this, tr("Invalid Binary"), tr("The selected file is not executable!"));
    return;
  }
  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon("application-x-executable",""), bin.section("/",-1) );
    it->setWhatsThis(bin); //command to be saved/run
  ui->list_session_start->addItem(it);
  ui->list_session_start->setCurrentItem(it);
  sessionoptchanged();
}

void MainUI::addsessionstartfile(){
  QString chkpath = QDir::homePath();
  QString bin = QFileDialog::getOpenFileName(this, tr("Select File"), chkpath, tr("All Files (*)") );
  if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon("unknown",""), bin.section("/",-1) );
    it->setWhatsThis("lumina-open "+bin); //command to be saved/run
  ui->list_session_start->addItem(it);
  ui->list_session_start->setCurrentItem(it);
  sessionoptchanged();
}

void MainUI::sessionoptchanged(){
  if(!loading){
    ui->push_save->setEnabled(true);
    modses = true;
  }
}

void MainUI::sessionstartchanged(){
  ui->tool_session_rmapp->setEnabled( ui->list_session_start->currentRow()>=0 );
}

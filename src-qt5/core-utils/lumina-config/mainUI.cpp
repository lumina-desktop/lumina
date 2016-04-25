//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h" //the designer *.ui file

#include <LuminaOS.h>
#include <QImageReader>
#include <QTime>
#include <QDate>
//#include <QTimeZone>
#include <QScrollBar>

#include <unistd.h>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI()){
  ui->setupUi(this); //load the designer file
  this->setWindowIcon( LXDG::findIcon("preferences-desktop-display","") );
  PINFO = new LPlugins(); //load the info class
  panadjust = false;
  DEFAULTBG = LOS::LuminaShare()+"desktop-background.jpg";
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
    ui->spin_screen->setValue(desktop->screenNumber(this->mapToGlobal(this->geometry().center()))+1); //have the current screen auto-selected
  //qDebug() << "Number of Screens:" << desktop->screenCount();
  sysApps = LXDG::sortDesktopNames( LXDG::systemDesktopFiles() );

  //Now finish setting up the UI
  setupIcons();
  setupMenus();
  setupConnections();

  //Start on the Desktop page (and first tab for all tab widgets)
  ui->stackedWidget->setCurrentWidget(ui->page_desktop);
  ui->tabWidget_desktop->setCurrentWidget(ui->tab_wallpaper);
  ui->tabWidget_session->setCurrentIndex(0);
  ui->tabWidget_apps->setCurrentIndex(0);
  ui->tabWidget_panels->setCurrentIndex(0);
  
  slotChangePage(false);
  
  QTimer::singleShot(10, this, SLOT(loadCurrentSettings()) );

  //Disable the incomplete pages/items at the moment

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
  ui->actionDesktop->setIcon( LXDG::findIcon("preferences-desktop-display","") );
  ui->actionPanels->setIcon( LXDG::findIcon("preferences-desktop-icons","") );
  //ui->actionMenu->setIcon( LXDG::findIcon("preferences-desktop-icons","") );
  ui->actionShortcuts->setIcon( LXDG::findIcon("configure-shortcuts","") );
  ui->actionDefaults->setIcon( LXDG::findIcon("preferences-system-windows","") );
  ui->actionSession->setIcon( LXDG::findIcon("preferences-system-session-services","") );
  ui->push_save->setIcon( LXDG::findIcon("document-save","") );


  //Desktop Page
  ui->tool_desk_addbg->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_desk_rmbg->setIcon( LXDG::findIcon("list-remove","") );
  ui->tabWidget_desktop->setTabIcon( ui->tabWidget_desktop->indexOf(ui->tab_wallpaper), LXDG::findIcon("preferences-desktop-wallpaper","") );
  ui->tabWidget_desktop->setTabIcon( ui->tabWidget_desktop->indexOf(ui->tab_themes), LXDG::findIcon("preferences-desktop-theme","") );
  ui->tool_desktop_addplugin->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_desktop_rmplugin->setIcon( LXDG::findIcon("list-remove","") );
  
  //Panels Page
  ui->tool_panels_add->setIcon( LXDG::findIcon("list-add","") );
  
  //Menu Page
  ui->tool_menu_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_menu_rm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_menu_up->setIcon( LXDG::findIcon("go-up","") );
  ui->tool_menu_dn->setIcon( LXDG::findIcon("go-down","") );

  //Shortcuts Page
  ui->tool_shortcut_set->setIcon( LXDG::findIcon("input-keyboard","") );
  ui->tool_shortcut_clear->setIcon( LXDG::findIcon("edit-clear","") );

  //Defaults Page
  //ui->tool_defaults_addextension->setIcon( LXDG::findIcon("list-add","") );
  //ui->tool_defaults_addgroup->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_defaults_clear->setIcon( LXDG::findIcon("edit-clear","") );
  ui->tool_defaults_set->setIcon( LXDG::findIcon("system-run","") );
  ui->tool_defaults_setbin->setIcon( LXDG::findIcon("application-x-executable","") );
  ui->tabWidget_apps->setTabIcon( ui->tabWidget_apps->indexOf(ui->tab_auto), LXDG::findIcon("system-run", "") );
  ui->tabWidget_apps->setTabIcon( ui->tabWidget_apps->indexOf(ui->tab_defaults), LXDG::findIcon("preferences-desktop-filetype-association", "") );

  //Session Page
  //ui->tool_session_rmapp->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_session_addapp->setIcon( LXDG::findIcon("system-run","") );
  ui->tool_session_addbin->setIcon( LXDG::findIcon("system-search","") );
  ui->tool_session_addfile->setIcon( LXDG::findIcon("run-build-file","") );
  ui->tool_session_newtheme->setIcon( LXDG::findIcon("preferences-desktop-theme","") );
  ui->tool_session_newcolor->setIcon( LXDG::findIcon("preferences-desktop-color","") );
  ui->push_session_resetSysDefaults->setIcon( LXDG::findIcon("pcbsd","view-refresh") );
  ui->push_session_resetLuminaDefaults->setIcon( LXDG::findIcon("Lumina-DE","") );
  ui->tool_help_time->setIcon( LXDG::findIcon("help-about","") );
  ui->tool_help_date->setIcon( LXDG::findIcon("help-about","") );
}

void MainUI::setupConnections(){
  //General UI
  connect(ui->actionDesktop, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionPanels, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  //connect(ui->actionMenu, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionShortcuts, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionDefaults, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->actionSession, SIGNAL(triggered(bool)), this, SLOT( slotChangePage(bool)) );
  connect(ui->push_save, SIGNAL(clicked()), this, SLOT(saveCurrentSettings()) );
  connect(ui->spin_screen, SIGNAL(valueChanged(int)), this, SLOT(slotChangeScreen()) );

  //Desktop Page
  //connect(ui->combo_desk_plugs, SIGNAL(currentIndexChanged(int)), this, SLOT(deskplugchanged()) );
  connect(ui->combo_desk_bg, SIGNAL(currentIndexChanged(int)), this, SLOT(deskbgchanged()) );
  connect(ui->radio_desk_multi, SIGNAL(toggled(bool)), this, SLOT(desktimechanged()) );
  connect(ui->tool_desktop_addplugin, SIGNAL(clicked()), this, SLOT(deskplugadded()) );
  connect(ui->tool_desktop_rmplugin, SIGNAL(clicked()), this, SLOT(deskplugremoved()) );
  connect(ui->tool_desk_addbg, SIGNAL(clicked()), this, SLOT(deskbgadded()) );
  connect(ui->tool_desk_rmbg, SIGNAL(clicked()), this, SLOT(deskbgremoved()) );
  connect(ui->spin_desk_min, SIGNAL(valueChanged(int)), this, SLOT(desktimechanged()) );
  connect(ui->check_desktop_autolaunchers, SIGNAL(clicked()), this, SLOT(desktimechanged()) ); //just need to poke the save routines
  connect(ui->combo_desk_layout, SIGNAL(currentIndexChanged(int)), this, SLOT(desktimechanged()) ); //just need to poke the save routines
  	
  //Panels Page
  connect(ui->tool_panels_add, SIGNAL(clicked()), this, SLOT(newPanel()) );

  //Menu Page
  connect(ui->tool_menu_add, SIGNAL(clicked()), this, SLOT(addmenuplugin()) );
  connect(ui->tool_menu_rm, SIGNAL(clicked()), this, SLOT(rmmenuplugin()) );
  connect(ui->tool_menu_up, SIGNAL(clicked()), this, SLOT(upmenuplugin()) );
  connect(ui->tool_menu_dn, SIGNAL(clicked()), this, SLOT(downmenuplugin()) );
  connect(ui->list_menu, SIGNAL(currentRowChanged(int)), this, SLOT(checkmenuicons()) );

  //Shortcuts Page
  connect(ui->tool_shortcut_clear, SIGNAL(clicked()), this, SLOT(clearKeyBinding()) );
  connect(ui->tool_shortcut_set, SIGNAL(clicked()), this, SLOT(applyKeyBinding()) );
  connect(ui->tree_shortcut, SIGNAL(itemSelectionChanged()), this, SLOT(updateKeyConfig()) );
  
  //Defaults Page
  connect(ui->tool_default_filemanager, SIGNAL(clicked()), this, SLOT(changeDefaultFileManager()) );
  connect(ui->tool_default_terminal, SIGNAL(clicked()), this, SLOT(changeDefaultTerminal()) );
  connect(ui->tool_default_webbrowser, SIGNAL(clicked()), this, SLOT(changeDefaultBrowser()) );
  connect(ui->tool_default_email, SIGNAL(clicked()), this, SLOT(changeDefaultEmail()) );
  connect(ui->tool_defaults_clear, SIGNAL(clicked()), this, SLOT(cleardefaultitem()) );
  connect(ui->tool_defaults_set, SIGNAL(clicked()), this, SLOT(setdefaultitem()) );
  connect(ui->tool_defaults_setbin, SIGNAL(clicked()), this, SLOT(setdefaultbinary()) );
  connect(ui->tree_defaults, SIGNAL(itemSelectionChanged()), this, SLOT(checkdefaulticons()) );

  //Session Page
  connect(ui->tool_session_addapp, SIGNAL(clicked()), this, SLOT(addsessionstartapp()) );
  connect(ui->tool_session_addbin, SIGNAL(clicked()), this, SLOT(addsessionstartbin()) );
  connect(ui->tool_session_addfile, SIGNAL(clicked()), this, SLOT(addsessionstartfile()) );
  connect(ui->combo_session_wfocus, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_session_wloc, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_session_wtheme, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionthemechanged()) );
  connect(ui->combo_session_cursortheme, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionCursorChanged()) );
  connect(ui->check_session_numlock, SIGNAL(stateChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->check_session_playloginaudio, SIGNAL(stateChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->check_session_playlogoutaudio, SIGNAL(stateChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->spin_session_wkspaces, SIGNAL(valueChanged(int)), this, SLOT(sessionoptchanged()) );
  //connect(ui->list_session_start, SIGNAL(currentRowChanged(int)), this, SLOT(sessionstartchanged()) );
  connect(ui->list_session_start, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(sessionoptchanged()) );
  connect(ui->spin_session_fontsize, SIGNAL(valueChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_session_themefile, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_session_colorfile, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_session_icontheme, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->font_session_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->tool_session_newcolor, SIGNAL(clicked()), this, SLOT(sessionEditColor()) );
  connect(ui->tool_session_newtheme, SIGNAL(clicked()), this, SLOT(sessionEditTheme()) );
  connect(ui->push_session_setUserIcon, SIGNAL(clicked()), this, SLOT(sessionChangeUserIcon()) );
  connect(ui->push_session_resetSysDefaults, SIGNAL(clicked()), this, SLOT(sessionResetSys()) );
  connect(ui->push_session_resetLuminaDefaults, SIGNAL(clicked()), this, SLOT(sessionResetLumina()) );
  connect(ui->tool_help_time, SIGNAL(clicked()), this, SLOT(sessionShowTimeCodes()) );
  connect(ui->tool_help_date, SIGNAL(clicked()), this, SLOT(sessionShowDateCodes()) );
  connect(ui->line_session_time, SIGNAL(textChanged(QString)), this, SLOT(sessionLoadTimeSample()) );
  connect(ui->line_session_date, SIGNAL(textChanged(QString)), this, SLOT(sessionLoadDateSample()) );
  connect(ui->combo_session_datetimeorder, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_locale_lang, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_locale_collate, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_locale_ctype, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_locale_message, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_locale_monetary, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_locale_numeric, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
  connect(ui->combo_locale_time, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionoptchanged()) );
}

void MainUI::setupMenus(){
  //Background file menu (different ways of loading files)
  if(ui->tool_desk_addbg->menu()==0){ ui->tool_desk_addbg->setMenu(new QMenu(this)); }
  ui->tool_desk_addbg->menu()->clear();
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("document-new",""), tr("File(s)"), this, SLOT(deskbgadded()) );
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("folder-new",""), tr("Directory (Single)"), this, SLOT(deskbgdiradded()) );
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("document-open-folder",""), tr("Directory (Recursive)"), this, SLOT(deskbgdirradded()) );
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("format-fill-color",""), tr("Solid Color"), this, SLOT(deskbgcoloradded()) );
  
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
  QStringList dirs; dirs << LOS::AppPrefix()+"share/fluxbox/styles" << QDir::homePath()+"/.fluxbox/styles";
  QFileInfoList fbstyles; 
  for(int i=0; i<dirs.length(); i++){
    QDir fbdir(dirs[i]);
    fbstyles << fbdir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
  }
  QString lastdir;
  for(int i=0; i<fbstyles.length(); i++){
    if(lastdir!=fbstyles[i].absolutePath()){
      lastdir = fbstyles[i].absolutePath(); //save for checking later
      if(ui->combo_session_wtheme->count()>0){ ui->combo_session_wtheme->insertSeparator(ui->combo_session_wtheme->count()); }
    }
    ui->combo_session_wtheme->addItem(fbstyles[i].fileName(), fbstyles[i].absoluteFilePath());
  }
  //Display formats for panel clock
  ui->combo_session_datetimeorder->clear();
  ui->combo_session_datetimeorder->addItem( tr("Time (Date as tooltip)"), "timeonly");
  ui->combo_session_datetimeorder->addItem( tr("Date (Time as tooltip)"), "dateonly");
  ui->combo_session_datetimeorder->addItem( tr("Time first then Date"), "timedate");
  ui->combo_session_datetimeorder->addItem( tr("Date first then Time"), "datetime");

  //Available Cursor Themes
  ui->combo_session_cursortheme->clear();
  ui->combo_session_cursortheme->addItems( LTHEME::availableSystemCursors() );
  //int cur = ui->combo_session_cursortheme->findText( LTHEME::currentCursor() );
  //if(cur>=0){ ui->combo_session_cursortheme->setCurrentIndex(cur); }
  
  //Available Wallpaper layout options
  ui->combo_desk_layout->clear();
  ui->combo_desk_layout->addItem(tr("Automatic"), "stretch");
  ui->combo_desk_layout->addItem(tr("Tile"), "tile");
  ui->combo_desk_layout->addItem(tr("Center"), "center");
  ui->combo_desk_layout->addItem(tr("Top Left"), "topleft");
  ui->combo_desk_layout->addItem(tr("Top Right"), "topright");
  ui->combo_desk_layout->addItem(tr("Bottom Left"), "bottomleft");
  ui->combo_desk_layout->addItem(tr("Bottom Right"), "bottomright");
  
  
  
  //Available localizations
  QStringList langs = LUtils::knownLocales();
    langs.sort();
  QString def = tr("System Default");
  ui->combo_locale_lang->addItem(def,"");
  ui->combo_locale_collate->addItem(def,"");
  ui->combo_locale_ctype->addItem(def,"");
  ui->combo_locale_message->addItem(def,"");
  ui->combo_locale_monetary->addItem(def,"");
  ui->combo_locale_numeric->addItem(def,"");
  ui->combo_locale_time->addItem(def,"");
  for(int i=0; i<langs.length(); i++){
    QString lan = QLocale(langs[i]).nativeLanguageName();
      ui->combo_locale_lang->addItem(lan,langs[i]);
      ui->combo_locale_collate->addItem(lan,langs[i]);
      ui->combo_locale_ctype->addItem(lan,langs[i]);
      ui->combo_locale_message->addItem(lan,langs[i]);
      ui->combo_locale_monetary->addItem(lan,langs[i]);
      ui->combo_locale_numeric->addItem(lan,langs[i]);
      ui->combo_locale_time->addItem(lan,langs[i]);
  }
}

int MainUI::currentDesktop(){
  return ui->spin_screen->value()-1; //backend starts at 0, not 1
}

QString MainUI::getColorStyle(QString current, bool allowTransparency){
  QString out;
  //Convert the current color string into a QColor
  QStringList col = current.section(")",0,0).section("(",1,1).split(",");
  if(col.length()!=4){ col.clear(); col << "255" << "255" << "255" << "255"; }
  QColor ccol = QColor(col[0].toInt(), col[1].toInt(), col[2].toInt(), col[3].toInt()); //RGBA
  QColor ncol;
    if(allowTransparency){ ncol= QColorDialog::getColor(ccol, this, tr("Select Color"), QColorDialog::ShowAlphaChannel); }
    else{ ncol= QColorDialog::getColor(ccol, this, tr("Select Color")); }
  //Now convert the new color into a usable string and return
  if(ncol.isValid()){ //if the dialog was not cancelled
    if(allowTransparency){
      out = "rgba("+QString::number(ncol.red())+","+QString::number(ncol.green())+","+QString::number(ncol.blue())+","+QString::number(ncol.alpha())+")";
    }else{
      out = "rgb("+QString::number(ncol.red())+","+QString::number(ncol.green())+","+QString::number(ncol.blue())+")";
    }
  }
  return out;
}

XDGDesktop MainUI::getSysApp(bool allowreset){
  AppDialog dlg(this, sysApps);
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
  in.replace("+"," ");
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
    //ui->actionMenu->setChecked(ui->stackedWidget->currentWidget()==ui->page_menu);
    ui->actionShortcuts->setChecked(ui->stackedWidget->currentWidget()==ui->page_shortcuts);
    ui->actionDefaults->setChecked(ui->stackedWidget->currentWidget()==ui->page_defaults);
    ui->actionSession->setChecked(ui->stackedWidget->currentWidget()==ui->page_session);
    showScreen = (ui->actionDesktop->isChecked() || ui->actionPanels->isChecked());
    //Ask if they want to reset any changes on the current page

  }else{
    //uncheck the button associated with the currently open page
    if(ui->stackedWidget->currentWidget()==ui->page_desktop){ ui->actionDesktop->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_panels){ ui->actionPanels->setChecked(false); }
    //if(ui->stackedWidget->currentWidget()==ui->page_menu){ ui->actionMenu->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_shortcuts){ ui->actionShortcuts->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_defaults){ ui->actionDefaults->setChecked(false); }
    if(ui->stackedWidget->currentWidget()==ui->page_session){ ui->actionSession->setChecked(false); }
    //switch to the new page
    if(ui->actionDesktop->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_desktop); showScreen=true;}
    else if(ui->actionPanels->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_panels); showScreen=true; }
    //else if(ui->actionMenu->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_menu); }
    else if(ui->actionShortcuts->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_shortcuts); }
    else if(ui->actionDefaults->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_defaults); }
    else if(ui->actionSession->isChecked()){ ui->stackedWidget->setCurrentWidget(ui->page_session); }
  }
  ui->group_screen->setVisible(showScreen && (ui->spin_screen->maximum()>1) );
  //Hide the save button for particular pages
  //ui->push_save->setVisible(!ui->actionDefaults->isChecked() || moddesk || modpan || modmenu || modshort || moddef || modses); //hide on the default page if nothing waiting to be saved
  //Special functions for particular pages
  //if(ui->page_panels->isVisible()){ checkpanels(); }

}

void MainUI::slotChangeScreen(){
  static int cscreen = 0; //current screen
  int newscreen = currentDesktop();
  if(cscreen!=newscreen){
    if(moddesk || modpan){
      if(QMessageBox::Yes == QMessageBox::question(this, tr("Save Changes?"), tr("You currently have unsaved changes for this screen. Do you want to save them first?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ){
	ui->spin_screen->setValue(cscreen+1); //Make sure the old screen is selected for a moment
        saveCurrentSettings(true); //only save current screen settings
	ui->spin_screen->setValue(newscreen+1); //Now reset back to the new screen
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
    else if(bgs[i].startsWith("rgb(")){ui->combo_desk_bg->addItem(QString(tr("Solid Color: %1")).arg(bgs[i]), bgs[i]); }
    //else{ ui->combo_desk_bg->addItem( QIcon(QPixmap(bgs[i]).scaled(64,64)), bgs[i].section("/",-1), bgs[i] ); }
    else{ ui->combo_desk_bg->addItem( bgs[i].section("/",-1), bgs[i] ); } //disable the thumbnail - takes a long time for large collections of files
  }
  ui->check_desktop_autolaunchers->setChecked(settings->value(DPrefix+"generateDesktopIcons", false).toBool());
  ui->radio_desk_multi->setEnabled(bgs.length()>1);
  if(bgs.length()>1){ ui->radio_desk_multi->setChecked(true);}
  else{ ui->radio_desk_single->setChecked(true); }
  ui->spin_desk_min->setValue( settings->value(DPrefix+"background/minutesToChange", 5).toInt() );
  desktimechanged(); //ensure the display gets updated (in case the radio selection did not change);
  ui->label_desk_res->setText( tr("Screen Resolution:")+"\n"+QString::number(desktop->screenGeometry(cdesk).width())+"x"+QString::number(desktop->screenGeometry(cdesk).height()) );
  int tmp = ui->combo_desk_layout->findData(settings->value(DPrefix+"background/format","stretch"));
  if(tmp>=0){ ui->combo_desk_layout->setCurrentIndex(tmp); }
  QStringList dplugs = settings->value(DPrefix+"pluginlist",QStringList()).toStringList();
  ui->list_desktop_plugins->clear();
  for(int i=0; i<dplugs.length(); i++){
    QListWidgetItem* it = new QListWidgetItem();
    it->setWhatsThis(dplugs[i]); //save the full thing instantly
    //Now load the rest of the info about the plugin
    QString num;
    if(dplugs[i].contains("---")){ 
      num = dplugs[i].section("---",1,1).section(".",1,1).simplified(); //Skip the screen number
      if(num=="1"){ num.clear(); } //don't bother showing the number
      dplugs[i] = dplugs[i].section("---",0,0);
    }
    if(dplugs[i].startsWith("applauncher::")){
      bool ok = false;
      XDGDesktop app = LXDG::loadDesktopFile(dplugs[i].section("::",1,50), ok);
      if(!ok){ continue; } //invalid for some reason
      //Now fill the item with the necessary info
      it->setText(app.name);
      it->setIcon(LXDG::findIcon(app.icon,"") );
      it->setToolTip(app.comment);
    }else{
      //Load the info for this plugin
      LPI info = PINFO->desktopPluginInfo(dplugs[i]);
      if( info.ID.isEmpty() ){ continue; } //invalid plugin for some reason
      it->setText(info.name);
      it->setToolTip(info.description);
      it->setIcon( LXDG::findIcon(info.icon,"") );
    }
    if(!num.isEmpty()){ it->setText( it->text()+" ("+num+")"); } //append the number
    ui->list_desktop_plugins->addItem(it);
  }
  
  //Panels Page
  int panels = settings->value(DPrefix+"panels",-1).toInt();
  if(panels==-1 && primary){ panels=1; }
  panelnumber = panels;
  loadPanels();

  if(!screenonly){
  // Menu Page
  //Default terminal and filemanager binary
  //ui->line_menu_term->setText( settings->value("default-terminal","xterm").toString() );
  //ui->line_menu_fm->setText( settings->value("default-filemanager","lumina-fm").toString() );
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
  bool needreload = false;
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
      if(bgs.isEmpty()){ bgs << "default"; } //Make sure to always fall back on the default
      settings->setValue(DPrefix+"background/filelist", bgs);
      settings->setValue(DPrefix+"background/minutesToChange", ui->spin_desk_min->value());
      settings->setValue(DPrefix+"generateDesktopIcons", ui->check_desktop_autolaunchers->isChecked());
      settings->setValue(DPrefix+"background/format", ui->combo_desk_layout->currentData().toString());
      QStringList plugs;
      for(int i=0; i<ui->list_desktop_plugins->count(); i++){
	plugs << ui->list_desktop_plugins->item(i)->whatsThis();
      }
      if(settings->value(DPrefix+"pluginlist",QStringList()).toStringList() != plugs){
        settings->setValue(DPrefix+"pluginlist", plugs);
	needreload = true;
      }
    }

    // Panels Page
    if(modpan){ 
	settings->setValue(DPrefix+"panels", PANELS.length());
	savePanels(); 
    }

    // Menu Page
    if(modmenu && !screenonly){
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
      //saveDefaultSettings();
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
    //ui->push_save->setVisible(!ui->actionDefaults->isChecked() || modmenu || modshort || moddef || modses);
    if(needreload){
      //Wait 1 second
      for(int i=0; i<10; i++){ QApplication::processEvents(); usleep(100000); }
      loadCurrentSettings(screenonly);
    }
}


//===============
//    DESKTOP PAGE
//===============
void MainUI::deskbgchanged(){
  //Load the new image preview
  if(ui->combo_desk_bg->count()==0){
    ui->label_desk_bgview->setPixmap(QPixmap());
    ui->label_desk_bgview->setText(tr("No Background")+"\n"+tr("(use system default)"));
    ui->label_desk_bgview->setStyleSheet("");
  }else{
    QString path = ui->combo_desk_bg->itemData( ui->combo_desk_bg->currentIndex() ).toString();
    if(path=="default"){ path = DEFAULTBG; }
    if(QFile::exists(path)){
      QSize sz = ui->label_desk_bgview->size();
      sz.setWidth( sz.width() - (2*ui->label_desk_bgview->frameWidth()) );
      sz.setHeight( sz.height() - (2*ui->label_desk_bgview->frameWidth()) );
      //Update the preview/thumbnail for this item
      QPixmap pix(path);
      ui->label_desk_bgview->setPixmap( pix.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation) );
      ui->combo_desk_bg->setItemIcon(ui->combo_desk_bg->currentIndex(), pix.scaled(64,64) );
      ui->label_desk_bgview->setStyleSheet("");
    }else if(path.startsWith("rgb(")){
      ui->label_desk_bgview->setPixmap(QPixmap());
      ui->label_desk_bgview->setText("");
      ui->label_desk_bgview->setStyleSheet("background-color: "+path+";");
    }else{
      ui->label_desk_bgview->setPixmap(QPixmap());
      ui->label_desk_bgview->setText(tr("File does not exist"));
      ui->label_desk_bgview->setStyleSheet("");
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
  QString dir = LOS::LuminaShare().section("/Lumina-DE",0,0)+"/wallpapers/Lumina-DE";
  qDebug() << "Looking for wallpaper dir:" << dir;
  if( !QFile::exists(dir) ){ dir = QDir::homePath(); }
  QStringList imgs = LUtils::imageExtensions();
  for(int i=0; i<imgs.length(); i++){ imgs[i].prepend("*."); }
  QStringList bgs = QFileDialog::getOpenFileNames(this, tr("Find Background Image(s)"), dir, "Images ("+imgs.join(" ")+");;All Files (*)");
  if(bgs.isEmpty()){ return; }
  for(int i=0; i<bgs.length(); i++){
    ui->combo_desk_bg->addItem( QIcon(bgs[i]), bgs[i].section("/",-1), bgs[i]);
  }
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );
  //If multiple items selected, automatically enable the background rotation option
  if(bgs.length() > 1 && !ui->radio_desk_multi->isChecked()){
    ui->radio_desk_multi->setChecked(true);
  }
  ui->push_save->setEnabled(true); //this is definitely a change
  moddesk = true;
}

void MainUI::deskbgcoloradded(){
  //Prompt the user to select a color (no transparency allowed)
  QString color = getColorStyle("",false); //no initial color
  if(color.isEmpty()){ return; }
  //Add it to the list
  ui->combo_desk_bg->addItem( QString(tr("Solid Color: %1")).arg(color), color);
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );
  
  ui->push_save->setEnabled(true); //this is definitely a change
  moddesk = true;
}

void MainUI::deskbgdiradded(){
  //Add the files from a single directory
 QString dir = LOS::LuminaShare().section("/Lumina-DE",0,0)+"/wallpapers/Lumina-DE";
  qDebug() << "Looking for wallpaper dir:" << dir;
  if( !QFile::exists(dir) ){ dir = QDir::homePath(); }
  dir = QFileDialog::getExistingDirectory(this, tr("Find Background Image Directory"), dir, QFileDialog::ReadOnly);
  if(dir.isEmpty()){ return; }
  //Got a directory - go ahead and find all the valid image files within it
  QStringList imgs = LUtils::imageExtensions();
  for(int i=0; i<imgs.length(); i++){ imgs[i].prepend("*."); }
  QDir qdir(dir);
  QStringList bgs = qdir.entryList(imgs, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  if(bgs.isEmpty()){ return; }
  for(int i=0; i<bgs.length(); i++){
    ui->combo_desk_bg->addItem( bgs[i], qdir.absoluteFilePath(bgs[i]));
  }
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );
  //If multiple items selected, automatically enable the background rotation option
  if(bgs.length() > 1 && !ui->radio_desk_multi->isChecked()){
    ui->radio_desk_multi->setChecked(true);
  }
  ui->push_save->setEnabled(true); //this is definitely a change
  moddesk = true;	
}

void MainUI::deskbgdirradded(){
  //Recursively add files from a directory
 QString dir = LOS::LuminaShare().section("/Lumina-DE",0,0)+"/wallpapers/Lumina-DE";
  qDebug() << "Looking for wallpaper dir:" << dir;
  if( !QFile::exists(dir) ){ dir = QDir::homePath(); }
  dir = QFileDialog::getExistingDirectory(this, tr("Find Background Image Directory"), dir, QFileDialog::ReadOnly);
  if(dir.isEmpty()){ return; }
  //Got a directory - go ahead and get all the valid image file formats
  QStringList imgs = LUtils::imageExtensions();
  for(int i=0; i<imgs.length(); i++){ imgs[i].prepend("*."); }
  //Now load the directory and add all the valid files
  QStringList dirs = LUtils::listSubDirectories(dir, true); //find/list all the dirs
  dirs.prepend(dir); //make sure the main dir is also listed
  QStringList bgs;
  for(int d=0; d<dirs.length(); d++){
    QDir qdir(dirs[d]);
    QStringList tmp = qdir.entryList(imgs, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for(int j=0; j<tmp.length(); j++){ bgs << qdir.absoluteFilePath(tmp[j]); }
  }
  //Now add all the files into the widget
  for(int i=0; i<bgs.length(); i++){
    ui->combo_desk_bg->addItem( bgs[i].section("/",-1), bgs[i] );
  }
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );
  //If multiple items selected, automatically enable the background rotation option
  if(bgs.length() > 1 && !ui->radio_desk_multi->isChecked()){
    ui->radio_desk_multi->setChecked(true);
  }
  ui->push_save->setEnabled(true); //this is definitely a change
  moddesk = true;		
}


void MainUI::deskplugadded(){
  GetPluginDialog dlg(this);
    dlg.LoadPlugins("desktop", PINFO);
    dlg.exec();
  if( !dlg.selected ){ return; } //cancelled
  QString newplug = dlg.plugID;
  QListWidgetItem *it = new QListWidgetItem();
  if(newplug=="applauncher"){
    //Prompt for the application to add
    XDGDesktop app = getSysApp();
    if(app.filePath.isEmpty()){ return; } //cancelled
    newplug.append("::"+app.filePath);
    //Now fill the item with the necessary info
    it->setWhatsThis(newplug);
    it->setText(app.name);
    it->setIcon(LXDG::findIcon(app.icon,"") );
    it->setToolTip(app.comment);
  }else{
    //Load the info for this plugin
    LPI info = PINFO->desktopPluginInfo(newplug);
    if( info.ID.isEmpty() ){ return; } //invalid plugin for some reason (should never happen)
    it->setWhatsThis(newplug);
    it->setText(info.name);
    it->setToolTip(info.description);
    it->setIcon( LXDG::findIcon(info.icon,"") );
  }
  ui->list_desktop_plugins->addItem(it);
  ui->list_desktop_plugins->scrollToItem(it);
  ui->push_save->setEnabled(true); 
  moddesk = true;
}

void MainUI::deskplugremoved(){
  QList<QListWidgetItem*> sel = ui->list_desktop_plugins->selectedItems();
  if(sel.isEmpty()){ return; } //nothing to do
  for(int i=0; i<sel.length(); i++){
    delete sel[i];
  }
  ui->push_save->setEnabled(true); 
  moddesk = true; 
}

//=============
//  PANELS PAGE
//=============
void MainUI::panelValChanged(){
  ui->tool_panels_add->setEnabled(panelnumber < 12);
  if(!loading){ ui->push_save->setEnabled(true); modpan = true; }
}

void MainUI::newPanel(){

  if(panelnumber<0){ panelnumber=0; } //just in case
  panelnumber++;
  //Now create a new Panel widget with this number
  PanelWidget *tmp = new PanelWidget(ui->scroll_panels->widget(), this, PINFO);
    tmp->LoadSettings(settings, currentDesktop(), panelnumber-1);
    PANELS << tmp;
    connect(tmp, SIGNAL(PanelChanged()), this, SLOT(panelValChanged()) );
    connect(tmp, SIGNAL(PanelRemoved(int)), this, SLOT(removePanel(int)) );
    static_cast<QBoxLayout*>(ui->scroll_panels->widget()->layout())->insertWidget(PANELS.length()-1, tmp);
     //update the widget first (2 necessary for scroll below to work)
    ui->scroll_panels->update();
    QApplication::processEvents();
    QApplication::processEvents();
    ui->scroll_panels->ensureWidgetVisible(tmp);
    panelValChanged();
}

void MainUI::removePanel(int pan){ 
  //connected to a signal from the panel widget
  bool changed = false;
  for(int i=0; i<PANELS.length(); i++){
    int num = PANELS[i]->PanelNumber();
    if(num==pan){
      delete PANELS.takeAt(i);
      i--;
      changed = true;
    }else if(num > pan){
      PANELS[i]->ChangePanelNumber(num-1);
      changed = true;
    }
  }
  if(!changed){ return; } //nothing done
  panelnumber--;
  panelValChanged();
}

void MainUI::loadPanels(){
  //First clean any current panels
  for(int i=0; i<PANELS.length(); i++){ delete PANELS.takeAt(i); i--; }
  //Now create new panels
  int dnum = currentDesktop();
  if(ui->scroll_panels->widget()->layout()==0){ 
    ui->scroll_panels->widget()->setLayout( new QHBoxLayout() ); 
    ui->scroll_panels->widget()->layout()->setContentsMargins(0,0,0,0);
  }
  ui->scroll_panels->widget()->layout()->setAlignment(Qt::AlignLeft);
  //Clear anything left over in the layout
  for(int i=0; i<ui->scroll_panels->widget()->layout()->count(); i++){
    delete ui->scroll_panels->widget()->layout()->takeAt(i);
  }
  for(int i=0; i<panelnumber; i++){
    PanelWidget *tmp = new PanelWidget(ui->scroll_panels->widget(), this, PINFO);
    tmp->LoadSettings(settings, dnum, i);
    PANELS << tmp;
    connect(tmp, SIGNAL(PanelChanged()), this, SLOT(panelValChanged()) );
    connect(tmp, SIGNAL(PanelRemoved(int)), this, SLOT(removePanel(int)) );
    ui->scroll_panels->widget()->layout()->addWidget(tmp);
  }
  static_cast<QHBoxLayout*>(ui->scroll_panels->widget()->layout())->addStretch();
}

void MainUI::savePanels(){
  for(int i=0; i<PANELS.length(); i++){
    PANELS[i]->SaveSettings(settings);
  }
}

//============
//    MENU PAGE
//============
void MainUI::addmenuplugin(){
  GetPluginDialog dlg(this);
	dlg.LoadPlugins("menu", PINFO);
	dlg.exec();
  if(!dlg.selected){ return; } //cancelled
  QString plug = dlg.plugID;
  //Now add the item to the list
  LPI info = PINFO->menuPluginInfo(plug);
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

void MainUI::checkmenuicons(){
  ui->tool_menu_up->setEnabled( ui->list_menu->currentRow() > 0 );
  ui->tool_menu_dn->setEnabled( ui->list_menu->currentRow() < (ui->list_menu->count()-1) );
  ui->tool_menu_rm->setEnabled( ui->list_menu->currentRow() >=0 );
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
	<< "Exec lumina-screenshot::::"+tr("Take Screenshot") \
	<< "Exec xscreensaver-command -lock::::"+tr("Lock Screen");
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
    QString exec = info[i].section(":",1,100);
    QString showexec = exec;
    if(showexec.startsWith("If {Matches")){ showexec = showexec.section("{",2,2).section("}",0,0); }
    if(showexec.startsWith("Exec ")){ showexec.replace("Exec ","Run "); }
    else{ showexec = showexec.section("(",0,0).section("{",0,0); } //built-in command - remove the extra commands on some of them
    QTreeWidgetItem *it = new QTreeWidgetItem();
      it->setText(0, showexec.simplified() );
      it->setWhatsThis(0, exec);
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

void MainUI::applyKeyBinding(){
  QKeySequence seq = ui->keyEdit_shortcut->keySequence();
  qDebug() << "New Key Sequence:" << seq.toString(QKeySequence::NativeText) << seq.toString(QKeySequence::PortableText);
  if(seq.isEmpty()){
    //Verify removal of the action first
	  
    //Now remove the action
    delete ui->tree_shortcut->currentItem();
  }else{
    QTreeWidgetItem *it = ui->tree_shortcut->currentItem();
     it->setText(1,seq.toString(QKeySequence::NativeText));
     it->setWhatsThis(1,dispToFluxKeys(seq.toString(QKeySequence::PortableText)));
     qDebug() << " - Flux Sequence:" << it->whatsThis(1);
  }
  ui->keyEdit_shortcut->clear();
  ui->push_save->setEnabled(true);
  modshort=true;
}

void MainUI::updateKeyConfig(){
  ui->group_shortcut_modify->setEnabled(ui->tree_shortcut->currentItem()!=0);
  ui->keyEdit_shortcut->clear();
}

//===========
// Defaults Page
//===========
void MainUI::changeDefaultBrowser(){
  //Prompt for the new app
  XDGDesktop desk = getSysApp(true);
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    if(desk.filePath=="reset"){
      desk.filePath="";
    }
  //save the new app setting and adjust the button appearance
  appsettings->setValue("default/webbrowser", desk.filePath);
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

void MainUI::changeDefaultEmail(){
  //Prompt for the new app
  XDGDesktop desk = getSysApp(true); //allow reset to default
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    if(desk.filePath=="reset"){
      desk.filePath="";
    }
  //save the new app setting and adjust the button appearance
  appsettings->setValue("default/email", desk.filePath);
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

void MainUI::changeDefaultFileManager(){
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

void MainUI::changeDefaultTerminal(){
  //Prompt for the new app
  XDGDesktop desk = getSysApp(true);
    if(desk.filePath.isEmpty()){ return; }//nothing selected
    if(desk.filePath=="reset"){
      desk.filePath="xterm";
    }
  //save the new app setting and adjust the button appearance
  sessionsettings->setValue("default-terminal", desk.filePath);
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

void MainUI::loadDefaultSettings(){
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
  tmp = sessionsettings->value("default-terminal", "xterm").toString();
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
  tmp = appsettings->value("default/email", "").toString();
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
    //Clear it in the back end
    LXDG::setDefaultAppForMime(list[i]->whatsThis(0), "");
    //Now clear it in the UI
    list[i]->setWhatsThis(1,""); //clear the app path
    list[i]->setIcon(1,QIcon()); //clear the icon
    list[i]->setText(1,""); //clear the name
  }
  //ui->push_save->setEnabled(true);
  //moddef = true;
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
    //Set it in the back end
    LXDG::setDefaultAppForMime(list[i]->whatsThis(0), desk.filePath);
    //Set it in the UI
    list[i]->setWhatsThis(1,desk.filePath); //app path
    list[i]->setIcon(1,LXDG::findIcon(desk.icon,"")); //reset the icon
    list[i]->setText(1,desk.name); //reset the name
  }
  //ui->push_save->setEnabled(true);
  //moddef = true;
}

void MainUI::setdefaultbinary(){
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
  //ui->push_save->setEnabled(true);
  //moddef = true;
}

void MainUI::checkdefaulticons(){
  QTreeWidgetItem *it = ui->tree_defaults->currentItem();
  ui->tool_defaults_set->setEnabled(it!=0);
  ui->tool_defaults_clear->setEnabled(it!=0);
  ui->tool_defaults_setbin->setEnabled(it!=0);
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
  STARTAPPS = LXDG::findAutoStartFiles(true); //also want invalid/disabled items
  //qDebug() << "StartApps:";
  ui->list_session_start->clear();
  for(int i=0; i<STARTAPPS.length(); i++){
  //qDebug() << STARTAPPS[i].filePath +" -> " +STARTAPPS[i].name << STARTAPPS[i].isHidden;
    if( !LXDG::checkValidity(STARTAPPS[i],false) || !QFile::exists(STARTAPPS[i].filePath) ){ continue; }
    QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(STARTAPPS[i].icon,"application-x-executable"), STARTAPPS[i].name );
	it->setWhatsThis(STARTAPPS[i].filePath); //keep the file location
        it->setToolTip(STARTAPPS[i].comment);
	if(STARTAPPS[i].isHidden){ it->setCheckState( Qt::Unchecked); }
	else{it->setCheckState( Qt::Checked); }
	ui->list_session_start->addItem(it);
  }
  

  //Now do the general session options
  ui->check_session_numlock->setChecked( sessionsettings->value("EnableNumlock", true).toBool() );
  ui->check_session_playloginaudio->setChecked( sessionsettings->value("PlayStartupAudio",true).toBool() );
  ui->check_session_playlogoutaudio->setChecked( sessionsettings->value("PlayLogoutAudio",true).toBool() );
  ui->push_session_setUserIcon->setIcon( LXDG::findIcon(QDir::homePath()+"/.loginIcon.png", "user-identity") );
  ui->line_session_time->setText( sessionsettings->value("TimeFormat","").toString() );
  ui->line_session_date->setText( sessionsettings->value("DateFormat","").toString() );
  index = ui->combo_session_datetimeorder->findData( sessionsettings->value("DateTimeOrder","timeonly").toString() );
  ui->combo_session_datetimeorder->setCurrentIndex(index);
  
  //Now do the localization settings
  val = sessionsettings->value("InitLocale/LANG", "").toString();
    index = ui->combo_locale_lang->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_lang->setCurrentIndex(index);
  val = sessionsettings->value("InitLocale/LC_MESSAGES", "").toString();
    index = ui->combo_locale_message->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_message->setCurrentIndex(index);
  val = sessionsettings->value("InitLocale/LC_TIME", "").toString();
    index = ui->combo_locale_time->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_time->setCurrentIndex(index);
      val = sessionsettings->value("InitLocale/NUMERIC", "").toString();
    index = ui->combo_locale_numeric->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_numeric->setCurrentIndex(index);
      val = sessionsettings->value("InitLocale/MONETARY", "").toString();
    index = ui->combo_locale_monetary->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_monetary->setCurrentIndex(index);
      val = sessionsettings->value("InitLocale/COLLATE", "").toString();
    index = ui->combo_locale_collate->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_collate->setCurrentIndex(index);
      val = sessionsettings->value("InitLocale/CTYPE", "").toString();
    index = ui->combo_locale_ctype->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_ctype->setCurrentIndex(index);
  
  //Now do the session theme options
  ui->combo_session_themefile->clear();
  ui->combo_session_colorfile->clear();
  ui->combo_session_icontheme->clear();
  QStringList current = LTHEME::currentSettings();
  // - local theme templates
  QStringList tmp = LTHEME::availableLocalThemes();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_themefile->addItem(tmp[i].section("::::",0,0)+" ("+tr("Local")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==current[0]){ ui->combo_session_themefile->setCurrentIndex(ui->combo_session_themefile->count()-1); }
  }
  // - system theme templates
  tmp = LTHEME::availableSystemThemes();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_themefile->addItem(tmp[i].section("::::",0,0)+" ("+tr("System")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==current[0]){ ui->combo_session_themefile->setCurrentIndex(ui->combo_session_themefile->count()-1); }
  }
  // - local color schemes
  tmp = LTHEME::availableLocalColors();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_colorfile->addItem(tmp[i].section("::::",0,0)+" ("+tr("Local")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==current[1]){ ui->combo_session_colorfile->setCurrentIndex(ui->combo_session_colorfile->count()-1); }
  }
  // - system color schemes
  tmp = LTHEME::availableSystemColors();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_colorfile->addItem(tmp[i].section("::::",0,0)+" ("+tr("System")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==current[1]){ ui->combo_session_colorfile->setCurrentIndex(ui->combo_session_colorfile->count()-1); }
  }
  // - icon themes
  tmp = LTHEME::availableSystemIcons();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_icontheme->addItem(tmp[i]);
    if(tmp[i]==current[2]){ ui->combo_session_icontheme->setCurrentIndex(i); }
  }
  // - Font
  ui->font_session_theme->setCurrentFont( QFont(current[3]) );
  // - Font Size
  ui->spin_session_fontsize->setValue( current[4].section("p",0,0).toInt() );
  
  int cur = ui->combo_session_cursortheme->findText( LTHEME::currentCursor() );
  if(cur>=0){ ui->combo_session_cursortheme->setCurrentIndex(cur); }
  
  //sessionstartchanged(); //make sure to update buttons
  sessionLoadTimeSample();
  sessionLoadDateSample();
  sessionCursorChanged();
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
  if(!ok){ qDebug() << "Warning: Could not save ~/.lumina/fluxbox-init"; }
  
  //Now do the start apps
  bool newstartapps = false;
  for(int i=0; i<ui->list_session_start->count(); i++){
    QString file = ui->list_session_start->item(i)->whatsThis();
    bool enabled = ui->list_session_start->item(i)->checkState()==Qt::Checked;
    bool found = false;
    for(int i=0; i<STARTAPPS.length(); i++){
      if(STARTAPPS[i].filePath==file){
        found = true;
	if(enabled != !STARTAPPS[i].isHidden){
	  //value is different
	  qDebug() << "Setting Autostart:" << enabled << STARTAPPS[i].filePath;
	  LXDG::setAutoStarted(enabled, STARTAPPS[i]);
	}
	break;
      }
    }
    if(!found && enabled){
      //New file/binary/app
      qDebug() << "Adding new AutoStart File:" << file;
      LXDG::setAutoStarted(enabled, file);
      newstartapps = true;
    }
  }

  

  if( !ui->push_session_setUserIcon->whatsThis().isEmpty()){
    QString filepath = ui->push_session_setUserIcon->whatsThis();
    if(filepath.isEmpty()){ filepath = QDir::homePath()+"/.loginIcon.png"; }
    if(filepath=="reset"){
      QFile::remove(QDir::homePath()+"/.loginIcon.png");
    }else{
      QPixmap pix(filepath);
      //Now scale it down if necessary
      if(pix.width() > 64 || pix.height()>64){
        pix = pix.scaled(64,64,Qt::KeepAspectRatio, Qt::SmoothTransformation);
      }
      //Now save that to the icon file (will automatically convert it to a PNG file format)
      pix.save(QDir::homePath()+"/.loginIcon.png");
    }
    ui->push_session_setUserIcon->setWhatsThis(""); //clear it for later
    //Now touch the settings file so that it re-loads the panel
    QProcess::startDetached("touch \""+settings->fileName()+"\"");
  }

  //Now do the general session options
  sessionsettings->setValue("EnableNumlock", ui->check_session_numlock->isChecked());
  sessionsettings->setValue("PlayStartupAudio", ui->check_session_playloginaudio->isChecked());
  sessionsettings->setValue("PlayLogoutAudio", ui->check_session_playlogoutaudio->isChecked());
  sessionsettings->setValue("TimeFormat", ui->line_session_time->text());
  sessionsettings->setValue("DateFormat", ui->line_session_date->text());
  sessionsettings->setValue("DateTimeOrder", ui->combo_session_datetimeorder->currentData().toString());
  
  //Now do the locale settings
  sessionsettings->setValue("InitLocale/LANG", ui->combo_locale_lang->currentData().toString() );
  sessionsettings->setValue("InitLocale/LC_MESSAGES", ui->combo_locale_message->currentData().toString() );
  sessionsettings->setValue("InitLocale/LC_TIME", ui->combo_locale_time->currentData().toString() );
  sessionsettings->setValue("InitLocale/LC_NUMERIC", ui->combo_locale_numeric->currentData().toString() );
  sessionsettings->setValue("InitLocale/LC_MONETARY", ui->combo_locale_monetary->currentData().toString() );
  sessionsettings->setValue("InitLocale/LC_COLLATE", ui->combo_locale_collate->currentData().toString() );
  sessionsettings->setValue("InitLocale/LC_CTYPE", ui->combo_locale_ctype->currentData().toString() );
  
  
  //Now do the theme options
  QString themefile = ui->combo_session_themefile->itemData( ui->combo_session_themefile->currentIndex() ).toString();
  QString colorfile = ui->combo_session_colorfile->itemData( ui->combo_session_colorfile->currentIndex() ).toString();
  QString iconset = ui->combo_session_icontheme->currentText();
  QString font = ui->font_session_theme->currentFont().family();
  QString fontsize = QString::number(ui->spin_session_fontsize->value())+"pt";
  //qDebug() << "Saving theme options:" << themefile << colorfile << iconset << font << fontsize;
  LTHEME::setCurrentSettings( themefile, colorfile, iconset, font, fontsize);
  LTHEME::setCursorTheme(ui->combo_session_cursortheme->currentText());
  if(newstartapps){ loadSessionSettings(); } //make sure to re-load the session settings to catch the new files
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
    it->setWhatsThis(desk.filePath);
    it->setToolTip(desk.comment);
    it->setCheckState(Qt::Checked);
  
  ui->list_session_start->addItem(it);
  ui->list_session_start->setCurrentItem(it);
  sessionoptchanged();
}

void MainUI::addsessionstartbin(){
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
  sessionoptchanged();
}

void MainUI::addsessionstartfile(){
  QString chkpath = QDir::homePath();
  QString bin = QFileDialog::getOpenFileName(this, tr("Select File"), chkpath, tr("All Files (*)") );
  if( bin.isEmpty() || !QFile::exists(bin) ){ return; } //cancelled
  QListWidgetItem *it = new QListWidgetItem( LXDG::findMimeIcon(bin), bin.section("/",-1) );
    it->setWhatsThis(bin); //file to be saved/run
    it->setToolTip(bin);
    it->setCheckState(Qt::Checked);
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

void MainUI::sessionthemechanged(){
  //Update the Fluxbox Theme preview
  QString previewfile = ui->combo_session_wtheme->itemData( ui->combo_session_wtheme->currentIndex() ).toString();
  previewfile.append( (previewfile.endsWith("/") ? "preview.jpg": "/preview.jpg") );
  if(QFile::exists(previewfile)){
    ui->label_session_wpreview->setPixmap(QPixmap(previewfile));
  }else{
    ui->label_session_wpreview->setText(tr("No Preview Available"));
  }
  sessionoptchanged();
}

void MainUI::sessionCursorChanged(){
  //Update the Cursor Theme preview
  QStringList info = LTHEME::cursorInformation(ui->combo_session_cursortheme->currentText());
  // - info format: [name, comment. sample file]
  qDebug() << "Cursor Information:" << ui->combo_session_cursortheme->currentText() << info;
  QPixmap img(info[2]);
  //qDebug() << "Image Data:" << img.isNull() << img.size();
  if(!img.isNull()){
    ui->label_cursor_sample->setPixmap( img.scaledToHeight(ui->label_cursor_sample->height(), Qt::SmoothTransformation) );
  }
  ui->label_cursor_sample->setToolTip(info[1]);
  ui->combo_session_cursortheme->setToolTip(info[1]);
  sessionoptchanged();
}

void MainUI::sessionEditColor(){
  //Get the current color file
  QString file = ui->combo_session_colorfile->itemData( ui->combo_session_colorfile->currentIndex() ).toString();
  //Open the color edit dialog
  ColorDialog dlg(this, PINFO, file);
  dlg.exec();
  //Check whether the file got saved/changed
  if(dlg.colorname.isEmpty() || dlg.colorpath.isEmpty() ){ return; } //cancelled
  //Reload the color list and activate the new color
  // - local color schemes
  ui->combo_session_colorfile->clear();
  QStringList tmp = LTHEME::availableLocalColors();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_colorfile->addItem(tmp[i].section("::::",0,0)+" ("+tr("Local")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==dlg.colorpath){ ui->combo_session_colorfile->setCurrentIndex(ui->combo_session_colorfile->count()-1); }
  }
  // - system color schemes
  tmp = LTHEME::availableSystemColors();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_colorfile->addItem(tmp[i].section("::::",0,0)+" ("+tr("System")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==dlg.colorpath){ ui->combo_session_colorfile->setCurrentIndex(ui->combo_session_colorfile->count()-1); }
  }
  
}

void MainUI::sessionEditTheme(){
  QString file = ui->combo_session_themefile->itemData( ui->combo_session_themefile->currentIndex() ).toString();
  //Open the theme editor dialog
  ThemeDialog dlg(this, PINFO, file);
  dlg.exec();
  //Check for file change/save
  if(dlg.themename.isEmpty() || dlg.themepath.isEmpty()){ return; } //cancelled
  //Reload the theme list and activate the new theme
  ui->combo_session_themefile->clear();
  // - local theme templates
  QStringList tmp = LTHEME::availableLocalThemes();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_themefile->addItem(tmp[i].section("::::",0,0)+" ("+tr("Local")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==dlg.themepath){ ui->combo_session_themefile->setCurrentIndex(ui->combo_session_themefile->count()-1); }
  }
  // - system theme templates
  tmp = LTHEME::availableSystemThemes();
  tmp.sort();
  for(int i=0; i<tmp.length(); i++){ 
    ui->combo_session_themefile->addItem(tmp[i].section("::::",0,0)+" ("+tr("System")+")", tmp[i].section("::::",1,1));
    if(tmp[i].section("::::",1,1)==dlg.themepath){ ui->combo_session_themefile->setCurrentIndex(ui->combo_session_themefile->count()-1); }
  }
}

void MainUI::sessionChangeUserIcon(){
  //Prompt for a new image file
  QStringList imgformats;
  QList<QByteArray> fmts = QImageReader::supportedImageFormats();
  for(int i=0; i<fmts.length(); i++){
    imgformats << "*."+QString(fmts[i]);
  }
  QString filepath = QFileDialog::getOpenFileName(this, tr("Select an image"), QDir::homePath(), \
				tr("Images")+" ("+imgformats.join(" ")+")");
  if(filepath.isEmpty()){
    //User cancelled the operation
    if(QFile::exists(QDir::homePath()+"/.loginIcon.png")){
      if(QMessageBox::Yes == QMessageBox::question(this,tr("Reset User Image"), tr("Would you like to reset the user image to the system default?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ){
	//QFile::remove(QDir::homePath()+"/.loginIcon.png");
	ui->push_session_setUserIcon->setWhatsThis("reset");
      }else{
	return;
      }
    }
  }else{
    ui->push_session_setUserIcon->setWhatsThis(filepath);	
  }
  //Now re-load the icon in the UI
  QString path = ui->push_session_setUserIcon->whatsThis();
  if(path.isEmpty()){ path = QDir::homePath()+"/.loginIcon.png"; }
  if(path=="reset"){ path.clear(); }
  ui->push_session_setUserIcon->setIcon( LXDG::findIcon(path, "user-identity") );
  sessionoptchanged();
}

void MainUI::sessionResetSys(){
  LUtils::LoadSystemDefaults();
  QTimer::singleShot(500,this, SLOT(loadCurrentSettings()) );
}

void MainUI::sessionResetLumina(){
  LUtils::LoadSystemDefaults(true); //skip OS customizations
  QTimer::singleShot(500,this, SLOT(loadCurrentSettings()) );	
}

void MainUI::sessionLoadTimeSample(){
  if(ui->line_session_time->text().simplified().isEmpty()){
    ui->label_session_timesample->setText( QTime::currentTime().toString(Qt::DefaultLocaleShortDate) );
  }else{
    ui->label_session_timesample->setText( QTime::currentTime().toString( ui->line_session_time->text() ) );
  }
  sessionoptchanged();
}

void MainUI::sessionShowTimeCodes(){
  QStringList msg;
    msg << tr("Valid Time Codes:") << "\n";
    msg << QString(tr("%1: Hour without leading zero (1)")).arg("h");
    msg << QString(tr("%1: Hour with leading zero (01)")).arg("hh");
    msg << QString(tr("%1: Minutes without leading zero (2)")).arg("m");
    msg << QString(tr("%1: Minutes with leading zero (02)")).arg("mm");
    msg << QString(tr("%1: Seconds without leading zero (3)")).arg("s");
    msg << QString(tr("%1: Seconds with leading zero (03)")).arg("ss");
    msg << QString(tr("%1: AM/PM (12-hour) clock (upper or lower case)")).arg("A or a");
    msg << QString(tr("%1: Timezone")).arg("t");
  QMessageBox::information(this, tr("Time Codes"), msg.join("\n") );
}

void MainUI::sessionLoadDateSample(){
  if(ui->line_session_date->text().simplified().isEmpty()){
    ui->label_session_datesample->setText( QDate::currentDate().toString(Qt::DefaultLocaleShortDate) );
  }else{
    ui->label_session_datesample->setText( QDate::currentDate().toString( ui->line_session_date->text() ) );
  }
  sessionoptchanged();
}

void MainUI::sessionShowDateCodes(){
  QStringList msg;
    msg << tr("Valid Date Codes:") << "\n";
    msg << QString(tr("%1: Numeric day without a leading zero (1)")).arg("d");
    msg << QString(tr("%1: Numeric day with leading zero (01)")).arg("dd");
    msg << QString(tr("%1: Day as abbreviation (localized)")).arg("ddd");
    msg << QString(tr("%1: Day as full name (localized)")).arg("dddd");
    msg << QString(tr("%1: Numeric month without leading zero (2)")).arg("M");
    msg << QString(tr("%1: Numeric month with leading zero (02)")).arg("MM");
    msg << QString(tr("%1: Month as abbreviation (localized)")).arg("MMM");
    msg << QString(tr("%1: Month as full name (localized)")).arg("MMMM");
    msg << QString(tr("%1: Year as 2-digit number (15)")).arg("yy");
    msg << QString(tr("%1: Year as 4-digit number (2015)")).arg("yyyy");
    msg << tr("Text may be contained within single-quotes to ignore replacements");
  QMessageBox::information(this, tr("Date Codes"), msg.join("\n") );
}

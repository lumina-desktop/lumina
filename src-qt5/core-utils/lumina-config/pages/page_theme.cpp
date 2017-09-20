//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_theme.h"
#include "ui_page_theme.h"

#include "../ColorDialog.h"
#include "../ThemeDialog.h"
//==========
//    PUBLIC
//==========
page_theme::page_theme(QWidget *parent) : PageWidget(parent), ui(new Ui::page_theme()){
  ui->setupUi(this);
  findQt5Themes();
  loading = false;
  PINFO = new LPlugins(); //load the info class
  connect(ui->spin_session_fontsize, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()) );
  connect(ui->combo_session_themefile, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()) );
  connect(ui->combo_session_colorfile, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()) );
  connect(ui->combo_session_icontheme, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()) );
  connect(ui->font_session_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()) );
  connect(ui->tool_session_newcolor, SIGNAL(clicked()), this, SLOT(sessionEditColor()) );
  connect(ui->tool_session_newtheme, SIGNAL(clicked()), this, SLOT(sessionEditTheme()) );
  connect(ui->combo_session_cursortheme, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()) );
  connect(ui->combo_qt5_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(checkQt5Theme()) );
  updateIcons();
}

page_theme::~page_theme(){

}

//================
//    PUBLIC SLOTS
//================
void page_theme::SaveSettings(){
  QString themefile = ui->combo_session_themefile->itemData( ui->combo_session_themefile->currentIndex() ).toString();
  QString colorfile = ui->combo_session_colorfile->itemData( ui->combo_session_colorfile->currentIndex() ).toString();
  QString iconset = ui->combo_session_icontheme->currentText();
  QString font = ui->font_session_theme->currentFont().family();
  QString fontsize = QString::number(ui->spin_session_fontsize->value())+"pt";
  QString qt5theme = ui->combo_qt5_theme->currentData().toString();
   if(qt5theme=="internal_custom"){ qt5theme = ui->line_qt5_custom_theme->text(); }
   QSettings sessionsettings("lumina-desktop","sessionsettings");
    sessionsettings.setValue("Qt5_theme_engine", qt5theme);

  //qDebug() << "Saving theme options:" << themefile << colorfile << iconset << font << fontsize;
  LTHEME::setCurrentSettings( themefile, colorfile, iconset, font, fontsize);
  LTHEME::setCursorTheme(ui->combo_session_cursortheme->currentText());
  emit HasPendingChanges(false);
}

void page_theme::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Theme Settings") );

  loading = true;
  //Load the available settings
  ui->combo_session_cursortheme->clear();
  ui->combo_session_cursortheme->addItems( LTHEME::availableSystemCursors() );

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

   QSettings sessionsettings("lumina-desktop","sessionsettings");
    QString qt5theme = sessionsettings.value("Qt5_theme_engine", "lthemeengine").toString();
  int index = ui->combo_qt5_theme->findData(qt5theme);
  if(index <0){
    ui->line_qt5_custom_theme->setText(qt5theme);
    index = ui->combo_qt5_theme->findData("internal_custom");
  }
  if(index>=0){ ui->combo_qt5_theme->setCurrentIndex(index); }
  checkQt5Theme(); //update item visibility
  QApplication::processEvents();
  loading = false;
}

void page_theme::updateIcons(){
  ui->tool_session_newtheme->setIcon( LXDG::findIcon("preferences-desktop-theme","") );
  ui->tool_session_newcolor->setIcon( LXDG::findIcon("preferences-desktop-color","") );
  ui->tabWidget->setTabIcon(0, LXDG::findIcon("user-desktop","desktop") );
  ui->tabWidget->setTabIcon(1, LXDG::findIcon("preferences-system-windows","") );
}

//=================
//       PRIVATE
//=================
void page_theme::findQt5Themes(){
  ui->combo_qt5_theme->clear();
  ui->combo_qt5_theme->addItem( tr("None"), "");
  ui->combo_qt5_theme->addItem( tr("Manual Setting"), "internal_custom");
  //Now probe the system and list any themes that are found
  QStringList paths = QCoreApplication::libraryPaths();
  qDebug() << "Known Library Paths:" << paths;
  QStringList engines;
  for(int i=0; i<paths.length(); i++){
    if(QFile::exists(paths[i]+"/platformthemes")){
      QDir dir(paths[i]+"/platformthemes");
      QStringList libs = dir.entryList(QStringList("lib*.so*"), QDir::Files, QDir::NoSort) ;
      for(int j=0; j<libs.length(); j++){ engines << libs[j].section("lib",1,-1).section(".",0,-2).simplified(); }
    }
  }
  engines.sort();
  for(int i=0; i<engines.length(); i++){
    if(i==0){ ui->combo_qt5_theme->insertSeparator(2); }
    ui->combo_qt5_theme->addItem( engines[i], engines[i] );
  }
}
//=================
//    PRIVATE SLOTS
//=================
void page_theme::sessionEditColor(){
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
  emit HasPendingChanges(true);
}

void page_theme::sessionEditTheme(){
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
  emit HasPendingChanges(true);
}

void page_theme::checkQt5Theme(){
  ui->line_qt5_custom_theme->setVisible( ui->combo_qt5_theme->currentData().toString()=="internal_custom");
  settingsChanged();
}

//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_session_options.h"
#include "ui_page_session_options.h"

//==========
//    PUBLIC
//==========
page_session_options::page_session_options(QWidget *parent) : PageWidget(parent), ui(new Ui::page_session_options()){
  ui->setupUi(this);
  loading = false;
  //Display formats for panel clock
  ui->combo_session_datetimeorder->clear();
  ui->combo_session_datetimeorder->addItem( tr("Time (Date as tooltip)"), "timeonly");
  ui->combo_session_datetimeorder->addItem( tr("Date (Time as tooltip)"), "dateonly");
  ui->combo_session_datetimeorder->addItem( tr("Time first then Date"), "timedate");
  ui->combo_session_datetimeorder->addItem( tr("Date first then Time"), "datetime");

  connect(ui->push_session_setUserIcon, SIGNAL(clicked()), this, SLOT(sessionChangeUserIcon()) );
  connect(ui->push_session_resetSysDefaults, SIGNAL(clicked()), this, SLOT(sessionResetSys()) );
  connect(ui->push_session_resetLuminaDefaults, SIGNAL(clicked()), this, SLOT(sessionResetLumina()) );
  connect(ui->tool_help_time, SIGNAL(clicked()), this, SLOT(sessionShowTimeCodes()) );
  connect(ui->tool_help_date, SIGNAL(clicked()), this, SLOT(sessionShowDateCodes()) );
  connect(ui->line_session_time, SIGNAL(textChanged(QString)), this, SLOT(sessionLoadTimeSample()) );
  connect(ui->line_session_date, SIGNAL(textChanged(QString)), this, SLOT(sessionLoadDateSample()) );
  connect(ui->combo_session_datetimeorder, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->check_session_numlock, SIGNAL(toggled(bool)), this, SLOT(settingChanged()) );
  connect(ui->check_session_playloginaudio, SIGNAL(toggled(bool)), this, SLOT(settingChanged()) );
  connect(ui->check_session_playlogoutaudio, SIGNAL(toggled(bool)), this, SLOT(settingChanged()) );
  connect(ui->check_autoapplinks, SIGNAL(toggled(bool)), this, SLOT(settingChanged()) );
  connect(ui->check_watch_app_procs, SIGNAL(toggled(bool)), this, SLOT(settingChanged()) );
  connect(ui->check_quotes, SIGNAL(toggled(bool)), this, SLOT(settingChanged()) );
  connect(ui->mywindowmanager, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()));
  updateIcons();
}

page_session_options::~page_session_options(){

}

//================
//    PUBLIC SLOTS
//================
void page_session_options::SaveSettings(){
  QSettings sessionsettings("lumina-desktop","sessionsettings");
  sessionsettings.setValue("AutomaticDesktopAppLinks",  ui->check_autoapplinks->isChecked());
  sessionsettings.setValue("EnableNumlock", ui->check_session_numlock->isChecked());
  sessionsettings.setValue("PlayStartupAudio", ui->check_session_playloginaudio->isChecked());
  sessionsettings.setValue("PlayLogoutAudio", ui->check_session_playlogoutaudio->isChecked());
  sessionsettings.setValue("TimeFormat", ui->line_session_time->text());
  sessionsettings.setValue("DateFormat", ui->line_session_date->text());
  sessionsettings.setValue("DateTimeOrder", ui->combo_session_datetimeorder->currentData().toString());
  sessionsettings.setValue("DisableQuotes", ui->check_quotes->isChecked());

  QString my_win = ui->mywindowmanager->currentData().toString();
  // Warn user if they select a non-default window manager
  if (! my_win.isEmpty() )
      QMessageBox::information(this, tr("Window manager"), "Warning: Please note window managers other than Lumina are not supported." );
  // If we selected "Lumina" as the window manager, leave the field blank to get default
  if ( my_win.isEmpty() )
     sessionsettings.remove("WindowManager");
  else
     sessionsettings.setValue("WindowManager", my_win);



  QString lopenWatchFile = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/nowatch";
  if(QFile::exists(lopenWatchFile) && ui->check_watch_app_procs->isChecked()){
    QFile::remove(lopenWatchFile);
  }else if(!QFile::exists(lopenWatchFile) && !ui->check_watch_app_procs->isChecked()){
    QFile file(lopenWatchFile);
    if(file.open(QIODevice::WriteOnly) ){ file.close(); } //just need to touch it to create the file
  }

  //User Icon
  QString path = QDir::homePath()+"/.loginIcon.png"; //where the icon should be placed
  QString icopath = ui->push_session_setUserIcon->whatsThis();
  qDebug() << "User Icon:" << icopath;
  if(icopath != path && QFile::exists(icopath)){
    QPixmap ico(icopath);
    ico = ico.scaled(64,64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    bool ok = ico.save(path);
    qDebug() << "  - Saved User Icon:" << ok;
  }

  emit HasPendingChanges(false);
}

void page_session_options::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Desktop Settings") );
  loading = true;
  QSettings sessionsettings("lumina-desktop","sessionsettings");
  ui->check_session_numlock->setChecked( sessionsettings.value("EnableNumlock", true).toBool() );
  ui->check_session_playloginaudio->setChecked( sessionsettings.value("PlayStartupAudio",true).toBool() );
  ui->check_session_playlogoutaudio->setChecked( sessionsettings.value("PlayLogoutAudio",true).toBool() );
  ui->check_autoapplinks->setChecked( sessionsettings.value("AutomaticDesktopAppLinks",true).toBool() );
  ui->check_quotes->setChecked( sessionsettings.value("DisableQuotes",true).toBool() );
  ui->push_session_setUserIcon->setIcon( LXDG::findIcon(QDir::homePath()+"/.loginIcon.png", "user-identity") );
  ui->line_session_time->setText( sessionsettings.value("TimeFormat","").toString() );
  ui->line_session_date->setText( sessionsettings.value("DateFormat","").toString() );
  int index = ui->combo_session_datetimeorder->findData( sessionsettings.value("DateTimeOrder","timeonly").toString() );
  ui->combo_session_datetimeorder->setCurrentIndex(index);

  FindWindowManagerOptions();  // check system for available options
  QString old_wm = sessionsettings.value("WindowManager").toString();

  // Check to see if old window manager is in our list and, if not, add it
  if ( old_wm.length() > 0 )
  {
      index = ui->mywindowmanager->findData( old_wm );
      if (index == -1)  // did not find existing option in list, so add it
      {
           ui->mywindowmanager->addItem( old_wm, old_wm);
          // Past window manager is now in list so we can select it, even if it did not exist before
          index = ui->mywindowmanager->findData( old_wm );
      }
  }
  else   // there was no "old" window manager, default to using Lumina/default
     index = 0;

  ui->mywindowmanager->setCurrentIndex(index);

  QString lopenWatchFile = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/nowatch";
  ui->check_watch_app_procs->setChecked( !QFile::exists(lopenWatchFile) );

  sessionLoadTimeSample();
  sessionLoadDateSample();
  QApplication::processEvents(); //throw away any interaction events from loading
  loading = false;
}

void page_session_options::updateIcons(){
  ui->push_session_resetSysDefaults->setIcon( LXDG::findIcon("start-here-lumina","view-refresh") );
  ui->push_session_resetLuminaDefaults->setIcon( LXDG::findIcon("Lumina-DE","") );
  ui->tool_help_time->setIcon( LXDG::findIcon("help-about","") );
  ui->tool_help_date->setIcon( LXDG::findIcon("help-about","") );
}

//=================
//         PRIVATE
//=================

void page_session_options::FindWindowManagerOptions(){
    // Try to find all available window managers and add them to drop-down box.
    ui->mywindowmanager->clear();
    ui->mywindowmanager->addItem("Default (fluxbox)"); // make sure there is a default at position 0
    QStringList wms; wms << "kwin" << "openbox" << "i3";
    //NOTE: the "fluxbox" window manager is already assumed by the selection of the "Lumina" default
    wms.sort();
    for(int i=0; i<wms.length(); i++){
      QString path = wms[i];
      if(LUtils::isValidBinary(path)){ //This will change the "path" variable to the full path if it exists
        ui->mywindowmanager->addItem(wms[i], path);
      }
    }
    /*if (QFileInfo::exists("/usr/bin/fluxbox"))
        ui->mywindowmanager->addItem("/usr/bin/fluxbox");
    else if (QFileInfo::exists("/usr/local/bin/fluxbox"))
        ui->mywindowmanager->addItem("/usr/local/bin/fluxbox");
    if (QFileInfo::exists("/usr/bin/kwin"))
        ui->mywindowmanager->addItem("/usr/bin/kwin");
    else if (QFileInfo::exists("/usr/local/bin/kwin"))
        ui->mywindowmanager->addItem("/usr/local/bin/kwin");
    if (QFileInfo::exists("/usr/bin/openbox"))
        ui->mywindowmanager->addItem("/usr/bin/openbox");
    else if (QFileInfo::exists("/usr/local/bin/openbox"))
        ui->mywindowmanager->addItem("/usr/local/bin/openbox");*/
}

bool page_session_options::verifySettingsReset(){
  bool ok =(QMessageBox::Yes ==  QMessageBox::warning(this, tr("Verify Settings Reset"), tr("Are you sure you want to reset your desktop settings? This change cannot be reversed!"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) );
  return ok;
}

//=================
//    PRIVATE SLOTS
//=================
void page_session_options::sessionChangeUserIcon(){
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
  settingChanged();
}

void page_session_options::sessionResetSys(){
  if( !verifySettingsReset() ){ return; } //cancelled
  LDesktopUtils::LoadSystemDefaults();
  QTimer::singleShot(500,this, SLOT(LoadSettings()) );
}

void page_session_options::sessionResetLumina(){
  if( !verifySettingsReset() ){ return; } //cancelled
  LDesktopUtils::LoadSystemDefaults(true); //skip OS customizations
  QTimer::singleShot(500,this, SLOT(LoadSettings()) );
}

void page_session_options::sessionLoadTimeSample(){
  if(ui->line_session_time->text().simplified().isEmpty()){
    ui->label_session_timesample->setText( QTime::currentTime().toString(Qt::DefaultLocaleShortDate) );
  }else{
    ui->label_session_timesample->setText( QTime::currentTime().toString( ui->line_session_time->text() ) );
  }
  settingChanged();
}

void page_session_options::sessionShowTimeCodes(){
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

void page_session_options::sessionLoadDateSample(){
  if(ui->line_session_date->text().simplified().isEmpty()){
    ui->label_session_datesample->setText( QDate::currentDate().toString(Qt::DefaultLocaleShortDate) );
  }else{
    ui->label_session_datesample->setText( QDate::currentDate().toString( ui->line_session_date->text() ) );
  }
  settingChanged();
}

void page_session_options::sessionShowDateCodes(){
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

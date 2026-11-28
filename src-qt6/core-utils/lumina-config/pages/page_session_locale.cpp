//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_session_locale.h"
#include "ui_page_session_locale.h"

//==========
//    PUBLIC
//==========
page_session_locale::page_session_locale(QWidget *parent) : PageWidget(parent), ui(new Ui::page_session_locale()){
  ui->setupUi(this);
  setupLocales();
  connect(ui->combo_locale_lang, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_locale_collate, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_locale_ctype, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_locale_message, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_locale_monetary, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_locale_numeric, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_locale_time, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
 updateIcons();
}

page_session_locale::~page_session_locale(){

}

//================
//    PUBLIC SLOTS
//================
void page_session_locale::SaveSettings(){
  QSettings sessionsettings("lumina-desktop","sessionsettings");
  sessionsettings.setValue("InitLocale/LANG", ui->combo_locale_lang->currentData().toString() );
  sessionsettings.setValue("InitLocale/LC_MESSAGES", ui->combo_locale_message->currentData().toString() );
  sessionsettings.setValue("InitLocale/LC_TIME", ui->combo_locale_time->currentData().toString() );
  sessionsettings.setValue("InitLocale/LC_NUMERIC", ui->combo_locale_numeric->currentData().toString() );
  sessionsettings.setValue("InitLocale/LC_MONETARY", ui->combo_locale_monetary->currentData().toString() );
  sessionsettings.setValue("InitLocale/LC_COLLATE", ui->combo_locale_collate->currentData().toString() );
  sessionsettings.setValue("InitLocale/LC_CTYPE", ui->combo_locale_ctype->currentData().toString() );
  emit HasPendingChanges(false);
}

//Index of a saved locale in one of the locale combo boxes. The lists are built from Lumina's own translation
// files, so a locale set elsewhere (or one Lumina has no translation for) was not listed: it showed as "System
// Default" and was then cleared by the next save. Add such a value as its own entry instead.
static int localeIndex(QComboBox *combo, const QString &val){
  int index = combo->findData(val);
  if(index<0 && !val.isEmpty()){
    QString name = QLocale(val).nativeLanguageName();
    combo->addItem( name.isEmpty() ? val : name+" ("+val+")", val);
    index = combo->count()-1;
  }
  if(index<0){ index = 0; } //system default
  return index;
}

void page_session_locale::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Desktop Settings") );
  QSettings sessionsettings("lumina-desktop","sessionsettings");

  QString val = sessionsettings.value("InitLocale/LANG", "").toString();
    int index = localeIndex(ui->combo_locale_lang, val);
    ui->combo_locale_lang->setCurrentIndex(index);
  val = sessionsettings.value("InitLocale/LC_MESSAGES", "").toString();
    index = localeIndex(ui->combo_locale_message, val);
    ui->combo_locale_message->setCurrentIndex(index);
  val = sessionsettings.value("InitLocale/LC_TIME", "").toString();
    index = localeIndex(ui->combo_locale_time, val);
    ui->combo_locale_time->setCurrentIndex(index);
      val = sessionsettings.value("InitLocale/LC_NUMERIC", "").toString();
    index = localeIndex(ui->combo_locale_numeric, val);
    ui->combo_locale_numeric->setCurrentIndex(index);
      val = sessionsettings.value("InitLocale/LC_MONETARY", "").toString();
    index = localeIndex(ui->combo_locale_monetary, val);
    ui->combo_locale_monetary->setCurrentIndex(index);
      val = sessionsettings.value("InitLocale/LC_COLLATE", "").toString();
    index = localeIndex(ui->combo_locale_collate, val);
    ui->combo_locale_collate->setCurrentIndex(index);
      val = sessionsettings.value("InitLocale/LC_CTYPE", "").toString();
    index = localeIndex(ui->combo_locale_ctype, val);
    ui->combo_locale_ctype->setCurrentIndex(index);
}

void page_session_locale::updateIcons(){

}

//=================
//         PRIVATE 
//=================
void page_session_locale::setupLocales(){
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
//=================
//    PRIVATE SLOTS
//=================

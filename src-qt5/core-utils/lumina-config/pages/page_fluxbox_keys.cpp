//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_fluxbox_keys.h"
#include "ui_page_fluxbox_keys.h"

//==========
//    PUBLIC
//==========
page_fluxbox_keys::page_fluxbox_keys(QWidget *parent) : PageWidget(parent), ui(new Ui::page_fluxbox_keys()){
  ui->setupUi(this);
  loading = false;
  ui->radio_simple->setChecked(true);
  ui->radio_advanced->setChecked(false);
  connect(ui->tool_shortcut_clear, SIGNAL(clicked()), this, SLOT(clearKeyBinding()) );
  connect(ui->tool_shortcut_set, SIGNAL(clicked()), this, SLOT(applyKeyBinding()) );
  connect(ui->tree_shortcut, SIGNAL(itemSelectionChanged()), this, SLOT(updateKeyConfig()) );
  connect(ui->text_file, SIGNAL(textChanged()), this, SLOT(settingChanged()) );
  connect(ui->radio_simple, SIGNAL(toggled(bool)), this, SLOT(switchEditor()) );
  connect(ui->group_keys, SIGNAL(toggled(bool)), this, SLOT(togglesyntaxgroup()) );
  updateIcons();
}

page_fluxbox_keys::~page_fluxbox_keys(){

}

//================
//    PUBLIC SLOTS
//================
void page_fluxbox_keys::SaveSettings(){
  QStringList info;
  if(ui->radio_simple->isChecked()){
    //Basic Editor
    QStringList current;
    for(int i=0; i<ui->tree_shortcut->topLevelItemCount(); i++){
      QTreeWidgetItem *it = ui->tree_shortcut->topLevelItem(i);
      current << it->whatsThis(1)+" :"+it->whatsThis(0); //Full Fluxbox command line
    }

    info = readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-keys");
    for(int i=0; i<info.length(); i++){
      if(info[i].isEmpty() || info[i].startsWith("#") || info[i].startsWith("!")){ continue; }
      if(current.filter(info[i].section(":",1,-1)).length() > 0){
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
  }else{
    //advanced editor
    info = ui->text_file->toPlainText().split("\n");
  }
  bool ok = overwriteFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-keys", info);
  if(!ok){ qDebug() << "Warning: Could not save fluxbox-keys"; }
  emit HasPendingChanges(false);
}

void page_fluxbox_keys::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Keyboard Shortcuts") );
  loading = true;
  QStringList info = readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-keys");
  if(ui->radio_simple->isChecked()){
    ui->stackedWidget->setCurrentWidget(ui->page_simple); //ensure the proper page is visible
    //First take care of the special Lumina options
    ui->tree_shortcut->clear();
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
  }else{
    //Advanced Editor
    ui->stackedWidget->setCurrentWidget(ui->page_advanced);
    ui->text_file->setPlainText( info.join("\n") );
  }
  QApplication::processEvents();
  loading = false;
}

void page_fluxbox_keys::updateIcons(){
  ui->tool_shortcut_set->setIcon( LXDG::findIcon("input-keyboard","") );
  ui->tool_shortcut_clear->setIcon( LXDG::findIcon("edit-clear","") );
}

//=================
//   PRIVATE
//=================
//Convert to/from fluxbox key codes
QString page_fluxbox_keys::dispToFluxKeys(QString in){
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

QString page_fluxbox_keys::fluxToDispKeys(QString in){
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
QStringList page_fluxbox_keys::readFile(QString path){
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

bool page_fluxbox_keys::overwriteFile(QString path, QStringList contents){
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

//=================
//    PRIVATE SLOTS
//=================
void page_fluxbox_keys::clearKeyBinding(){
  if(ui->tree_shortcut->currentItem()==0){ return; }
  ui->tree_shortcut->currentItem()->setText(1,"");
  ui->tree_shortcut->currentItem()->setWhatsThis(1,"");
  settingChanged();
}

void page_fluxbox_keys::applyKeyBinding(){
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
  settingChanged();
}

void page_fluxbox_keys::updateKeyConfig(){
  ui->group_shortcut_modify->setEnabled(ui->tree_shortcut->currentItem()!=0);
  ui->keyEdit_shortcut->clear();
}

void page_fluxbox_keys::switchEditor(){
  LoadSettings(-1);
}

void page_fluxbox_keys::togglesyntaxgroup(){
  ui->plainTextEdit->setVisible(ui->group_keys->isChecked());
}

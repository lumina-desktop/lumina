//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LFileDialog.h"
#include "ui_LFileDialog.h"

LFileDialog::LFileDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LFileDialog() ){
  ui->setupUi(this); //load the designer file
  //set the output flags to the defaults
  appSelected = false;
  setDefault = false;
  appExec.clear();
  appPath.clear();
  appFile.clear();
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  settings = new QSettings("LuminaDE", "lumina-open",this);
  //Connect the signals/slots
  connect(ui->tree_apps, SIGNAL(itemSelectionChanged()), this, SLOT(updateUI()) );
  connect(ui->radio_rec, SIGNAL(toggled(bool)), this, SLOT(radioChanged()) );
  connect(ui->radio_avail, SIGNAL(toggled(bool)), this, SLOT(radioChanged()) );
  connect(ui->radio_custom, SIGNAL(toggled(bool)), this, SLOT(radioChanged()) );
}

LFileDialog::~LFileDialog(){
	
}

// ----------
//   PUBLIC
// ----------
void LFileDialog::setFileInfo(QString filename, QString extension, bool isFile){
  //Set the labels for the file
  ui->label_file->setText( this->fontMetrics().elidedText( filename, Qt::ElideMiddle, 300 ) );
  if(isFile){ ui->label_extension->setText( "("+extension+")"); }
  else{ ui->label_extension->setText("("+extension+" link)"); }
  fileEXT = extension;
  generateAppList();
}

//static functions
QString LFileDialog::getDefaultApp(QString extension){
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
    return QSettings("LuminaDE", "lumina-open").value("default/"+extension,"").toString();
}

void LFileDialog::setDefaultApp(QString extension, QString appFile){
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  if(appFile.isEmpty()){
    QSettings("LuminaDE", "lumina-open").remove("default/"+extension);
  }else{
    QSettings("LuminaDE", "lumina-open").setValue("default/"+extension,appFile);
  }
}

// -----------
//   PRIVATE
// -----------
QStringList LFileDialog::getPreferredApplications(){
  //Now search the settings for that extension
  QStringList keys = settings->allKeys();
  QStringList out;
  for(int i=0; i<keys.length(); i++){
    if(keys[i].startsWith("default/")){ continue; } //ignore the defaults (they will also be in the main)
    if(keys[i].toLower() == fileEXT.toLower()){
      QStringList files = settings->value(keys[i]).toString().split(":::");
      qDebug() << "Found Files:" << keys[i] << files;
      bool cleaned = false;
      for(int j=0; j<files.length(); j++){
        if(QFile::exists(files[j])){ out << files[j]; }
        else{ files.removeAt(j); j--; cleaned=true; } //file no longer available - remove it
      }
      if(cleaned){ settings->setValue(keys[i], files.join(":::")); } //update the registry
      if(!out.isEmpty()){ break; } //already found files
    }
  }
  return out;
}

void LFileDialog::setPreferredApplication(QString desktopfile){
  QStringList keys = settings->allKeys();
  for(int i=0; i<keys.length(); i++){
    if(keys[i].toLower() == fileEXT.toLower()){
      QStringList files = settings->value(keys[i]).toString().split(":::");
      files.removeAll(desktopfile);
      QStringList newfiles;
      newfiles << desktopfile;
      //Only keep the 5 most recent preferred applications per extension
      for(int j=0; j<5 && j<files.length(); j++){
      	  newfiles << files[j];	
      }   
      settings->setValue(keys[i], newfiles.join(":::"));
      return;
    }
  }
  //No key found for this extension - make a new one
  settings->setValue(fileEXT.toLower(), desktopfile);
}

QString LFileDialog::translateCat(QString cat){
  QString out;
  if(cat=="Audio"){ out = tr("Audio"); }
  else if(cat=="Video"){ out = tr("Video"); }
  else if(cat=="Multimedia"){ out = tr("Multimedia"); }
  else if(cat=="Development"){ out = tr("Development"); }
  else if(cat=="Education"){ out = tr("Education"); }
  else if(cat=="Game"){ out = tr("Game"); }
  else if(cat=="Graphics"){ out = tr("Graphics"); }
  else if(cat=="Network"){ out = tr("Network"); }
  else if(cat=="Office"){ out = tr("Office"); }
  else if(cat=="Science"){ out = tr("Science"); }
  else if(cat=="Settings"){ out = tr("Settings"); }
  else if(cat=="System"){ out = tr("System"); }
  else if(cat=="Utility"){ out = tr("Utilities"); }
  else if(cat=="Other"){ out = tr("Other"); }
  else{ out = cat; }
  return out;
}

// -----------------
//   PRIVATE SLOTS
// -----------------
void LFileDialog::updateUI(){
  //Check for a selected application
  bool good = false;
  if(ui->radio_custom->isChecked()){
    if(!ui->line_bin->text().isEmpty()){
      good = true; 
      //Now verify that the file exists and is executable
      QFileInfo FI(ui->line_bin->text());
      if( FI.exists() && FI.isExecutable() && FI.isFile() ){
        ui->label_goodbin->setPixmap(QPixmap(":/icons/good.png"));
      }else{
        ui->label_goodbin->setPixmap(QPixmap(":/icons/bad.png"));
      }
    }
  }
  else if(ui->radio_rec->isChecked()){
    good = true; //a valid app is always selected on this page if it is available
  }
  else if(ui->tree_apps->topLevelItemCount() > 0 && ui->tree_apps->currentItem()!=0 ){
    if(!ui->tree_apps->currentItem()->whatsThis(0).isEmpty()){ 
      good=true;
    }
  }
  ui->tool_ok->setEnabled(good);
}

void LFileDialog::generateAppList(){
  //Now load the preferred applications
  PREFAPPS = getPreferredApplications();
  ui->combo_rec->clear();
  //Now get the application mimetype for the file extension (if available)
  QString mimetype = LXDG::findAppMimeForFile(fileEXT);
  //Now add all the detected applications
  QHash< QString, QList<XDGDesktop> > hash = LXDG::sortDesktopCats( LXDG::systemDesktopFiles() );
  QStringList cat = hash.keys();
  cat.sort(); //sort alphabetically
  ui->tree_apps->clear();
  for(int c=0; c<cat.length(); c++){
    QList<XDGDesktop> app = hash[cat[c]];
    QTreeWidgetItem *ci = new QTreeWidgetItem(ui->tree_apps, QStringList() << translateCat(cat[c]));
    for(int a=0; a<app.length(); a++){
      QTreeWidgetItem *ti = new QTreeWidgetItem(ci, QStringList() << app[a].name);
        ti->setWhatsThis(0, app[a].filePath);
        ti->setIcon(0, LXDG::findIcon(app[a].icon, "application-x-desktop"));
        ti->setToolTip(0, app[a].comment);
      ci->addChild(ti);
      //Check to see if this app matches the mime type
      if(app[a].mimeList.contains(mimetype) && !mimetype.isEmpty()){
        // also put this app in the preferred list
	PREFAPPS.append(app[a].filePath);
      }
    }
    ui->tree_apps->addTopLevelItem(ci);
  }
  //Now add all the preferred applications
  PREFAPPS.removeDuplicates();
  for(int i=0; i<PREFAPPS.length(); i++){
    bool ok = false;
    XDGDesktop dFile = LXDG::loadDesktopFile(PREFAPPS[i], ok);
    if( LXDG::checkValidity(dFile) && ok ){
      ui->combo_rec->addItem( LXDG::findIcon(dFile.icon, "application-x-desktop"), dFile.name);
      if(i==0){ ui->combo_rec->setCurrentIndex(0); } //make sure the first item is selected
    }else{
      PREFAPPS.removeAt(i); //invalid app
      i--;
    }
  }
  //Update the UI
  if(PREFAPPS.isEmpty()){
    ui->radio_rec->setEnabled(false); //no preferred apps
    ui->radio_avail->setChecked(true);
  }else{
    ui->radio_rec->setChecked(true);
  }
}

void LFileDialog::radioChanged(){
  if(ui->radio_rec->isChecked()){
    ui->stackedWidget->setCurrentWidget(ui->page_rec);
  }else if(ui->radio_avail->isChecked()){
    ui->stackedWidget->setCurrentWidget(ui->page_avail);
  }else{
    ui->stackedWidget->setCurrentWidget(ui->page_custom);
  }
  ui->check_default->setEnabled( !ui->radio_custom->isChecked() );
  updateUI();
}

/*void LFileDialog::on_group_binary_toggled(bool checked){
  ui->label_goodbin->setVisible(checked);
  ui->line_bin->setVisible(checked);
  ui->tool_findBin->setVisible(checked);
  updateUI();
}*/

void LFileDialog::on_tool_ok_clicked(){
  appSelected = true;
  if(ui->radio_custom->isChecked()){
    appExec = ui->line_bin->text();  
  }else if(ui->radio_rec->isChecked()){
    //application selected
    bool ok = false;
    XDGDesktop app = LXDG::loadDesktopFile(PREFAPPS[ui->combo_rec->currentIndex()], ok);
    //Set the output variables
    setDefault = ui->check_default->isChecked();
    appExec = app.exec;
    appPath = app.path;
    appFile = app.filePath;
    setPreferredApplication(app.filePath); //bump this to the top of the preferred list for next time
  }else{
    //application selected
    bool ok = false;
    XDGDesktop app = LXDG::loadDesktopFile(ui->tree_apps->currentItem()->whatsThis(0), ok);
    //Set the output variables
    setDefault = ui->check_default->isChecked();
    appExec = app.exec;
    appPath = app.path;
    appFile = app.filePath;
    setPreferredApplication(app.filePath); //save this app to this extension as a recommendation
  }
  this->close();
}

void LFileDialog::on_tool_cancel_clicked(){
  appSelected = false;
  this->close();
}

void LFileDialog::on_tool_findBin_clicked(){
  QString filepath = QFileDialog::getOpenFileName(this, tr("Find Application Binary"), QDir::homePath() );
  if(filepath.isEmpty()){ return; }
  ui->line_bin->setText(filepath);
}

void LFileDialog::on_line_bin_textChanged(){
  updateUI();
}


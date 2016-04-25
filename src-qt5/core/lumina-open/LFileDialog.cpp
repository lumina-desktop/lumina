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
  connect(ui->combo_apps, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()) );
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
  qDebug() << "SetFileInfo:" << filename << extension << isFile;
  ui->label_file->setText( this->fontMetrics().elidedText( filename, Qt::ElideMiddle, 300 ) );
  bool shownetwork = false;
  if(isFile){ ui->label_extension->setText( "("+extension+")"); }
  else if(extension=="email"){ ui->label_extension->setText( QString(tr("(Email Link)")) ); shownetwork = true; }
  else if(extension.startsWith("x-scheme-handler/")){  ui->label_extension->setText( QString(tr("(Internet URL - %1)")).arg(extension.section("/",-1)) ); shownetwork = true; }
  else{ui->label_extension->setText("("+extension+" link)"); }
  fileEXT = extension; //NOTE: this is the mime-type for the file now, not the extension
  generateAppList(shownetwork);
}

//static functions
QString LFileDialog::getDefaultApp(QString extension){
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  if(extension.contains("/")){
    return LXDG::findDefaultAppForMime(extension);
  }else{
    return QSettings("LuminaDE", "lumina-open").value("default/"+extension,"").toString();
  }
}

void LFileDialog::setDefaultApp(QString extension, QString appFile){
  if(extension.contains("/")){
    //mime type default: set on the system itself
    if(appFile.endsWith(".desktop")){ appFile = appFile.section("/",-1); } //only need the relative path
    LXDG::setDefaultAppForMime(extension, appFile);
  }else{
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
    if(appFile.isEmpty()){
      QSettings("LuminaDE", "lumina-open").remove("default/"+extension);
    }else{
      QSettings("LuminaDE", "lumina-open").setValue("default/"+extension,appFile);
    }
    }
}

// -----------
//   PRIVATE
// -----------
QStringList LFileDialog::getPreferredApplications(){
  QStringList out;
  //First list all the applications registered for that same mimetype
  QString mime = fileEXT;
  out << LXDG::findAvailableAppsForMime(mime);
	
  //Now search the internal settings for that extension and find any applications last used
  QStringList keys = settings->allKeys();
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
  //Make sure we don't have any duplicates before we return the list
  out.removeDuplicates();
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
      QString bin = ui->line_bin->text();
      good = LUtils::isValidBinary(bin); 
      //Now verify that the file exists and is executable
      if( good ){
        ui->label_goodbin->setPixmap(QPixmap(":/icons/good.png"));
      }else{
        ui->label_goodbin->setPixmap(QPixmap(":/icons/bad.png"));
      }
    }
  }
  else if(ui->radio_rec->isChecked()){
    good = true; //a valid app is always selected on this page if it is available
  }
  else if(ui->combo_apps->count() > 0 && !ui->combo_apps->currentData().toString().isEmpty() ){
      good=true;
  }
  ui->tool_ok->setEnabled(good);
}

void LFileDialog::generateAppList(bool shownetwork){
  //Now load the preferred applications
  PREFAPPS = getPreferredApplications();
  ui->combo_rec->clear();
  //Now get the application mimetype for the file extension (if available)
  QString mimetype = fileEXT;
  //Now add all the detected applications
  QHash< QString, QList<XDGDesktop> > hash = LXDG::sortDesktopCats( LXDG::systemDesktopFiles() );
  QStringList cat = hash.keys();
  cat.sort(); //sort alphabetically
  ui->combo_apps->clear();
  for(int c=0; c<cat.length(); c++){
    QList<XDGDesktop> app = hash[cat[c]];
    if(app.length()<1){ continue; }
    if(ui->combo_apps->count() >1){ ui->combo_apps->insertSeparator(ui->combo_apps->count()); }
    ui->combo_apps->addItem(translateCat(cat[c]));
    ui->combo_apps->insertSeparator(ui->combo_apps->count());
    for(int a=0; a<app.length(); a++){
      if(shownetwork && (cat[c].toLower()=="network" || cat[c].toLower()=="utility") ){ 
	//Need to show preferred internet applications - look for ones that handle URL's
	if(app[a].exec.contains("%u") || app[a].exec.contains("%U")){
          PREFAPPS << app[a].filePath; 
	}
      }
      ui->combo_apps->addItem(LXDG::findIcon(app[a].icon, "application-x-desktop"), app[a].name, app[a].filePath);
      //Check to see if this app matches the mime type
      if(app[a].mimeList.contains(mimetype) && !mimetype.isEmpty()){
        // also put this app in the preferred list
	PREFAPPS.append(app[a].filePath);	      
	//If this is the first preferred app found - select this app initially
	if(ui->combo_apps->currentIndex()<=0){ ui->combo_apps->setCurrentIndex(ui->combo_apps->count()-1); }
      }
    }
  }
  if(ui->combo_apps->currentIndex()<=0){ ui->combo_apps->setCurrentIndex(2); } //Start on the first "real" app - not the first category header
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
  //ui->check_default->setEnabled( !ui->radio_custom->isChecked() );
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
  setDefault = ui->check_default->isChecked();
  if(ui->radio_custom->isChecked()){
    appExec = ui->line_bin->text();  
  }else if(ui->radio_rec->isChecked()){
    //application selected
    bool ok = false;
    XDGDesktop app = LXDG::loadDesktopFile(PREFAPPS[ui->combo_rec->currentIndex()], ok);
    //Set the output variables
    appExec =  LXDG::getDesktopExec(app);
    appPath = app.path;
    appFile = app.filePath;
    setPreferredApplication(app.filePath); //bump this to the top of the preferred list for next time
  }else{
    //application selected
    bool ok = false;
    XDGDesktop app = LXDG::loadDesktopFile(ui->combo_apps->currentData().toString(), ok);
    //Set the output variables
    appExec = LXDG::getDesktopExec(app);
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


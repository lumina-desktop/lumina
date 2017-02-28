//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_fluxbox_settings.h"
#include "ui_page_fluxbox_settings.h"

//==========
//    PUBLIC
//==========
page_fluxbox_settings::page_fluxbox_settings(QWidget *parent) : PageWidget(parent), ui(new Ui::page_fluxbox_settings()){
  ui->setupUi(this);
  loading= false;
  ui->radio_simple->setChecked(true);
  ui->radio_advanced->setChecked(false);
  connect(ui->combo_session_wfocus, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_session_wloc, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->combo_session_wtheme, SIGNAL(currentIndexChanged(int)), this, SLOT(sessionthemechanged()) );
  connect(ui->spin_session_wkspaces, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()) );
  connect(ui->text_file, SIGNAL(textChanged()), this, SLOT(settingChanged()) );
  connect(ui->radio_simple, SIGNAL(toggled(bool)), this, SLOT(switchEditor()) );
  updateIcons();
}

page_fluxbox_settings::~page_fluxbox_settings(){

}

//================
//    PUBLIC SLOTS
//================
void page_fluxbox_settings::SaveSettings(){
  QStringList FB;
  if(ui->radio_simple->isChecked()){
    FB = readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-init");
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
  }else{
    //advanced editor
    FB = ui->text_file->toPlainText().split("\n");
  }
  //Save the fluxbox settings
  bool ok = overwriteFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-init", FB);
  if(!ok){ qDebug() << "Warning: Could not save fluxbox-init"; }
  emit HasPendingChanges(false);
}

void page_fluxbox_settings::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Window Manager Settings") );
  loading = true;
  if(ui->radio_simple->isChecked()){
    ui->stackedWidget->setCurrentWidget(ui->page_simple); //ensure the proper page is visible
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

    QStringList FB = readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-init");
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
  }else{
    //Advanced editor
    ui->stackedWidget->setCurrentWidget(ui->page_advanced); //ensure the proper page is visible
    ui->text_file->setPlainText( readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-init").join("\n") );
  }
  QApplication::processEvents();
  loading = false;
}

void page_fluxbox_settings::updateIcons(){

}

//=================
//         PRIVATE
//=================
//Read/overwrite a text file
QStringList page_fluxbox_settings::readFile(QString path){
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

bool page_fluxbox_settings::overwriteFile(QString path, QStringList contents){
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
void page_fluxbox_settings::sessionthemechanged(){
  //Update the Fluxbox Theme preview
  QString previewfile = ui->combo_session_wtheme->itemData( ui->combo_session_wtheme->currentIndex() ).toString();
  previewfile.append( (previewfile.endsWith("/") ? "preview.jpg": "/preview.jpg") );
  if(QFile::exists(previewfile)){
    ui->label_session_wpreview->setPixmap(QPixmap(previewfile));
  }else{
    ui->label_session_wpreview->setText(tr("No Preview Available"));
  }
  settingChanged();
}

void page_fluxbox_settings::switchEditor(){
  LoadSettings(-1);
}

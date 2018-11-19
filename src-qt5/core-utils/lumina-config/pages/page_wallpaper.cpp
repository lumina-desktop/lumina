//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_wallpaper.h"
#include "ui_page_wallpaper.h"

//==========
//    PUBLIC
//==========
page_wallpaper::page_wallpaper(QWidget *parent) : PageWidget(parent), ui(new Ui::page_wallpaper()){
  ui->setupUi(this);
  DEFAULTBG = LOS::LuminaShare()+"/desktop-background.jpg";
  updateIcons();
  connect(ui->combo_desk_bg, SIGNAL(currentIndexChanged(int)), this, SLOT(deskbgchanged()) );
  connect(ui->radio_desk_multi, SIGNAL(toggled(bool)), this, SLOT(desktimechanged()) );
  connect(ui->tool_desk_addbg, SIGNAL(clicked()), this, SLOT(deskbgadded()) );
  connect(ui->tool_desk_rmbg, SIGNAL(clicked()), this, SLOT(deskbgremoved()) );
  connect(ui->spin_desk_min, SIGNAL(valueChanged(int)), this, SLOT(desktimechanged()) );
  connect(ui->combo_desk_layout, SIGNAL(currentIndexChanged(int)), this, SLOT(desktimechanged()) ); //just need to poke the save routines
}

page_wallpaper::~page_wallpaper(){

}

//================
//    PUBLIC SLOTS
//================
void page_wallpaper::SaveSettings(){
  QSettings settings("lumina-desktop","desktopsettings");
  QString screenID = QApplication::screens().at(cScreen)->name();
  QString DPrefix = "desktop-"+screenID+"/";
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
      settings.setValue(DPrefix+"background/filelist", bgs);
      settings.setValue(DPrefix+"background/minutesToChange", ui->spin_desk_min->value());
      settings.setValue(DPrefix+"background/format", ui->combo_desk_layout->currentData().toString());

  emit HasPendingChanges(false);
}

void page_wallpaper::LoadSettings(int screennum){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Wallpaper Settings") );
  cScreen = screennum; //save for later
  loading = true;
  QSettings settings("lumina-desktop","desktopsettings");
  QString screenID = QApplication::screens().at(cScreen)->name();
  QString DPrefix = "desktop-"+screenID+"/";

  QStringList bgs = settings.value(DPrefix+"background/filelist", QStringList()<<"default").toStringList();
  ui->combo_desk_bg->clear();
  for(int i=0; i<bgs.length(); i++){
    if(bgs[i]=="default"){ ui->combo_desk_bg->addItem( QIcon(DEFAULTBG), tr("System Default"), bgs[i] ); }
    else if(bgs[i].startsWith("rgb(")){ui->combo_desk_bg->addItem(QString(tr("Solid Color: %1")).arg(bgs[i]), bgs[i]); }
    //else{ ui->combo_desk_bg->addItem( QIcon(QPixmap(bgs[i]).scaled(64,64)), bgs[i].section("/",-1), bgs[i] ); }
    else{ ui->combo_desk_bg->addItem( bgs[i].section("/",-1), bgs[i] ); } //disable the thumbnail - takes a long time for large collections of files
  }

  ui->radio_desk_multi->setEnabled(bgs.length()>1);
  if(bgs.length()>1){ ui->radio_desk_multi->setChecked(true);}
  else{ ui->radio_desk_single->setChecked(true); }
  ui->spin_desk_min->setValue( settings.value(DPrefix+"background/minutesToChange", 5).toInt() );
  desktimechanged(); //ensure the display gets updated (in case the radio selection did not change);
  QRect geom = QApplication::desktop()->screenGeometry(cScreen);
  ui->label_desk_res->setText( tr("Screen Resolution:")+"\n"+QString::number(geom.width())+"x"+QString::number(geom.height()) );
  int tmp = ui->combo_desk_layout->findData(settings.value(DPrefix+"background/format","stretch"));
  if(tmp>=0){ ui->combo_desk_layout->setCurrentIndex(tmp); }
  loading = false;
}

void page_wallpaper::updateIcons(){
  ui->tool_desk_addbg->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_desk_rmbg->setIcon( LXDG::findIcon("list-remove","") );
  updateMenus();
}

//=================
//        PRIVATE
//=================
QString page_wallpaper::getColorStyle(QString current, bool allowTransparency){
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

//=================
//    PRIVATE SLOTS
//=================
void page_wallpaper::updateMenus(){
  //Background file menu (different ways of loading files)
  if(ui->tool_desk_addbg->menu()==0){ ui->tool_desk_addbg->setMenu(new QMenu(this)); }
  ui->tool_desk_addbg->menu()->clear();
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("document-new",""), tr("File(s)"), this, SLOT(deskbgadded()) );
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("folder-new",""), tr("Directory (Single)"), this, SLOT(deskbgdiradded()) );
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("document-open-folder",""), tr("Directory (Recursive)"), this, SLOT(deskbgdirradded()) );
  ui->tool_desk_addbg->menu()->addAction(LXDG::findIcon("format-fill-color",""), tr("Solid Color"), this, SLOT(deskbgcoloradded()) );

  //Available Wallpaper layout options
  ui->combo_desk_layout->clear();
  ui->combo_desk_layout->addItem(tr("Automatic"), "stretch");
  ui->combo_desk_layout->addItem(tr("Fullscreen"), "full");
  ui->combo_desk_layout->addItem(tr("Fit screen"), "fit");
  ui->combo_desk_layout->addItem(tr("Tile"), "tile");
  ui->combo_desk_layout->addItem(tr("Center"), "center");
  ui->combo_desk_layout->addItem(tr("Top Left"), "topleft");
  ui->combo_desk_layout->addItem(tr("Top Right"), "topright");
  ui->combo_desk_layout->addItem(tr("Bottom Left"), "bottomleft");
  ui->combo_desk_layout->addItem(tr("Bottom Right"), "bottomright");

}

void page_wallpaper::deskbgchanged(){
  //Load the new image preview
  bool allow_time_set = true;
  if(ui->combo_desk_bg->count()==0){
    ui->label_desk_bgview->setPixmap(QPixmap());
    ui->label_desk_bgview->setText(tr("No Background")+"\n"+tr("(use system default)"));
    ui->label_desk_bgview->setStyleSheet("");
    allow_time_set = false;
  }else{
    allow_time_set = (ui->combo_desk_bg->count()>1);
    QString path = ui->combo_desk_bg->itemData( ui->combo_desk_bg->currentIndex() ).toString();
    if(path=="default"){ path = DEFAULTBG; }
    if(QFileInfo(path).isDir()){
      allow_time_set = true; //always allow setting the time if a directory is set
      QDir dir(path);
      //Got a directory - go ahead and get all the valid image files
      QStringList imgs = LUtils::imageExtensions();
      for(int i=0; i<imgs.length(); i++){ imgs[i].prepend("*."); }
      QStringList files = dir.entryList(imgs, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
      //Now update the item/text
      ui->combo_desk_bg->setItemIcon(ui->combo_desk_bg->currentIndex(), LXDG::findIcon("folder-image","folder"));
      ui->label_desk_bgview->setPixmap(QPixmap());
      ui->label_desk_bgview->setText( QString(tr("Image Directory: %1 valid images")).arg(QString::number(files.length()) ));
      ui->label_desk_bgview->setStyleSheet("");
      ui->label_desk_bgview->setToolTip(files.join("\n"));
    }else if(QFile::exists(path)){
      QSize sz = ui->label_desk_bgview->size();
      sz.setWidth( sz.width() - (2*ui->label_desk_bgview->frameWidth()) );
      sz.setHeight( sz.height() - (2*ui->label_desk_bgview->frameWidth()) );
      //Update the preview/thumbnail for this item
      QPixmap pix(path);
      ui->label_desk_bgview->setPixmap( pix.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation) );
      ui->combo_desk_bg->setItemIcon(ui->combo_desk_bg->currentIndex(), pix.scaled(64,64) );
      ui->label_desk_bgview->setStyleSheet("");
      ui->label_desk_bgview->setToolTip("");
    }else if(path.startsWith("rgb(")){
      ui->label_desk_bgview->setPixmap(QPixmap());
      ui->label_desk_bgview->setText("");
      ui->label_desk_bgview->setStyleSheet("background-color: "+path+";");
      ui->label_desk_bgview->setToolTip("");
    }else{
      ui->label_desk_bgview->setPixmap(QPixmap());
      ui->label_desk_bgview->setText(tr("File does not exist"));
      ui->label_desk_bgview->setStyleSheet("");
      ui->label_desk_bgview->setToolTip("");
    }
  }
  //See if this constitues a change to the current settings and enable the save button
  if(!loading && ui->radio_desk_single->isChecked() && cBG!=ui->combo_desk_bg->currentIndex()){ emit HasPendingChanges(true); }
  cBG = ui->combo_desk_bg->currentIndex(); //keep track of this for later
  //Disable the background rotation option if only one background selected
  if(ui->combo_desk_bg->count()<2){
    ui->radio_desk_single->setChecked(true);
    ui->radio_desk_multi->setEnabled(false);
  }else{
    ui->radio_desk_multi->setEnabled(true);
  }
  ui->spin_desk_min->setEnabled(allow_time_set);

  //Disable the bg remove button if no backgrounds loaded
  ui->tool_desk_rmbg->setEnabled(ui->combo_desk_bg->count()>0);
  ui->label_desk_bgview->setMinimumSize(10,10);
}

void page_wallpaper::desktimechanged(){
  //ui->spin_desk_min->setEnabled(ui->radio_desk_multi->isChecked());
  if(!loading){ emit HasPendingChanges(true); }
}

void page_wallpaper::deskbgremoved(){
  if(ui->combo_desk_bg->count()<1){ return; } //nothing to remove
  ui->combo_desk_bg->removeItem( ui->combo_desk_bg->currentIndex() );
  emit HasPendingChanges(true);
}

void page_wallpaper::deskbgadded(){
  //Prompt the user to find an image file to use for a background
  QString dir = LOS::LuminaShare().section("/lumina-desktop",0,0)+"/wallpapers";
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
  emit HasPendingChanges(true);
}

void page_wallpaper::deskbgcoloradded(){
  //Prompt the user to select a color (no transparency allowed)
  QString color = getColorStyle("",false); //no initial color
  if(color.isEmpty()){ return; }
  //Add it to the list
  ui->combo_desk_bg->addItem( QString(tr("Solid Color: %1")).arg(color), color);
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );

  emit HasPendingChanges(true);
}

void page_wallpaper::deskbgdiradded(){
  //Add the files from a single directory
 QString dir = LOS::LuminaShare().section("/lumina-desktop",0,0)+"/wallpapers";
  qDebug() << "Looking for wallpaper dir:" << dir;
  if( !QFile::exists(dir) ){ dir = QDir::homePath(); }
  dir = QFileDialog::getExistingDirectory(this, tr("Find Background Image Directory"), dir, QFileDialog::ReadOnly);
  if(dir.isEmpty()){ return; }
  //Got a directory - go ahead and find all the valid image files within it
  ui->combo_desk_bg->addItem(dir.section("/",-1), dir);
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );
  //If multiple items selected, automatically enable the background rotation option
  /*if(bgs.length() > 1 && !ui->radio_desk_multi->isChecked()){
    ui->radio_desk_multi->setChecked(true);
  }*/
  emit HasPendingChanges(true);
}

void page_wallpaper::deskbgdirradded(){
  //Recursively add files from a directory
 QString dir = LOS::LuminaShare().section("/lumina-desktop",0,0)+"/wallpapers";
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
  for(int d=0; d<dirs.length(); d++){
     ui->combo_desk_bg->addItem(dirs[d].section("/",-1), dirs[d]);
  }
  //Now move to the last item in the list (the new image(s));
  ui->combo_desk_bg->setCurrentIndex( ui->combo_desk_bg->count()-1 );
  //If multiple items selected, automatically enable the background rotation option
  if(dirs.length() > 1 && !ui->radio_desk_multi->isChecked()){
    ui->radio_desk_multi->setChecked(true);
  }
  emit HasPendingChanges(true);
}

//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PanelWidget.h"
#include "ui_PanelWidget.h"

#include "GetPluginDialog.h"
#include "AppDialog.h"
#include "ScriptDialog.h"

PanelWidget::PanelWidget(QWidget *parent, QWidget *Main, LPlugins *Pinfo) : QWidget(parent), ui(new Ui::PanelWidget){
  ui->setupUi(this);
  mainui = Main;
  PINFO = Pinfo;
  //Fill up the menu's with the valid entries
  ui->combo_align->addItem(tr("Top/Left"), "left");
  ui->combo_align->addItem(tr("Center"), "center");
  ui->combo_align->addItem(tr("Bottom/Right"), "right");
  ui->combo_edge->addItem(tr("Top"), "top");
  ui->combo_edge->addItem(tr("Bottom"), "bottom");
  ui->combo_edge->addItem(tr("Left"), "left");
  ui->combo_edge->addItem(tr("Right"), "right");

  LoadIcons();
  //Now connect any other signals/slots
  connect(ui->combo_edge, SIGNAL(currentIndexChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->combo_align, SIGNAL(currentIndexChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->spin_plength, SIGNAL(valueChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->spin_pxthick, SIGNAL(valueChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->check_autohide, SIGNAL(stateChanged(int)), this, SLOT(ItemChanged()) );
  connect(ui->group_customcolor, SIGNAL(toggled(bool)), this, SLOT(ItemChanged()) );

}

PanelWidget::~PanelWidget(){

}

void PanelWidget::LoadSettings(QSettings *settings, int Dnum, int Pnum){
  pnum = Pnum; dnum = Dnum; //save these for later
  ui->label->setText( QString(tr("Panel %1")).arg(QString::number(Pnum+1) ) );
  QString screenID = QApplication::screens().at(Dnum)->name();
  QString prefix = "panel_"+screenID+"."+QString::number(Pnum)+"/";
  qDebug() << "Loading Panel Settings:" << prefix;
  //Now load the settings into the GUI
  int tmp = ui->combo_align->findData( settings->value(prefix+"pinLocation","center").toString().toLower() );
  if(tmp>=0){ ui->combo_align->setCurrentIndex( tmp ); }
  tmp = ui->combo_edge->findData( settings->value(prefix+"location","top").toString().toLower() );
  if(tmp>=0){ ui->combo_edge->setCurrentIndex( tmp ); }
  ui->spin_plength->setValue( qRound(settings->value( prefix+"lengthPercent",100).toDouble()) );
  ui->spin_pxthick->setValue( qRound(settings->value( prefix+"height",30).toDouble()) );
  ui->check_autohide->setChecked( settings->value(prefix+"hidepanel", false).toBool() );
  ui->group_customcolor->setChecked( settings->value(prefix+"customColor",false).toBool() );
  ui->label_color_sample->setWhatsThis( settings->value(prefix+"color","rgba(255,255,255,160)").toString());
  ui->list_plugins->clear();
  QStringList plugs = settings->value(prefix+"pluginlist",QStringList()).toStringList();
  for(int i=0; i<plugs.length(); i++){
    QString pid = plugs[i].section("---",0,0);
      if(pid.startsWith("applauncher")){
	XDGDesktop desk(pid.section("::",1,1));
	if(desk.type!=XDGDesktop::BAD){ //still need to allow invalid apps
	  QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(desk.icon,""), desk.name );
	      it->setWhatsThis(plugs[i]); //make sure to preserve the entire plugin ID (is the unique version)
	  ui->list_plugins->addItem(it);
	}

      }else if(pid.startsWith("jsonmenu")){
        LPI info = PINFO->panelPluginInfo( plugs[i].section("::::",0,0) );
        if(info.ID.isEmpty()){ continue; } //invalid plugin type (no longer available?)
        QString exec = plugs[i].section("::::",1,1);
        QListWidgetItem *item = new QListWidgetItem();
          item->setWhatsThis( plugs[i] );
          item->setIcon( LXDG::findIcon(plugs[i].section("::::",3,3),info.icon) );
          item->setText( plugs[i].section("::::",2,2) +" ("+info.name+")" );
          item->setToolTip( info.description );
        ui->list_plugins->addItem(item);

      }else{
        LPI info = PINFO->panelPluginInfo(pid);
        if(!info.ID.isEmpty()){
          QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name );
	      it->setWhatsThis(plugs[i]); //make sure to preserve the entire plugin ID (is the unique version)
	  ui->list_plugins->addItem(it);
        }
      }
  }
  reloadColorSample();
}

void PanelWidget::SaveSettings(QSettings *settings, QString screenID){//save the current settings
  if(screenID.isEmpty()){ screenID = QApplication::screens().at(dnum)->name(); }
  QString prefix = "panel_"+screenID+"."+QString::number(pnum)+"/";
  qDebug() << "Saving panel settings:" << prefix;
  settings->setValue(prefix+"location", ui->combo_edge->currentData().toString() );
  settings->setValue(prefix+"pinLocation", ui->combo_align->currentData().toString() );
  settings->setValue(prefix+"lengthPercent", ui->spin_plength->value() );
  settings->setValue(prefix+"height", ui->spin_pxthick->value() );
  settings->setValue(prefix+"hidepanel", ui->check_autohide->isChecked() );
  settings->setValue(prefix+"customColor", ui->group_customcolor->isChecked() );
  settings->setValue(prefix+"color", ui->label_color_sample->whatsThis() );
  QStringList plugs;
  for(int i=0; i<ui->list_plugins->count(); i++){
     plugs << ui->list_plugins->item(i)->whatsThis();
  }
  settings->setValue(prefix+"pluginlist", plugs );
	
}

int PanelWidget::PanelNumber(){
  return pnum;
}

void PanelWidget::ChangePanelNumber(int newnum){
  ui->label->setText( QString(tr("Panel %1")).arg(QString::number(newnum+1) ) );
  pnum = newnum; //So we can retain the current settings, but will save them with a different number
}

void PanelWidget::LoadIcons(){
  ui->tool_rm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_remplugin->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_addplugin->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_upplugin->setIcon( LXDG::findIcon("go-up","") );
  ui->tool_downplugin->setIcon( LXDG::findIcon("go-down","") );
  ui->tool_selectcolor->setIcon( LXDG::findIcon("preferences-desktop-color","") );
  ui->tabWidget->setTabIcon(0,LXDG::findIcon("transform-move",""));
  ui->tabWidget->setTabIcon(1,LXDG::findIcon("preferences-desktop-display",""));
  ui->tabWidget->setTabIcon(2,LXDG::findIcon("preferences-plugin",""));
}

void PanelWidget::reloadColorSample(){
  ui->label_color_sample->setStyleSheet("background: "+ui->label_color_sample->whatsThis());
}

QString PanelWidget::getSysApp(bool allowreset){
  AppDialog dlg(this);
    dlg.allowReset(allowreset);
    dlg.exec();
  if(dlg.appreset && allowreset){
    return "reset";
  }else{
    return dlg.appselected;
  }
}

QString PanelWidget::getColorStyle(QString current, bool allowTransparency){
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

void PanelWidget::on_tool_rm_clicked(){
  emit PanelRemoved(pnum);
}

void PanelWidget::ItemChanged(){
  emit PanelChanged();
}

void PanelWidget::UseColorChanged(){
	
  emit PanelChanged();
}

void PanelWidget::on_tool_selectcolor_clicked(){
  QString color = getColorStyle(ui->label_color_sample->whatsThis());
  if( color.isEmpty()){ return; }
  ui->label_color_sample->setWhatsThis(color);
  reloadColorSample();
  emit PanelChanged();
}

void PanelWidget::on_tool_addplugin_clicked(){
  GetPluginDialog dlg(mainui);
	dlg.LoadPlugins("panel", PINFO);
	dlg.exec();
  if(!dlg.selected){ return; } //cancelled
  QString pan = dlg.plugID; //getNewPanelPlugin();
  QListWidgetItem *it = 0;
  if(pan == "applauncher"){
    //Prompt for the application to add
    QString app =getSysApp();
    if(app.isEmpty()){ return; } //cancelled
    pan.append("::"+app);
    XDGDesktop desk(app);
    it = new QListWidgetItem( LXDG::findIcon(desk.icon,""), desk.name);
      it->setWhatsThis(pan);

  }else if(pan=="jsonmenu"){
    //Need to prompt for the script file, name, and icon to use
    //new ID format: "jsonmenu"::::<exec to run>::::<name>::::<icon>
    ScriptDialog SD(this);
    SD.exec();
    if(!SD.isValid()){ return; }
    LPI info = PINFO->panelPluginInfo(pan);
    it = new QListWidgetItem( LXDG::findIcon(SD.icon(),"text-x-script"), SD.name()+" ("+info.ID+")" );
    it->setWhatsThis(info.ID+"::::"+SD.command()+"::::"+SD.name()+"::::"+SD.icon());
    it->setToolTip( info.description );
  }else{
    if(pan.isEmpty()){ return; } //nothing selected
    //Add the new plugin to the list
    LPI info = PINFO->panelPluginInfo(pan);
    it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name);
      it->setWhatsThis(info.ID);
  }
  //Now add the new item to the list
  if(it!=0){
    ui->list_plugins->addItem(it);
    ui->list_plugins->setCurrentItem(it);
    ui->list_plugins->scrollToItem(it);
  }
  emit PanelChanged();
}

void PanelWidget::on_tool_remplugin_clicked(){
  if(ui->list_plugins->currentRow() < 0){ return; }
  delete ui->list_plugins->takeItem( ui->list_plugins->currentRow() );
  emit PanelChanged();
}

void PanelWidget::on_tool_upplugin_clicked(){
  int row = ui->list_plugins->currentRow();
  if( row <= 0){ return; }
  ui->list_plugins->insertItem(row-1, ui->list_plugins->takeItem(row));
  ui->list_plugins->setCurrentRow(row-1);
  emit PanelChanged();
}

void PanelWidget::on_tool_downplugin_clicked(){
  int row = ui->list_plugins->currentRow();
  if( row < 0 || row >= (ui->list_plugins->count()-1) ){ return; }
  ui->list_plugins->insertItem(row+1, ui->list_plugins->takeItem(row));
  ui->list_plugins->setCurrentRow(row+1);	
  emit PanelChanged();
}

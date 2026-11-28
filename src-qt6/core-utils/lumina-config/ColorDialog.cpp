#include "ColorDialog.h"
#include "ui_ColorDialog.h"

#include <LUtils.h>

ColorDialog::ColorDialog(QWidget *parent, LPlugins *plugs, QString colorFilePath) : QDialog(parent), ui(new Ui::ColorDialog){
  ui->setupUi(this); //load the designer file
  filepath = colorFilePath;
  this->setWindowIcon( LXDG::findIcon("format-stroke-color","") );
  ui->line_name->setText( colorFilePath.section("/",-1).section(".qss",0,0) );
  //Load the icons for the window
  ui->push_cancel->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->push_save->setIcon( LXDG::findIcon("document-save","") );
  ui->tool_getcolor->setIcon( LXDG::findIcon("color-picker","") );
  ui->tool_editcolor->setIcon( LXDG::findIcon("edit-rename","") );
  //Now create entries for the available colors in the database
  ui->tree_color->clear();
  QStringList colors = plugs->colorItems();
  colors.sort();
  for(int i=0; i<colors.length(); i++){
    LPI info = plugs->colorInfo(colors[i]);
    QTreeWidgetItem *it = new QTreeWidgetItem(QStringList() << info.name);
      it->setWhatsThis(0,info.ID);
      it->setToolTip(0,info.description);
    ui->tree_color->addTopLevelItem(it);
  }
  //Now load the given file
  loadColors();
  //Now center the window on the parent
if(parent!=0){
    QWidget *top = parent;
    while(!top->isWindow()){ top = top->parentWidget(); }
    QPoint center = top->geometry().center();
    this->move(center.x()-(this->width()/2), center.y()-(this->height()/2) );
  }
}

void ColorDialog::loadColors(){
  QStringList contents = LUtils::readFile(filepath);
  for(int i=0; i<ui->tree_color->topLevelItemCount(); i++){
    QTreeWidgetItem *it = ui->tree_color->topLevelItem(i);
    //Get the current value and update the item
    QStringList fil = contents.filter(it->whatsThis(0)+"=");
    QString val;
    for(int i=0; i<fil.length(); i++){ 
      if( fil[i].startsWith(it->whatsThis(0)+"=") ){ val = fil[i]; }
    }
    updateItem(it, val.section("=",1,1));
  }
}

void ColorDialog::saveColors(){
  QString name = ui->line_name->text();
  QStringList contents;
  for(int i=0; i<ui->tree_color->topLevelItemCount(); i++){
    QTreeWidgetItem *it = ui->tree_color->topLevelItem(i);
    contents << it->whatsThis(0)+"="+it->text(1);
  }
  bool ok = LTHEME::saveLocalColors(name, contents);
  if(!ok){ qDebug() << "Could not save colors:" << name; }
}

QColor ColorDialog::StringToColor(QString value){
  QColor color;
  if(value.startsWith("rgb(")){
    QStringList vals = value.section("(",1,1).section(")",0,0).split(",");
    if(vals.length()==3){
      color = QColor(vals[0].toInt(), vals[1].toInt(), vals[2].toInt());
    }
  }else if(value.startsWith("rgba(")){
    QStringList vals = value.section("(",1,1).section(")",0,0).split(",");
    if(vals.length()==4){
      color = QColor(vals[0].toInt(), vals[1].toInt(), vals[2].toInt(), vals[3].toInt());
    }
  }else{
    color = QColor(value);
  }
  if(!color.isValid()){ color = QColor(); }
  return color;
}

void ColorDialog::updateItem(QTreeWidgetItem *it, QString value){
  it->setText(1,value);
  if(value.isEmpty()){ return; }
  //qDebug() << "Load Color:" << it->whatsThis(0) << value;
  //Now try to load the color and set the sample
  QBrush brush(StringToColor(value));
  it->setBackground(2, brush);
}

// BUTTONS
void ColorDialog::on_push_save_clicked(){
  //Now set the output values
  colorname = ui->line_name->text();
  colorpath = QDir::homePath()+"/.lumina/colors/"+colorname+".qss.colors";
  //Check if that color already exists
  if(QFile::exists(colorpath)){
    if( QMessageBox::Yes != QMessageBox::question(this, tr("Color Exists"), tr("This color scheme already exists.\n Overwrite it?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ){
      colorname.clear();
      colorpath.clear();
      return; //cancelled
    }
  }
  //save the colors and close
  saveColors();
  this->close();
}

void ColorDialog::on_push_cancel_clicked(){
  //Now clear the output values (just in case)
  colorname.clear();
  colorpath.clear();
  this->close();	
}

void ColorDialog::on_tool_getcolor_clicked(){
  QTreeWidgetItem *it = ui->tree_color->currentItem();
  if(it==0){ return; } //no item selected  
  QColor ccol = StringToColor(it->text(1));
  QColor ncol;
  if(it->whatsThis(0).contains("BASE")){ ncol = QColorDialog::getColor(ccol, this, tr("Select Color")); }
  else{ ncol = QColorDialog::getColor(ccol, this, tr("Select Color"), QColorDialog::ShowAlphaChannel ); }
  
  if(ncol.isValid()){
    QString out;
    if(ncol.alpha()!=255){	  
      //Convert to rgba
      out = "rgba("+QString::number(ncol.red())+","+QString::number(ncol.green())+","+QString::number(ncol.blue())+","+QString::number(ncol.alpha())+")";
    }else{
      //Convert to rgb
      out = "rgb("+QString::number(ncol.red())+","+QString::number(ncol.green())+","+QString::number(ncol.blue())+")";
    }
    updateItem(it, out);
  }
}

void ColorDialog::on_tool_editcolor_clicked(){
  QTreeWidgetItem *it = ui->tree_color->currentItem();
  if(it==0){ return; } //no item selected
  //Get a string from the user
  bool ok = false;
  QString value = QInputDialog::getText(this, tr("Color Value"), tr("Color:"), QLineEdit::Normal, it->text(1), &ok);
  if(!ok || value.isEmpty()){ return; } //cancelled
  updateItem(it, value);
}

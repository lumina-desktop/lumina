#include "ThemeDialog.h"
#include "ui_ThemeDialog.h"

#include <LUtils.h>

ThemeDialog::ThemeDialog(QWidget *parent, LPlugins *plugs, QString themeFilePath) : QDialog(parent), ui(new Ui::ThemeDialog){
  ui->setupUi(this); //load the designer file
  filepath = themeFilePath;
  this->setWindowIcon( LXDG::findIcon("preferences-desktop-theme","") );
  ui->line_name->setText( themeFilePath.section("/",-1).section(".qss",0,0) );
  //Load the icons for the window
  ui->push_cancel->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->push_save->setIcon( LXDG::findIcon("document-save","") );
  ui->push_apply->setIcon( LXDG::findIcon("dialog-ok","") );
  ui->tool_color->setIcon( LXDG::findIcon("color-picker","") );
  //Now create entries for the available colors in the database
  QStringList colors = plugs->colorItems();
  colors.sort();
  colormenu = new QMenu(this);
  for(int i=0; i<colors.length(); i++){
    LPI info = plugs->colorInfo(colors[i]);
    QAction *act = new QAction(info.name, this);
	act->setWhatsThis("%%"+info.ID+"%%");
	act->setToolTip(info.description);
    colormenu->addAction(act);
  }
  ui->tool_color->setMenu(colormenu);
  //Now load the given file
  loadTheme();
  connect(colormenu, SIGNAL(triggered(QAction*)),this, SLOT(menuTriggered(QAction*)) );
  connect(ui->text_file, SIGNAL(textChanged()), this, SLOT(themeChanged()) );
  
  //Now center the window on the parent
  QPoint cen = parent->geometry().center();
  this->move( cen.x() - (this->width()/2) , cen.y() - (this->height()/2) );
}

void ThemeDialog::loadTheme(){
  QStringList contents = LUtils::readFile(filepath);
  ui->text_file->setPlainText( contents.join("\n") );
  ui->push_save->setEnabled(false);
  ui->push_apply->setEnabled(false);
}

void ThemeDialog::saveTheme(){
  QString name = ui->line_name->text();
  QStringList contents = ui->text_file->toPlainText().split("\n");
  LTHEME::saveLocalTheme(name, contents);
  ui->push_save->setEnabled(false);
  ui->push_apply->setEnabled(false);
}

void ThemeDialog::themeChanged(){
  ui->push_save->setEnabled(true);
  ui->push_apply->setEnabled(true);
}

// BUTTONS
void ThemeDialog::on_push_save_clicked(){
  //Now set the output values
  themename = ui->line_name->text();
  themepath = QDir::homePath()+"/.lumina/themes/"+themename+".qss.template";
  //Check if that theme already exists
  if(QFile::exists(themepath)){
    if( QMessageBox::Yes != QMessageBox::question(this, tr("Theme Exists"), tr("This theme already exists.\n Overwrite it?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ){
      themename.clear();
      themepath.clear();
      return; //cancelled
    }
  }
  //save the colors and close
  saveTheme();
  //this->close();
}

void ThemeDialog::on_push_apply_clicked(){
  //Now set the output values
  themename = ui->line_name->text();
  themepath = QDir::homePath()+"/.lumina/themes/"+themename+".qss.template";
  //Check if that theme already exists
  if(QFile::exists(themepath)){
    if( QMessageBox::Yes != QMessageBox::question(this, tr("Theme Exists"), tr("This theme already exists.\n Overwrite it?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ){
      themename.clear();
      themepath.clear();
      return; //cancelled
    }
  }
  saveTheme();
  this->close();
}

void ThemeDialog::on_push_cancel_clicked(){
  //Now clear the output values (just in case)
  themename.clear();
  themepath.clear();
  this->close();	
}

void ThemeDialog::menuTriggered(QAction *act){
  ui->text_file->insertPlainText( act->whatsThis() );
}

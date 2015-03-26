#include "AppLauncherPlugin.h"
#include "../../LSession.h"

AppLauncherPlugin::AppLauncherPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  QVBoxLayout *lay = new QVBoxLayout();
  this->setLayout(lay);
    lay->setContentsMargins(0,0,0,0);
  button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setAutoRaise(true);
    button->setText("..."); //Need to set something here so that initial sizing works properly
    
  lay->addWidget(button, 0, Qt::AlignCenter);
	connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
  menu = new QMenu(this);
	menu->addAction(LXDG::findIcon("zoom-in",""), tr("Increase Size"), this, SLOT(increaseIconSize()));
	menu->addAction(LXDG::findIcon("zoom-out",""), tr("Decrease Size"), this, SLOT(decreaseIconSize()));
  int icosize = settings->value("iconsize",64).toInt();
    button->setIconSize(QSize(icosize,icosize));
  this->setInitialSize(icosize,icosize+10+this->fontMetrics().height());
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openContextMenu()) );
  watcher = new QFileSystemWatcher(this);
	connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT( loadButton()) );
  QTimer::singleShot(1,this, SLOT(loadButton()) );
}
	
void AppLauncherPlugin::loadButton(){
  QString def = this->ID().section("::",1,50).section("---",0,0).simplified();
  QString path = this->settings->value("applicationpath",def).toString(); //use the default if necessary
  //qDebug() << "Default Application Launcher:" << def << path;
  bool ok = false;
  XDGDesktop file = LXDG::loadDesktopFile(path, ok);
  if(path.isEmpty() || !QFile::exists(path) || !ok){
    button->setWhatsThis("");
    button->setIcon( LXDG::findIcon("quickopen-file","") );
    button->setText( tr("Click to Set") );
    if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
  }else{
    button->setWhatsThis(file.filePath);
    button->setIcon( LXDG::findIcon(file.icon,"quickopen") );
    button->setText( this->fontMetrics().elidedText(file.name, Qt::ElideRight, 64) );
    if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
    watcher->addPath(file.filePath); //make sure to update this shortcut if the file changes
  }
  this->adjustSize(); //make sure to adjust the button on first show.
  QTimer::singleShot(100, this, SLOT(update()) ); //Make sure to re-draw the image in a moment
}
	
void AppLauncherPlugin::buttonClicked(){
  QString path = button->whatsThis();
  if(path.isEmpty() || !QFile::exists(path) ){
    //prompt for the user to select an application
    QList<XDGDesktop> apps = LXDG::sortDesktopNames( LXDG::systemDesktopFiles() );
    QStringList names;
    for(int i=0; i<apps.length(); i++){ names << apps[i].name; }
    bool ok = false;
    QString app = QInputDialog::getItem(this, tr("Select Application"), tr("Name:"), names, 0, false, &ok);
    if(!ok || names.indexOf(app)<0){ return; } //cancelled
    this->settings->setValue("applicationpath", apps[ names.indexOf(app) ].filePath);
    QTimer::singleShot(0,this, SLOT(loadButton()));
  }else{
    LSession::LaunchApplication("lumina-open \""+path+"\"");
  }
	  
}

void AppLauncherPlugin::openContextMenu(){
  if(button->underMouse()){
    menu->popup(QCursor::pos());
  }else{
    emit OpenDesktopMenu();
  }
}
	
void AppLauncherPlugin::increaseIconSize(){
  int icosize = settings->value("iconsize",64).toInt();
  icosize += 16;
  button->setIconSize(QSize(icosize,icosize));
  settings->setValue("iconsize",icosize);
}

void AppLauncherPlugin::decreaseIconSize(){
  int icosize = settings->value("iconsize",64).toInt();
  if(icosize < 20){ return; } //cannot get smaller
  icosize -= 16;
  button->setIconSize(QSize(icosize,icosize));
  settings->setValue("iconsize",icosize);
}

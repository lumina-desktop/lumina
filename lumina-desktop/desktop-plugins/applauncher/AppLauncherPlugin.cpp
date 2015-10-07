#include "AppLauncherPlugin.h"
#include "../../LSession.h"

AppLauncherPlugin::AppLauncherPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  QVBoxLayout *lay = new QVBoxLayout();
  this->setLayout(lay);
    lay->setContentsMargins(0,0,0,0);
  button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setAutoRaise(true);
    button->setText("...\n..."); //Need to set something here so that initial sizing works properly
    button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  lay->addWidget(button, 0, Qt::AlignCenter);
	connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
  //menu = new QMenu(this);
  /*int icosize = this->readSetting("iconsize",-1).toInt();
  if(icosize <1){ 
    icosize = LSession::handle()->sessionSettings()->value("DefaultIconSize",64).toInt();
    this->saveSetting("iconsize",icosize);
  }*/
  //int icosize
  //button->setIconSize(QSize(icosize,icosize));
  button->setContextMenuPolicy(Qt::NoContextMenu);
  //connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openContextMenu()) );
  watcher = new QFileSystemWatcher(this);
	connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT( loadButton()) );
  //Calculate the initial size of the button
  //qDebug() << "Button Size:" << button->size();
  //qDebug() << "Calculated:" << icosize+4 << icosize+8+qRound(2.15*button->fontMetrics().height());
  //qDebug() << "Preferred Size:" << button->sizeHint();
  //QSize sz(qRound(1.1*icosize), icosize+qRound(2.7*button->fontMetrics().height()) );
  //button->setFixedSize(sz); //make sure to adjust the button on first show.
  this->setInitialSize(120, 100); //give the container a bit of a buffer
  QTimer::singleShot(100,this, SLOT(loadButton()) );
}
	
void AppLauncherPlugin::Cleanup(){
  //This is run only when the plugin was forcibly closed/removed
  /*if(QFile::exists(button->whatsThis()) && button->whatsThis().startsWith(QDir::homePath()+"/Desktop") ){
    deleteFile();
  }*/
}

void AppLauncherPlugin::loadButton(){
  QString def = this->ID().section("::",1,50).section("---",0,0).simplified();
  QString path = this->readSetting("applicationpath",def).toString(); //use the default if necessary
  //qDebug() << "Default Application Launcher:" << def << path;
  bool ok = QFile::exists(path);
  int icosize = this->width()/1.8; //This is the same calculation as in the LDesktopPluginSpace
  button->setFixedSize( this->width()-4, this->height()-4);
  button->setIconSize( QSize(icosize,icosize) );
  QString txt;
  if(path.endsWith(".desktop") && ok){
    XDGDesktop file = LXDG::loadDesktopFile(path, ok);
    if(path.isEmpty() || !QFile::exists(path) || !ok){
      button->setWhatsThis("");
      button->setIcon( LXDG::findIcon("quickopen-file","") );
      txt = tr("Click to Set");
      if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
    }else{
      button->setWhatsThis(file.filePath);
      button->setIcon( LXDG::findIcon(file.icon,"quickopen") );
      txt = file.name;
      if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
      watcher->addPath(file.filePath); //make sure to update this shortcut if the file changes
    }
  }else if(ok){
    QFileInfo info(path);
    button->setWhatsThis(info.absoluteFilePath());
    if(info.isDir()){
	button->setIcon( LXDG::findIcon("folder","") );
    }else if(LUtils::imageExtensions().contains(info.suffix().toLower()) ){
      button->setIcon( QIcon(QPixmap(path).scaled(256,256)) ); //max size for thumbnails in memory	     
    }else{
      button->setIcon( LXDG::findMimeIcon(path) );
    }
    txt = info.fileName();
    if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
    watcher->addPath(path); //make sure to update this shortcut if the file changes
  }else{
    //InValid File
    button->setWhatsThis("");
    button->setIcon( LXDG::findIcon("quickopen","") );
    button->setText( tr("Click to Set") );
    if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
  }
  //Now adjust the visible text as necessary based on font/grid sizing
  button->setToolTip(txt);
  //int icosize = this->readSetting("iconsize",64).toInt();
  //int bwid = qRound(1.1*icosize);
  //this->setFixedSize(bwid, icosize+qRound(2.5*button->fontMetrics().height()) ); //make sure to adjust the button on first show.
  //if(onchange){ this->adjustSize( bwid+4, icosize+8+qRound(2.5*button->fontMetrics().height())); }
    //qDebug() << "Initial Button Text:" << txt << icosize;
    if(button->fontMetrics().width(txt) > (button->width()-2) ){
      //int dash = this->fontMetrics().width("-");
      //Text too long, try to show it on two lines
      txt = txt.section(" ",0,2).replace(" ","\n"); //First take care of any natural breaks
      if(txt.contains("\n")){
        //need to check each line
	QStringList txtL = txt.split("\n");
	for(int i=0; i<txtL.length(); i++){ 
	  if(i>1){ txtL.removeAt(i); i--; } //Only take the first two lines
	  else{ txtL[i] = button->fontMetrics().elidedText(txtL[i], Qt::ElideRight, (button->width()-2) );  }
	}
	txt = txtL.join("\n");
      }else{
        txt = this->fontMetrics().elidedText(txt,Qt::ElideRight, 2*button->width() -4);
        //Now split the line in half for the two lines
        txt.insert( (txt.count()/2), "\n");
      }
    }
    if(!txt.contains("\n")){ txt.append("\n "); } //always use two lines
    //qDebug() << " - Setting Button Text:" << txt;
    button->setText(txt);
  //Now setup the menu again
  //menu->clear();
  /*menu->addAction(LXDG::findIcon("zoom-in",""), tr("Increase Size"), this, SLOT(increaseIconSize()));
  menu->addAction(LXDG::findIcon("zoom-out",""), tr("Decrease Size"), this, SLOT(decreaseIconSize()));
  if( !button->whatsThis().isEmpty() && button->whatsThis().startsWith(QDir::homePath()+"/Desktop") ){
    menu->addAction(LXDG::findIcon("list-remove",""), tr("Delete File"), this, SLOT(deleteFile()) );	  
  }*/
  
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
    this->saveSetting("applicationpath", apps[ names.indexOf(app) ].filePath);
    QTimer::singleShot(0,this, SLOT(loadButton()));
  }else{
    LSession::LaunchApplication("lumina-open \""+path+"\"");
  }
	  
}

/*void AppLauncherPlugin::openContextMenu(){
  if(button->underMouse()){
    menu->popup(QCursor::pos());
  }else{
    emit OpenDesktopMenu();
  }
}*/
	
/*void AppLauncherPlugin::increaseIconSize(){
  int icosize = this->readSetting("iconsize",64).toInt();
  icosize += 16;
  button->setIconSize(QSize(icosize,icosize));
  this->saveSetting("iconsize",icosize);
  this->loadButton(true); //redo size calculations
	
}

void AppLauncherPlugin::decreaseIconSize(){
  int icosize = this->readSetting("iconsize",64).toInt();
  if(icosize < 20){ return; } //cannot get smaller
  icosize -= 16;
  button->setIconSize(QSize(icosize,icosize));
  this->saveSetting("iconsize",icosize);
  this->loadButton(true); //redo size calculations
}

void AppLauncherPlugin::deleteFile(){
  if(QFileInfo(button->whatsThis()).isDir()){
    QProcess::startDetached("rm -r \""+button->whatsThis()+"\"");
  }else{
    QFile::remove(button->whatsThis());
  }
}*/
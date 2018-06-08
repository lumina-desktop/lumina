#include "AppLauncherPlugin.h"
#include "../../LSession.h"
#include "OutlineToolButton.h"
#include <QClipboard>

#include <LIconCache.h>

#define OUTMARGIN 10 //special margin for fonts due to the outlining effect from the OutlineToolbutton
extern LIconCache *ICONS;

AppLauncherPlugin::AppLauncherPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  connect(ICONS, SIGNAL(IconAvailable(QString)), this, SLOT(iconLoaded(QString)) );
  QVBoxLayout *lay = new QVBoxLayout();
  inputDLG = 0;
  this->setLayout(lay);
    lay->setContentsMargins(0,0,0,0);
  button = new OutlineToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setAutoRaise(true);
    button->setText("...\n..."); //Need to set something here so that initial sizing works properly
    button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  lay->addWidget(button, 0, Qt::AlignCenter);
	connect(button, SIGNAL(DoubleClicked()), this, SLOT(buttonClicked()) );
  button->setContextMenuPolicy(Qt::NoContextMenu);
  watcher = new QFileSystemWatcher(this);
	connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT( loadButton()) );

  connect(this, SIGNAL(PluginActivated()), this, SLOT(buttonClicked()) ); //in case they use the context menu to launch it.
  this->setContextMenu( new QMenu(this) );
  connect(this->contextMenu(), SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)) );

  loadButton();
  //QTimer::singleShot(0,this, SLOT(loadButton()) );
}

void AppLauncherPlugin::Cleanup(){
  //This is run only when the plugin was forcibly closed/removed

}

void AppLauncherPlugin::loadButton(){
  QString def = this->ID().section("::",1,50).section("---",0,0).simplified();
  QString path = this->readSetting("applicationpath",def).toString(); //use the default if necessary
  QFileInfo info(path);
  this->contextMenu()->clear();
  //qDebug() << "Default Application Launcher:" << def << path;
  bool ok = info.canonicalPath().startsWith("/net/");
  if(!ok){ ok = QFile::exists(path); } //do it this way to ensure the file existance check never runs for /net/ files
  if(!ok){ emit RemovePlugin(this->ID()); return;}
  this->setAcceptDrops( info.isDir() );
  icosize = this->height()-4 - 2.2*button->fontMetrics().height();
  button->setFixedSize( this->width()-4, this->height()-4);
  button->setIconSize( QSize(icosize,icosize) );
  button->setToolTip("");
  QString txt;
  iconID.clear();
  if(path.endsWith(".desktop") && ok){
    XDGDesktop file(path);
    ok = !file.name.isEmpty();
    if(!ok){
      button->setWhatsThis("");
      iconID = "quickopen-file";
      //button->setIcon( QIcon(LXDG::findIcon("quickopen-file","").pixmap(QSize(icosize,icosize)).scaledToHeight(icosize, Qt::SmoothTransformation) ) );
      txt = tr("Click to Set");
      if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
    }else{
      button->setWhatsThis(file.filePath);
      if(ICONS->exists(file.icon)){ iconID = file.icon; }
      else if(ICONS->exists(file.icon.section("-",0,-2)) ){ iconID = file.icon.section("-",0,-2); } //some icons get very specific with "-" delimiters, look for a more generic icon if possible
      else{ iconID = "system-run"; }
      //button->setIcon( QIcon(LXDG::findIcon(file.icon,"system-run").pixmap(QSize(icosize,icosize)).scaledToHeight(icosize, Qt::SmoothTransformation) ) );
      if(!file.comment.isEmpty()){button->setToolTip(file.comment); }
      txt = file.name;
      //Put the simple Open action first (no open-with for .desktop files)
      QAction *tmp = this->contextMenu()->addAction( QString(tr("Launch %1")).arg(file.name), this, SLOT(buttonClicked()) );
      ICONS->loadIcon(tmp, file.icon);
      //See if there are any "actions" listed for this file, and put them in the context menu as needed.
      if(!file.actions.isEmpty()){
        for(int i=0; i<file.actions.length(); i++){
          tmp = this->contextMenu()->addAction( file.actions[i].name );
            if(ICONS->exists(file.actions[i].icon)){ ICONS->loadIcon(tmp, file.actions[i].icon); }
            else{ ICONS->loadIcon(tmp, "quickopen-file"); }
            //tmp->setIcon( LXDG::findIcon(file.actions[i].icon,"quickopen-file") );
            tmp->setWhatsThis( file.actions[i].ID );
        }
      }
      if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
      watcher->addPath(file.filePath); //make sure to update this shortcut if the file changes
    }
  }else if(ok){
    button->setWhatsThis(info.absoluteFilePath());
    QString iconame;
    if(info.isDir()){
    if(path.startsWith("/media/") || path.startsWith("/run/media/")){
           iconame = "drive-removable-media";
          //Could add device ID parsing here to determine what "type" of device it is - will be OS-specific though
	  //button->setIcon( LXDG::findIcon("drive-removable-media","") );
	}
        else{ iconame = "folder"; } //button->setIcon( LXDG::findIcon("folder","") );
    }else if(LUtils::imageExtensions().contains(info.suffix().toLower()) ){
      iconame = info.absoluteFilePath();
      //QPixmap pix;
      //if(pix.load(path)){ button->setIcon( QIcon(pix.scaled(256,256)) ); } //max size for thumbnails in memory
      //else{ iconame = "dialog-cancel"; } //button->setIcon( LXDG::findIcon("dialog-cancel","") );
    }else{
      iconame = LXDG::findAppMimeForFile(path).replace("/","-");
      //button->setIcon( QIcon(LXDG::findMimeIcon(path).pixmap(QSize(icosize,icosize)).scaledToHeight(icosize, Qt::SmoothTransformation) ) );
    }
    if(!iconame.isEmpty()){ iconID = iconame; }
    txt = info.fileName();
    if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
    watcher->addPath(path); //make sure to update this shortcut if the file changes
  }else{
    //InValid File
    button->setWhatsThis("");
    iconID = "quickopen";
    //button->setIcon( QIcon(LXDG::findIcon("quickopen","dialog-cancel").pixmap(QSize(icosize,icosize)).scaledToHeight(icosize, Qt::SmoothTransformation) ) );
    button->setText( tr("Click to Set") );
    if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
  }
  if(!iconID.isEmpty()){
    if(ICONS->isLoaded(iconID)){
      ICONS->loadIcon(button, iconID);
      iconLoaded(iconID); //will not get a signal - already loaded right now
    }else{
      //Not loaded yet - verify that the icon exists first
      if(!ICONS->exists(iconID) && iconID.contains("/") ){ iconID = iconID.replace("/","-"); } //quick mimetype->icon replacement just in case
      if(!ICONS->exists(iconID)){ iconID = "unknown"; }
      //Now load the icon
      ICONS->loadIcon(button, iconID);
    }
  }
  //Now adjust the context menu for the button as needed
  QAction *tmp = 0;
  if(this->contextMenu()->isEmpty()){
    tmp = this->contextMenu()->addAction( tr("Open"), this, SLOT(buttonClicked()) );
    ICONS->loadIcon(tmp, "document-open");
    this->contextMenu()->addAction( tr("Open With"), this, SLOT(openWith()) );
    ICONS->loadIcon(tmp, "document-preview");
  }
  tmp = this->contextMenu()->addAction( tr("View Properties"), this, SLOT(fileProperties()) );
  ICONS->loadIcon(tmp, "document-properties");
  this->contextMenu()->addSection(tr("File Operations"));
  if(!path.endsWith(".desktop")){
    tmp = this->contextMenu()->addAction( tr("Rename"), this, SLOT(fileRename()) );
    ICONS->loadIcon(tmp, "edit-rename");
  }
  tmp = this->contextMenu()->addAction( tr("Copy"), this, SLOT(fileCopy()) );
  ICONS->loadIcon(tmp, "edit-copy");
  if(info.isWritable() || (info.isSymLink() && QFileInfo(info.absolutePath()).isWritable() ) ){
    tmp = this->contextMenu()->addAction( tr("Cut"), this, SLOT(fileCut()) );
    ICONS->loadIcon(tmp, "edit-cut");
    tmp = this->contextMenu()->addAction( tr("Delete"), this, SLOT(fileDelete()) );
    ICONS->loadIcon(tmp, "document-close");
  }
  tmp = this->contextMenu()->addAction( tr("Drag to Application"), this, SLOT(startDragNDrop()) );
  ICONS->loadIcon(tmp, "edit-redo");
  iconLoaded(iconID); //make sure the emblem is layered on top
  //If the file is a symlink, put the overlay on the icon
  /*if(info.isSymLink()){
    QImage img = button->icon().pixmap(QSize(icosize,icosize)).toImage();
    int oSize = icosize/3; //overlay size
    QPixmap overlay = ICONS->loadIcon("emblem-symbolic-link").pixmap(oSize,oSize).scaled(oSize,oSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPainter painter(&img);
      painter.drawPixmap(icosize-oSize,icosize-oSize,overlay); //put it in the bottom-right corner
    button->setIcon( QIcon(QPixmap::fromImage(img)) );
  }*/
  //Now adjust the visible text as necessary based on font/grid sizing
  if(button->toolTip().isEmpty()){ button->setToolTip(txt); }
  //Double check that the visual icon size matches the requested size - otherwise upscale the icon
    if(button->fontMetrics().width(txt) > (button->width()-OUTMARGIN) ){
      //Text too long, try to show it on two lines
      //txt = button->fontMetrics().elidedText(txt, Qt::ElideRight, 2*(button->width()-OUTMARGIN), Qt::TextWordWrap);
      txt =txt.section(" ",0,2).replace(" ","\n"); //First take care of any natural breaks
      //Go through and combine any lines
       if(txt.contains("\n")){
        //need to check each line
	QStringList txtL = txt.split("\n");
	for(int i=0; i<txtL.length(); i++){
	  if(( i+1<txtL.length()) && (button->fontMetrics().width(txtL[i]) < button->width()/2) ){
	    txtL[i] = txtL[i]+" "+txtL[i+1];
	    txtL.removeAt(i+1);
	  }
	}
	txt = txtL.join("\n").section("\n",0,2);
      }

      if(txt.contains("\n")){
        //need to check each line
	QStringList txtL = txt.split("\n");
	for(int i=0; i<txtL.length(); i++){
	  if(i>1){ txtL.removeAt(i); i--; } //Only take the first two lines
	  else{ txtL[i] = button->fontMetrics().elidedText(txtL[i], Qt::ElideRight, (button->width()-OUTMARGIN) );  }
	}
	txt = txtL.join("\n");
      }else{
        txt = this->fontMetrics().elidedText(txt,Qt::ElideRight, 2*(button->width()-OUTMARGIN));
        //Now split the line in half for the two lines
        txt.insert( ((txt.count())/2), "\n");
      }
    }
    if(!txt.contains("\n")){ txt.append("\n "); } //always use two lines
    //qDebug() << " - Setting Button Text:" << txt;
    button->setText(txt);

  QTimer::singleShot(100, this, SLOT(update()) ); //Make sure to re-draw the image in a moment
}

void AppLauncherPlugin::buttonClicked(bool openwith){
  QString path = button->whatsThis();
  if(path.isEmpty() || !QFile::exists(path) ){
    //prompt for the user to select an application
    QList<XDGDesktop*> apps = LSession::handle()->applicationMenu()->currentAppHash()->value("All"); //LXDG::sortDesktopNames( LXDG::systemDesktopFiles() );
    QStringList names;
    for(int i=0; i<apps.length(); i++){ names << apps[i]->name; }
    bool ok = false;
    QString app = QInputDialog::getItem(this, tr("Select Application"), tr("Name:"), names, 0, false, &ok);
    if(!ok || names.indexOf(app)<0){ return; } //cancelled
    this->saveSetting("applicationpath", apps[ names.indexOf(app) ]->filePath);
    QTimer::singleShot(0,this, SLOT(loadButton()));
  }else if(openwith){
    LSession::LaunchApplication("lumina-open -select \""+path+"\"");
  }else{
    LSession::LaunchApplication("lumina-open \""+path+"\"");
  }

}

void AppLauncherPlugin::iconLoaded(QString ico){
  if(ico == iconID){
    //Reload/scale the icon as needed
    QPixmap pix = button->icon().pixmap(QSize(icosize,icosize)).scaledToHeight(icosize, Qt::SmoothTransformation);
    if(QFileInfo(button->whatsThis()).isSymLink()){
      QImage img = pix.toImage();
      int oSize = icosize/3; //overlay size
      QPixmap overlay = ICONS->loadIcon("emblem-symbolic-link").pixmap(oSize,oSize).scaled(oSize,oSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      QPainter painter(&img);
        painter.drawPixmap(icosize-oSize,icosize-oSize,overlay); //put it in the bottom-right corner
      pix = QPixmap::fromImage(img);
    }
    button->setIcon( QIcon(pix) );
  }
}

void AppLauncherPlugin::startDragNDrop(){
  //Start the drag event for this file
  QDrag *drag = new QDrag(this);
  QMimeData *md = new QMimeData;
    md->setUrls( QList<QUrl>() << QUrl::fromLocalFile(button->whatsThis()) );
    drag->setMimeData(md);
  //Now perform the drag and react appropriately
  Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
  if(dropAction == Qt::MoveAction){
    // File Moved, remove it from here
    //qDebug() << "File Moved:" << button->whatsThis();
    //DO NOT DELETE FILES - return code often is wrong (browser drops for instance)
  }
}

void AppLauncherPlugin::actionTriggered(QAction *act){
  if(act->whatsThis().isEmpty()){ return; }
  QString path = button->whatsThis();
  if(path.isEmpty() || !QFile::exists(path)){ return; } //invalid file
  LSession::LaunchApplication("lumina-open -action \""+act->whatsThis()+"\" \""+path+"\"");
}

void AppLauncherPlugin::openWith(){
  buttonClicked(true);
}

void AppLauncherPlugin::fileProperties(){
  QString path = button->whatsThis();
  if(path.isEmpty() || !QFile::exists(path)){ return; } //invalid file
  LSession::LaunchApplication("lumina-fileinfo \""+path+"\"");
}

void AppLauncherPlugin::fileDelete(){
  QString path = button->whatsThis();
  if(path.isEmpty() || !QFile::exists(path)){ return; } //invalid file
  if(QFileInfo(path).isDir()){ QProcess::startDetached("rm -r \""+path+"\""); }
  else{ QFile::remove(path); }
}

void AppLauncherPlugin::fileCut(){
  QString path = button->whatsThis();
  QList<QUrl> urilist; //Also assemble a URI list for cros-app compat (no copy/cut distinguishing)
  urilist << QUrl::fromLocalFile(path);
  path.prepend("cut::::");
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", path.toLocal8Bit());
	dat->setUrls(urilist); //the text/uri-list mimetype - built in Qt conversion/use
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);
}

void AppLauncherPlugin::fileCopy(){
  QString path = button->whatsThis();
  QList<QUrl> urilist; //Also assemble a URI list for cros-app compat (no copy/cut distinguishing)
  urilist << QUrl::fromLocalFile(path);
  path.prepend("copy::::");
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", path.toLocal8Bit());
	dat->setUrls(urilist); //the text/uri-list mimetype - built in Qt conversion/use
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);
}

void AppLauncherPlugin::fileRename(){
  if(inputDLG == 0){
    inputDLG = new QInputDialog(0, Qt::Dialog | Qt::WindowStaysOnTopHint);
    inputDLG->setInputMode(QInputDialog::TextInput);
    inputDLG->setTextValue(button->whatsThis().section("/",-1));
    inputDLG->setTextEchoMode(QLineEdit::Normal);
    inputDLG->setLabelText( tr("New Filename") );
    connect(inputDLG, SIGNAL(finished(int)), this, SLOT(renameFinished(int)) );
  }
  inputDLG->showNormal();
}

void AppLauncherPlugin::renameFinished(int result){
  QString newname = inputDLG->textValue();
  inputDLG->deleteLater();
  inputDLG = 0;
  qDebug() << "Got Rename Result:" << result << QDialog::Accepted << newname;
  if(result != QDialog::Accepted){ return; }
  QString newpath = button->whatsThis().section("/",0,-2)+"/"+newname;
  qDebug() << "Move File:" << button->whatsThis() << newpath;
  if( QFile::rename(button->whatsThis(), newpath) ){
    //No special actions here yet - TODO
    qDebug() << " - SUCCESS";
  }
}

void AppLauncherPlugin::fileDrop(bool copy, QList<QUrl> urls){
  for(int i=0; i<urls.length(); i++){
    QString oldpath = urls[i].toLocalFile();
    if(!QFile::exists(oldpath)){ continue; } //not a local file?
    QString filename = oldpath.section("/",-1);
    if(copy){
      qDebug() << "Copying File:" << oldpath << "->" << button->whatsThis()+"/"+filename;
      QFile::copy(oldpath, button->whatsThis()+"/"+filename);
    }else{
      qDebug() << "Moving File:" << oldpath << "->" << button->whatsThis()+"/"+filename;
      QFile::rename(oldpath, button->whatsThis()+"/"+filename);
    }
  }
}

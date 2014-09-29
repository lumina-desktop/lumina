#include "AppLauncherPlugin.h"
#include "../../LSession.h"

AppLauncherPlugin::AppLauncherPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  this->setLayout( new QVBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);
  button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIconSize(QSize(64,64));
    button->setAutoRaise(true);
  this->layout()->addWidget(button);
	connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
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
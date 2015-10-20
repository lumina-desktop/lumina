#include "NotepadPlugin.h"

#include <LuminaXDG.h>
#include "LSession.h"
#include <LuminaUtils.h>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>

NotePadPlugin::NotePadPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  //qDebug() << "Creating Notepad Plugin:";
  QVBoxLayout *vlay = new QVBoxLayout();
  this->setLayout( new QVBoxLayout() );
    this->layout()->setContentsMargins(0,0,0,0);
    vlay->setContentsMargins(3,3,3,3);
    frame = new QFrame(this);
      frame->setObjectName("notepadbase");
      //frame->setStyleSheet("QFrame#notepadbase{border-width: 1px; background: rgba(255,255,255,50); color: black;} QFrame{ border: none; border-radius: 3px; background: rgba(255,255,255,100); color: black;}");
    this->layout()->addWidget(frame);
    frame->setLayout(vlay);
   
  if(!QFile::exists(QDir::homePath()+"/Notes")){
    //Create the notes directory if non-existant
    QDir dir;
	dir.mkpath(QDir::homePath()+"/Notes"); 
  }
  watcher = new QFileSystemWatcher(this);
     //Always watch the notes directory for new files/changes
     watcher->addPath(QDir::homePath()+"/Notes");
  
  typeTimer = new QTimer(this);
    typeTimer->setInterval(1000); // 1 second before it saves
    typeTimer->setSingleShot(true); //compress lots of signals into a single save
  
  updating = false;
  //Setup the title bar header buttons
  QHBoxLayout *hlay = new QHBoxLayout();
  open = new QToolButton(this);
    open->setAutoRaise(true);
  add = new QToolButton(this);
    add->setAutoRaise(true);
  rem = new QToolButton(this);
    rem->setAutoRaise(true);
  cnote = new QComboBox(this);
	
    hlay->addWidget(cnote);
    hlay->addWidget(open);
    hlay->addWidget(add);
    hlay->addWidget(rem);
    vlay->addLayout(hlay);
	
  //Setup the main text widget
  edit = new QPlainTextEdit(this);
    edit->setReadOnly(false);
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vlay->addWidget(edit);
	
  //Now load the new file-based system for saving notes
  //qDebug() << "Saving a new setting";
  this->saveSetting("customFile",""); //always clear this when the plugin is initialized (only maintained per-session)
  //qDebug() << "Loading Notes Dir";
  QTimer::singleShot(2000, this, SLOT(notesDirChanged()));
  //qDebug() << "Set Sizing";
  
  //qDebug() << "Connect Signals/slots";
  //Setup the button connections
  connect(open, SIGNAL(clicked()), this, SLOT(openNote()) );
  connect(add, SIGNAL(clicked()), this, SLOT(newNote()) );
  connect(rem, SIGNAL(clicked()), this, SLOT(remNote()) );
  connect(edit, SIGNAL(textChanged()), this, SLOT(newTextAvailable()) );
  connect(cnote, SIGNAL(currentIndexChanged(QString)), this, SLOT(noteChanged()) );
  connect(typeTimer, SIGNAL(timeout()), this, SLOT(updateContents()) );
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(notesDirChanged()) ); //re-load the available notes
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(noteChanged()) ); //re-load the current file
  QTimer::singleShot(0,this, SLOT(loadIcons()) );
  //qDebug() << " - Done with init";
}

NotePadPlugin::~NotePadPlugin(){

}


void NotePadPlugin::openNote(){
  //qDebug() << "Open New Note:";
  //Prompt for a name for the new note
  QFileDialog dlg(0, Qt::Dialog | Qt::WindowStaysOnTopHint );
      dlg.setFileMode(QFileDialog::ExistingFile);
      dlg.setAcceptMode(QFileDialog::AcceptOpen);
      dlg.setNameFilters( QStringList() << tr("Note Files (*.note)") << tr("Text Files (*)"));
      dlg.setWindowTitle(tr("Open a note file"));
      dlg.setWindowIcon( LXDG::findIcon("document-open","") );
      dlg.setDirectory(QDir::homePath()); //start in the home directory
      //ensure it is centered on the current screen
      QPoint center = QApplication::desktop()->screenGeometry(this).center();
      dlg.move( center.x()-(dlg.width()/2), center.y()-(dlg.height()/2) );
  dlg.show();
  while( dlg.isVisible() ){
    QApplication::processEvents();
  }
  QStringList sel = dlg.selectedFiles();
  if(sel.isEmpty()  || dlg.result()!=QDialog::Accepted){ return; } //cancelled
  QString fullpath = sel.first();
  QString name = fullpath.section("/",-1);
  //qDebug() << " - Found Note:" << name << fullpath;
  int index = cnote->findText(name, Qt::MatchExactly | Qt::MatchCaseSensitive);
  if(QFile::exists(fullpath) && index <0){
    //Alternate option of searching for the file in the list
    index = cnote->findText(fullpath, Qt::MatchExactly | Qt::MatchCaseSensitive);
  }
  if(index>=0){
    //This note already exists: just load it
    cnote->setCurrentIndex(index);
  }else{
    //New note - add it to the end of the list and then load it
    cnote->addItem(name, fullpath);
    this->saveSetting("customFile", fullpath); //save this as a custom file
    cnote->setCurrentIndex( cnote->count()-1 ); 
    QTimer::singleShot(1000, this, SLOT(notesDirChanged())); //Make sure to refresh the list (only one custom file at a time)
  }
}

void NotePadPlugin::newNote(){
  //Prompt for a name for the new note
  //qDebug() << "Create new note";
  QInputDialog dlg(0, Qt::Dialog | Qt::WindowStaysOnTopHint );
      dlg.setInputMode(QInputDialog::TextInput);
      dlg.setLabelText(tr("New Note Name:"));
      dlg.setTextEchoMode(QLineEdit::Normal);
      dlg.setWindowTitle(tr("Create a new note"));
      dlg.setWindowIcon( LXDG::findIcon("document-new","") );
      //ensure it is centered on the current screen
      QPoint center = QApplication::desktop()->screenGeometry(this).center();
      dlg.move( center.x()-(dlg.width()/2), center.y()-(dlg.height()/2) );
  dlg.show();
  while( dlg.isVisible() ){
    QApplication::processEvents();
  }
  QString name = dlg.textValue();
  if(name.isEmpty()  || dlg.result()!=QDialog::Accepted){ return; } //cancelled
  QString fullpath = QDir::homePath()+"/Notes/"+name;
  if(!fullpath.endsWith(".note")){ fullpath.append(".note"); }
  //qDebug() << " - New Note:" << name << fullpath;
  int index = cnote->findText(name, Qt::MatchExactly | Qt::MatchCaseSensitive);
  if(QFile::exists(fullpath) && index <0){
    //Alternate option of searching for the file in the list
    index = cnote->findText(fullpath, Qt::MatchExactly | Qt::MatchCaseSensitive);
  }
  if(index>=0){
    //This note already exists: just load it
    cnote->setCurrentIndex(index);
  }else{
    //New note - add it to the end of the list and then load it
    cnote->addItem(name, fullpath);
    cnote->setCurrentIndex( cnote->count()-1 ); 
  }
}

void NotePadPlugin::remNote(){
  QString note = cnote->currentData().toString();
  if(note.isEmpty()){ return; }
  watcher->removePath(note); //remove this file from the watcher
  this->saveSetting("currentFile",""); //reset the internal value
  QFile::remove(note); //remove the file
  //if(!note.startsWith(QDir::homePath()+"/Notes/") ){
    //If the file was not in the notes directory, need to manually prompt for a re-load
    // otherwise, the directory watcher will catch it and trigger a re-load (no need to double-load)
    notesDirChanged();
  //}
}

void NotePadPlugin::newTextAvailable(){
  if(updating){ return; } //programmatic change of the widget
  if(typeTimer->isActive()){ typeTimer->stop(); }
  typeTimer->start();  
}

void NotePadPlugin::updateContents(){
  if(updating){ return; } //this was a programmatic change to the widget
  //The text was changed in the plugin - save it in the file
  QString note = cnote->currentData().toString();
  updating = true;
  LUtils::writeFile(note, edit->toPlainText().split("\n"), true);
  QApplication::processEvents(); //make sure to process/discard the file changed signal before disabling the flag
  updating = false;
}

void NotePadPlugin::notesDirChanged(){
  if(updating){ return; }
  QString cfile = this->readSetting("currentFile","").toString();
  QStringList notes;
  QDir dir(QDir::homePath()+"/Notes");
  QStringList files = dir.entryList(QStringList() << "*.note", QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  for(int i=0; i<files.length(); i++){
    notes << dir.absoluteFilePath(files[i]);
  }
  QString custom = this->readSetting("customFile","").toString();
  if(!custom.isEmpty() && QFile::exists(custom) ){ notes << custom; } 
  //qDebug() << "Available Notes:" << notes << cfile;
  //Now update the UI list
  updating = true; //don't refresh the UI until done changing lists
  cnote->clear();
  bool found = false;
  for(int i=0; i<notes.length(); i++){
    QString name = notes[i].section("/",-1);
    if(name.endsWith(".note")){ name.chop(5); }
    cnote->addItem(name, notes[i]);
    if(notes[i]==cfile){ cnote->setCurrentIndex(i); found = true;}
  }
  if(!found && !cfile.isEmpty()){
    //Current note is a manually-loaded text file
    cnote->addItem(cfile.section("/",-1), cfile);
    cnote->setCurrentIndex( cnote->count()-1 ); //last item
    found = true;
  }
  if(!found && cnote->count()>0){ cnote->setCurrentIndex(0); }
  updating =false;
  noteChanged();
}

void NotePadPlugin::noteChanged(){
  if(updating){ return; }
  updating =true;
  QString note;
  if(cnote->currentIndex()>=0){
    note = cnote->currentData().toString();
  }
  if(note.isEmpty() && cnote->count()>0){ 
    updating=false; 
    cnote->setCurrentIndex(0); 
    return; 
  }
  QString oldnote = this->readSetting("currentFile","").toString();
  //qDebug() << "Note Changed:" << note << oldnote;
  if( oldnote!=note ){ 
    //Clear the old note file/setting
    if(!oldnote.isEmpty()){
      watcher->removePath(oldnote); 
      this->saveSetting("currentFile","");
    }
    if(!note.isEmpty()){
      this->saveSetting("currentFile",note);
      watcher->addPath(note);
    }
  }

  if(!note.isEmpty()){
    QString text = LUtils::readFile(note).join("\n");
    if(text!=edit->toPlainText()){
      edit->setPlainText( text );  
    }
  }else{
    edit->clear();
  }
  //If no notes available - disable the editor until a new one is created
  edit->setEnabled(!note.isEmpty());
  rem->setEnabled(!note.isEmpty());
  cnote->setEnabled(!note.isEmpty());
  //leave the new/open buttons enabled all the time
  updating = false;
}


void NotePadPlugin::loadIcons(){
  open->setIcon( LXDG::findIcon("document-open","") );
  add->setIcon( LXDG::findIcon("document-new","") );
  rem->setIcon( LXDG::findIcon("document-close","") );
}

#include "NotepadPlugin.h"

#include <LuminaXDG.h>
#include "LSession.h"


NotePadPlugin::NotePadPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  QVBoxLayout *vlay = new QVBoxLayout();
  this->setLayout( new QVBoxLayout() );
    this->layout()->setContentsMargins(0,0,0,0);
    vlay->setContentsMargins(3,3,3,3);
    frame = new QFrame(this);
      frame->setStyleSheet("QFrame{border-size: 1px; background: rgba(255,255,255,100);}");
    this->layout()->addWidget(frame);
    frame->setLayout(vlay);
   
  //Setup the title bar header buttons
  QHBoxLayout *hlay = new QHBoxLayout();
  next = new QToolButton(this);
  prev = new QToolButton(this);
  add = new QToolButton(this);
  rem = new QToolButton(this);
  label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    hlay->addWidget(prev);
    hlay->addWidget(next);
    hlay->addWidget(label);
    hlay->addWidget(add);
    hlay->addWidget(rem);
    vlay->addLayout(hlay);
	
  //Setup the main text widget
  edit = new QPlainTextEdit(this);
    edit->setReadOnly(false);
    vlay->addWidget(edit);
	
  //Now setup the initial values
  cnote = this->settings->value("currentNote", 1).toInt();
  maxnote = this->settings->value("availableNotes",1).toInt();
  this->setInitialSize(200,300);
  //Setup the button connections
  connect(next, SIGNAL(clicked()), this, SLOT(nextNote()) );
  connect(prev, SIGNAL(clicked()), this, SLOT(prevNote()) );
  connect(add, SIGNAL(clicked()), this, SLOT(newNote()) );
  connect(rem, SIGNAL(clicked()), this, SLOT(remNote()) );
  connect(edit, SIGNAL(textChanged()), this, SLOT(noteChanged()) );
  QTimer::singleShot(0,this, SLOT(loadIcons()) );
  QTimer::singleShot(0,this, SLOT(updateContents()) );
  
}

NotePadPlugin::~NotePadPlugin(){

}

void NotePadPlugin::nextNote(){
  cnote++;
  if(cnote>maxnote){ cnote = 1; } //go to the first
  updateContents();  
}

void NotePadPlugin::prevNote(){
  cnote--;
  if(cnote<1){ cnote = maxnote; } //go to the last
  updateContents();  	
}

void NotePadPlugin::newNote(){
  maxnote++;
  cnote = maxnote;
  updateContents();
}

void NotePadPlugin::remNote(){
  //Clear the current note
  settings->remove("Note-"+QString::number(cnote));
  //If the last note, also decrease the max number
  if(cnote==maxnote && maxnote>1){ maxnote--; }
  //Now go to the previous note
  cnote--;
  if(cnote<1){ cnote = maxnote; }
  updateContents();
}

void NotePadPlugin::updateContents(){
  next->setEnabled(cnote<maxnote);
  prev->setEnabled(cnote>1);
  label->setText( QString(tr("Note #%1")).arg(QString::number(cnote)) );
  settings->setValue("currentNote", cnote);
  settings->setValue("availableNotes", maxnote);
  edit->setPlainText( settings->value("Note-"+QString::number(cnote), "").toString() );
}


void NotePadPlugin::noteChanged(){
  //Save the current text
  settings->setValue("Note-"+QString::number(cnote), edit->toPlainText());
}


void NotePadPlugin::loadIcons(){
  next->setIcon( LXDG::findIcon("go-next-view","") );
  prev->setIcon( LXDG::findIcon("go-previous-view","") );
  add->setIcon( LXDG::findIcon("document-new","") );
  rem->setIcon( LXDG::findIcon("document-close","") );
}

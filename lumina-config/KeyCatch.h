//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for catching keyboard events and converting them to X11 keycodes
//===========================================
#ifndef _LUMINA_FILE_MANAGER_KEY_CATCH_DIALOG_H
#define _LUMINA_FILE_MANAGER_KEY_CATCH_DIALOG_H

// Qt includes
#include <QDialog>
#include <QKeyEvent>
#include <QString>
#include <QDebug>

#include "ui_KeyCatch.h"

namespace Ui{
	class KeyCatch;
};

class KeyCatch : public QDialog{
	Q_OBJECT

private:
	Ui::KeyCatch *ui;
	QList<int> mods;
public:
	QString xkeys, qkeys;
	bool cancelled;

	KeyCatch(QWidget *parent = 0) : QDialog(parent), ui(new Ui::KeyCatch){
	  ui->setupUi(this);
	  mods << Qt::Key_Escape << Qt::Key_Tab << Qt::Key_Enter << Qt::Key_Return << Qt::Key_Shift << Qt::Key_Control << Qt::Key_Meta << Qt::Key_Alt;
	  cancelled=true; //assume cancelled in case the user closes the window
	  this->show();
	  this->grabKeyboard(); //will automatically release when the window closes
	}
	~KeyCatch(){}

private slots:
	void on_buttonBox_rejected(){
	  cancelled=true;
	  this->close();
	}

protected:
	void keyPressEvent(QKeyEvent *event){
	  //Don't catch if the main key is a modifier (shift,ctrl,alt,other..)
	  if( !mods.contains(event->key()) ){
	    //Get the modifiers first (if any)
	    if(!QKeySequence(event->modifiers()).toString().isEmpty()){// && event->nativeModifiers()!=16){
	      if(event->modifiers()!=Qt::KeypadModifier){
	        qkeys = QKeySequence(event->modifiers()).toString();
	      }
	      
	      /*//Ignore modifiers that result in a different keycode entirely (shift+a != (shift) + (a) )
	      if(event->modifiers()!=Qt::ShiftModifier && event->modifiers()!=Qt::KeypadModifier){
		//Convert the modifier to the fluxbox equivilent
		QStringList mod = qkeys.split("+");
		for(int i=0; i<mod.length(); i++){
		  QString key = mod[i].toLower();
		  if(key=="shift"){} //this is also a valid fluxbox code
		  else if(key=="meta"){ key = "Mod4"; }
		  else if(key=="ctrl"){ key = "control"; }
		  else if(key=="alt"){ key = "Mod1"; }
		  else{ key.clear(); } //unknown fluxbox modifier
	          if(!key.isEmpty()){ xkeys.append(key+" "); }
		}
	      }*/

	    }
	    //Now get the main key
	    qkeys.replace("+"," ");
	    if(event->key()==0){
	      qkeys.append( QString::number(event->nativeVirtualKey()) );
	    }else{
	      qkeys.append( QKeySequence(event->key()).toString() ); //also save the text version (for display)
	    }
	    //Remove the modifier if it is only "shift", and the main key is not a symbol
	    xkeys = qkeys;
	    if(!xkeys.section(" ",-1).isEmpty() && xkeys.contains("Shift ")){
	      if(!xkeys.section(" ",-1).at(0).isLetter()){
		xkeys.remove("Shift "); //The symbol/keycode is already different
		qkeys.remove("Shift ");
	      }
	    }
	    qDebug() << "Found Key Press:";
	    qDebug() << " - Native Virtual Key:" << event->nativeVirtualKey();
	    qDebug() << " - Qt Key Sequence:" << QKeySequence(event->key()).toString();
	    qDebug() << " - Full Detected Sequence (Display):" << qkeys;
	    qDebug() << " - Full Detected Sequence (backend):" << xkeys;
	    //Now close the dialog
	    cancelled=false;
	    this->close();
	  }
	}

};

#endif

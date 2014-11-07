//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a simple text editor for notes on the desktop
//===========================================
#ifndef _LUMINA_DESKTOP_NOTEPAD_PLUGIN_H
#define _LUMINA_DESKTOP_NOTEPAD_PLUGIN_H

#include <QPlainTextEdit>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include "../LDPlugin.h"

class NotePadPlugin : public LDPlugin{
	Q_OBJECT
public:
	NotePadPlugin(QWidget* parent, QString ID);
	~NotePadPlugin();
	
private:
	QPlainTextEdit *edit;
	QToolButton *next, *prev, *add, *rem;
	QLabel *label;
	QFrame *frame;
	int cnote, maxnote; //current/max note
	
private slots:
	void nextNote();
	void prevNote();
	void newNote();
	void remNote();
	void updateContents();

	void noteChanged();

	void loadIcons();

public slots:
	void LocaleChange(){
	  QTimer::singleShot(0,this, SLOT(updateContents()));
	}
	void ThemeChange(){
	  QTimer::singleShot(0,this, SLOT(loadIcons()));
	}

};
#endif

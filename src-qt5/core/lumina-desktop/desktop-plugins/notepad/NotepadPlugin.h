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
#include <QComboBox>
#include <QVBoxLayout>
#include <QTimer>
#include <QFileSystemWatcher>
#include "../LDPlugin.h"

class NotePadPlugin : public LDPlugin{
	Q_OBJECT
public:
	NotePadPlugin(QWidget* parent, QString ID);
	~NotePadPlugin();
	
	virtual QSize defaultPluginSize(){
	  // The returned QSize is in grid points (typically 100 or 200 pixels square)
	  return QSize(3,3);
	}
private:
	QPlainTextEdit *edit;
	QToolButton *config; // *open, *add, *rem;
	QComboBox *cnote;
	QFrame *frame;
	QFileSystemWatcher *watcher;
	bool updating;
	QTimer *typeTimer;

	void openNote();
	QString newNoteName(QString oldname = "", bool tryagain = false);

private slots:
	void updateConfigMenu();

	void openNoteClicked();
	void newNoteClicked();
	void remNote();
	void renameNote();
	void newTextAvailable();
	void updateContents();

	void notesDirChanged();
	void noteChanged();

	void loadIcons();
	void showContextMenuForEdit(const QPoint &pos);
	void resetContextMenu();

public slots:
	void LocaleChange(){
	  QTimer::singleShot(0,this, SLOT(noteChanged()));
	}
	void ThemeChange(){
	  QTimer::singleShot(0,this, SLOT(loadIcons()));
	}

};
#endif

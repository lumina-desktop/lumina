//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PLAIN_TEXT_EDITOR_MAIN_UI_H
#define _LUMINA_PLAIN_TEXT_EDITOR_MAIN_UI_H

#include <QMainWindow>
#include <QStringList>
#include <QSettings>
#include <QShortcut>

#include "PlainTextEditor.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void LoadArguments(QStringList args); //CLI arguments

public slots:
	void updateIcons();

private:
	Ui::MainUI *ui;
	QSettings *settings;
	QShortcut *closeFindS;

	//Simplification functions
	PlainTextEditor* currentEditor();
	QString currentFileDir();

private slots:
	//Main Actions
	void NewFile();
	void OpenFile(QString file = "");
	void CloseFile(); //current file only
	void SaveFile();
	void SaveFileAs();

	//Other Menu Actions
	void UpdateHighlighting(QAction*);
	void showLineNumbers(bool);
	void wrapLines(bool);
	void ModifyColors();

	//Tab Interactions
	void updateTab(QString);
	void tabChanged();
	void tabClosed(int);

	//Find/Replace functions
	void closeFindReplace();
	void openFind();
	void openReplace();
	
	void findNext();
	void findPrev();
	void replaceOne();
	void replaceAll();
	
protected:
	void resizeEvent(QResizeEvent *ev){
	  settings->setValue("lastSize", ev->size());
	}
};
#endif
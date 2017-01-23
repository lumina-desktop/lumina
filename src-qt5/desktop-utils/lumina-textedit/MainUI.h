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
#include <QFontComboBox>

#include "PlainTextEditor.h"
#include "ColorDialog.h"

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
	QFontComboBox *fontbox;
	ColorDialog *colorDLG;
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
	void fontChanged(const QFont &font);
	void updateStatusTip();

	//Other Menu Actions
	void UpdateHighlighting(QAction *act = 0);
	void showLineNumbers(bool);
	void wrapLines(bool);
	void ModifyColors();
	void showPopupWarnings(bool);

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
	void closeEvent(QCloseEvent *ev);
};
#endif

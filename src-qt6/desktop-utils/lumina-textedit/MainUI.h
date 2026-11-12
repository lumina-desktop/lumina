//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015-2017, Ken Moore
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
#include <QSpinBox>
#include <QAction>
#include <QApplication>

#include "PlainTextEditor.h"
#include "ColorDialog.h"
#include "DnDTabBar.h"
#include "Word.h"

//#include <hunspell/hunspell.hxx>

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

public slots:
	void LoadArguments(QStringList args); //CLI arguments
	void updateIcons();

private:
	Ui::MainUI *ui;
	DnDTabWidget *tabWidget;
	QFontComboBox *fontbox;
	ColorDialog *colorDLG;
	QSettings *settings;
	QShortcut *closeFindS, *nextTabS, *prevTabS, *closeTabS;
	QSpinBox *fontSizes;
	QAction *label_readonly;
	//Hunspell *hunspell;
	QList<Word*> wordList;
	QString hunspellPath;

	//Simplification functions
	PlainTextEditor* currentEditor();
	QString currentFile();
	QString currentFileDir();
	QStringList unsavedFiles();
	void checkWord(QTextBlock);

private slots:
	//Main Actions
	void NewFile();
	void OpenFile(QString file = "");
	void CloseFile(); //current file only
	bool SaveFile();
	bool SaveFileAs();
	bool SaveAllFiles();
	void Print();
	void fontChanged(const QFont &font);
	void updateStatusTip();
	void changeFontSize(int newFontSize);
	void changeTabsLocation(QAction*);
	void checkSpelling(int bpos, int epos = -1);
	void SetLanguage();

	//Other Menu Actions
	void UpdateHighlighting(QAction *act = 0);
	void showToolbar(bool);
	void showLineNumbers(bool);
	void wrapLines(bool);
	void enableSpellcheck(bool);
	void ModifyColors();
	void showPopupWarnings(bool);

	//Tab Interactions
	void updateTab(QString);
	void tabChanged();
	void tabClosed(int);
	void tabDetached(int);
	void tabDraggedOut(int, Qt::DropAction);
	void nextTab();
	void prevTab();

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

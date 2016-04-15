//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PLAIN_TEXT_EDITOR_WIDGET_H
#define _LUMINA_PLAIN_TEXT_EDITOR_WIDGET_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QResizeEvent>
#include <QPaintEvent>

//QPlainTextEdit subclass for providing the actual text editor functionality
class PlainTextEditor : public QPlainTextEdit{
	Q_OBJECT
public:
	PlainTextEditor(QWidget *parent = 0);
	~PlainTextEditor();
	
	//Functions for setting up the editor
	void showLineNumbers(bool show = true);

	//Functions for managing the line number widget (internal - do not need to run directly)
	int LNWWidth(); //replacing the LNW size hint detection
	void paintLNW(QPaintEvent *ev); //forwarded from the LNW paint event
	
private:
	QWidget *LNW; //Line Number Widget
	bool showLNW;

	//Bracket/Perentheses matching functions
	int matchleft, matchright; //positions within the document
	void clearMatchData();
	void highlightMatch(QChar ch, bool forward, int fromPos);

private slots:
	//Functions for managing the line number widget
	void LNW_updateWidth();  	// Tied to the QPlainTextEdit::blockCountChanged() signal
	void LNW_highlightLine();  		// Tied to the QPlainTextEdit::cursorPositionChanged() signal
	void LNW_update(const QRect&, int); 	// Tied to the QPlainTextEdit::updateRequest() signal
	//Function for running the matching routine
	void checkMatchChar();

protected:
	void resizeEvent(QResizeEvent *ev);
};

//===========================================================
// Small Widget for painting the line numbers in the PlainTextEditor
//===========================================================
class LNWidget : public QWidget{
	Q_OBJECT
private:
	PlainTextEditor *TE;
public:
	LNWidget( PlainTextEditor *edit) : QWidget(edit){
	  TE = edit;
	}
	~LNWidget(){}
	//Replace the virtual QWidget size hint function
	//  since the main text editor controls the size/location of this widget
	QSize sizeHint() const{
	  return QSize(TE->LNWWidth(),0);
	}
protected:
	//Replace the virtual QWidget paint event function
	// since the main text editor control the size/location of this widget
	void paintEvent(QPaintEvent *ev){
	  TE->paintLNW(ev);
	}
};
#endif


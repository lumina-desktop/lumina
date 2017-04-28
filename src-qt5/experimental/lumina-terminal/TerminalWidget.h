//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_UTILITIES_TERMINAL_PROCESS_WIDGET_H
#define _LUMINA_DESKTOP_UTILITIES_TERMINAL_PROCESS_WIDGET_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QSocketNotifier>
#include <QTimer>
#include <QMenu>
#include <QClipboard>

#include "TtyProcess.h"

class TerminalWidget : public QTextEdit{
	Q_OBJECT
public:
	TerminalWidget(QWidget *parent =0, QString dir="");
	~TerminalWidget();

	void setTerminalFont(QFont);
	void aboutToClose();

private:
	TTYProcess *PROC;
	QTimer *resizeTimer;
	QTextCharFormat DEFFMT, CFMT; //default/current text format
	QTextCursor selCursor, lastCursor;
	QMenu *contextMenu;
	QAction *copyA, *pasteA;
	int selectionStart;
	bool closing;

	//Incoming Data parsing
	void InsertText(QString);
	void applyData(QByteArray data); //overall data parsing
	void applyANSI(QByteArray code); //individual code application
	void applyANSIColor(int code); //Add the designated color code to the  CFMT structure

	//Outgoing Data parsing
	void sendKeyPress(int key);

	//Special incoming data flags
	int startrow, endrow; //indexes for the first/last row ("\x1b[A;Br" CC)
	bool altkeypad;
private slots:
	void UpdateText();
	void ShellClosed();

	void copySelection();
	void pasteSelection();

	void updateTermSize();

signals:
	void ProcessClosed(QString);

protected:
	void keyPressEvent(QKeyEvent *ev);
	void mousePressEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseDoubleClickEvent(QMouseEvent *ev);
	//void contextMenuEvent(QContextMenuEvent *ev);
	void resizeEvent(QResizeEvent *ev);
};

#endif

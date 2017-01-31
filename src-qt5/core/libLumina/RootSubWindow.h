//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class embeds a native window
//  within the RootWindow area
//===========================================
#ifndef _LUMINA_ROOT_WINDOW_SUB_WINDOW_H
#define _LUMINA_ROOT_WINDOW_SUB_WINDOW_H

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QWindow>
#include <QWidget>
#include <QCloseEvent>

class RootSubWindow : public QMdiSubWindow{
	Q_OBJECT
public:
	RootSubWindow(QMdiArea *root, WId window, Qt::WindowFlags flags = Qt::WindowFlags());
	~RootSubWindow();

	WId id();

private:
	WId CID; //client window ID
	QWindow *WIN; //Embedded window container
	QWidget *WinWidget;
	bool closing;
public slots:
	void clientClosed();
	void clientHidden();
	void clientShown();
	
private slots:
	void aboutToActivate();
	void adjustHeight(int);
	void adjustWidth(int);

protected:
	void closeEvent(QCloseEvent*);

signals:
	void Activated(WId);

};

#endif

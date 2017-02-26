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

#include <NativeWindow.h>

class RootSubWindow : public QMdiSubWindow{
	Q_OBJECT
public:
	RootSubWindow(QMdiArea *root, NativeWindow *win);
	~RootSubWindow();

	WId id();

private:
	NativeWindow *WIN;
	QWidget *WinWidget;
	bool closing;

	void LoadProperties( QList< NativeWindow::Property> list);

public slots:
	void clientClosed();
	
private slots:
	void clientHidden();
	void clientShown();
	void aboutToActivate();
	void propertyChanged(NativeWindow::Property, QVariant);


protected:
	void closeEvent(QCloseEvent*);

};

#endif

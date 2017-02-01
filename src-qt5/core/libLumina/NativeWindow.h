//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is a container object for setting/announcing changes
//    in a native window's properties. 
//    The WM will usually run the "setProperty" function on this object, 
//     and any other classes/widgets which watch this window can act appropriatly after-the-fact
//  Non-WM classes should use the "Request" signals to ask the WM to do something, and listen for changes later
//===========================================
#ifndef _LUMINA_DESKTOP_NATIVE_WINDOW_H
#define _LUMINA_DESKTOP_NATIVE_WINDOW_H

#include <QString>
#include <QRect>
#include <QSize>
#include <QObject>
#include <QWindow>
#include <QHash>
#include <QVariant>

class NativeWindow : public QObject{
	Q_OBJECT
public:
	enum Property{ 	 /*QVariant Type*/
		None, 		/*null*/
		MinSize,  	/*QSize*/
		MaxSize, 	/*QSize*/
		Size, 		/*int*/
		Title, 		/*QString*/
		ShortTitle,	/*QString*/
		Icon, 		/*QIcon*/
		Name, 		/*QString*/
		Workspace,	/*int*/
		WindowFlags,	/*Qt::WindowFlags*/
		Active, 		/*bool*/
		Visible 		/*bool*/
		};

	NativeWindow(WId id);
	~NativeWindow();

	WId id();
	QWindow* window();	

	QVariant property(NativeWindow::Property);
	void setProperty(NativeWindow::Property, QVariant);

private:
	QHash <NativeWindow::Property, QVariant> hash;
	QWindow *WIN;
	WId winid;

signals:
	//General Notifications
	void PropertyChanged(NativeWindow::Property, QVariant);
	void WindowClosed(WId);

	//Action Requests (not automatically emitted)
	//Note: "WId" should be the NativeWindow id()
	void RequestActivate(WId);
	void RequestClose(WId);
	
	// System Tray Icon Embed/Unembed Requests
	//void RequestEmbed(WId, QWidget*);
	//void RequestUnEmbed(WId, QWidget*);
};
#endif

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
	enum State{ S_MODAL, S_STICKY, S_MAX_VERT, S_MAX_HORZ, S_SHADED, S_SKIP_TASKBAR, S_SKIP_PAGER, S_HIDDEN, S_FULLSCREEN, S_ABOVE, S_BELOW, S_ATTENTION };
	enum Type{T_DESKTOP, T_DOCK, T_TOOLBAR, T_MENU, T_UTILITY, T_SPLASH, T_DIALOG, T_DROPDOWN_MENU, T_POPUP_MENU, T_TOOLTIP, T_NOTIFICATION, T_COMBO, T_DND, T_NORMAL };
	enum Action {A_MOVE, A_RESIZE, A_MINIMIZE, A_SHADE, A_STICK, A_MAX_VERT, A_MAX_HORZ, A_FULLSCREEN, A_CHANGE_DESKTOP, A_CLOSE, A_ABOVE, A_BELOW};

	enum Property{ 	 /*QVariant Type*/
		None, 		/*null*/
		MinSize,  	/*QSize*/
		MaxSize, 	/*QSize*/
		Size, 		/*QSize*/
		GlobalPos,	/*QPoint*/
		Title, 		/*QString*/
		ShortTitle,	/*QString*/
		Icon, 		/*QIcon*/
		Name, 		/*QString*/
		Workspace,	/*int*/
		States,		/*QList<NativeWindow::State> : Current state of the window */
		WinTypes,	/*QList<NativeWindow::Type> : Current type of window (typically does not change)*/
		WinActions, /*QList<NativeWindow::Action> : Current actions that the window allows (Managed/set by the WM)*/
		FrameExtents, /*QList<int> : [Left, Right, Top, Bottom] in pixels */
		Active, 		/*bool*/
		Visible 		/*bool*/
		};

	static QList<NativeWindow::Property> allProperties(){
	  //Return all the available properties (excluding "None")
	  QList<NativeWindow::Property> props;
	  props << MinSize << MaxSize << Size << GlobalPos << Title << ShortTitle << Icon << Name << Workspace \
	    << States << WinTypes << WinActions << Active << Visible;
	  return props;
	};

	NativeWindow(WId id);
	~NativeWindow();

	WId id();
	QWindow* window();	

	QVariant property(NativeWindow::Property);
	void setProperty(NativeWindow::Property, QVariant);
	void setProperties(QList<NativeWindow::Property>, QList<QVariant>);
	void requestProperty(NativeWindow::Property, QVariant);
	void requestProperties(QList<NativeWindow::Property>, QList<QVariant>);

public slots:
	void requestClose(); //ask the app to close the window (may/not depending on activity)
	void requestKill();	//ask the WM to kill the app associated with this window (harsh - only use if not responding)
	void requestPing();	//ask the app if it is still active (a WindowNotResponding signal will get sent out if there is no reply);

private:
	QHash <NativeWindow::Property, QVariant> hash;
	QWindow *WIN;
	WId winid;

signals:
	//General Notifications
	void PropertiesChanged(QList<NativeWindow::Property>, QList<QVariant>);
	void RequestPropertiesChange(WId, QList<NativeWindow::Property>, QList<QVariant>);
	void WindowClosed(WId);
	void WindowNotResponding(WId); //will be sent out if a window does not respond to a ping request

	//Action Requests (not automatically emitted - typically used to ask the WM to do something)
	//Note: "WId" should be the NativeWindow id()
	void RequestClose(WId);				//Close the window
	void RequestKill(WId);				//Kill the window/app (usually from being unresponsive)
	void RequestPing(WId);				//Verify that the window is still active (such as not closing after a request
	
	// System Tray Icon Embed/Unembed Requests
	//void RequestEmbed(WId, QWidget*);
	//void RequestUnEmbed(WId, QWidget*);
};
#endif

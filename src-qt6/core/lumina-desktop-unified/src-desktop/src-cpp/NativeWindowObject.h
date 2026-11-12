//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017-2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is a container object for setting/announcing changes
//    in a native window's properties.
//    The WM will usually run the "setProperty" function on this object,
//     and any other classes/widgets which watch this window can act appropriatly after-the-fact
//  Non-WM classes should use the "Request" signals to ask the WM to do something, and listen for changes later
//===========================================
#ifndef _LUMINA_SOURCES_NATIVE_WINDOW_OBJECT_H
#define _LUMINA_SOURCES_NATIVE_WINDOW_OBJECT_H
#include "global-includes.h"

class NativeWindowObject : public QObject{
	Q_OBJECT
	// QML-ACCESSIBLE PROPERTIES
	Q_PROPERTY( QString winImage READ winImage NOTIFY winImageChanged)
	Q_PROPERTY( QString name READ name NOTIFY nameChanged)
	Q_PROPERTY( QString title READ title NOTIFY titleChanged)
	Q_PROPERTY( QString shortTitle READ shortTitle NOTIFY shortTitleChanged)
	Q_PROPERTY( QString icon READ icon NOTIFY iconChanged)
	Q_PROPERTY( bool sticky READ isSticky NOTIFY stickyChanged)
	Q_PROPERTY(bool isVisible READ isVisible NOTIFY visibilityChanged)
	//Button/Titlebar visibility
	Q_PROPERTY( bool showCloseButton READ showCloseButton NOTIFY winTypeChanged)
	Q_PROPERTY( bool showMinButton READ showMinButton NOTIFY winTypeChanged)
	Q_PROPERTY( bool showMaxButton READ showMaxButton NOTIFY winTypeChanged)
	Q_PROPERTY( bool showTitlebar READ showTitlebar NOTIFY winTypeChanged)
	Q_PROPERTY( bool showGenericButton READ showGenericButton NOTIFY winTypeChanged)
	Q_PROPERTY( bool showWindowFrame READ showWindowFrame NOTIFY winTypeChanged)
	//Geometry information
	Q_PROPERTY( QRect frameGeometry READ frameGeometry NOTIFY geomChanged)
	Q_PROPERTY( QRect imageGeometry READ imageGeometry NOTIFY geomChanged)

public:
	enum State{ S_MODAL, S_STICKY, S_MAX_VERT, S_MAX_HORZ, S_SHADED, S_SKIP_TASKBAR, S_SKIP_PAGER, S_HIDDEN, S_FULLSCREEN, S_ABOVE, S_BELOW, S_ATTENTION };
	enum Type{T_DESKTOP, T_DOCK, T_TOOLBAR, T_MENU, T_UTILITY, T_SPLASH, T_DIALOG, T_DROPDOWN_MENU, T_POPUP_MENU, T_TOOLTIP, T_NOTIFICATION, T_COMBO, T_DND, T_NORMAL };
	enum Action {A_MOVE, A_RESIZE, A_MINIMIZE, A_SHADE, A_STICK, A_MAX_VERT, A_MAX_HORZ, A_FULLSCREEN, A_CHANGE_DESKTOP, A_CLOSE, A_ABOVE, A_BELOW};
	enum Location { TOP_LEFT, TOP, TOP_RIGHT, RIGHT, BOTTOM_RIGHT, BOTTOM, BOTTOM_LEFT, LEFT };
	Q_ENUM(Location)

	enum Property{ 	 /*QVariant Type*/
		None=0, 		/*null*/
		MinSize=1,  		/*QSize*/
		MaxSize=2, 		/*QSize*/
		Size=3, 			/*QSize*/
		GlobalPos=4,	/*QPoint*/
		Title=5, 		/*QString*/
		ShortTitle=6,	/*QString*/
		Icon=7, 		/*QIcon*/
		Name=8, 		/*QString*/
		Workspace=9,	/*int*/
		States=10,		/*QList<NativeWindowObject::State> : Current state of the window */
		WinTypes=11,	/*QList<NativeWindowObject::Type> : Current type of window (typically does not change)*/
		WinActions=12, 	/*QList<NativeWindowObject::Action> : Current actions that the window allows (Managed/set by the WM)*/
		FrameExtents=13, 	/*QList<int> : [Left, Right, Top, Bottom] in pixels */
		RelatedWindows=14, /* QList<WId> - better to use the "isRelatedTo(WId)" function instead of reading this directly*/
		Active=15, 		/*bool*/
		Visible=16, 		/*bool*/
		WinImage=17	/*QImage*/
		};

	static QList<NativeWindowObject::Property> allProperties(){
	  //Return all the available properties (excluding "None" and "FrameExtents" (WM control only) )
	  QList<NativeWindowObject::Property> props;
	  props << MinSize << MaxSize << Size << GlobalPos << Title << ShortTitle << Icon << Name << Workspace \
	    << States << WinTypes << WinActions << RelatedWindows << Active << Visible;
	  return props;
	};

	static void RegisterType();

	NativeWindowObject(WId id = 0);
	~NativeWindowObject();

	void addFrameWinID(WId);
	void addDamageID(unsigned int);
	bool isRelatedTo(WId);

	WId id();
	WId frameId();
	unsigned int damageId();

	//QWindow* window();

	QVariant property(NativeWindowObject::Property);
	void setProperty(NativeWindowObject::Property, QVariant, bool force = false);
	void setProperties(QList<NativeWindowObject::Property>, QList<QVariant>, bool force = false);
	void requestProperty(NativeWindowObject::Property, QVariant, bool force = false);
	void requestProperties(QList<NativeWindowObject::Property>, QList<QVariant>, bool force = false);

	Q_INVOKABLE QRect geometry(); //this returns the "full" geometry of the window (window + frame)
	void setGeometryNow(QRect geom);

	// QML ACCESS FUNCTIONS (shortcuts for particular properties in a format QML can use)
	Q_INVOKABLE QString winImage();
	Q_INVOKABLE QString name();
	Q_INVOKABLE QString title();
	Q_INVOKABLE QString shortTitle();
	Q_INVOKABLE QString icon();
	//QML Button states
	Q_INVOKABLE bool showCloseButton();
	Q_INVOKABLE bool showMaxButton();
	Q_INVOKABLE bool showMinButton();
	Q_INVOKABLE bool showTitlebar();
	Q_INVOKABLE bool showGenericButton();
	Q_INVOKABLE bool showWindowFrame();
	//QML Window States
	Q_INVOKABLE bool isSticky();
	Q_INVOKABLE bool isVisible();
	Q_INVOKABLE int workspace();

	//QML Geometry reporting
	Q_INVOKABLE QRect frameGeometry();
	Q_INVOKABLE QRect imageGeometry();
	Q_INVOKABLE void updateGeometry(int x, int y, int width, int height, bool now = false); //For QML to change the current window position

public slots:
	Q_INVOKABLE void toggleVisibility();
	Q_INVOKABLE void toggleMaximize();
	Q_INVOKABLE void requestClose(); //ask the app to close the window (may/not depending on activity)
	Q_INVOKABLE void requestKill();	//ask the WM to kill the app associated with this window (harsh - only use if not responding)
	Q_INVOKABLE void requestPing();	//ask the app if it is still active (a WindowNotResponding signal will get sent out if there is no reply);
	Q_INVOKABLE void requestActivate();
	Q_INVOKABLE void announceClosed();

private:
	QHash <NativeWindowObject::Property, QVariant> hash;
	//QWindow *WIN;
	WId winid, frameid;
	QList<WId> relatedTo;
	unsigned int dmgID, dmg, icodmg;
	//Collation/Delay for window resize events
	QTimer *geomTimer;
	QRect newgeom, lastgeom;

	void emitSinglePropChanged(NativeWindowObject::Property);

private slots:
	void sendNewGeom();

signals:
	//General Notifications
	void PropertiesChanged(QList<NativeWindowObject::Property>, QList<QVariant>);
	void RequestPropertiesChange(WId, QList<NativeWindowObject::Property>, QList<QVariant>);
	void WindowClosed(WId);
	void WindowNotResponding(WId); //will be sent out if a window does not respond to a ping request

	//Action Requests (not automatically emitted - typically used to ask the WM to do something)
	//Note: "WId" should be the NativeWindowObject id()
	void RequestClose(WId);				//Close the window
	void RequestKill(WId);					//Kill the window/app (usually from being unresponsive)
	void RequestPing(WId);				//Verify that the window is still active (such as not closing after a request
	void RequestReparent(WId, WId, QPoint); //client window, frame window, relative origin point in frame
	void VerifyNewGeometry(WId);
	// System Tray Icon Embed/Unembed Requests
	//void RequestEmbed(WId, QWidget*);
	//void RequestUnEmbed(WId, QWidget*);

	// QML update signals
	void winImageChanged();
	void nameChanged();
	void titleChanged();
	void shortTitleChanged();
	void iconChanged();
	void stickyChanged();
	void winTypeChanged();
	void geomChanged();
	void visibilityChanged();
};

// Declare the enumerations as Qt MetaTypes
Q_DECLARE_METATYPE(NativeWindowObject::Type);
Q_DECLARE_METATYPE(NativeWindowObject::Action);
Q_DECLARE_METATYPE(NativeWindowObject::State);
Q_DECLARE_METATYPE(NativeWindowObject::Property);

#endif

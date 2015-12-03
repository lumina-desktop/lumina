//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_MAIN_UI_H
#define _LUMINA_CONFIG_MAIN_UI_H

// Qt includes
#include <QMainWindow>
#include <QDir>
#include <QDesktopWidget>
#include <QSettings>
#include <QStringList>
#include <QString>
#include <QIcon>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QColorDialog>
#include <QColor>
#include <QInputDialog>
#include <QListWidgetItem>

// libLumina includes
#include <LuminaXDG.h>
#include <LuminaThemes.h>

// local includes
#include "LPlugins.h"
//#include "KeyCatch.h"
#include "AppDialog.h"
#include "ColorDialog.h"
#include "ThemeDialog.h"
#include "GetPluginDialog.h"
#include "PanelWidget.h"

//namespace for using the *.ui file
namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	//Panels Page simplifications
	QString getColorStyle(QString current, bool allowTransparency = true);

	//Get an application on the system
	XDGDesktop getSysApp(bool allowreset = false);

private:
	Ui::MainUI *ui; //the *.ui file access
	QSettings *settings, *appsettings, *sessionsettings;
	QDesktopWidget *desktop;
	LPlugins *PINFO;
	QMenu *ppmenu, *mpmenu;
	QString panelcolor;
	QString DEFAULTBG;
	QList<XDGDesktop> sysApps;
	QList<XDGDesktop> STARTAPPS;
	bool loading, panadjust;
	bool moddesk, modpan, modmenu, modshort, moddef, modses; //page modified flags
	int panelnumber;
	QList<PanelWidget*> PANELS;

	//General purpose functions (not connected to buttons)
	void setupMenus(); //called during initialization
	void setupConnections(); //called during intialization

	int currentDesktop(); //the number for the current desktop

	//Convert to/from fluxbox keyboard shortcuts
	QString dispToFluxKeys(QString);
	QString fluxToDispKeys(QString);

	//Read/overwrite a text file
	QStringList readFile(QString path);
	bool overwriteFile(QString path, QStringList contents);

public slots:
	void setupIcons(); //called during initialization

private slots:
	void slotSingleInstance();

	//General UI Behavior
	void slotChangePage(bool enabled);
	void slotChangeScreen();
	void saveAndQuit();

	//General Utility Functions
	void loadCurrentSettings(bool screenonly = false);
	void saveCurrentSettings(bool screenonly = false);

	//Desktop Page
	//void deskplugchanged();
	void deskbgchanged();
	void desktimechanged();
	void deskbgremoved();
	void deskbgadded();
	void deskbgcoloradded();
	void deskplugadded();
	void deskplugremoved();


	//Panels Page
	void panelValChanged();
	void newPanel();
	void removePanel(int); //connected to a signal from the panel widget
	void loadPanels();
	void savePanels();

	//Menu Page/Tab
	void addmenuplugin();
	void rmmenuplugin();
	void upmenuplugin();
	void downmenuplugin();
	void checkmenuicons();

	//Shortcuts Page
	void loadKeyboardShortcuts();
	void saveKeyboardShortcuts();
	void clearKeyBinding();
	void applyKeyBinding();
	void updateKeyConfig();
	//void getKeyPress();

	//Defaults Page
	void changeDefaultBrowser();
	void changeDefaultEmail();
	void changeDefaultFileManager();
	void changeDefaultTerminal();
	void loadDefaultSettings();
	//void saveDefaultSettings();
	void cleardefaultitem();
	void setdefaultitem();
	void setdefaultbinary();
	void checkdefaulticons();

	//Session Page
	void loadSessionSettings();
	void saveSessionSettings();
	void rmsessionstartitem();
	void addsessionstartapp();
	void addsessionstartbin();
	void addsessionstartfile();
	void sessionoptchanged();
	void sessionthemechanged();
	void sessionCursorChanged();
	//void sessionstartchanged();
	void sessionEditColor();
	void sessionEditTheme();
	void sessionChangeUserIcon();
	void sessionResetSys();
	void sessionResetLumina();
	void sessionLoadTimeSample();
	void sessionShowTimeCodes();
	void sessionLoadDateSample();
	void sessionShowDateCodes();
};

#endif

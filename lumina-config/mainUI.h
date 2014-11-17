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
#include "KeyCatch.h"
#include "AppDialog.h"
#include "ColorDialog.h"
#include "ThemeDialog.h"

//namespace for using the *.ui file
namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

private:
	Ui::MainUI *ui; //the *.ui file access
	QSettings *settings, *appsettings, *sessionsettings;
	QDesktopWidget *desktop;
	LPlugins *PINFO;
	QMenu *ppmenu, *mpmenu;
	QString panelcolor;
	QString DEFAULTBG;
	QList<XDGDesktop> sysApps;
	bool loading;
	bool moddesk, modpan, modmenu, modshort, moddef, modses; //page modified flags
	int panelnumber;

	//General purpose functions (not connected to buttons)
	void setupMenus(); //called during initialization
	void setupConnections(); //called during intialization

	int currentDesktop(); //the number for the current desktop

	//Panels Page simplifications
	QString getColorStyle(QString current);
	QString getNewPanelPlugin();

	//Get an application on the system
	XDGDesktop getSysApp();

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
	void deskplugchanged();
	void deskbgchanged();
	void desktimechanged();
	void deskbgremoved();
	void deskbgadded();
	void deskplugadded();


	//Panels Page
	void panelValChanged();
	void addpanel1();
	void addpanel2();
	void rmpanel1();
	void rmpanel2();

	void checkpanels();
	void adjustpanel1();
	void adjustpanel2();

	void getpanel1color();
	void getpanel2color();
	void addpanel1plugin();
	void addpanel2plugin();
	void rmpanel1plugin();
	void rmpanel2plugin();
	void uppanel1plugin();
	void uppanel2plugin();
	void dnpanel1plugin();
	void dnpanel2plugin();

	//Menu Page
	void addmenuplugin();
	void rmmenuplugin();
	void upmenuplugin();
	void downmenuplugin();
	void findmenuterminal();
	void findmenufilemanager();
	void checkmenuicons();

	//Shortcuts Page
	void loadKeyboardShortcuts();
	void saveKeyboardShortcuts();
	void clearKeyBinding();
	void getKeyPress();

	//Defaults Page
	void changeDefaultBrowser();
	void changeDefaultEmail();
	void changeDefaultFileManager();
	void changeDefaultTerminal();
	void loadDefaultSettings();
	//void saveDefaultSettings();
	//void adddefaultgroup();
	//void adddefaultextension();
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
	void sessionstartchanged();
	void sessionEditColor();
	void sessionEditTheme();
};

#endif

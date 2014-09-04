//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the main interface for the Lumina File Manager (Insight)
//===========================================
#ifndef _LUMINA_FILE_MANAGER_UI_H
#define _LUMINA_FILE_MANAGER_UI_H
// Qt includes
#include <QMainWindow>
#include <QTabBar>
#include <QLineEdit>
#include <QFileSystemModel>
#include <QStringList>
#include <QDebug>
#include <QAction>
#include <QProcess>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QShortcut>
#include <QCompleter>
#include <QClipboard>
#include <QMimeData>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QRadioButton>
#include <QWidgetAction>
#include <QFileSystemWatcher>
#include <QImageReader>
#include <QScrollBar>
#include <QFileDialog>

//Phonon widgets
#include <Phonon/BackendCapabilities>
#include <Phonon/MediaObject>
#include <Phonon/VideoWidget>
#include <Phonon/AudioOutput>
#include <Phonon/SeekSlider>
#include <Phonon/MediaSource>

// libLumina includes
#include <LuminaXDG.h>
#include <LuminaOS.h>

// Local includes
#include "FODialog.h" //file operation dialog
#include "BMMDialog.h" //bookmark manager dialog
#include "MimeIconProvider.h" //icon provider for the view widgets

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void OpenDirs(QStringList); //called from the main.cpp after initialization

private:
	Ui::MainUI *ui;
	//Internal non-ui widgets
	QTabBar *tabBar;
	QLineEdit *currentDir;
	QFileSystemModel *fsmod, *snapmod;
	QFileSystemWatcher *fswatcher;
	MimeIconProvider *iconProv;
	QMenu *contextMenu;
	QRadioButton *radio_view_details, *radio_view_list, *radio_view_icons;
	QWidgetAction *detWA, *listWA, *icoWA;

	//Phonon Widgets for the multimedia player
	Phonon::MediaObject *mediaObj;
	Phonon::VideoWidget *videoDisplay;
	Phonon::AudioOutput *audioOut;
	Phonon::SeekSlider *playerSlider;
	QString playerTTime; //total time - to prevent recalculation every tick

	//Internal variables
	QStringList snapDirs; //internal saved variable for the discovered zfs snapshot dirs
	QString CItem; //the item that was right-clicked (for the context menu)
	QStringList imgFilter, multiFilter; //image/multimedia filters
	QSettings *settings;
	QShortcut *nextTabLShort, *nextTabRShort, *closeTabShort, *copyFilesShort, *pasteFilesShort, *deleteFilesShort;
	QCompleter *dirCompleter;
	bool isUserWritable, keepFocus;

	//Simplification Functions
	void setupIcons(); 			//used during initialization
	void setupConnections(); 	//used during initialization
	void loadSettings(); 		//used during initialization
	
	void RebuildBookmarksMenu();
	void RebuildDeviceMenu();
	
	bool checkUserPerms();
	QString msToText(qint64 ms);
	
	//Common functions for browser info/usage
	QString getCurrentDir();
	void setCurrentDir(QString);
	QFileInfoList getSelectedItems();

private slots:
	void slotSingleInstance(const QString &in){
	  this->show();
	  this->raise();
	  this->OpenDirs(in.split("\n"));
	}
	
	//General button check functions (start in a new thread!)
	void checkForMultimediaFiles();
	void checkForBackups();
	void checkForPictures();
	
	//General page switching
	void goToMultimediaPage();
	void goToRestorePage();
	void goToSlideshowPage();
	void goToBrowserPage();
	
	//Menu Actions
	void on_actionNew_Tab_triggered();
	void on_actionClose_triggered();
	void on_actionView_Hidden_Files_triggered();
	void goToBookmark(QAction*);
	void goToDevice(QAction*);
	void viewModeChanged(bool);
	
	//Toolbar Actions
	void on_actionBack_triggered();
	void on_actionUpDir_triggered();
	void on_actionHome_triggered();
	void on_actionBookMark_triggered();

	//Browser Functions
	void startEditDir(QWidget *old, QWidget *now);
	void goToDirectory(); //go to a manually typed in directory
	void reloadDirectory(); //Update the widget with the dir contents
	void on_tool_addToDir_clicked();
	void tabChanged(int tab);
	void tabClosed(int tab = -1);
	void prevTab();
	void nextTab();
	void ItemRun( const QModelIndex&);
	void ItemRun(QTreeWidgetItem *item);
	void ItemRun(QListWidgetItem *item);
	void OpenContextMenu(const QPoint&);

	//Slideshow Functions
	void showNewPicture();
	void firstPicture();
	void prevPicture();
	void nextPicture();
	void lastPicture();
	
	//ZFS Restore Functions
	void snapshotLoaded();
	void showSnapshot();
	void nextSnapshot();
	void prevSnapshot();
	void restoreItems();
	
	//Multimedia Player Functions
	void playerStart();
	void playerStop();
	void playerPause();
	void playerNext();
	void playerPrevious();
	void playerFinished(); //automatically called by the media object
	void playerStateChanged(Phonon::State newstate, Phonon::State oldstate); //automatically called by the media object
	void playerVideoAvailable(bool showVideo); //automatically called by the media object
	void playerTimeChanged(qint64 ctime); //automatically called by the media object
	void playerFileChanged();
	
	//Context Menu Actions
	 // - single item actions
	void OpenItem(); //run "lumina-open" on it
	void OpenItemWith(); //run "lumina-open -select" on it
	void OpenDir(); //open the dir in a new tab
	void RemoveItem(); //Remove the item permanently
	void RenameItem();
	 // - full selection actions
	void CutItems();
	void CopyItems();
	void PasteItems();

};

#endif
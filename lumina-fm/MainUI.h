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
#include <QImageReader>
#include <QScrollBar>
#include <QFileDialog>
#include <QResizeEvent>
#include <QDesktopWidget>
#include <QThread>
#include <QUrl>
#include <QThread>

//Multimedia Widgets
#include <QVideoWidget>
#include <QMediaPlayer>

// libLumina includes
#include <LuminaXDG.h>
#include <LuminaOS.h>

// Local includes
#include "FODialog.h" //file operation dialog
#include "BMMDialog.h" //bookmark manager dialog
#include "MimeIconProvider.h" //icon provider for the view widgets
#include "BackgroundWorker.h"
#include "DDFileSystemModel.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void OpenDirs(QStringList); //called from the main.cpp after initialization

public slots:
	void setupIcons(); 			//used during initialization

private:
	Ui::MainUI *ui;
	QThread *workThread;
	BackgroundWorker *worker;
	//Internal non-ui widgets
	QTabBar *tabBar;
	QLineEdit *currentDir;
	DDFileSystemModel *fsmod;
	QFileSystemModel *snapmod;
	//QFileSystemWatcher *fswatcher;
	MimeIconProvider *iconProv;
	QMenu *contextMenu;
	QRadioButton *radio_view_details, *radio_view_list, *radio_view_icons;
	QWidgetAction *detWA, *listWA, *icoWA;
	QString favdir;

	//Phonon Widgets for the multimedia player
	QMediaPlayer *mediaObj;
	QVideoWidget *videoDisplay;
	QSlider *playerSlider;
	QString playerTTime; //total time - to prevent recalculation every tick

	//Internal variables
	QStringList snapDirs; //internal saved variable for the discovered zfs snapshot dirs
	QString CItem; //the item that was right-clicked (for the context menu)
	//QStringList imgFilter, multiFilter; //image/multimedia filters
	QSettings *settings;
	QShortcut *nextTabLShort, *nextTabRShort, *closeTabShort, *copyFilesShort, *pasteFilesShort, *deleteFilesShort;
	QCompleter *dirCompleter;
	bool isUserWritable, keepFocus;
	QTimer *syncTimer;

	//Simplification Functions
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
	//QModelIndexList getVisibleItems();
	
private slots:
	void slotSingleInstance(QStringList in){
	  this->show();
	  this->raise();
	  this->OpenDirs(in);
	}
	
	void slotStartSyncTimer();
	
	//General button check functions (started in a seperate thread!)
	void AvailableMultimediaFiles(QStringList files);
	void AvailableBackups(QString basedir, QStringList snapdirs);
	void AvailablePictures(QStringList files);
	
	//General page switching
	void goToMultimediaPage();
	void goToRestorePage();
	void goToSlideshowPage();
	void goToBrowserPage();
	
	//Menu Actions
	void on_actionNew_Tab_triggered();
	void on_actionClose_triggered();
	void on_actionView_Hidden_Files_triggered();
	void on_actionShow_Action_Buttons_triggered();
	void on_actionShow_Thumbnails_triggered();
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
	void currentDirectoryLoaded(); //The file system model re-loaded the directory
	void on_tool_addToDir_clicked();
        void on_tool_addNewFile_clicked();
        void tabChanged(int tab);
	void tabClosed(int tab = -1);
	void prevTab();
	void nextTab();
	void ItemRun( const QModelIndex&);
	//void ItemRun(QTreeWidgetItem *item);
	//void ItemRun(QListWidgetItem *item);
	void OpenContextMenu(const QPoint&);
	void ItemSelectionChanged();

	//Slideshow Functions
	void showNewPicture();
	void firstPicture();
	void prevPicture();
	void nextPicture();
	void lastPicture();
	void removePicture();
	void rotatePictureLeft();
	void rotatePictureRight();
	
	//ZFS Restore Functions
	void snapshotLoaded();
	void showSnapshot();
	void nextSnapshot();
	void prevSnapshot();
	void restoreItems();
	
	//Multimedia Player Functions
	void playerStart();
	void playerError();
	void playerStop();
	void playerPause();
	void playerNext();
	void playerPrevious();
	void playerFinished(); //automatically called by the media object
	void playerStatusChanged(QMediaPlayer::MediaStatus stat); //automatically called
	void playerStateChanged(QMediaPlayer::State newstate); //automatically called by the media object
	void playerVideoAvailable(bool showVideo); //automatically called by the media object
	void playerDurationChanged(qint64);
	void playerTimeChanged(qint64 ctime); //automatically called by the media object
	void playerSliderMoved(int);
	void playerSliderHeld();
	void playerSliderChanged();
	void playerFileChanged();
	
	//Context Menu Actions
	void OpenItem(); //run "lumina-open" on it
	void OpenItemWith(); //run "lumina-open -select" on it
	//void OpenDir(); //open the dir in a new tab
	void RemoveItem(); //Remove the item permanently
	 // - single item actions
	void RenameItem();
	void FavoriteItem();
	 // - full selection actions
	void ViewPropertiesItem();
	void openTerminal();
	void CutItems();
	void CopyItems();
	void PasteItems();
	void ChecksumItems();
	
	//file info in status bar
	void DisplayStatusBar(QString);

signals:
	void DirChanged(QString path);
	void Si_AdaptStatusBar(QFileInfoList fileList, QString path, QString messageFolders, QString messageFiles);

protected:
	void resizeEvent(QResizeEvent*);

};

#endif

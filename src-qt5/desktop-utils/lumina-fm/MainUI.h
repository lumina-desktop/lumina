//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
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
//#include "FODialog.h" //file operation dialog
#include "TrayUI.h"
#include "BMMDialog.h" //bookmark manager dialog

#include "DirData.h"
#include "widgets/MultimediaWidget.h"
#include "widgets/SlideshowWidget.h"
#include "widgets/DirWidget2.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

public slots:
	void OpenDirs(QStringList);	 //also called from the main.cpp after initialization
	void setupIcons(); 			//used during initialization

private:
	Ui::MainUI *ui;
	QThread *workThread;
	DirData *worker;
	//Internal non-ui widgets
	QTabBar *tabBar;
	//QFileSystemModel *fsmod;
	QMenu *contextMenu;
	QRadioButton *radio_view_details, *radio_view_list;//, *radio_view_tabs, *radio_view_cols;
	QWidgetAction *detWA, *listWA;//, *tabsWA, *colsWA;

	//UI Widgets
	QList<DirWidget*> DWLIST;
	MultimediaWidget *MW;
	SlideshowWidget *SW;
	TrayUI *TRAY;
	bool waitingToClose;

	QSettings *settings;
	QShortcut *nextTabLShort, *nextTabRShort, *togglehiddenfilesShort, *focusDirWidgetShort;
	//QCompleter *dirCompleter;

	//Simplification Functions
	void setupConnections(); 	//used during initialization
	void loadSettings(); 		//used during initialization
	
	void RebuildBookmarksMenu();
	void RebuildDeviceMenu();
	
	DirWidget* FindActiveBrowser();
	
private slots:
	void slotSingleInstance(QStringList in){
	  this->show();
	  this->raise();
	  this->activateWindow();
	  this->OpenDirs(in);
	}
	
	//void slotStartSyncTimer();
	
	//Menu Actions
	void on_actionNew_Window_triggered();
	void on_actionNew_Tab_triggered();
	void on_actionSearch_triggered();
	void on_actionClose_Browser_triggered();
	void on_actionClose_triggered();
	/*void on_actionRename_triggered();
	void on_actionCut_Selection_triggered();
	void on_actionCopy_Selection_triggered();
	void on_actionPaste_triggered();
	void on_actionDelete_Selection_triggered();*/
	void on_actionRefresh_triggered();
	void on_actionView_Hidden_Files_triggered();
	//void on_actionShow_Action_Buttons_triggered();
	void on_actionShow_Thumbnails_triggered();
	void goToBookmark(QAction*);
	void goToDevice(QAction*);
	void viewModeChanged(bool);
	//void groupModeChanged(bool);
	void on_actionLarger_Icons_triggered();
	void on_actionSmaller_Icons_triggered();
	void CreateBookMark();
	//Git Menu options
	void on_menuGit_aboutToShow();
	void on_actionRepo_Status_triggered();
	void on_actionClone_Repository_triggered();	

	//Tab interactions
	void tabChanged(int tab = -1);
	void tabClosed(int tab = -1);
	void nextTab(); //For keyboard shortcuts
	void prevTab(); //For keyboard shortcuts

	//Other Shortcuts
	void togglehiddenfiles();
	void focusDirWidget();

	//Backend Info passing
	//void DirDataAvailable(QString, QString, LFileInfoList);
	void SnapshotDataAvailable(QString, QString, QStringList);
	
	//Dir Browser Interactions
	void OpenPlayer(LFileInfoList);
	void OpenImages(LFileInfoList);
	void OpenTerminal(QString dirpath);
	void CutFiles(QStringList); //file selection
	void CopyFiles(QStringList); //file selection
	void PasteFiles(QString, QStringList raw = QStringList() ); //current dir, optional list of commands
	void FavoriteFiles(QStringList); //file selection
	void RenameFiles(QStringList); //file selection
	void RemoveFiles(QStringList); //file selection
	void CloseBrowser(QString); //ID
	void TabNameChanged(QString, QString); // ID/name

	//file info in status bar
	void DisplayStatusBar(QString);

	void TrayJobsFinished();

signals:
	void Si_AdaptStatusBar(QFileInfoList fileList, QString path, QString messageFolders, QString messageFiles);

protected:
	void resizeEvent(QResizeEvent *ev){
	  //Save the new size to the settings file for later
	  settings->setValue("preferences/MainWindowSize", ev->size());
	  QMainWindow::resizeEvent(ev); //just in case the window needs to see the event too
	}

	void closeEvent(QCloseEvent *ev);

};

#endif

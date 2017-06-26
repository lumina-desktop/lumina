//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_FM_DIRECTORY_BROWSER_WIDGET_H
#define _LUMINA_FM_DIRECTORY_BROWSER_WIDGET_H

#include <QList>
#include <QWidget>
#include <QObject>
#include <QMenu>
#include <QToolBar>
#include <QLineEdit>
#include <QShortcut>
#include <QFileSystemWatcher>
#include <QFileSystemModel>
#include <QTimer>
#include <QFuture>

#include "../BrowserWidget.h"


#define ZSNAPDIR QString("/.zfs/snapshot/")

namespace Ui{
	class DirWidget;
};

class DirWidget : public QWidget{
	Q_OBJECT
public:
	enum DETAILTYPES{ NAME, SIZE, TYPE, DATEMOD, DATECREATE};
	DirWidget(QString objID, QWidget *parent = 0); //needs a unique ID (to distinguish from other DirWidgets)
	~DirWidget();

	void cleanup(); //called before the browser is closed down

	//Directory Managment
	void ChangeDir(QString dirpath);
	void setDirCompleter(QCompleter *comp);

	//Information
	QString id();
	QString currentDir();
	QFileSystemModel *dirtreeModel;
	QStringList PREFAPPS;

	//View Settings
	void setShowDetails(bool show);
	void showHidden(bool show);
	void showThumbnails(bool show);
	void setThumbnailSize(int px);
	void setFocusLineDir();
	void adjustTreeWidget(float percent); //percent between 0-100



public slots:
	//void LoadDir(QString dir, LFileInfoList list);
	void LoadSnaps(QString basedir, QStringList snaps);

	//Refresh options
	void refresh(); //Refresh current directory

	//Theme change functions
	void UpdateIcons();
	void UpdateText();


private:
	Ui::DirWidget *ui;
	BrowserWidget *BW, *RCBW; //Main BrowserWidget and right-column browser widget
	QString ID, cBID; //unique ID assigned by the parent, and currently active browser widget
	QString normalbasedir, snapbasedir, snaprelpath; //for maintaining directory context while moving between snapshots
	QStringList snapshots, needThumbs, tmpSel;
	bool canmodify;

	//The Toolbar and associated items
	QToolBar *toolbar;
	QLineEdit *line_dir;

	//The context menu and associated items
    QMenu *contextMenu, *cNewMenu, *cOpenMenu, *cFModMenu, *cFViewMenu, *cOpenWithMenu;

	//The keyboard shortcuts for context menu items
    QShortcut *kZoomIn, *kZoomOut, *kNewFile, *kNewDir, *kNewXDG, *kCut, *kCopy, *kPaste, *kRename, \
        *kFav, *kDel, *kOpSS, *kOpMM, *kOpTerm, *kExtract;

	//Functions for internal use
	void createShortcuts(); //on init only
	void createMenus(); //on init only

	BrowserWidget* currentBrowser();
	QStringList currentDirFiles(); //all the "files" available within the current dir/browser

    //QProcess *pExtract;

    //OpenWithMenu
    QString fileEXT, filePath;
    QStringList mimetypes, keys, files;
    //QStringList getPreferredApplications();


private slots:
	//UI BUTTONS/Actions
	void splitterMoved();

	// -- Bottom Action Buttons
	void on_tool_zoom_in_clicked();
	void on_tool_zoom_out_clicked();

	// -- Top Snapshot Buttons
	void on_tool_snap_newer_clicked();
	void on_tool_snap_older_clicked();
	void on_slider_snap_valueChanged(int val = -1);
	void direct_snap_selected(QAction*);

	//Top Toolbar buttons
	void on_actionBack_triggered();
	void on_actionUp_triggered();
	void on_actionHome_triggered();
	void dir_changed(); //user manually changed the directory
	void on_actionSingleColumn_triggered(bool);
	void on_actionDualColumn_triggered(bool);
	void on_actionMenu_triggered();

	// - Other Actions without a specific button on the side
	void fileCheckSums();
	void fileProperties();
	void openTerminal();


	//Browser Functions
	void OpenContextMenu();
	void UpdateContextMenu();
	void currentDirectoryChanged(bool widgetonly = false);
	void dirStatusChanged(QString);
	void setCurrentBrowser(QString);
	void on_folderViewPane_clicked(const QModelIndex &index);

	//Context Menu Functions
	// - DIRECTORY operations
	void createNewFile();
	void createNewDir();
	void createNewXDGEntry();
	//void createNewSymlink();

	// - Selected FILE operations
	void cutFiles();
	void copyFiles();
	void pasteFiles();
	void renameFiles();
	void favoriteFiles();
	void removeFiles();
	void runFiles();
	void runWithFiles();
	//void attachToNewEmail();
	void autoExtractFiles();

	// - Context-specific operations
	void openInSlideshow();
	void openMultimedia();


signals:
	//Directory loading/finding signals
	void OpenDirectories(QStringList); //Directories to open in other tabs/columns
	void findSnaps(QString, QString); //ID, dirpath (Request snapshot information for a directory)
	void CloseBrowser(QString); //ID (Request that this browser be closed)
	void treeWidgetSizeChanged(float); //percent width

	//External App/Widget launching
	void PlayFiles(LFileInfoList); //open in multimedia player
	void ViewFiles(LFileInfoList); //open in slideshow
	void LaunchTerminal(QString); //dirpath

	//System Interactions
	void CutFiles(QStringList); //file selection
	void CopyFiles(QStringList); //file selection
	void PasteFiles(QString, QStringList); //current dir
	void FavoriteFiles(QStringList); //file selection
	void RenameFiles(QStringList); //file selection
	void RemoveFiles(QStringList); //file selection
	void TabNameChanged(QString, QString); //objID, new tab name

protected:
	void mouseReleaseEvent(QMouseEvent *);

};

#endif

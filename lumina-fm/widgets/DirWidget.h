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
#include <QTimer>

#include "../DirData.h"
#include "DDListWidgets.h"

#define ZSNAPDIR QString("/.zfs/snapshot/")

#ifndef DEBUG
#define DEBUG 0
#endif

namespace Ui{
	class DirWidget;
};

class DirWidget : public QWidget{
	Q_OBJECT
public:
	enum DETAILTYPES{ NAME, SIZE, TYPE, DATEMOD, DATECREATE};
	DirWidget(QString objID, QWidget *parent = 0); //needs a unique ID (to distinguish from other DirWidgets)
	~DirWidget();
	
	//Directory Managment
	void ChangeDir(QString dirpath);
	void setDirCompleter(QCompleter *comp);
	
	//Information
	QString id();
	QString currentDir();

	//View Settings
	void setShowDetails(bool show);
	void setShowSidebar(bool show);
	void setShowThumbnails(bool show);
	void setDetails(QList<DETAILTYPES> list); //Which details to show and in which order (L->R)
	void setThumbnailSize(int px);
	void setShowCloseButton(bool show);
	
public slots:
	void LoadDir(QString dir, LFileInfoList list);
	void LoadSnaps(QString basedir, QStringList snaps);
	
	//Refresh options
	void refresh(); //Refresh current directory
	void refreshButtons(); //Refresh action buttons only

	//Theme change functions
	void UpdateIcons();
	void UpdateText();
	
	//Button updates
	void UpdateButtons();

private:
	Ui::DirWidget *ui;
	QString ID, CDIR; //unique ID assigned by the parent and the current dir path
	LFileInfoList CLIST; //current item list (snap or not)
	QString normalbasedir, snapbasedir, snaprelpath; //for maintaining direcoty context while moving between snapshots
	QStringList snapshots;
	bool showDetails, showThumbs, canmodify, stopload; //which widget to use for showing items
	QList<DETAILTYPES> listDetails;
	QMenu *contextMenu;
	//The Toolbar and associated items
	QToolBar *toolbar;
	QLineEdit *line_dir;
	QStringList history;
	//The drag and drop brower widgets
	DDListWidget *listWidget;
	DDTreeWidget *treeWidget;

	//Keyboard Shortcuts
	QShortcut *copyFilesShort, *cutFilesShort, *pasteFilesShort, *deleteFilesShort, *refreshShort;
	//Watcher to determine when the dir changes
	QFileSystemWatcher *watcher;
	QTimer *synctimer;

	//Functions for internal use
	void setupConnections();
	QStringList currentSelection();

private slots:
	//UI BUTTONS/Actions
	// -- Left Action Buttons
	void on_tool_act_copy_clicked();
	void on_tool_act_cut_clicked();
	void on_tool_act_fav_clicked();
	void on_tool_act_paste_clicked();
	void on_tool_act_rename_clicked();
	void on_tool_act_rm_clicked();
	void on_tool_act_run_clicked();
	void on_tool_act_runwith_clicked();
	// -- Bottom Action Buttons
	void on_tool_goToImages_clicked();
	void on_tool_goToPlayer_clicked();
	void on_tool_new_file_clicked();
	void on_tool_new_dir_clicked();
	// -- Top Snapshot Buttons
	void on_tool_snap_newer_clicked();
	void on_tool_snap_older_clicked();
	void on_slider_snap_valueChanged(int);
	//Top Toolbar buttons
	void on_actionBack_triggered();
	void on_actionUp_triggered();
	void on_actionHome_triggered();
	void on_actionStopLoad_triggered();
	void dir_changed(); //user manually changed the directory
	void on_actionClose_Browser_triggered();
	
	// - Other Actions without a specific button on the side
	void fileCheckSums();
	void fileProperties();
	void openTerminal();
	

	//Browser Functions
	void OpenContextMenu();
	void SelectionChanged();
	void startSync(); //used internally to collect/pause before updating the dir

signals:
	//Directory loading/finding signals
	void OpenDirectories(QStringList); //Directories to open in other tabs/columns
	void LoadDirectory(QString, QString); //ID, dirpath (Directory to load here)
	void findSnaps(QString, QString); //ID, dirpath (Request snapshot information for a directory)
	void CloseBrowser(QString); //ID (Request that this browser be closed)
	
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
	
protected:
	void mouseReleaseEvent(QMouseEvent *);
	
};

/*
 * Virtual class for managing the sort of folders/files items. The problem with base class is that it only manages texts fields and
 * we have dates and sizes.
 *
 * On this class, we overwrite the function operator<.
 */

class CQTreeWidgetItem : public QTreeWidgetItem {
public:
    CQTreeWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    CQTreeWidgetItem(const QStringList & strings, int type = Type) : QTreeWidgetItem(strings, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, const QStringList & strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, QTreeWidgetItem * preceding, int type = Type) : QTreeWidgetItem(parent, preceding, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, const QStringList & strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, QTreeWidgetItem * preceding, int type = Type) : QTreeWidgetItem(parent, preceding, type) {}
    virtual ~CQTreeWidgetItem() {}
    inline virtual bool operator<(const QTreeWidgetItem &tmp) const {
      int column = this->treeWidget()->sortColumn();
      // We are in date text
      if(column == DirWidget::DATEMOD || column == DirWidget::DATECREATE) {
        // Get the stored text and try to convert to QDateTime
        QString text = this->text(column);
        QString text_tmp = tmp.text(column);
        QDateTime date_time = QDateTime::fromString(text, Qt::DefaultLocaleShortDate);
        QDateTime date_time_tmp = QDateTime::fromString(text_tmp, Qt::DefaultLocaleShortDate);
        // If the conversion are ok in both objects, compare them
        if(date_time.isValid() && date_time_tmp.isValid())
          return date_time < date_time_tmp;
        // If some of the dates are invalid, use the base class implementation (order by string)
        else {
          if(DEBUG)
            qDebug() << "Cannot convert the date. Texts arrived are " << text << " and " << text_tmp;
          return QTreeWidgetItem::operator <(tmp);
        }
      }
      // We are in size text
      else if(column == DirWidget::SIZE) {
        QString text = this->text(column);
        QString text_tmp = tmp.text(column);
        double filesize, filesize_tmp;
        // On folders, text is empty so we check for that
        // In case we are in folders, we put -1 for differentiate of regular files with 0 bytes.
        // Doing so, all folders we'll be together instead of mixing with files with 0 bytes.
        if(text.isEmpty())
          filesize = -1;
        else
          filesize = LUtils::DisplaySizeToBytes(text);
        if(text_tmp.isEmpty())
          filesize_tmp = -1;
        else
          filesize_tmp = LUtils::DisplaySizeToBytes(text_tmp);
        return filesize < filesize_tmp;
      }
      // In other cases, we trust base class implementation
      return QTreeWidgetItem::operator<(tmp);
    }
};
#endif

//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the main browsing frontend for the file manager
//===========================================
#ifndef _LUMINA_FM_BROWSE_FRONTEND_H
#define _LUMINA_FM_BROWSE_FRONTEND_H

#include <QString>
#include <QWidget>
#include <QThread>

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <LVideoWidget.h>
#endif
#include <LFileInfo.h>

#include "Browser.h"
#include "widgets/DDListWidgets.h"

class BrowserWidget : public QWidget{
	Q_OBJECT
private:
	Browser *BROWSER;
	QThread *bThread; //browserThread
	int numItems; //used for checking if all the items have loaded yet
	QTimer *statsTimer; //debounces loadStatistics() (was one background task per item, touching GUI widgets off-thread)
	QString ID, statustip;
	QStringList date_format, historyList;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QMap<QString,QPair<QTreeWidgetItem*, LVideoWidget*>> videoMap;
#endif
	bool freshload;
	int treeSortColumn;
	QMutex widgetMutex;

	//The drag and drop brower widgets
	DDListWidget *listWidget;
	DDTreeWidget *treeWidget;
	//path -> item for the current list/tree (each arriving item used to be found with a linear findItems(), making
	// a large directory load quadratic). Kept in step with every place items are created/deleted below.
	QHash<QString, QListWidgetItem*> listItems;
	QHash<QString, QTreeWidgetItem*> treeItems;

	QString DTtoString(QDateTime dt);  //QDateTime to string simplification routine

public:
	BrowserWidget(QString objID, QWidget *parent = 0);
	~BrowserWidget();

	QString id(){ return ID; }
	void stop();

	void changeDirectory(QString dir);
	QString currentDirectory(){ return BROWSER->currentDirectory(); }

	void showDetails(bool show);
	bool hasDetails();

	void showHiddenFiles(bool show);
	bool hasHiddenFiles();

	void showThumbnails(bool show);
	bool hasThumbnails();
	void setThumbnailSize(int px);
	int thumbnailSize();

	void setHistory(QStringList);
	QStringList history();

	void setShowActive(bool show); //used for accenting if the widget is "active"

	void setTreeWidgetSortColumn(int col, bool now = false);

	QString status(){ return statustip; }

	//Date format for show items
	void readDateFormat();

	//Return all the items which are currently selected
	QStringList currentSelection();
	QStringList currentItems(int type = 0); //type: 0=all, -1=files, +1=dirs

public slots:
	void retranslate();

private slots:
	//Browser connections
	void clearItems();
	void itemRemoved(QString);
	void itemDataAvailable(QIcon, LFileInfo*); //takes ownership of the LFileInfo
	void itemsLoading(int total);
	void selectionChanged();
	void loadStatistics(BrowserWidget *bw); //runs on the GUI thread (it reads the list/tree widgets)
	void updateStatistics(); //statsTimer timeout


protected:
	void resizeEvent(QResizeEvent *ev);

signals:
	//External signals
	void itemsActivated();
	void updateDirectoryStatus(QString);
	void contextMenuRequested();
	void DataDropped(QString, QStringList);
	void hasFocus(QString); //ID output
	void stopLoop();

	//Internal signal
	void dirChange(QString, bool); //current dir path, force
	void treeWidgetSortColumn(int);
};
#endif
